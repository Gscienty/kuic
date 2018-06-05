#ifndef _KUIC_CRYPT_AEAD_
#define _KUIC_CRYPT_AEAD_

#include "type.h"
#include <utility>
#include <unistd.h>
#include <string>

namespace kuic {
    namespace crypt {
        class aead {
        public:
            virtual kuic::bytes_count_t overhead() const = 0;
            virtual std::string seal(
                    const std::string plain,
                    const kuic::packet_number_t packet_number,
                    const std::string a_data) = 0;
            virtual std::string open(
                    const std::string secret,
                    const kuic::packet_number_t packet_number,
                    const std::string a_data) = 0;
        };
    }
}

#endif

