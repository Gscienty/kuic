#ifndef _KUIC_CRYPT_SEALER_
#define _KUIC_CRYPT_SEALER_

#include "type.h"
#include <utility>
#include <unistd.h>

namespace kuic {
    namespace crypt {
        class sealer {
        public:
            virtual kuic::bytes_count_t overhead() = 0;
            virtual std::pair<kuic::byte_t *, size_t> seal(kuic::byte_t *src, const size_t len) = 0;
        };
    }
}

#endif

