#include "stream/send_stream.h"
#include "frame/stream_blocked_frame.h"
#include "frame/rst_stream_frame.h"
#include <chrono>
#include <algorithm>

kuic::stream::send_stream::send_stream(
    kuic::stream_id_t stream_id,
    stream_sender &sender,
    std::shared_ptr<kuic::flowcontrol::stream_flow_controller> flow_controller)
    : sender(sender)
    , stream_id(stream_id)
    , write_offset(0)
    , cancel_write_error(kuic::no_error)
    , close_for_shutdown_error(kuic::no_error)
    , _close_for_shutdown(false)
    , finished_writing(false)
    , _cancel_write(false)
    , fin_sent(false)
    , write_deadline(kuic::special_clock({ 0, 0 }))
    , flow_controller(flow_controller) { }

kuic::stream_id_t kuic::stream::send_stream::get_stream_id() const {
    return this->stream_id;
}

kuic::bytes_count_t kuic::stream::send_stream::write(const std::string &data) {
    std::unique_lock<std::mutex> lock(this->mutex);

    if (data.empty()) { return 0; }
    if (this->finished_writing) { return 0; }
    if (this->_cancel_write) { return 0; }
    if (this->_close_for_shutdown) { return 0; }
    if (this->write_deadline.is_zero() == false && this->write_deadline >= kuic::current_clock()) {
        return 0;
    }

    this->data_for_waiting.assign(data.begin(), data.end());
    this->sender.on_has_stream_data(this->stream_id);

    int bytes_written = 0;
    kuic::error_t error = kuic::no_error;
    while (true) {
        bytes_written = data.size() - this->data_for_waiting.size();
        kuic::special_clock deadline = this->write_deadline;
        if (deadline.is_zero() == false && kuic::current_clock() > deadline) {
            this->data_for_waiting.clear();
            error = kuic::deadline_timeout;
            break;
        }
        if (this->data_for_waiting.empty() || this->_cancel_write || this->_close_for_shutdown) {
            break;
        }
        
        if (this->write_deadline.is_zero()) {
            this->write_cond.wait(lock);
        }
        else {
            this->write_cond.wait_until(lock, 
                    std::chrono::system_clock::now() + 
                    std::chrono::milliseconds(
                        (this->write_deadline - kuic::current_clock()) / kuic::clock_millisecond));
        }
    }
    return bytes_written;
}

std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>
kuic::stream::send_stream::pop_stream_frame(kuic::bytes_count_t max_bytes) {
    std::unique_lock<std::mutex> lock(this->mutex);

    if (this->_close_for_shutdown) {
        return std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>(
                std::shared_ptr<kuic::frame::stream_frame>(), false);
    }

    std::shared_ptr<kuic::frame::stream_frame> frame(new kuic::frame::stream_frame());
    frame->get_stream_id() = this->stream_id;
    frame->get_offset() = this->write_offset;
    frame->get_data_length_present() = true;

    kuic::bytes_count_t max_data_length = frame->max_data_length(max_bytes);
    if (max_data_length == 0) {
        return std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>(
                std::shared_ptr<kuic::frame::stream_frame>(), this->data_for_waiting.empty() == false);
    }

    if (this->data_for_waiting.empty()) {
        frame->get_fin_bit() = this->finished_writing && this->fin_sent == false;
    }

    max_bytes = std::min(max_bytes, this->flow_controller->send_window_size());

    if (max_bytes == 0) {
        frame->get_fin_bit() = false;
    }

    if (this->data_for_waiting.size() > max_bytes) {
        frame->get_data().assign(this->data_for_waiting.begin(), this->data_for_waiting.begin() + max_bytes);
        this->data_for_waiting.assign(this->data_for_waiting.begin() + max_bytes, this->data_for_waiting.end());
    }
    else {
        frame->get_data().assign(this->data_for_waiting.begin(), this->data_for_waiting.end());
        this->data_for_waiting.clear();
        this->signal_write();
    }

    this->write_offset += frame->get_data().size();
    this->flow_controller->add_bytes_sent(frame->get_data().size());
    frame->get_fin_bit() = this->finished_writing && this->data_for_waiting.empty() && this->fin_sent == false;

    if (frame->get_data().empty() && frame->get_fin_bit() == false) {
        if (this->data_for_waiting.empty()) {
            return std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>(
                    std::shared_ptr<kuic::frame::stream_frame>(), false);
        }
        return std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>(
                std::shared_ptr<kuic::frame::stream_frame>(), this->flow_controller->is_blocked().first == false);
    }
    if (frame->get_fin_bit()) {
        this->fin_sent = true;
        this->sender.on_stream_completed(this->stream_id);
    }
    else {
        bool is_blocked;
        kuic::bytes_count_t offset;
        std::tie(is_blocked, offset) = this->flow_controller->is_blocked();
        if (is_blocked) {
            kuic::frame::stream_blocked_frame blocked;
            blocked.get_stream_id() = this->stream_id;
            blocked.get_offset() = offset;
            this->sender.queue_control_frame(blocked);
            return std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>(frame, false);
        }
    }
    return std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool>(frame, this->data_for_waiting.empty() == false);
}

bool kuic::stream::send_stream::cancel_write(kuic::application_error_code_t error) {
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->_cancel_write) {
        return true;
    }

    if (this->finished_writing) {
        return false;
    }

    this->_cancel_write = true;
    this->signal_write();
    kuic::frame::rst_stream_frame rst_frame;
    rst_frame.get_stream_id() = this->stream_id;
    rst_frame.get_error_code() = error;
    rst_frame.get_offset() = this->write_offset;
    this->sender.queue_control_frame(rst_frame);
    this->sender.on_stream_completed(this->stream_id);

    return true;
}

void kuic::stream::send_stream::handle_max_stream_data_frame(std::shared_ptr<kuic::frame::max_stream_data_frame> &frame) {
    this->flow_controller->update_send_window(frame->get_byte_offset());

    std::lock_guard<std::mutex> lock(this->mutex);
    if (this->data_for_waiting.empty() == false) {
        this->sender.on_has_stream_data(this->stream_id);
    }
}

void kuic::stream::send_stream::set_write_deadline(kuic::special_clock clock) {
    kuic::special_clock old_deadline;
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        old_deadline = this->write_deadline;
        this->write_deadline = clock;
    }

    if (clock < old_deadline) {
        this->signal_write();
    }
}

void kuic::stream::send_stream::close_for_shutdown(kuic::error_t error) {
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->close_for_shutdown_error = error;
        this->_close_for_shutdown = true;
    }
    this->signal_write();
}

kuic::bytes_count_t &kuic::stream::send_stream::get_write_offset() {
    return this->write_offset;
}

void kuic::stream::send_stream::handle_stop_sending_frame(std::shared_ptr<kuic::frame::stop_sending_frame> &frame) {
    std::lock_guard<std::mutex> lock(this->mutex);

    this->cancel_write(frame->get_application_error());
}

void kuic::stream::send_stream::signal_write() {
    this->write_cond.notify_all();
}

bool kuic::stream::send_stream::close() {
    std::lock_guard<std::mutex> lock(this->mutex);
    if (this->_cancel_write) {
        return false;
    }

    this->finished_writing = true;
    this->sender.on_has_stream_data(this->stream_id);
    return true;
}


