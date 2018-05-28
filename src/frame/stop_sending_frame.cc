#include "frame/stop_sending_frame.h"
#include "variable_integer.h"
#include "define.h"
#include "eys.h"

kuic::frame::stop_sending_frame
kuic::frame::stop_sending_frame::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    // ignore type
    seek++;
    
    if (seek >= len) {
        return kuic::frame::stop_sending_frame(kuic::reader_buffer_remain_not_enough);
    }

    kuic::frame::stop_sending_frame frame;
    frame.stream_id = kuic::variable_integer::read(buffer, len, seek);
    frame.error = eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::deserialize(buffer, len, seek);

    return frame;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::stop_sending_frame::serialize() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *buffer = new kuic::byte_t[size];

    buffer[seek++] = 0x0C;
    kuic::frame::frame::fill(buffer, size, seek, kuic::variable_integer::write(this->stream_id));
    kuic::frame::frame::fill(buffer, size, seek, eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::serialize(this->error));

    return std::pair<kuic::byte_t *, size_t>(buffer, size);
}

size_t kuic::frame::stop_sending_frame::length() const {
    return 1 + 2 + kuic::variable_integer::length(this->stream_id);
}

kuic::frame_type_t
kuic::frame::stop_sending_frame::type() const {
    return kuic::frame_type_stop_sending;
}

kuic::application_error_code_t &
kuic::frame::stop_sending_frame::get_application_error() {
    return this->error;
}

kuic::stream_id_t &
kuic::frame::stop_sending_frame::get_stream_id() {
    return this->stream_id;
}
