#ifndef _KUIC_HANDSHAKE_KBR_KDC_RESPONSE_
#define _KUIC_HANDSHAKE_KBR_KDC_RESPONSE_

#include "type.h"
#include "clock.h"
#include "package_serializer.h"
#include "lawful_package.h"
#include "handshake/kbr_padata.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_ticket.h"
#include "handshake/kbr_encrypted_data.h"
#include "handshake/kbr_encryption_key.h"
#include "handshake/kbr_host_address.h"
#include "handshake/handshake_message.h"
#include <vector>
#include <string>

namespace kuic {
    namespace handshake {
        const kuic::kbr_message_type_t kbr_kdc_as_response = 11;
        const kuic::kbr_message_type_t kbr_kdc_tgs_response = 13;

        class kbr_kdc_last_request {
        
        };

        class kbr_kdc_response_part : public kuic::package_serializable {
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
            
            kbr_kdc_response_part();
        public:
            kbr_kdc_response_part(unsigned int nonce);

            void set_nonce(unsigned int nonce);
            void set_key(kbr_encryption_key key);
            void set_key_expiration(kuic::special_clock &clock);
            void set_server_realm(std::string realm);
            void set_server_name(kbr_principal_name server_name);

            unsigned int get_nonce() const;
            kbr_encryption_key get_encryption_key() const;
            kuic::special_clock get_key_expiration() const;
            std::string get_server_realm() const;
            kbr_principal_name get_server_name() const;

            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            static kbr_kdc_response_part deserialize(kuic::byte_t *buffer, const size_t size, size_t &seek);
        };


        class kbr_kdc_response
            : public kuic::package_serializable
            , public kuic::lawful_package {
        private:
            kuic::kbr_protocol_version_t version;
            kuic::kbr_message_type_t message_type;
            std::vector<kbr_padata> padatas;
            std::string realm;
            kbr_principal_name client_name;
            kbr_ticket ticket;
            kbr_encrypted_data encrypted_data;
            
            kuic::handshake::handshake_message __serialize() const;
            kbr_kdc_response();
            kbr_kdc_response(kuic::error_t err);
        public:
            static kbr_kdc_response build_as_response(
                    std::string realm,
                    kuic::kbr_encryption_type_t encryption_type,
                    kuic::byte_t *secret_key,
                    size_t secret_key_size,
                    kuic::handshake::kbr_kdc_response_part &part,
                    kuic::handshake::kbr_ticket &ticket);

            static kbr_kdc_response deserialize(kuic::byte_t *buffer, size_t len, size_t &seek);
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;

            kuic::kbr_protocol_version_t get_version() const;
            kuic::kbr_message_type_t get_message_type() const;
            std::string get_realm() const;
            kbr_principal_name get_client_name() const;
            kbr_encrypted_data get_encryption_key() const;
        };
    }
}

#endif
