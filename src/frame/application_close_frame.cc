#include "frame/application_close_frame.h"
#include "variable_integer.h"
#include "define.h"
#include "eys.h"
#include <algorithm>

kuic::frame::application_close_frame
kuic::frame::application_close_frame::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::frame::application_close_frame frame;

    // deserialize error code
    frame.error_code = eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::deserialize(buffer, seek);
    // deserialize reason length
    int reason_phrase_length = kuic::variable_integer::read(buffer, seek);
    // deserialize reason
    frame.reason_phrase.assign(buffer.begin() + seek, buffer.begin() + seek + reason_phrase_length);
    seek += reason_phrase_length;

    return frame;
}

std::basic_string<kuic::byte_t>
kuic::frame::application_close_frame::serialize() const {
    std::basic_string<kuic::byte_t> result;
    result.push_back(this->type());
    // serialize error code
    result.append(eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::serialize(this->error_code));
    // serialize reason length
    result.append(kuic::variable_integer::write(this->reason_phrase.length()));
    // serialize reason
    result.append(this->reason_phrase.begin(), this->reason_phrase.end());

    return result;
}

size_t kuic::frame::application_close_frame::length() const {
    return 1 + 2 + kuic::variable_integer::length(this->reason_phrase.length()) + this->reason_phrase.size();
}

kuic::frame_type_t
kuic::frame::application_close_frame::type() const {
    return kuic::frame_type_connection_close;
}
