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

char *kuic::handshake::kbr_principal_name::serialize(size_t &size) {
    size = sizeof(kuic::kbr_name_type_t) + this->name.length();
    char *result = new char[size];
    size_t inner_size;
    std::unique_ptr<kuic::byte_t> serialize_buffer(
        reinterpret_cast<kuic::byte_t *>(kuic::handshake::kbr_name_type_serializer::serialize(this->type, inner_size)));
    std::copy(serialize_buffer.get(), serialize_buffer.get() + inner_size, result);
    std::copy(this->name.begin(), this->name.end(), result + inner_size);

    return result;
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_principal_name::deserialize(
        const char *buffer, size_t len) {
    kuic::handshake::kbr_principal_name ret;

    ssize_t seek = 0;
    ret.type = kuic::handshake::kbr_name_type_serializer::deserialize(
            buffer, len, seek);
    ret.name = std::string(buffer + seek, buffer + len);
    
    return ret;
}
