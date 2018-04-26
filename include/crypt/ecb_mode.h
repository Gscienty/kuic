#ifndef _KUIC_CRYPT_ECB_MODE_
#define _KUIC_CRYPT_ECB_MODE_

#include "crypt/mode.h"

namespace kuic {
    namespace crypt {
        class ecb_mode : public mode {
        public:
            virtual std::pair<kuic::byte_t *, size_t> encrypt() override;
            virtual std::pair<kuic::byte_t *, size_t> decrypt() override;
        };
    }
}

#endif

