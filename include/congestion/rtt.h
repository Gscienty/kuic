#ifndef _KUIC_CONGESTION_RTT_
#define _KUIC_CONGESTION_RTT_

#include "type.h"
#include "clock.h"

namespace kuic {
    namespace congestion {

        const kuic::kuic_time_t rtt_initial_rtt_us = 100 * kuic::clock_microsecond;
        const float rtt_alpha = 0.125;
        const float rtt_beta = 0.25;
        const float rtt_half_window = 0.5;
        const float rtt_quarter_window = 0.25;

        struct rtt_sample {
            kuic::kuic_time_t rtt;
            kuic::clock t;
        };

        class rtt {
        private:
            kuic::kuic_time_t initial_rtt_us;
            kuic::kuic_time_t recent_min_rtt_window;
            kuic::kuic_time_t min_rtt;
            kuic::kuic_time_t latest_rtt;
            kuic::kuic_time_t smoothed_rtt;
            kuic::kuic_time_t mean_deviation;

            unsigned int min_rtt_samples_remaining_count;

            rtt_sample new_min_rtt;
            rtt_sample recent_min_rtt;
            rtt_sample half_window_rtt;
            rtt_sample quarter_window_rtt;

            void update_recent_min_rtt(kuic::kuic_time_t sample, kuic::clock now);
        public:
            rtt();
            kuic::kuic_time_t get_initial_rtt_us() const;
            kuic::kuic_time_t get_min_rtt() const;
            kuic::kuic_time_t get_latest_rtt() const;
            kuic::kuic_time_t get_recent_min_rtt() const;
            kuic::kuic_time_t get_smoothed_rtt() const;
            kuic::kuic_time_t get_quarter_window_rtt() const;
            kuic::kuic_time_t get_half_window_rtt() const;
            kuic::kuic_time_t get_mean_deviation() const;

            void set_recent_min_rtt_window(const kuic::kuic_time_t recent_min_rtt_window);

            void update_rtt(kuic::kuic_time_t send_delta, kuic::kuic_time_t ack_delay, kuic::clock now);
            void sample_new_recent_min_rtt(unsigned int samples_count);
            void on_connection_migration();

            void expire_smoothed_metrics();
        };
    }
}

#endif