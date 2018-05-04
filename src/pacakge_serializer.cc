#include "package_serializer.h"

std::pair<kuic::byte_t *, size_t>
kuic::package_serializer::serialize() const {
    return std::pair<kuic::byte_t *, size_t>(nullptr, 0);
}
