#include "package_serializer.h"

std::pair<kuic::byte_t *, size_t>
kuic::package_serializable::serialize() const {
    return std::pair<kuic::byte_t *, size_t>(nullptr, 0);
}
