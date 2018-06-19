#include "frame/stop_sending_frame.h"
#include "variable_integer.h"
#include "define.h"
#include "eys.h"

kuic::frame::stop_sending_frame
kuic::frame::stop_sending_frame::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    // ignore type
    seek++;
    
    if (seek >= buffer.size()) {
        return kuic::frame::stop_sending_frame(kuic::reader_buffer_remain_not_enough);
    }

    kuic::frame::stop_sending_frame frame;
    frame.stream_id = kuic::variable_integer::read(buffer, seek);
    frame.error = eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::deserialize(buffer, seek);

    return frame;
}

std::basic_string<kuic::byte_t>
kuic::frame::stop_sending_frame::serialize() const {
    std::basic_string<kuic::byte_t> result;
    result.push_back(this->type());

    result.append(kuic::variable_integer::write(this->stream_id));
    result.append(eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::serialize(this->error));

    return result;
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
