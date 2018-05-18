#ifndef _KUIC_CONGESTION_PRR_
#define _KUIC_CONGESTION_PRR_

#include "type.h"

namespace kuic {
    namespace congestion {
        class prr {
        private:
            kuic::bytes_count_t bytes_sent_since_loss;
            kuic::bytes_count_t bytes_delivered_since_loss;
            kuic::bytes_count_t ack_count_since_loss;
            kuic::bytes_count_t bytes_in_flight_before_loss;

        public:
            void initialize();
            void on_packet_sent(kuic::bytes_count_t sent_bytes);
            void on_packet_lost(kuic::bytes_count_t bytes_in_flight);
            void on_packet_acked(kuic::bytes_count_t acked_bytes);
            bool can_send(
                kuic::bytes_count_t congestion_window,
                kuic::bytes_count_t bytes_in_flight,
                kuic::bytes_count_t slowstart_threhold);
        };
    }
}

#endif
