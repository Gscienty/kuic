#ifndef _KUIC_PACKET_NUMBER_GENERATOR_
#define _KUIC_PACKET_NUMBER_GENERATOR_

#include "type.h"

namespace kuic {
    class packet_number_generator {
    private:
        kuic::packet_number_t average_period;
        kuic::packet_number_t next;
        kuic::packet_number_t next_to_skip;

    public:
        packet_number_generator(kuic::packet_number_t initial, kuic::packet_number_t average_period);
        kuic::packet_number_t peek();
        kuic::packet_number_t pop();
        bool generate_new_skip();
        unsigned short get_random_number();
    };
}

#endif
