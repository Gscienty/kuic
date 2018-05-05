#ifndef _KUIC_HANDSHAKE_LAWFUL_PACKAGE_
#define _KUIC_HANDSHAKE_LAWFUL_PACKAGE_

#include "error.h"

namespace kuic {
    class lawful_package {
    private:
        kuic::error_t package_error;
    public:
        lawful_package();
        lawful_package(kuic::error_t err);

        bool is_lawful() const;
        kuic::error_t get_error() const;
        void set_error(kuic::error_t err);
    };
}

#endif

