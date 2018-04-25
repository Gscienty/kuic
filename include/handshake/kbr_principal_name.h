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
            static kbr_principal_name deserialize(const char *buffer, size_t len);
            kbr_principal_name();
            kbr_principal_name(std::string name);

            char *serialize(size_t &size);

            kuic::kbr_name_type_t get_type() const;
            std::string get_name() const;
        };
    }
}

#endif
