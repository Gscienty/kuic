#include "stream/stream.h"
#include "type.h"

kuic::stream::stream::stream(
        kuic::stream_id_t stream_id,
        kuic::stream::stream_sender &sender,
        kuic::flowcontrol::stream_flow_controller &flow_controller)
    : sender(sender)
    , send_sender(new kuic::stream::unicast_stream_sender(
                sender,
                [this] () -> void {
                    std::lock_guard<std::mutex> lock(this->completed_mutex);
                    this->send_stream_completed = true;
                    this->check_if_completed();
                }))
    , receive_sender(new kuic::stream::unicast_stream_sender(
                sender,
                [this] () -> void {
                    std::lock_guard<std::mutex> lock(this->completed_mutex);
                    this->receive_stream_completed = true;
                    this->check_if_completed();
                }))
    , send_stream(stream_id, *this->send_sender, flow_controller)
    , receive_stream(stream_id, *this->receive_sender, flow_controller)
    , receive_stream_completed(false)
    , send_stream_completed(false) { }

kuic::stream_id_t kuic::stream::stream::get_stream_id() const {
    return this->send_stream.get_stream_id();
}

bool kuic::stream::stream::close() {
    if (this->send_stream.close() == false)  {
        return false;
    }

    return true;
}

void kuic::stream::stream::set_deadline(kuic::special_clock clock) {
   this->receive_stream.set_read_deadline(clock);
   this->send_stream.set_write_deadline(clock);
}

void kuic::stream::stream::close_for_shutdown(kuic::error_t error) {
    this->send_stream.close_for_shutdown(error);
    this->receive_stream.close_for_shutdown(error);
}

bool kuic::stream::stream::handle_rst_stream_frame(kuic::frame::rst_stream_frame &frame) {
    if (this->receive_stream.handle_rst_stream_frame(frame) == false) {
        return false;
    }
    return true;
}

void kuic::stream::stream::check_if_completed() {
    if (this->send_stream_completed && this->receive_stream_completed) {
        this->sender.on_stream_completed(this->get_stream_id());
    }
}

kuic::bytes_count_t &kuic::stream::stream::get_write_offset() {
    return this->send_stream.get_write_offset();
}

void kuic::stream::stream::cancel_read(kuic::application_error_code_t error) {
    this->receive_stream.cancel_read(error);
}

kuic::bytes_count_t kuic::stream::stream::write(const kuic::byte_t *p, const kuic::bytes_count_t size) {
    return this->send_stream.write(p, size);
}

kuic::bytes_count_t kuic::stream::stream::read(kuic::byte_t *p, const kuic::bytes_count_t size) {
    return this->receive_stream.read(p, size);
}

bool kuic::stream::stream::handle_stream_frame(kuic::frame::stream_frame &frame) {
    return this->receive_stream.handle_stream_frame(frame);
}

kuic::stream::stream_frame_sorter &
kuic::stream::stream::get_frame_queue() {
    return this->receive_stream.get_frame_queue();
}

void kuic::stream::crypto_stream::set_read_offset(kuic::bytes_count_t offset) {
    this->receive_stream.get_read_offset() = offset;
    this->receive_stream.get_frame_queue().get_read_position() = offset;
}
