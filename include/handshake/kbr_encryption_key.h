#ifndef _KUIC_HANDSHAKE_KBR_ENCRYPTION_KEY_
#define _KUIC_HANDSHAKE_KBR_ENCRYPTION_KEY_

#include "type.h"
#include <vector>
#include <stddef.h>


namespace kuic {
    namespace handshake {
        class kbr_encryption_key {
        private:
            kuic::kbr_encryption_key_t key_type;
            std::vector<kuic::byte_t> key_value;
            
        public:
            kbr_encryption_key();
            kbr_encryption_key(kuic::kbr_encryption_key_t key_type, std::vector<kuic::byte_t> &&key_value);
            static kbr_encryption_key deserialize(
                    kuic::byte_t *buffer,
                    size_t size);

            kuic::byte_t *serialize(size_t &size);
            
            kuic::kbr_encryption_key_t get_type() const;
            std::vector<kuic::byte_t> get_value() const;
        };
    }
}

#endif
