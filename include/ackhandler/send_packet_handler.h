#ifndef _KUIC_ACKHANDLER_SEND_PACKET_HANDLER_
#define _KUIC_ACKHANDLER_SEND_PACKET_HANDLER_

#include "ackhandler/send_packet_history.h"
#include "congestion/cubic_sender.h"
#include "frame/ack_frame.h"
#include "clock.h"
#include "type.h"
#include <list>

namespace kuic {
    namespace ackhandler {

        const int max_tlps = 2;
        const float time_reordering_fraction = 1.0 / 8;
        const kuic::kuic_time_t default_rto_timeout = 500 * kuic::clock_millisecond;
        const kuic::kuic_time_t min_tlp_timeout = 10 * kuic::clock_millisecond;
        const kuic::kuic_time_t min_rto_timeout = 200 * kuic::clock_millisecond;
        const kuic::kuic_time_t max_rto_timeout = 60 * kuic::clock_second;

        class send_packet_handler {
        private:
            kuic::packet_number_t last_sent_packet_number;
            kuic::special_clock last_sent_retransmittable_packet_time;
            kuic::special_clock last_sent_handshake_packet_time;

            kuic::special_clock next_packet_send_time;
            std::list<kuic::packet_number_t> skipped_packets;

            kuic::packet_number_t largest_acked;
            kuic::packet_number_t largest_received_packet_with_ack;

            kuic::packet_number_t lowest_packet_not_confirmed_acked;
            kuic::packet_number_t largest_sent_before_rto;

            send_packet_history packet_history;
            
            std::list<packet> retransmission_queue;

            kuic::bytes_count_t bytes_in_flight;

            kuic::congestion::cubic_sender congestion;
            kuic::congestion::rtt &_rtt;
            
            bool handshake_complete;
            unsigned int handshake_count;

            unsigned int tlp_count;
            bool allow_tlp;

            unsigned int rto_count;
            int num_rtos;

            kuic::special_clock loss_time;
            
            kuic::special_clock alarm;
        public:
            kuic::packet_number_t &get_largest_acked() { return this->largest_acked; }
            send_packet_history &get_send_packet_history() { return this->packet_history; }
            kuic::bytes_count_t &get_bytes_in_flight() { return this->bytes_in_flight; }
            std::list<kuic::packet_number_t> &get_skipped_packets() { return this->skipped_packets; }
            kuic::packet_number_t &get_last_sent_packet_number() { return this->last_sent_packet_number; }
            kuic::special_clock &get_last_sent_retransmittable_packet_time() { return this->last_sent_retransmittable_packet_time; }
            kuic::special_clock &get_last_sent_handshake_packet_time() { return this->last_sent_handshake_packet_time; }
            
            send_packet_handler(kuic::congestion::rtt &_rtt);
            kuic::packet_number_t lowest_unacked() const;
            void set_handshake_complete();
            void sent_packet(packet p);
            bool sent_packet_implement(packet &p);
            void update_loss_detection_alarm();
            
            template<typename InputIterator> void sent_packets_as_retransmission(InputIterator begin, InputIterator end, size_t size, kuic::packet_number_t retransmission_of) {
                std::list<packet> p;
                std::for_each(begin, end,
                        [&] (packet &packet) -> void {
                             if (this->sent_packet_implement(packet)) {
                                p.push_back(packet);
                             }
                        });
                this->packet_history.send_packets_as_retrainsmission(p.begin(), p.end(), p.size(), retransmission_of);
                this->update_loss_detection_alarm();
            }

            bool received_ack(kuic::frame::ack_frame &ack_frame, kuic::packet_number_t with_packet_number, bool is_handshake, kuic::special_clock rcv_time);
            bool skipped_packets_acked(kuic::frame::ack_frame &ack_frame);
            std::list<packet> determine_newly_acked_packets(kuic::frame::ack_frame &ack_frame);
            bool on_packet_acked(packet &p);
            bool maybe_update_rtt(kuic::packet_number_t largest_acked, kuic::kuic_time_t ack_delay, kuic::special_clock rcv_time);
            bool detect_lost_packets(kuic::special_clock now, kuic::bytes_count_t prior_in_flight);
            void garbage_collect_skipped_packets();
            kuic::packet_number_t get_lowest_packet_not_confirmd_acked();
            kuic::kuic_time_t compute_handshake_timeout();
            kuic::kuic_time_t compute_rto_timeout();
            kuic::kuic_time_t compute_tlp_timeout();
            bool queue_packet_for_retransmission(const packet &packet);
            bool on_alarm();
            bool queue_handshake_packets_for_retransmission();
            bool queue_rtos();
            kuic::special_clock get_alarm_timeout();

            void verify_rto(kuic::packet_number_t packet_number);

            bool stop_retransmission_for(const packet &p);
            kuic::nullable<packet> dequeue_packet_for_retransmission();
            size_t get_packet_number_length(kuic::packet_number_t p);
            kuic::send_mode_t get_send_mode();
            kuic::special_clock time_until_send();
            int should_send_num_packets();
        };
    }
}

#endif

