#include "package_serializer.h"

std::basic_string<kuic::byte_t>
kuic::package_serializable::serialize() const {
    return std::basic_string<kuic::byte_t>();
}
