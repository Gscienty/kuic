#ifndef _KUIC_HANDSHAKE_KBR_PADATA_
#define _KUIC_HANDSHAKE_KBR_PADATA_

#include "type.h"
#include <string>

namespace kuic {
    namespace handshake {
        class kbr_padata {
        private:
            kuic::kbr_padata_type_t type;
            std::string value;
        };
    }
}

#endif