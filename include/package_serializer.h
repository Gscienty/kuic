#ifndef _KUIC_HANDSHAKE_PACKAGE_SERIALIZER_
#define _KUIC_HANDSHAKE_PACKAGE_SERIALIZER_

#include "type.h"
#include <utility>
#include <unistd.h>

namespace kuic {
    class package_serializer {
    public:
        virtual std::pair<kuic::byte_t *, size_t> serialize() const;
    };
}

#endif

