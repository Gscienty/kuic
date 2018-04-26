#ifndef _KUIC_HANDSHAKE_KBR_ENCRYPTION_KEY_
#define _KUIC_HANDSHAKE_KBR_ENCRYPTION_KEY_

#include "type.h"
#include <vector>

namespace kuic {
    namespace handshake {
        class kbr_encryption_key {
        private:
            kuic::kbr_encryption_key_t key_type;
            std::vector<kuic::byte_t> key_value;
        };
    }
}

#endif
