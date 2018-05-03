#ifndef _KUIC_HANDSHAKE_KBR_ENCRYPTED_DATA_
#define _KUIC_HANDSHAKE_KBR_ENCRYPTED_DATA_

#include "type.h"
#include "package_serializer.h"
#include "crypt/crypter.h"
#include "crypt/mode.h"
#include <vector>
#include <memory>
#include <stddef.h>

namespace kuic {
    namespace handshake {
        class kbr_encrypted_data : public kuic::package_serializer {
        private:
            kuic::kbr_key_version_t version;
            kuic::kbr_encryption_type_t encryption_type;
            std::vector<kuic::byte_t> cipher;

            kuic::crypt::crypter *get_crypter();

            std::unique_ptr<kuic::crypt::mode> get_mode(kuic::crypt::crypter *_crypter);
        public:
            kbr_encrypted_data();

            kbr_encrypted_data(kuic::kbr_key_version_t key_version, kuic::kbr_encryption_type_t encryption_type);

            void set_plain_message(kuic::byte_t *plain_text, size_t plain_text_size, kuic::byte_t *secret_key, size_t secret_key_size);
            std::pair<kuic::byte_t *, size_t> get_plain_message(kuic::byte_t *secret_key, size_t secret_key_size);

            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            static kbr_encrypted_data deserialize(kuic::byte_t *buffer, size_t size, size_t &seek);
        };
    }
}

#endif
