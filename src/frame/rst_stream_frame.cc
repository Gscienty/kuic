#include "frame/rst_stream_frame.h"
#include "variable_integer.h"
#include "define.h"
#include "eys.h"

kuic::frame::rst_stream_frame
kuic::frame::rst_stream_frame::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::frame::rst_stream_frame result;
    
    // deserialize stream id
    result.stream_id = kuic::variable_integer::read(buffer, seek);
    // deserialize error code
    result.error_code = eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::deserialize(buffer, seek);
    // deserialize offset
    result.offset = kuic::variable_integer::read(buffer, seek);

    return result;
}

std::basic_string<kuic::byte_t>
kuic::frame::rst_stream_frame::serialize() const {
    std::basic_string<kuic::byte_t> result;
    result.push_back(this->type());
    
    // serialize stream id
    result.append(kuic::variable_integer::write(this->stream_id));
    // serialize error code
    result.append(eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::serialize(this->error_code));
    // serialize offset
    result.append(kuic::variable_integer::write(this->offset));

    return result;
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
