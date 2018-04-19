#ifndef _KUIC_CONGESTION_SLOW_START_
#define _KUIC_CONGESTION_SLOW_START_

#include "type.h"

namespace kuic {
    namespace congestion {
        const kuic::packet_number_t slow_start_low_window = 16;
        const unsigned int slow_start_min_samples = 8;
        const int slow_start_delay_factor_exp = 3;
        const long slow_start_delay_min_threshole_us = 4000;
        const long slow_start_delay_max_threshold_us = 16000;

        class slow_start {
        private:
            kuic::packet_number_t end_packet_number;
            kuic::packet_number_t last_sent_packet_number;
            bool is_started;
            kuic::kuic_time_t current_min_rtt;
            unsigned int rtt_samples_count;
            bool hystart_found;
        public:
            void start_receive_round(kuic::packet_number_t last_sent);
            bool is_end_of_round(kuic::packet_number_t ack);
            bool should_exist_slow_start(
                kuic::kuic_time_t lastest_rtt, kuic::kuic_time_t min_rtt, kuic::bytes_count_t congestion_window);
            void on_packet_sent(kuic::packet_number_t packet_number);
            void on_packet_acked(kuic::packet_number_t acked_packet_number);
            bool started() const;
            void restart();
        };
    }
}

#endif