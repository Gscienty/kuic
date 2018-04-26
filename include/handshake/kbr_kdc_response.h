#ifndef _KUIC_HANDSHAKE_KBR_KDC_RESPONSE_
#define _KUIC_HANDSHAKE_KBR_KDC_RESPONSE_

#include "type.h"
#include "clock.h"
#include "handshake/kbr_padata.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_ticket.h"
#include "handshake/kbr_encrypted_data.h"
#include "handshake/kbr_encryption_key.h"
#include "handshake/kbr_host_address.h"
#include <vector>
#include <string>

namespace kuic {
    namespace handshake {
        const kuic::kbr_message_type_t kbr_kdc_as_response = 11;
        const kuic::kbr_message_type_t kbr_kdc_tgs_response = 13;

        class kbr_kdc_last_request {
        
        };

        class kbr_kdc_response_part {
        private:
            kbr_encryption_key key;
            std::vector<kbr_kdc_last_request> last_req;    
            unsigned int nonce;
            kuic::kbr_flag_t flags;
            kuic::special_clock auth_time;
            kuic::special_clock start_time;
            kuic::special_clock end_time;
            kuic::special_clock renew_till;
            kuic::special_clock key_expiration;
            std::string server_realm;
            kbr_principal_name server_name;
            kbr_host_address client_address;

        public:
            kbr_kdc_response_part(
                    std::string server_name,
                    std::string server_realm,
                    unsigned int nonce);

            void set_key_expiration(kuic::special_clock &clock);
        };


        class kbr_kdc_response {
        private:
            kuic::kbr_protocol_version_t version;
            kuic::kbr_message_type_t message_type;
            std::vector<kbr_padata> padatas;
            std::string realm;
            kbr_principal_name client_name;
            kbr_ticket ticket;
            kbr_encrypted_data encrypted_data;
            kbr_kdc_response_part encryped_part;
            
            kbr_kdc_response();
        public:
            static kbr_kdc_response build_as_response();
        };
    }
}

#endif
