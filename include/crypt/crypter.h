#ifndef _KUIC_CRYPT_CRYPTER_
#define _KUIC_CRYPT_CRYPTER_

#include "type.h"
#include <stddef.h>

namespace kuic {
    namespace crypt {

        class crypter {
        public:
            virtual kuic::byte_t *encrypt(
                    const kuic::byte_t *message,
                    const kuic::byte_t *key) = 0;
            virtual kuic::byte_t *decrypt(
                    const kuic::byte_t *message,
                    const kuic::byte_t *key) = 0;

            virtual size_t get_message_length() const = 0;
            virtual size_t get_key_length() const = 0;
            virtual size_t get_cipher_length() const = 0;
        };
    }
}

#endif

