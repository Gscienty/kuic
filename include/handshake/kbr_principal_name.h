#ifndef _KUIC_HANDSHAKE_KBR_PRINCIPAL_NAME_
#define _KUIC_HANDSHAKE_KBR_PRINCIPAL_NAME_

#include "type.h"
#include "package_serializer.h"
#include <string>

namespace kuic {
    namespace handshake {
        class kbr_principal_name : public package_serializable {
        private:
            kuic::kbr_name_type_t type;
            std::string name;
        public:
            kbr_principal_name();
            kbr_principal_name(std::string name);

            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            static kbr_principal_name deserialize(kuic::byte_t *buffer, size_t len, size_t &seek);

            kuic::kbr_name_type_t get_type() const;
            std::string get_name() const;
        };
    }
}

#endif
