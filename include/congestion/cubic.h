#ifndef _KUIC_CONGESTION_CUBIC_
#define _KUIC_CONGESTION_CUBIC_

#include "type.h"
#include "clock.h"

namespace kuic {
    namespace congestion {
        const int cubic_scale = 40;
        const int cubic_congestion_window_scale = 410;
        const unsigned long cubic_factor = 1 << cubic_scale / cubic_congestion_window_scale;
        const int cubic_default_connections_count = 2;

        const float cubic_beta = 0.7;
        const float cubic_beta_last_max = 0.85;
        const long cubic_max_time_interval = 30 * kuic::clock_millisecond;

        class cubic {
        private:
            kuic::clock &clock;
            size_t connections_count;
            kuic::special_clock epoch;
            kuic::special_clock app_limit_start_time;
            kuic::special_clock last_update_time;
            
            kuic::packet_number_t last_congestion_window;
            kuic::packet_number_t last_max_congestion_window;
            kuic::packet_number_t acked_packets_count;
            kuic::packet_number_t estimated_tcp_congestion_window;
            kuic::packet_number_t origin_point_congestion_window;
            kuic::packet_number_t last_target_congestion_window;

            unsigned int time_to_origin_point;

        public:
            cubic(kuic::clock &);

            void reset();
            float alpha() const;
            float beta() const;
            void on_application_limited();
            kuic::packet_number_t congestion_window_after_packet_loss(
                kuic::packet_number_t current_congestion_window);
            kuic::packet_number_t congestion_window_after_ack(
                kuic::packet_number_t current_congestion_window, kuic::kuic_time_t delay_min);
            void set_connection_counts(size_t count);
        };
    }
}

#endif