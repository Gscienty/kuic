#ifndef _KUIC_CONGESTION_RTT_
#define _KUIC_CONGESTION_RTT_

#include "type.h"
#include "clock.h"

namespace kuic {
    namespace congestion {

        const float rtt_alpha = 0.125;
        const float rtt_beta = 0.25;
        const kuic::kuic_time_t default_initial_rtt = 100 * kuic::clock_millisecond;

        class rtt {
        private:
            kuic::kuic_time_t min_rtt;
            kuic::kuic_time_t latest_rtt;
            kuic::kuic_time_t smoothed_rtt;
            kuic::kuic_time_t mean_deviation;

        public:
            rtt();
            kuic::kuic_time_t get_min_rtt() const;
            kuic::kuic_time_t get_latest_rtt() const;
            kuic::kuic_time_t get_smoothed_rtt() const;
            kuic::kuic_time_t get_mean_deviation() const;

            kuic::kuic_time_t get_smoothed_or_initial_rtt() const;

            void update_rtt(kuic::kuic_time_t send_delta, kuic::kuic_time_t ack_delay);
            void on_connection_migration();
            void expire_smoothed_metrics();
        };
    }
}

#endif
