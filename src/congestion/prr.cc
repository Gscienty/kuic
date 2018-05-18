#include "congestion/prr.h"
#include "define.h"
#include <limits>

void kuic::congestion::prr::initialize() {
    this->bytes_sent_since_loss = 0;
    this->bytes_in_flight_before_loss = 0;
    this->bytes_delivered_since_loss = 0;
    this->ack_count_since_loss = 0;
}

void kuic::congestion::prr::on_packet_sent(kuic::bytes_count_t sent_bytes) {
    this->bytes_sent_since_loss += sent_bytes;
}

void kuic::congestion::prr::on_packet_lost(kuic::bytes_count_t bytes_in_flight) {
    this->bytes_sent_since_loss = 0;
    this->bytes_in_flight_before_loss = bytes_in_flight;
    this->bytes_delivered_since_loss = 0;
    this->ack_count_since_loss = 0;
}

void kuic::congestion::prr::on_packet_acked(kuic::bytes_count_t acked_bytes) {
    this->bytes_delivered_since_loss += acked_bytes;
    this->ack_count_since_loss++;
}

bool kuic::congestion::prr::can_send(
    kuic::bytes_count_t congestion_window,
    kuic::bytes_count_t bytes_in_flight,
    kuic::bytes_count_t slowstart_threshold) {
    
    if (this->bytes_delivered_since_loss == 0 || bytes_in_flight < kuic::default_tcp_mss) {
        return true;
    }
    if (congestion_window > bytes_in_flight) {
        return this->bytes_delivered_since_loss + this->ack_count_since_loss * kuic::default_tcp_mss > this->bytes_sent_since_loss;
    }

    return this->bytes_delivered_since_loss * slowstart_threshold > this->bytes_sent_since_loss * this->bytes_in_flight_before_loss;
}
