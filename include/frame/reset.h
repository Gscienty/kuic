#ifndef _KUIC_FRAME_RESET_
#define _KUIC_FRAME_RESET_

#include "connection_id.h"
#include "lawful_package.h"
#include "type.h"
#include <string>

namespace kuic {
    namespace frame {
        class reset 
            : kuic::lawful_package {
        private:
            kuic::packet_number_t rejected_packet_number;
            unsigned long nonce;

            reset(kuic::error_t error) : lawful_package(error) { }
        public:

            reset() { }
            static std::basic_string<kuic::byte_t> write_public_reset(
                    kuic::connection_id conn_id, kuic::packet_number_t rejected_packet_number, unsigned long nonce_proof);

            static reset parse_public_reset(std::basic_string<kuic::byte_t> &buffer, size_t &seek);
        };
    }
}

#endif

