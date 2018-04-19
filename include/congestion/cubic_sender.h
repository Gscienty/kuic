#ifndef _KUIC_CONGESTION_CUBIC_SENDER_
#define _KUIC_CONGESTION_CUBIC_SENDER_

#include "define.h"
#include "type.h"
#include "congestion/cubic.h"
#include "congestion/slow_start.h"
#include "congestion/prr.h"
#include "congestion/rtt.h"

namespace kuic {
    namespace congestion {
        
        const kuic::packet_number_t default_minimum_congestion_window = 2;
        const kuic::bytes_count_t max_burst_bytes = 3 * kuic::default_tcp_mss;

        struct connection_stats {
            kuic::packet_number_t slowstart_packets_lost;
            kuic::packet_number_t slowstart_bytes_lost;
        };

        class cubic_sender {
        private:
            slow_start slowstart;
            prr _prr;
            rtt _rtt;
            connection_stats stats;
            cubic _cubic;

            kuic::packet_number_t largest_sent_packet_number;
            kuic::packet_number_t largest_acked_packet_number;
            kuic::packet_number_t largest_sent_at_last_cutback;
            kuic::packet_number_t congestion_window;
            kuic::packet_number_t _slowstart_threshold;
            kuic::packet_number_t min_congestion_window;
            kuic::packet_number_t max_tcp_congestion_window;
            kuic::packet_number_t initial_congestion_window;
            kuic::packet_number_t initial_max_congestion_window;
            bool last_cutback_exited_slowstart;
            bool slowstart_large_reduction;
            size_t connections_count;
            kuic::bytes_count_t congestion_window_count;

            void try_increase_cwnd(
                kuic::packet_number_t acked_packet_number,
                kuic::bytes_count_t acked_bytes,
                kuic::bytes_count_t bytes_in_flight);
            bool is_cwnd_limited(kuic::bytes_count_t bytes_in_flight);
        public:
            cubic_sender(
                kuic::clock &clock,
                rtt &rtt_stat,
                kuic::packet_number_t initial_congestion_window,
                kuic::packet_number_t initial_max_congestion_window);
            
            kuic::kuic_time_t time_until_send(kuic::packet_number_t bytes_in_flight);
            bool on_packet_sent(
                kuic::clock &t,
                kuic::bytes_count_t bytes_in_flight,
                kuic::packet_number_t packet_number,
                kuic::bytes_count_t bytes,
                bool is_retransmittable);
            
            bool in_recovery();
            bool in_slowstart();
            kuic::bytes_count_t get_congestion_window();
            kuic::bytes_count_t get_slowstart_threshold();
            kuic::packet_number_t slowstart_threshold();

            void exit_slowstart();
            void try_exit_slowstart();
            void on_packet_acked(
                kuic::packet_number_t acked_packet_number,
                kuic::bytes_count_t acked_bytes,
                kuic::bytes_count_t bytes_in_flight);
            void on_packet_lost(
                kuic::packet_number_t packet_number,
                kuic::bytes_count_t lost_bytes,
                kuic::bytes_count_t bytes_in_flight);
            
            kuic::band_width_t bandwidth_estimate();

            slow_start &get_slowstart();

            void set_connections_count(size_t n);
            void on_retransmission_timeout(bool packets_retransmitted);
            void on_connection_migration();
            void set_slowstart_large_reduction(bool enabled);
        };
    }
}

#endif