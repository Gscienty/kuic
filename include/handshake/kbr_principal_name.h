#ifndef _KUIC_HANDSHAKE_KBR_PRINCIPAL_NAME_
#define _KUIC_HANDSHAKE_KBR_PRINCIPAL_NAME_

#include "type.h"
#include <string>

namespace kuic {
    namespace handshake {
        class kbr_principal_name {
        private:
            kuic::kbr_name_type_t type;
            std::string name;
        public:
            kbr_principal_name(std::string name);

            char *serialize(size_t &size);
        };
    }
}

#endif