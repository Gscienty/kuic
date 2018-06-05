#ifndef _KUIC_CRYPT_MODE_
#define _KUIC_CRYPT_MODE_

#include "crypt/crypter.h"
#include "type.h"
#include <stddef.h>
#include <memory>
#include <utility>
#include <algorithm>

namespace kuic {
    namespace crypt {
        
        template <typename Crypter> class mode {
        protected:
            std::unique_ptr<crypter> crypter_ptr;
            kuic::byte_t *message;
            size_t message_length;
            std::unique_ptr<kuic::byte_t> key;
            size_t key_length;

        public:
            mode()
                : crypter_ptr(std::unique_ptr<kuic::crypt::crypter>(new Crypter()))
                , message_length(0)
                , key_length(0) { }

            void set_secret_key(kuic::byte_t *key, size_t key_len) {
                // alloc
                this->key = std::unique_ptr<kuic::byte_t>(
                        new kuic::byte_t[this->crypter_ptr->get_key_length()]);

                if (key_len < this->crypter_ptr->get_key_length()) {
                    std::copy(key, key + key_len, this->key.get());
                    std::fill(
                            this->key.get() + key_len,
                            this->key.get() + (this->crypter_ptr->get_key_length() - key_len),
                            kuic::byte_t(0x00));
                }
                else {
                    std::copy_n(key, this->crypter_ptr->get_key_length(), this->key.get());
                }
            }

            void set_message(kuic::byte_t *message, size_t message_len) {
                this->message = message;
                this->message_length = message_len;
            }

            virtual std::pair<kuic::byte_t *, size_t> encrypt() = 0;
            virtual std::pair<kuic::byte_t *, size_t> decrypt() = 0;
        };
    }
}

#endif

