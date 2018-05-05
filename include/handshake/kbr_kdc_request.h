#ifndef _KUIC_HANDSHAKE_KDC_REQUEST_ 
#define _KUIC_HANDSHAKE_KDC_REQUEST_

#include "package_serializer.h"
#include "type.h"
#include "clock.h"
#include "lawful_package.h"
#include "handshake/kbr_padata.h"
#include "handshake/kbr_encrypted_data.h"
#include "handshake/kbr_principal_name.h"
#include "handshake/kbr_ticket.h"
#include "handshake/handshake_message.h"
#include <string>
#include <vector>

namespace kuic {
    namespace handshake {
        const kuic::kbr_message_type_t kbr_kdc_as_request   = 10;
        const kuic::kbr_message_type_t kbr_kdc_tgs_request  = 12;

        class kbr_kdc_request_body
            : public package_serializable
            , public lawful_package {
        private:
            kuic::kbr_message_type_t message_type;

            kuic::kbr_flag_t options;
            kbr_principal_name client_name;
            kbr_principal_name server_name;
            std::string realm;
            kuic::special_clock from;
            kuic::special_clock till;
            kuic::special_clock renew_time;
            unsigned int nonce;
            std::vector<kuic::kbr_encryption_type_t> encrypt_types;
            std::string address;
            kbr_encrypted_data authorization_data;
            std::vector<kbr_ticket> tickets;

            kbr_kdc_request_body(kuic::error_t err);
        public:
            kbr_kdc_request_body();
            kbr_kdc_request_body(
                    kuic::kbr_message_type_t message_type,
                    kbr_principal_name name,
                    std::string realm,
                    unsigned int nonce);

            unsigned int get_nonce() const;
            kbr_principal_name get_client_name() const;
            kbr_principal_name get_server_name() const;
            std::string get_realm() const;
            kuic::special_clock get_from() const;
            kuic::special_clock get_till() const;
            kuic::special_clock get_renew_time() const;
            kbr_encrypted_data get_authorization_data() const;
            
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            static kbr_kdc_request_body deserialize(
                    const kuic::byte_t *buffer, size_t len, size_t &seek);

            void support_encrypt_type(kuic::kbr_encryption_type_t encryption_type);
        };

        class kbr_kdc_request : public package_serializable {
        private:
            kuic::kbr_protocol_version_t version;
            kuic::kbr_message_type_t message_type;
            std::vector<kbr_padata> padatas;
            kbr_kdc_request_body body;

            static kbr_kdc_request __deserialize(handshake_message &msg);
            handshake_message __serialize() const;

            kbr_kdc_request();
        public:
            static kbr_kdc_request build_as_request(
                    kbr_principal_name client_name,
                    std::string realm,
                    unsigned int nonce);

            static kbr_kdc_request build_tgs_request(
                    kbr_principal_name server_name,
                    std::string realm,
                    unsigned int nonce);

            kbr_kdc_request_body get_body() const;
            kuic::kbr_message_type_t get_message_type() const;

            static kbr_kdc_request deserialize(
                    const kuic::byte_t *buffer, const size_t len, size_t &seek);
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
        };
    }
}

#endif
