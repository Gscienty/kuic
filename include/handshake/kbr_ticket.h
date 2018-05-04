#ifndef _KUIC_HANDSHAKE_KBR_TICKET_
#define _KUIC_HANDSHAKE_KBR_TICKET_

#include "type.h"
#include "clock.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_encrypted_data.h"
#include "handshake/kbr_encryption_key.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_host_address.h"
#include <string>

namespace kuic {
    namespace handshake {

        class kbr_ticket_body {
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
        };

        class kbr_ticket {
        private:
            kuic::kbr_ticket_version_t version;
            std::string realm;
            kbr_principal_name server_name;
            kbr_encrypted_data encrypted_data;
        public:
            kbr_ticket() { }
        };
    }
}

#endif
