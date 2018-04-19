#include "congestion/cubic_sender.h"
#include <algorithm>

kuic::congestion::cubic_sender::cubic_sender(
    kuic::clock &clock,
    rtt &rtt_stat,
    kuic::packet_number_t initial_congestion_window,
    kuic::packet_number_t initial_max_congestion_window)
        : _rtt(rtt_stat)
        , initial_congestion_window(initial_congestion_window)
        , initial_max_congestion_window(initial_max_congestion_window)
        , congestion_window(initial_congestion_window)
        , min_congestion_window(kuic::congestion::default_minimum_congestion_window)
        , _slowstart_threshold(initial_max_congestion_window)
        , max_tcp_congestion_window(initial_max_congestion_window)
        , connections_count(kuic::congestion::default_connections_count)
        , _cubic(clock) { }

kuic::kuic_time_t
kuic::congestion::cubic_sender::time_until_send(kuic::packet_number_t bytes_in_flight) {
    if (this->in_recovery() && 
        this->_prr.time_until_send(
            this->get_congestion_window(), bytes_in_flight, this->get_slowstart_threshold()) == 0) {
    
        return 0;
    }

    kuic::kuic_time_t delay = this->_rtt.get_smoothed_rtt() / 
        kuic::kuic_time_t(2 * this->get_congestion_window() / kuic::default_tcp_mss);
    
    if (this->in_slowstart() == false) {
        delay = delay * 8 / 5;
    }
    return delay;
}

bool kuic::congestion::cubic_sender::on_packet_sent(
    kuic::clock &clock,
    kuic::bytes_count_t bytes_in_flight,
    kuic::packet_number_t packet_number,
    kuic::bytes_count_t bytes,
    bool is_retransmittable) {
    
    if (is_retransmittable == false) {
        return false;
    }

    if (this->in_recovery()) {
        this->_prr.on_packet_sent(packet_number);
    }
    this->largest_sent_packet_number = packet_number;
    this->slowstart.on_packet_sent(packet_number);
    
    return true;
}

bool kuic::congestion::cubic_sender::in_recovery() {
    return this->largest_acked_packet_number <= this->largest_sent_at_last_cutback &&
        this->largest_acked_packet_number != 0;
}

bool kuic::congestion::cubic_sender::in_slowstart() {
    return this->get_congestion_window() < this->get_slowstart_threshold();
}

kuic::bytes_count_t
kuic::congestion::cubic_sender::get_congestion_window() {
    return kuic::bytes_count_t(this->congestion_window) * kuic::default_tcp_mss;
}

kuic::bytes_count_t
kuic::congestion::cubic_sender::get_slowstart_threshold() {
    return kuic::bytes_count_t(this->_slowstart_threshold) * kuic::default_tcp_mss;
}

kuic::packet_number_t
kuic::congestion::cubic_sender::slowstart_threshold() {
    return this->_slowstart_threshold;
}

void kuic::congestion::cubic_sender::exit_slowstart() {
    this->_slowstart_threshold = this->congestion_window;
}

void kuic::congestion::cubic_sender::try_exit_slowstart() {
    if (this->in_slowstart() &&
        this->slowstart.should_exist_slow_start(
            this->_rtt.get_latest_rtt(),
            this->_rtt.get_min_rtt(),
            this->get_congestion_window() / kuic::default_tcp_mss)) {
        
        this->exit_slowstart();
    }
}

void kuic::congestion::cubic_sender::on_packet_acked(
    kuic::packet_number_t acked_packet_number,
    kuic::bytes_count_t acked_bytes,
    kuic::bytes_count_t bytes_in_flight) {
    
    this->largest_acked_packet_number = std::max<kuic::packet_number_t>(
        this->largest_acked_packet_number, acked_packet_number);
    
    if (this->in_recovery()) {
        this->_prr.on_packet_acked(acked_bytes);
        return;
    }

    this->try_increase_cwnd(acked_packet_number, acked_bytes, bytes_in_flight);

    if (this->in_slowstart()) {
        this->slowstart.on_packet_acked(acked_packet_number);
    }
}

void kuic::congestion::cubic_sender::on_packet_lost(
    kuic::packet_number_t packet_number,
    kuic::bytes_count_t lost_bytes,
    kuic::bytes_count_t bytes_in_flight) {
    
    if (packet_number <= this->largest_sent_at_last_cutback) {
        if (this->last_cutback_exited_slowstart) {
            this->stats.slowstart_packets_lost++;
            this->stats.slowstart_bytes_lost += lost_bytes;
            if (this->slowstart_large_reduction) {
                if (this->stats.slowstart_packets_lost == 1 ||
                    (this->stats.slowstart_bytes_lost / kuic::default_tcp_mss) >
                        ((this->stats.slowstart_bytes_lost - lost_bytes) / kuic::default_tcp_mss)) {
                    
                    this->congestion_window = std::max<kuic::packet_number_t>(
                        this->congestion_window - 1, this->min_congestion_window);
                }
                this->_slowstart_threshold = this->congestion_window;
            }
        }
        return;
    }
    this->last_cutback_exited_slowstart = this->in_slowstart();
    if (this->in_slowstart()) {
        this->stats.slowstart_packets_lost++;
    }

    this->_prr.on_packet_lost(bytes_in_flight);

    if (this->slowstart_large_reduction && this->in_slowstart()) {
        this->congestion_window--;
    }
    else {
        this->congestion_window = this->_cubic.congestion_window_after_packet_loss(this->congestion_window);
    }

    this->congestion_window = std::max<kuic::packet_number_t>(
        this->congestion_window, this->min_congestion_window);
    
    this->_slowstart_threshold = this->congestion_window;
    this->largest_sent_at_last_cutback = this->largest_sent_packet_number;
    this->congestion_window_count = 0;
}

void kuic::congestion::cubic_sender::try_increase_cwnd(
    kuic::packet_number_t acked_packet_number,
    kuic::bytes_count_t acked_bytes,
    kuic::bytes_count_t bytes_in_flight) {
    
    if (this->is_cwnd_limited(bytes_in_flight) == false) {
        this->_cubic.on_application_limited();
        return;
    }

    if (this->congestion_window >= this->max_tcp_congestion_window) {
        return;
    }

    if (this->in_slowstart()) {
        this->congestion_window++;
        return;
    }

    this->congestion_window = std::min<kuic::packet_number_t>(
        this->max_tcp_congestion_window,
        this->_cubic.congestion_window_after_ack(
            this->congestion_window, this->_rtt.get_min_rtt()));
}

bool kuic::congestion::cubic_sender::is_cwnd_limited(kuic::bytes_count_t bytes_in_flight) {
    kuic::bytes_count_t congestion_window = this->get_congestion_window();
    if (bytes_in_flight >= congestion_window) {
        return true;
    }

    kuic::bytes_count_t available_bytes = congestion_window - bytes_in_flight;
    bool slowstart_limited = this->in_slowstart() && bytes_in_flight > congestion_window / 2;

    return slowstart_limited || available_bytes <= kuic::congestion::max_burst_bytes;
}

kuic::band_width_t
kuic::congestion::cubic_sender::bandwidth_estimate() {
    kuic::kuic_time_t srtt = this->_rtt.get_smoothed_rtt();
    if (srtt == 0) {
        return 0;
    }
    return kuic::__inl_bandwidth_from_delta(
        this->get_congestion_window(), srtt);
}

kuic::congestion::slow_start &
kuic::congestion::cubic_sender::get_slowstart() {
    return this->slowstart;
}

void kuic::congestion::cubic_sender::set_connections_count(size_t n) {
    this->connections_count = std::max<size_t>(1, n);
    this->_cubic.set_connections_count(this->connections_count);
}

void kuic::congestion::cubic_sender::on_retransmission_timeout(bool packets_retransmitted) {
    this->largest_sent_at_last_cutback = 0;
    if (packets_retransmitted == false) {
        return;
    }
    this->slowstart.restart();
    this->_cubic.reset();
    this->_slowstart_threshold = this->congestion_window / 2;
    this->congestion_window = this->min_congestion_window;
}

void kuic::congestion::cubic_sender::on_connection_migration() {
    this->slowstart.restart();
    this->_prr = kuic::congestion::prr();
    this->largest_sent_packet_number = 0;
    this->largest_acked_packet_number = 0;
    this->largest_sent_at_last_cutback = 0;
    this->last_cutback_exited_slowstart = false;
    this->_cubic.reset();
    this->congestion_window_count = 0;
    this->congestion_window = this->initial_congestion_window;
    this->_slowstart_threshold = this->initial_max_congestion_window;
    this->max_tcp_congestion_window = this->initial_max_congestion_window;
}

void kuic::congestion::cubic_sender::set_slowstart_large_reduction(bool enabled) {
    this->slowstart_large_reduction = enabled;
}