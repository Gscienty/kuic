#include "congestion/cubic.h"
#include <algorithm>

kuic::congestion::cubic::cubic()
    : connections_count(kuic::congestion::default_connections_count) {

    this->reset();
}

void kuic::congestion::cubic::reset() {
    this->epoch = kuic::special_clock({ 0, 0 });

    this->last_max_congestion_window = 0;
    this->acked_bytes_count = 0;
    this->estimated_tcp_congestion_window = 0;
    this->origin_point_congestion_window = 0;
    this->time_to_origin_point = 0;
    this->last_target_congestion_window = 0;
}

float kuic::congestion::cubic::beta() const {
    return (float(this->connections_count) - 1 + kuic::congestion::cubic_beta) / float(this->connections_count);
}

float kuic::congestion::cubic::alpha() const {
    float beta = this->beta();
    return 3 * float(this->connections_count) * float(this->connections_count) * (1 - beta) / (1 + beta);
}

void kuic::congestion::cubic::on_application_limited() {
    this->epoch = kuic::special_clock({ 0, 0 });
}

float
kuic::congestion::cubic::beta_last_max() const {
   return (float(this->connections_count) - 1 + kuic::congestion::cubic_beta_last_max)  / float(this->connections_count);
}

kuic::bytes_count_t
kuic::congestion::cubic::congestion_window_after_packet_loss(kuic::bytes_count_t current_congestion_window) {
    if (current_congestion_window + kuic::default_tcp_mss < this->last_max_congestion_window) {
        this->last_max_congestion_window = 
            kuic::bytes_count_t(this->beta_last_max() * float(current_congestion_window));
    }
    else {
        this->last_max_congestion_window = current_congestion_window;
    }

    this->epoch = kuic::special_clock({ 0, 0 });
    return kuic::packet_number_t(float(current_congestion_window) * this->beta());
}

kuic::bytes_count_t
kuic::congestion::cubic::congestion_window_after_ack(
        kuic::bytes_count_t acked_bytes,
        kuic::bytes_count_t current_congestion_window,
        kuic::kuic_time_t delay_min,
        kuic::special_clock &event_time) {

    this->acked_bytes_count += acked_bytes;

    if (this->epoch.is_zero()) {
        this->epoch = event_time;
        this->acked_bytes_count = acked_bytes;
        this->estimated_tcp_congestion_window = current_congestion_window;
        if (this->last_max_congestion_window <= current_congestion_window) {
            this->time_to_origin_point = 0;
            this->origin_point_congestion_window = current_congestion_window;
        }
        else {
            this->time_to_origin_point = (unsigned int)(std::cbrt(double(
                            kuic::congestion::cubic_factor *
                            (this->last_max_congestion_window - current_congestion_window))));
            this->origin_point_congestion_window = this->last_max_congestion_window;
        }
    }

    kuic::kuic_time_t elapsed_time = (kuic::kuic_time_t((event_time + delay_min - this->epoch) / kuic::clock_microsecond) << 10) / (1000 * 1000);
    unsigned long offset = std::abs(this->time_to_origin_point - elapsed_time);

    kuic::bytes_count_t delta_congestion_window = kuic::bytes_count_t(
            kuic::congestion::cubic_congestion_window_scale * offset * offset * offset) *
        kuic::default_tcp_mss >> kuic::congestion::cubic_scale;
    kuic::bytes_count_t target_congestion_window = 0;
     
    if (elapsed_time > this->time_to_origin_point) {
        target_congestion_window = this->origin_point_congestion_window + delta_congestion_window;
    }
    else {
        target_congestion_window = this->origin_point_congestion_window - delta_congestion_window;
    }

    target_congestion_window = std::min(target_congestion_window, current_congestion_window + this->acked_bytes_count / 2);
    
    this->estimated_tcp_congestion_window += kuic::bytes_count_t(
            double(this->acked_bytes_count) *
            this->alpha() *
            double(kuic::default_tcp_mss) / double(this->estimated_tcp_congestion_window));
    this->acked_bytes_count = 0;
    this->last_target_congestion_window = target_congestion_window;
    
    return std::max(target_congestion_window, this->estimated_tcp_congestion_window);
}

void kuic::congestion::cubic::set_connections_count(size_t count) {
    this->connections_count = count;
}
