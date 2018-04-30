#ifndef _KUIC_HANDSHAKE_KBR_TICKET_
#define _KUIC_HANDSHAKE_KBR_TICKET_

#include "type.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_encrypted_data.h"
#include <string>

namespace kuic {
    namespace handshake {
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
