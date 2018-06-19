#include "handshake/kbr_encryption_key.h"
#include "eys.h"
#include <memory>
#include <algorithm>

kuic::handshake::kbr_encryption_key::kbr_encryption_key() { }

kuic::handshake::kbr_encryption_key::kbr_encryption_key(
        kuic::kbr_encryption_key_t key_type,
        std::basic_string<kuic::byte_t> &&key_value)
    : key_type(key_type)
    , key_value(std::move(key_value)) { }


kuic::handshake::kbr_encryption_key
kuic::handshake::kbr_encryption_key::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::kbr_encryption_key result;

    result.key_type = eys::bigendian_serializer<kuic::byte_t, kuic::kbr_encryption_key_t>::deserialize(buffer, seek);

    result.key_value.assign(buffer.begin() + seek, buffer.end());
    seek = buffer.size();

    return result;
}

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_encryption_key::serialize() const {
    // declare result buffer
    std::basic_string<kuic::byte_t> result;

    // serialize key_type
    result.append(eys::bigendian_serializer<kuic::byte_t, kuic::kbr_encryption_key_t>::serialize(this->key_type));
    // copy secret key to result
    result.append(this->key_value);

    return result;
}

kuic::kbr_encryption_key_t
kuic::handshake::kbr_encryption_key::get_type() const {
    return this->key_type;
}

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_encryption_key::get_value() const {
    return this->key_value;
}
