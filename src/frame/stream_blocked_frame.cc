#include "frame/stream_blocked_frame.h"
#include "variable_integer.h"
#include "define.h"

kuic::frame::stream_blocked_frame
kuic::frame::stream_blocked_frame::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::frame::stream_blocked_frame frame;
    seek++;

    frame.stream_id = kuic::variable_integer::read(buffer, seek);
    frame.offset = kuic::variable_integer::read(buffer, seek);

    return frame;
}

std::basic_string<kuic::byte_t>
kuic::frame::stream_blocked_frame::serialize() const {
    std::basic_string<kuic::byte_t> result;
    result.push_back(this->type());

    result.append(kuic::variable_integer::write(this->stream_id));
    result.append(kuic::variable_integer::write(this->offset));

    return result;
}

size_t
kuic::frame::stream_blocked_frame::length() const {
    return 1 + kuic::variable_integer::length(this->stream_id) + kuic::variable_integer::length(this->offset);
}

kuic::frame_type_t
kuic::frame::stream_blocked_frame::type() const {
    return kuic::frame_type_stream_blocked;
} 

kuic::stream_id_t &
kuic::frame::stream_blocked_frame::get_stream_id() {
    return this->stream_id;
}

kuic::bytes_count_t &
kuic::frame::stream_blocked_frame::get_offset() {
    return this->offset;
}
