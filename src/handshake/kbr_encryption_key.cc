#include "handshake/kbr_encryption_key.h"
#include "eys.h"
#include <memory>
#include <algorithm>

kuic::handshake::kbr_encryption_key::kbr_encryption_key() { }

kuic::handshake::kbr_encryption_key::kbr_encryption_key(
        kuic::kbr_encryption_key_t key_type,
        std::vector<kuic::byte_t> &&key_value)
    : key_type(key_type)
    , key_value(std::move(key_value)) { }


kuic::handshake::kbr_encryption_key
kuic::handshake::kbr_encryption_key::deserialize(
        kuic::byte_t *buffer, size_t size) {
    kuic::handshake::kbr_encryption_key result;
    
    ssize_t seek = 0;
    result.key_type = eys::deserializer<unsigned int>::deserialize(
            reinterpret_cast<char *>(buffer), size, seek);
    result.key_value.assign(buffer + seek, buffer + size);

    return result;
}

kuic::byte_t *
kuic::handshake::kbr_encryption_key::serialize(size_t &size) {
    // calculate result buffer capacity (type + value)
    size = sizeof(kuic::kbr_encryption_key_t) +
        this->key_value.size();
    
    // declare result buffer
    kuic::byte_t *result = new kuic::byte_t[size];

    // serialize key_type
    size_t serialized_size = 0;
    std::unique_ptr<kuic::byte_t> serialized_buffer(
            reinterpret_cast<kuic::byte_t *>(
                eys::serializer<unsigned int>::serialize(
                    this->key_type, serialized_size)));
    // copy key_type from serialized_buffer to result
    std::copy_n(
            serialized_buffer.get(),
            serialized_size,
            result);
    // copy secret key to result
    std::copy_n(
            this->key_value.begin(),
            this->key_value.size(),
            result + serialized_size);

    return result;
}

kuic::kbr_encryption_type_t
kuic::handshake::kbr_encryption_key::get_type() const {
    return this->key_type;
}

std::vector<kuic::byte_t>
kuic::handshake::kbr_encryption_key::get_value() const {
    return this->key_value;
}
