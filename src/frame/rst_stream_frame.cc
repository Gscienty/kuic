#include "frame/rst_stream_frame.h"
#include "variable_integer.h"
#include "define.h"
#include "eys.h"

kuic::frame::rst_stream_frame
kuic::frame::rst_stream_frame::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::frame::rst_stream_frame result;
    
    // deserialize stream id
    result.stream_id = kuic::variable_integer::read(buffer, len, seek);
    // deserialize error code
    result.error_code = eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::deserialize(buffer, len, seek);
    // deserialize offset
    result.offset = kuic::variable_integer::read(buffer, len, seek);

    return result;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::rst_stream_frame::serialize() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *result = new kuic::byte_t[size];
    result[seek++] = this->type();
    
    // serialize stream id
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->stream_id));
    // serialize error code
    kuic::frame::frame::fill(result, size, seek,
            eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::serialize(this->error_code));
    // serialize offset
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->offset));

    return std::pair<kuic::byte_t *, size_t>(result, size);
}

size_t kuic::frame::rst_stream_frame::length() const {
    return 1 + 2 + kuic::variable_integer::length(this->stream_id) + kuic::variable_integer::length(this->offset);
}

kuic::frame_type_t kuic::frame::rst_stream_frame::type() const {
    return kuic::frame_type_rst_stream;
}

kuic::stream_id_t &
kuic::frame::rst_stream_frame::get_stream_id() {
    return this->stream_id;
}

kuic::application_error_code_t &
kuic::frame::rst_stream_frame::get_error_code() {
    return this->error_code;
}

kuic::bytes_count_t &
kuic::frame::rst_stream_frame::get_offset() {
    return this->offset;
}
