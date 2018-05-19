#include "frame/application_close_frame.h"
#include "variable_integer.h"
#include "define.h"
#include "eys.h"
#include <algorithm>

kuic::frame::application_close_frame
kuic::frame::application_close_frame::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::frame::application_close_frame frame;
    frame.error_code = eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::deserialize(buffer, len, seek);
    int reason_phrase_length = kuic::variable_integer::read(buffer, len, seek);
    frame.reason_phrase.assign(buffer + seek, buffer + seek + reason_phrase_length);
    seek += reason_phrase_length;

    return frame;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::application_close_frame::serialize() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *result = new kuic::byte_t[size];

    kuic::frame::frame::fill(result, size, seek, eys::bigendian_serializer<kuic::byte_t, kuic::application_error_code_t>::serialize(this->error_code));
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->reason_phrase.length()));
    std::copy(this->reason_phrase.begin(), this->reason_phrase.end(), result + seek);

    return std::pair<kuic::byte_t *, size_t>(result, seek);
}

size_t kuic::frame::application_close_frame::length() const {
    return 2 + kuic::variable_integer::length(this->reason_phrase.length()) + this->reason_phrase.size();
}

kuic::frame_type_t
kuic::frame::application_close_frame::type() const {
    return kuic::frame_type_connection_close;
}
