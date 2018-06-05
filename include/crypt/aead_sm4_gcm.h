#ifndef _KUIC_CRYPT_AEAD_SM4_GCM_
#define _KUIC_CRYPT_AEAD_SM4_GCM_

#include "crypt/aead.h"
#include <string>

namespace kuic {
    namespace crypt {
        class aead_sm4_gcm : public aead {
        private:
            std::string key;
            std::string iv;
        public:
            aead_sm4_gcm(
                    const std::string key,
                    const std::string iv);

            virtual kuic::bytes_count_t overhead() const override {
                return 16;
            }

            virtual std::string seal(
                    const std::string plain, const kuic::packet_number_t packet_number, const std::string a_data) override;

            virtual std::string open(
                    const std::string secret, const kuic::packet_number_t packet_number, const std::string a_data) override;

            std::string make_nonce(const kuic::packet_number_t packet_number);
        };
    }
}

#endif

