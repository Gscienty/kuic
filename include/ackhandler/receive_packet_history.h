#ifndef _KUIC_ACKHANDLER_RECEIVE_PACKET_HISTORY_
#define _KUIC_ACKHANDLER_RECEIVE_PACKET_HISTORY_

#include "type.h"
#include <utility>
#include <vector>
#include <list>

namespace kuic {
    namespace ackhandler {
        class receive_packet_history {
        private:
            std::list<std::pair<kuic::packet_number_t, kuic::packet_number_t>> ranges;
            kuic::packet_number_t lowest_in_received_packet_numbers;
        public:
            receive_packet_history() { }
            bool received_packet(kuic::packet_number_t packet_number);
            void delete_below(kuic::packet_number_t packet_number);
            std::vector<std::pair<kuic::packet_number_t, kuic::packet_number_t>> get_ack_ranges();
            std::pair<kuic::packet_number_t, kuic::packet_number_t> get_highest_ack_range();
        };
    }
}

#endif

