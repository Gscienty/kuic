#ifndef _KUIC_ACKHANDLER_RECEIVE_PACKET_HANDLER_
#define _KUIC_ACKHANDLER_RECEIVE_PACKET_HANDLER_

#include "congestion/rtt.h"
#include "ackhandler/receive_packet_history.h"
#include "frame/ack_frame.h"
#include "clock.h"
#include "type.h"
#include "define.h"
#include <memory>

namespace kuic {
    namespace ackhandler {
        
        const kuic::kuic_time_t ack_send_delay = 25 * kuic::clock_millisecond;
        const int initial_retransmittable_packets_before_ack = 2;
        const int retransmittable_packets_before_ack = 10;
        const float ack_decimation_delay = 1.0 / 4;
        const float short_ack_decimation_delay = 1.0 / 8;
        const int min_received_before_ack_decimation = 100;
        const int max_packets_after_new_missing = 4;


        class received_packet_handler {
        private:
            kuic::packet_number_t largest_observed;
            kuic::packet_number_t _ignore_below;
            kuic::special_clock largest_observed_received_time;

            kuic::ackhandler::receive_packet_history packet_history;

            kuic::kuic_time_t ack_send_delay;
            kuic::congestion::rtt &rtt;

            int packets_received_since_last_ack;
            int retransmittable_packets_received_since_last_ack;
            bool ack_queued;
            kuic::special_clock ack_alarm;
            std::shared_ptr<kuic::frame::ack_frame> last_ack;
        
        public:
            received_packet_handler(kuic::congestion::rtt &rtt);

            bool received_packet(
                    kuic::packet_number_t packet_number, kuic::special_clock rcv_time, bool should_instigate_ack);
            bool is_missing(kuic::packet_number_t packet_number);
            void try_queue_ack(
                    kuic::packet_number_t packet_number, kuic::special_clock rcv_time, bool should_instigate_ack, bool was_missing);
            void ignore_below(kuic::packet_number_t packet_number);
            bool has_new_missing_packets();
            std::shared_ptr<kuic::frame::ack_frame> get_ack_frame();
            kuic::special_clock get_alarm_timeout();
            virtual ~received_packet_handler();
        };
    }
}

#endif

