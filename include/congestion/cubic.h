#ifndef _KUIC_CONGESTION_CUBIC_
#define _KUIC_CONGESTION_CUBIC_

#include "type.h"
#include "clock.h"

namespace kuic {
    namespace congestion {
        const int cubic_scale = 40;
        const int cubic_congestion_window_scale = 410;
        const kuic::bytes_count_t cubic_factor = 1 << cubic_scale / cubic_congestion_window_scale;
        const int default_connections_count = 2;

        const float cubic_beta = 0.7;
        const float cubic_beta_last_max = 0.85;
        const long cubic_max_time_interval = 30 * kuic::clock_millisecond;

        class cubic {
        private:
            size_t connections_count;
            kuic::special_clock epoch;
            
            kuic::bytes_count_t last_max_congestion_window;
            kuic::bytes_count_t acked_bytes_count;
            kuic::bytes_count_t estimated_tcp_congestion_window;
            kuic::bytes_count_t origin_point_congestion_window;
            kuic::bytes_count_t last_target_congestion_window;

            unsigned int time_to_origin_point;

        public:
            cubic();

            float beta_last_max() const;
            void reset();
            float alpha() const;
            float beta() const;
            void on_application_limited();
            kuic::bytes_count_t congestion_window_after_packet_loss(kuic::bytes_count_t current_congestion_window);
            kuic::bytes_count_t congestion_window_after_ack(
                    kuic::bytes_count_t acked_bytes,
                    kuic::bytes_count_t current_congestion_window,
                    kuic::kuic_time_t delay_min,
                    kuic::special_clock &eventTime);
            void set_connections_count(size_t count);
        };
    }
}

#endif
