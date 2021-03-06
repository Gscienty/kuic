#ifndef _KUIC_HANDSHAKE_KBR_AUTHORIZATION_DATA_
#define _KUIC_HANDSHAKE_KBR_AUTHORIZATION_DATA_

#include "type.h"
#include "package_serializer.h"
#include "lawful_package.h"
#include "handshake/kbr_principal_name.h"
#include <vector>
#include <memory>

namespace kuic {
    namespace handshake {

        const kbr_authorization_data_item_type_t ad_type_not_expect         = 0;
        const kbr_authorization_data_item_type_t ad_type_if_relevant        = 1;
        const kbr_authorization_data_item_type_t ad_type_kdc_issued         = 4;
        const kbr_authorization_data_item_type_t ad_type_and_or             = 5;
        const kbr_authorization_data_item_type_t ad_type_mandatory_for_kdc  = 8;

        class ad_item
            : public kuic::package_serializable
            , public kuic::lawful_package {
        public:
            ad_item();
            ad_item(kuic::error_t err);

            virtual kuic::kbr_authorization_data_item_type_t get_type() const = 0;
        };
        
        // serialized ad_item
        class kbr_authorization_data_item : public kuic::package_serializable {
        protected:
            kuic::kbr_authorization_data_item_type_t type;
            std::basic_string<kuic::byte_t> data;
        public:
            kbr_authorization_data_item(ad_item *item);
            kbr_authorization_data_item();

            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static kbr_authorization_data_item deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            ad_item *get_item() const;
            kuic::kbr_authorization_data_item_type_t get_type() const;
        };
        
        // authorization data
        class kbr_authorization_data : public package_serializable {
        private:
            std::vector<kbr_authorization_data_item> elements;
        public:
            kbr_authorization_data();
            
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static kbr_authorization_data deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);

            void add_element(kbr_authorization_data_item element);
            std::vector<kbr_authorization_data_item> get_elements() const;
        };

        // AD-item not expect
        class not_expect_ad_item : public ad_item {
        public:
            not_expect_ad_item();
            virtual kuic::kbr_authorization_data_item_type_t get_type() const override;
        };
        
        // AD-if_relevant
        class if_relevant_ad_item : public ad_item {
        private:
            std::basic_string<kuic::byte_t> data;
        public:
            if_relevant_ad_item();
            if_relevant_ad_item(const std::basic_string<kuic::byte_t> &buffer);
            virtual kuic::kbr_authorization_data_item_type_t get_type() const override;

            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static if_relevant_ad_item deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            std::basic_string<kuic::byte_t> get_data() const;
        };

        // AD-issued
        class kdc_issued_ad_item : public ad_item {
        private:
            std::basic_string<kuic::byte_t> checksum;
            std::string issue_realm;
            kbr_principal_name issue_name;
            kuic::handshake::kbr_authorization_data elements;
        public:
            virtual kuic::kbr_authorization_data_item_type_t get_type() const override;

            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static kdc_issued_ad_item deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);

        };

        // AD-and_or 
        class and_or_ad_item : public ad_item {
        private:
            int condition_count;
            kuic::handshake::kbr_authorization_data elements;
        public:
            virtual kuic::kbr_authorization_data_item_type_t get_type() const override;

            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static and_or_ad_item deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
        };

        // mandatory for kdc
        class mandatory_ad_item : public ad_item {
        private:
            kuic::handshake::kbr_authorization_data elements;
        public:
            virtual kuic::kbr_authorization_data_item_type_t get_type() const override;
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static mandatory_ad_item deserialize(
                    const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
        };
    }
}

#endif

