#ifndef _KUIC_HANDSHAKE_KBR_ERROR_
#define _KUIC_HANDSHAKE_KBR_ERROR_

#include "handshake/kbr_principal_name.h"
#include "handshake/handshake_message.h"
#include "package_serializer.h"
#include "clock.h"
#include "error.h"
#include "type.h"
#include <string>

namespace kuic {
    namespace handshake {
        const kuic::kbr_message_type_t kbr_error_type = 30;

        class kbr_error : kuic::package_serializable {
        private:
            kuic::kbr_protocol_version_t version;
            kuic::kbr_message_type_t message_type;
            kuic::special_clock current_clock;
            kuic::special_clock server_clock;
            kuic::error_t error;
            kbr_principal_name client_name;
            std::string client_realm;
            std::string server_realm;
            kbr_principal_name server_name;
            std::string error_string;
        public:
            virtual std::basic_string<kuic::byte_t> serialize() const override;

            static kbr_error deserialize(handshake_message &msg);
        };
    }
}

#endif

