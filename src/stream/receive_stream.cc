#include "stream/receive_stream.h"
#include "frame/stop_sending_frame.h"
#include "error.h"
#include <chrono>
#include <algorithm>

kuic::stream::receive_stream::receive_stream(
        kuic::stream_id_t stream_id,
        kuic::stream::stream_sender &sender,
        kuic::flowcontrol::stream_flow_controller &flow_controller)
    : stream_id(stream_id)
    , sender(sender)
    , read_position_in_frame(0)
    , read_offset(0)
    , close_for_shutdown_error(kuic::no_error)
    , cancel_read_error(kuic::no_error)
    , reset_remote_error(kuic::no_error)
    , _close_for_shutdown(false)
    , fin_read(false)
    , canceled_read(false)
    , reset_remotely(false)
    , flow_controller(flow_controller) { }


kuic::stream_id_t
kuic::stream::receive_stream::get_stream_id() {
    return this->stream_id;
}

kuic::bytes_count_t kuic::stream::receive_stream::read(kuic::byte_t *buffer, const kuic::bytes_count_t size) {
    std::unique_lock<std::mutex> lock(this->mutex);

    if (this->fin_read) {
        return 0;
    }
    if (this->canceled_read) {
        return 0;
    }
    if (this->reset_remotely) {
        return 0;
    }
    if (this->_close_for_shutdown) {
        return 0;
    }

    kuic::bytes_count_t bytes_read = 0;
    while (bytes_read < size) {
        kuic::nullable<kuic::frame::stream_frame> frame = this->frame_queue.head();
        if (frame.is_null() && bytes_read > 0) {
            return bytes_read;
        }

        while (true) {
            if (this->_close_for_shutdown) {
                return bytes_read;
            }
            if (this->canceled_read) {
                return bytes_read;
            }
            if (this->reset_remotely) {
                return bytes_read;
            }

            kuic::special_clock deadline = this->read_deadline;
            if (deadline.is_zero() == false && kuic::current_clock() >= deadline) {
                return bytes_read;
            }

            if (frame.is_null() == false) {
                this->read_position_in_frame = this->read_offset - frame->get_offset();
                break;
            }

            if (deadline.is_zero()) {
                this->read_cond.wait(lock);
            }
            else {
                this->read_cond.wait_until(lock,
                        std::chrono::system_clock::now() +
                        std::chrono::milliseconds(
                            (this->read_deadline - kuic::current_clock()) / kuic::clock_millisecond));
            }
            kuic::nullable<kuic::frame::stream_frame> next_frame = this->frame_queue.head();
            frame = next_frame;
        }

        if (bytes_read > size) {
            return bytes_read;
        }

        if (this->read_position_in_frame > int(frame->get_data().size())) {
            return bytes_read;
        }

        lock.unlock();
        
        std::copy(frame->get_data().begin() + this->read_position_in_frame, frame->get_data().end(), buffer + bytes_read);

        kuic::bytes_count_t middle_value = std::min(size - bytes_read, frame->get_data().size() - this->read_position_in_frame);
        this->read_position_in_frame += middle_value;
        bytes_read += middle_value;
        this->read_offset += middle_value;

        lock.lock();
        
        if (this->reset_remotely == false) {
            this->flow_controller.add_bytes_read(middle_value);
        }
        
        this->flow_controller.try_queue_window_update();

        if (this->read_position_in_frame >= int(frame->get_data().size())) {
            this->frame_queue.pop();
            this->fin_read = frame->get_fin_bit();
            if (frame->get_fin_bit()) {
                this->sender.on_stream_completed(this->stream_id);
                return bytes_read;
            }
        }
    }

    return bytes_read;
}

void kuic::stream::receive_stream::cancel_read(kuic::application_error_code_t error) {
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->fin_read) {
        return;
    }
    if (this->canceled_read) {
        return;
    }
    this->canceled_read = true;
    this->cancel_read_error = error;
    this->signal_read();
    kuic::frame::stop_sending_frame frame;
    frame.get_stream_id() = this->stream_id;
    frame.get_application_error() = error;
    this->sender.queue_control_frame(frame);
}

bool kuic::stream::receive_stream::handle_stream_frame(kuic::frame::stream_frame &frame) {
    kuic::bytes_count_t max_offset = frame.get_offset() + frame.get_data().size();
    if (this->flow_controller.update_highest_received(max_offset, frame.get_fin_bit()) != kuic::no_error) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(this->mutex);
        if (this->frame_queue.push(frame) == false) {
            return false;
        }
    }
    this->signal_read();
    return true;
}

bool kuic::stream::receive_stream::handle_rst_stream_frame(kuic::frame::rst_stream_frame &frame) {
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->_close_for_shutdown) {
        return true;
    }

    if (this->flow_controller.update_highest_received(frame.get_offset(), true) != kuic::no_error) {
        return false;
    }

    if (frame.get_error_code() == 0) {
        return true;
    }

    if (this->reset_remotely) {
        return true;
    }

    this->reset_remotely = true;
    this->reset_remote_error = 1;

    this->signal_read();
    this->sender.on_stream_completed(this->stream_id);
    return true;
}

void kuic::stream::receive_stream::close_remote(kuic::bytes_count_t offset) {
    kuic::frame::stream_frame frame;
    frame.get_fin_bit() = true;
    frame.get_offset() = offset;

    this->handle_stream_frame(frame);
}

void kuic::stream::receive_stream::set_read_deadline(kuic::special_clock clock) {
    kuic::special_clock old_deadline;
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        old_deadline = this->read_deadline;
        this->read_deadline = clock;
    }

    if (clock < old_deadline) {
        this->signal_read();
    }
}

void kuic::stream::receive_stream::close_for_shutdown(kuic::error_t error) {
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->_close_for_shutdown = true;
        this->close_for_shutdown_error = error;
    }
    this->signal_read();
}

kuic::bytes_count_t kuic::stream::receive_stream::get_window_update() {
    return this->flow_controller.get_window_update();
}

void kuic::stream::receive_stream::signal_read() {
    this->read_cond.notify_all();
}

kuic::bytes_count_t &kuic::stream::receive_stream::get_read_offset() {
    return this->read_offset;
}

kuic::stream::stream_frame_sorter &kuic::stream::receive_stream::get_frame_queue() {
    return this->frame_queue;
}
