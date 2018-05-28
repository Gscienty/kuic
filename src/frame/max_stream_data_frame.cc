#include "frame/max_stream_data_frame.h"
#include "variable_integer.h"
#include "error.h"
#include "define.h"

kuic::frame::max_stream_data_frame 
kuic::frame::max_stream_data_frame::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    seek++;
    if (seek >= len) {
        return kuic::frame::max_stream_data_frame(kuic::not_expect);
    }

    kuic::frame::max_stream_data_frame frame;
    frame.stream_id = kuic::variable_integer::read(buffer, len, seek);
    frame.byte_offset = kuic::variable_integer::read(buffer, len, seek);

    return frame;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::max_stream_data_frame::serialize() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *buffer = new kuic::byte_t[size];

    buffer[seek++] = 0x05;
    kuic::frame::frame::fill(buffer, size, seek, kuic::variable_integer::write(this->stream_id));
    kuic::frame::frame::fill(buffer, size, seek, kuic::variable_integer::write(this->byte_offset));

    return std::pair<kuic::byte_t *, size_t>(buffer, size);
}

size_t kuic::frame::max_stream_data_frame::length() const {
    return 1 + kuic::variable_integer::length(this->stream_id) + kuic::variable_integer::length(this->byte_offset);
}

kuic::frame_type_t kuic::frame::max_stream_data_frame::type() const {
    return kuic::frame_type_max_stream_data;
}

kuic::stream_id_t &
kuic::frame::max_stream_data_frame::get_stream_id() {
    return this->stream_id;
}

kuic::bytes_count_t &
kuic::frame::max_stream_data_frame::get_byte_offset() {
    return this->byte_offset;
}
