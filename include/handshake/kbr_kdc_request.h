#ifndef _KUIC_HANDSHAKE_CLIENT_TO_KBR_
#define _KUIC_HANDSHAKE_CLIENT_TO_KBR_

#include "type.h"
#include "clock.h"
#include "handshake/kbr_padata.h"
#include "handshake/kbr_encrypted_data.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_ticket.h"
#include "handshake/handshake_message.h"
#include <string>
#include <vector>

namespace kuic {
    namespace handshake {
        const kuic::kbr_message_type_t kbr_kdc_as_request = 10;
        const kuic::kbr_message_type_t kbr_kdc_tgs_request = 12;

        class kbr_kdc_request {
        private:
            kuic::kbr_protocol_version_t version;
            kuic::kbr_message_type_t message_type;
            std::vector<kbr_padata> padatas;
            kuic::kbr_flag_t options;
            kbr_principal_name client_name;
            std::string realm;
            kbr_principal_name server_name;
            kuic::special_clock from;
            kuic::special_clock till;
            kuic::special_clock rtime;
            unsigned int nonce;
            std::vector<kuic::kbr_encryption_type_t> encrypt_types;
            std::string address;
            kbr_encrypted_data encrypted_data;
            std::vector<kbr_ticket> tickets;

        public:
            static kbr_kdc_request build_as_request(
                kbr_principal_name client_name,
                std::string realm,
                unsigned int nonce);

            handshake_message serialize();
        };
    }
}

#endif