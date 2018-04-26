#ifndef _KUIC_CRYPT_MODE_
#define _KUIC_CRYPT_MODE_

#include "crypt/crypter.h"
#include "type.h"
#include <stddef.h>
#include <memory>
#include <utility>

namespace kuic {
    namespace crypt {
        class mode {
        protected:
            std::unique_ptr<crypter> crypter_ptr;
            kuic::byte_t *message;
            size_t message_length;
            std::unique_ptr<kuic::byte_t> key;
            size_t key_length;

        public:
            mode(crypter *&&_crypter);
            void set_secret_key(kuic::byte_t *key, size_t key_len);
            void set_message(kuic::byte_t *message, size_t message_len);

            virtual std::pair<kuic::byte_t *, size_t> encrypt() = 0;
            virtual std::pair<kuic::byte_t *, size_t> decrypt() = 0;
        };
    }
}

#endif

