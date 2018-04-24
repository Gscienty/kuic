#ifndef _KUIC_HANDSHAKE_KBR_ENCRYPTED_DATA_
#define _KUIC_HANDSHAKE_KBR_ENCRYPTED_DATA_

#include "type.h"
#include <vector>

namespace kuic {
    namespace handshake {
        class kbr_encrypted_data {
            kuic::kbr_encryption_type_t encryption_type;
            kuic::kbr_key_version_t version;
            std::vector<kuic::byte_t> cipher;
        };
    }
}

#endif