#ifndef _KUIC_VARIABLE_INTEGER_
#define _KUIC_VARIABLE_INTEGER_

#include "type.h"
#include "unistd.h"
#include "utility"

namespace kuic {

    const unsigned long variable_integer_max_1 = 63;
    const unsigned long variable_integer_max_2 = 16383;
    const unsigned long variable_integer_max_4 = 1073741823;
    const unsigned long variable_integer_max_8 = 4611686018427387903; 

    class variable_integer {
    public:
        static unsigned long read(const kuic::byte_t *buffer, size_t size, size_t &seek);
        static std::pair<kuic::byte_t *, size_t> write(unsigned long value);
        static size_t length(unsigned long value);
    };
}

#endif
