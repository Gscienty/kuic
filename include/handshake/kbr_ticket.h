#ifndef _KUIC_HANDSHAKE_KBR_TICKET_
#define _KUIC_HANDSHAKE_KBR_TICKET_

#include "type.h"
#include "clock.h"
#include "package_serializer.h"
#include "lawful_package.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_encrypted_data.h"
#include "handshake/kbr_encryption_key.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_host_address.h"
#include <string>

namespace kuic {
    namespace handshake {

        class kbr_ticket_body
            : public package_serializable
            , public lawful_package {
        private:
            kuic::kbr_flag_t ticket_flags;
            kbr_encryption_key key;
            kbr_principal_name client_name;
            std::string client_realm;
            kuic::special_clock auth_time;
            kuic::special_clock start_time;
            kuic::special_clock end_time;
            kuic::special_clock renew_till;
            kbr_host_address client_address;

            kbr_ticket_body();
            kbr_ticket_body(kuic::error_t err);
        public:
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            
            static
            kbr_ticket_body
            deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
        };

        class kbr_ticket
            : public package_serializable
            , public lawful_package {
        private:
            kuic::kbr_ticket_version_t version;
            std::string realm;
            kbr_principal_name server_name;
            kbr_encrypted_data encrypted_data;
            kbr_ticket(kuic::error_t err);
        public:
            kbr_ticket();
            virtual std::basic_string<kuic::byte_t> serialize() const override;

            static
            kbr_ticket
            deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
        };
    }
}

#endif
