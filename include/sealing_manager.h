#ifndef _KUIC_SEALING_MANAGER_
#define _KUIC_SEALING_MANAGER_

#include "crypt/aead.h"

namespace kuic {
    class sealing_manager {
    public:
        virtual kuic::crypt::aead &get_sealer() = 0;
    };
}

#endif

