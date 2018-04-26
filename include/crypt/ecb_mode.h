#ifndef _KUIC_CRYPT_ECB_MODE_
#define _KUIC_CRYPT_ECB_MODE_

#include "crypt/mode.h"
#include "crypt/crypter.h"

namespace kuic {
    namespace crypt {
        class ecb_mode : public mode {
        public:
            ecb_mode(crypter *_crypter);
            virtual std::pair<kuic::byte_t *, size_t> encrypt() override;
            virtual std::pair<kuic::byte_t *, size_t> decrypt() override;
        };
    }
}

#endif

