#include "handshake/kbr_principal_name.h"
#include "handshake/serializer.h"
#include "define.h"
#include <memory>


kuic::handshake::kbr_principal_name::kbr_principal_name(std::string name)
    : name(name)
    , type(kuic::kbr_name_default_type) { }

char *kuic::handshake::kbr_principal_name::serialize(size_t &size) {
    size = sizeof(kuic::kbr_name_type_t) + this->name.length();
    char *result = new char[size];
    size_t inner_size;
    std::unique_ptr<kuic::byte_t []> serialize_buffer(
        kuic::handshake::kbr_name_type_serializer::serialize(this->type, inner_size));
    std::copy(serialize_buffer.get(), serialize_buffer.get() + inner_size, result);
    std::copy(this->name.begin(), this->name.end(), result + inner_size);

    return result;
}