#include "frame/blocked_frame.h"
#include "variable_integer.h"
#include "define.h"

size_t kuic::frame::blocked_frame::length() const {
    return 1 + kuic::variable_integer::length(this->offset);
}

kuic::frame_type_t kuic::frame::blocked_frame::type() const {
    return kuic::frame_type_blocked;
}

std::basic_string<kuic::byte_t> kuic::frame::blocked_frame::serialize() const {
    std::basic_string<kuic::byte_t> result;
    result.push_back(this->type());
    result.append(kuic::variable_integer::write(this->offset));
    return result;
}

kuic::frame::blocked_frame kuic::frame::blocked_frame::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    seek++; // ignore type
    kuic::frame::blocked_frame result;
    result.offset = kuic::variable_integer::read(buffer, seek);
    return result;
}

kuic::bytes_count_t &kuic::frame::blocked_frame::get_offset() {
    return this->offset;
}
