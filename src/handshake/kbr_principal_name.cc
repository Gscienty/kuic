#include "handshake/kbr_principal_name.h"
#include "handshake/serializer.h"
#include "define.h"
#include <memory>
#include <string>

kuic::handshake::kbr_principal_name::kbr_principal_name()
    : type(kuic::kbr_name_default_type)
    , name("") { }

kuic::handshake::kbr_principal_name::kbr_principal_name(std::string name)
    : type(kuic::kbr_name_default_type)
    , name(name) { }

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_principal_name::serialize() const {
    size_t size = sizeof(kuic::kbr_name_type_t) + this->name.length();
    kuic::byte_t *result = new kuic::byte_t[size];

    size_t inner_size;
    kuic::byte_t *name_type_buffer_ptr = nullptr;
    std::tie(name_type_buffer_ptr, inner_size) = kuic::handshake::kbr_name_type_serializer::serialize(this->type);
    std::unique_ptr<kuic::byte_t> name_type_buffer(name_type_buffer_ptr);

    std::copy(name_type_buffer.get(), name_type_buffer.get() + inner_size, result);
    std::copy(this->name.begin(), this->name.end(), result + inner_size);

    return std::pair<kuic::byte_t *, size_t>(result, size);
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_principal_name::deserialize(
        kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::kbr_principal_name ret;

    seek = 0;
    ret.type = kuic::handshake::kbr_name_type_serializer::deserialize(buffer, len, seek);
    ret.name = std::string(buffer + seek, buffer + len);
    seek = len;

    return ret;
}

kuic::kbr_name_type_t
kuic::handshake::kbr_principal_name::get_type() const {
    return this->type;
}

std::string
kuic::handshake::kbr_principal_name::get_name() const {
    return this->name;
}
