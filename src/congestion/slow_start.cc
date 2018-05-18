#include "congestion/slow_start.h"
#include "clock.h"
#include <algorithm>

void kuic::congestion::slow_start::start_receive_round(kuic::packet_number_t last_sent) {
    this->end_packet_number = last_sent;
    this->current_min_rtt = 0;
    this->rtt_samples_count = 0;
    this->is_started = true;
}

bool kuic::congestion::slow_start::is_end_of_round(kuic::packet_number_t ack) {
    return this->end_packet_number < ack;
}

bool kuic::congestion::slow_start::should_exist_slow_start(
    kuic::kuic_time_t latest_rtt, kuic::kuic_time_t min_rtt, kuic::bytes_count_t congestion_window) {
    
    if (this->is_started == false) {
        this->start_receive_round(this->last_sent_packet_number);
    }
    if (this->hystart_found) {
        return true;
    }

    this->rtt_samples_count++;
    if (this->rtt_samples_count <= kuic::congestion::slow_start_min_samples && 
        (this->current_min_rtt == 0 || this->current_min_rtt > latest_rtt)) {
        this->current_min_rtt = latest_rtt;
    }

    if (this->rtt_samples_count == kuic::congestion::slow_start_min_samples) {
        long min_rtt_increase_threshold = std::max<long>(
            kuic::congestion::slow_start_delay_min_threshole_us,
            std::min<long>(
                kuic::congestion::slow_start_delay_max_threshold_us,
                long(min_rtt / kuic::clock_microsecond >> kuic::congestion::slow_start_delay_factor_exp)));
        if (this->current_min_rtt > min_rtt + min_rtt_increase_threshold) {
            this->hystart_found = true;
        }
    }
    return congestion_window >= kuic::congestion::slow_start_low_window && this->hystart_found;
}

void kuic::congestion::slow_start::on_packet_sent(kuic::packet_number_t packet_number) {
    this->last_sent_packet_number = packet_number;
}

void kuic::congestion::slow_start::on_packet_acked(kuic::packet_number_t acked_packet_number) {
    if (this->is_end_of_round(acked_packet_number)) {
        this->is_started = false;
    }
}

bool kuic::congestion::slow_start::started() const {
    return this->is_started;
}

void kuic::congestion::slow_start::restart() {
    this->is_started = false;
    this->hystart_found = false;
}
