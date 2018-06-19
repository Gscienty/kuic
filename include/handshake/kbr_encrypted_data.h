#ifndef _KUIC_HANDSHAKE_KBR_ENCRYPTED_DATA_
#define _KUIC_HANDSHAKE_KBR_ENCRYPTED_DATA_

#include "type.h"
#include "package_serializer.h"
#include "crypt/aead.h"
#include <vector>
#include <memory>
#include <string>
#include <stddef.h>

namespace kuic {
    namespace handshake {
        class kbr_encrypted_data : public kuic::package_serializable {
        private:
            kuic::kbr_key_version_t version;
            kuic::crypt_mode_type_t crypt_mode_type;
            std::basic_string<kuic::byte_t> cipher;

        public:
            kbr_encrypted_data();

            kbr_encrypted_data(
                    kuic::kbr_key_version_t key_version,
                    kuic::crypt_mode_type_t encryption_type);

            void set_plain_message(
                    std::basic_string<kuic::byte_t> plain_text,
                    std::basic_string<kuic::byte_t> a_data,
                    kuic::crypt::aead &sealer);

            std::basic_string<kuic::byte_t>
            get_plain_message(
                    std::basic_string<kuic::byte_t> a_data,
                    kuic::crypt::aead &sealer);

            virtual std::basic_string<kuic::byte_t> serialize() const override;

            static kbr_encrypted_data deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
        };
    }
}

#endif
