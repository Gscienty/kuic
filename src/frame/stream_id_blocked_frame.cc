#include "frame/stream_id_blocked_frame.h"
#include "variable_integer.h"
#include "error.h"
#include "define.h"

std::basic_string<kuic::byte_t>
kuic::frame::stream_id_blocked_frame::serialize() const {
    std::basic_string<kuic::byte_t> result;

    result.push_back(this->type());
    result.append(kuic::variable_integer::write(this->stream_id));

    return result;
}

kuic::frame::stream_id_blocked_frame 
kuic::frame::stream_id_blocked_frame::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    seek++; // ignore type
    if (seek >= buffer.size()) {
        return kuic::frame::stream_id_blocked_frame(kuic::reader_buffer_remain_not_enough);
    }

    kuic::frame::stream_id_blocked_frame frame;
    frame.stream_id = kuic::variable_integer::read(buffer, seek);

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
