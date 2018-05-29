#include "frame/stream_id_blocked_frame.h"
#include "variable_integer.h"
#include "error.h"
#include "define.h"

std::pair<kuic::byte_t *, size_t>
kuic::frame::stream_id_blocked_frame::serialize() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *buffer = new kuic::byte_t[size];
    buffer[seek++] = 0x0A;
    kuic::frame::frame::fill(buffer, size, seek, kuic::variable_integer::write(this->stream_id));

    return std::pair<kuic::byte_t *, size_t>(buffer, size);
}

kuic::frame::stream_id_blocked_frame 
kuic::frame::stream_id_blocked_frame::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    seek++; // ignore type
    if (seek >= len) {
        return kuic::frame::stream_id_blocked_frame(kuic::reader_buffer_remain_not_enough);
    }

    kuic::frame::stream_id_blocked_frame frame;
    frame.stream_id = kuic::variable_integer::read(buffer, len, seek);

    return frame;
}

size_t kuic::frame::stream_id_blocked_frame::length() const {
    return 1 + kuic::variable_integer::length(this->stream_id);
}

kuic::frame_type_t kuic::frame::stream_id_blocked_frame::type() const {
    return kuic::frame_type_stream_blocked;
}

kuic::stream_id_t &kuic::frame::stream_id_blocked_frame::get_stream_id() {
    return this->stream_id;
}
