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

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_principal_name::serialize() const {
    std::basic_string<kuic::byte_t> result;
    
    // serialize type
    result.append(kuic::handshake::kbr_name_type_serializer::serialize(this->type));
    // copy name to buffer
    result.append(this->name.begin(), this->name.end());

    return result;
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_principal_name::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::kbr_principal_name ret;

    seek = 0;
    ret.type = kuic::handshake::kbr_name_type_serializer::deserialize(buffer, seek);
    ret.name = std::string(buffer.begin() + seek, buffer.end());
    seek = buffer.size();

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
