#ifndef _KUIC_HANDSHAKE_KBR_HOST_ADDRESS_
#define _KUIC_HANDSHAKE_KBR_HOST_ADDRESS_

#include "type.h"
#include <string>

namespace kuic {
    namespace handshake {
    
        class kbr_host_address {
        private:
            kuic::kbr_address_type_t address_type;
            std::string address;
        };
    
    }
}

#endif

