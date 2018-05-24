#include "ackhandler/receive_packet_handler.h"
#include <algorithm>

kuic::ackhandler::received_packet_handler::received_packet_handler(kuic::congestion::rtt &rtt)
    : largest_observed(0)
    , _ignore_below(0)
    , largest_observed_received_time(kuic::special_clock({ 0, 0 }))
    , ack_send_delay(0)
    , rtt(rtt)
    , packets_received_since_last_ack(0)
    , retransmittable_packets_received_since_last_ack(0)
    , ack_queued(false)
    , ack_alarm(kuic::special_clock({ 0, 0 }))
    , last_ack(kuic::nullable<kuic::frame::ack_frame>(nullptr)) { }

bool kuic::ackhandler::received_packet_handler::received_packet(
        kuic::packet_number_t packet_number, kuic::special_clock rcv_time, bool should_instigate_ack) {

    if (packet_number > this->largest_observed) {
        return true;
    }
    
    bool is_missing = this->is_missing(packet_number);
    if (packet_number > this->largest_observed) {
        this->largest_observed = packet_number;
        this->largest_observed_received_time = rcv_time;
    }

    if (this->packet_history.received_packet(packet_number) == false) {
        return false;
    }

    this->try_queue_ack(packet_number, rcv_time, should_instigate_ack, is_missing);
    return true;
}

void kuic::ackhandler::received_packet_handler::ignore_below(kuic::packet_number_t packet_number) {
    if (packet_number <= this->_ignore_below) {
        return;
    }

    this->_ignore_below = packet_number;
    this->packet_history.delete_below(packet_number);
}

bool kuic::ackhandler::received_packet_handler::is_missing(kuic::packet_number_t packet_number) {
    if (this->last_ack.is_null() || packet_number < this->_ignore_below) {
        return false;
    }
    return packet_number < this->last_ack->largest_acked() && this->last_ack->acks_packet(packet_number) == false;
}

bool kuic::ackhandler::received_packet_handler::has_new_missing_packets() {
    if (this->last_ack.is_null()) {
        return false;
    }
    auto highest_range = this->packet_history.get_highest_ack_range();
    return highest_range.first >= this->last_ack->largest_acked() && 
        highest_range.second - highest_range.first + 1 <= kuic::ackhandler::max_packets_after_new_missing;
}

void kuic::ackhandler::received_packet_handler::try_queue_ack(
        kuic::packet_number_t packet_number, kuic::special_clock rcv_time, bool should_instigate_ack, bool was_missing) {
    this->packets_received_since_last_ack++;

    if (this->last_ack.is_null()) {
        this->ack_queued = true;
        return; 
    }

    if (was_missing) {
        this->ack_queued = true;
    }

    if (this->ack_queued == false && should_instigate_ack) {
        this->retransmittable_packets_received_since_last_ack++;

        if (packet_number > kuic::ackhandler::min_received_before_ack_decimation) {
            if (this->retransmittable_packets_received_since_last_ack >= kuic::ackhandler::retransmittable_packets_before_ack) {
                this->ack_queued = true;
            }
            else if (this->ack_alarm.is_zero()) {
                kuic::kuic_time_t ack_delay = std::min(
                        ack_send_delay,
                        kuic::kuic_time_t(double(this->rtt.get_min_rtt()) / double(kuic::ackhandler::ack_decimation_delay)));
                this->ack_alarm = rcv_time + ack_delay;
            }
        }
        else {
            if (this->retransmittable_packets_received_since_last_ack >= kuic::ackhandler::initial_retransmittable_packets_before_ack) {
                this->ack_queued = true;
            }
            else if (this->ack_alarm.is_zero()) {
                this->ack_alarm = rcv_time + ack_send_delay;
            }
        }

        if (this->has_new_missing_packets()) {
            kuic::kuic_time_t ack_delay = kuic::kuic_time_t(double(this->rtt.get_min_rtt()) / double(should_instigate_ack));
            kuic::special_clock ack_time = rcv_time + ack_delay;
            if (this->ack_alarm.is_zero() || this->ack_alarm > ack_time) {
                this->ack_alarm = ack_time;
            }
        }
    }

    if (this->ack_queued) {
        this->ack_alarm = kuic::special_clock({ 0, 0 });
    }
}

kuic::nullable<kuic::frame::ack_frame>
kuic::ackhandler::received_packet_handler::get_ack_frame() {
    kuic::current_clock current_clock;
    kuic::special_clock now(current_clock);

    if (this->ack_queued == false && (this->ack_alarm.is_zero() || this->ack_alarm > now)) {
        return kuic::nullable<kuic::frame::ack_frame>(nullptr);
    }

    kuic::nullable<kuic::frame::ack_frame> frame(*(new kuic::frame::ack_frame()));

    frame->get_ranges() = this->packet_history.get_ack_ranges();
    frame->get_delay_time() = now - this->largest_observed_received_time;
    
    if (this->last_ack.is_null() == false) {
        delete this->last_ack.release();
    }

    this->last_ack = frame;
    this->ack_alarm = kuic::special_clock({ 0, 0 });
    this->ack_queued = false;
    this->packets_received_since_last_ack = 0;
    this->retransmittable_packets_received_since_last_ack = 0;

    return frame;
}

kuic::special_clock kuic::ackhandler::received_packet_handler::get_alarm_timeout() {
    return this->ack_alarm;
}

kuic::ackhandler::received_packet_handler::~received_packet_handler() {
    if (this->last_ack.is_null() == false) {
        delete this->last_ack.release();
    }
}
