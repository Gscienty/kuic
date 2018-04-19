#include "congestion/cubic.h"
#include <algorithm>

kuic::congestion::cubic::cubic(kuic::clock &clock)
    : clock(clock) 
    , connections_count(kuic::congestion::default_connections_count) {

    this->reset();
}

void kuic::congestion::cubic::reset() {
    this->epoch = kuic::special_clock({ 0, 0 });
    this->app_limit_start_time = kuic::special_clock({ 0, 0 });
    this->last_update_time = kuic::special_clock({ 0, 0 });

    this->last_congestion_window = 0;
    this->last_max_congestion_window = 0;
    this->acked_packets_count = 0;
    this->estimated_tcp_congestion_window = 0;
    this->origin_point_congestion_window = 0;
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
    if (this->app_limit_start_time.is_zero()) {
        this->app_limit_start_time = kuic::special_clock(this->clock); 
    }
    else {
        this->epoch = kuic::special_clock({ 0, 0 });
    }
}

kuic::packet_number_t
kuic::congestion::cubic::congestion_window_after_packet_loss(
    kuic::packet_number_t current_congestion_window) {
    
    if (current_congestion_window < this->last_max_congestion_window) {
        this->last_max_congestion_window = kuic::packet_number_t(
            kuic::congestion::cubic_beta_last_max * float(current_congestion_window));
    }
    else {
        this->last_max_congestion_window = current_congestion_window;
    }

    this->epoch = kuic::special_clock({ 0, 0 });
    return kuic::packet_number_t(float(current_congestion_window) * this->beta());
}

kuic::packet_number_t
kuic::congestion::cubic::congestion_window_after_ack(
    kuic::packet_number_t current_congestion_window, kuic::kuic_time_t delay_min) {
    
    this->acked_packets_count++;
    kuic::special_clock current_time(this->clock);

    if (this->last_congestion_window == current_congestion_window &&
        current_time - this->last_update_time <= kuic::congestion::cubic_max_time_interval) {
        return std::max<kuic::packet_number_t>(this->last_target_congestion_window, this->estimated_tcp_congestion_window);
    }

    this->last_congestion_window = current_congestion_window;
    this->last_update_time = current_time;

    if (this->epoch.is_zero()) {
        this->epoch = current_time;
        this->acked_packets_count = 1;

        this->estimated_tcp_congestion_window = current_congestion_window;
        if (this->last_max_congestion_window <= current_congestion_window) {
            this->time_to_origin_point = 0;
            this->origin_point_congestion_window = current_congestion_window;
        }
        else {
            this->time_to_origin_point = kuic::packet_number_t(
                cbrt(float(kuic::congestion::cubic_factor * this->last_max_congestion_window - current_congestion_window)));
            this->origin_point_congestion_window = this->last_max_congestion_window;
        }
    }
    else if (this->app_limit_start_time.is_zero() == false) {
        kuic::kuic_time_t shift = current_time - this->app_limit_start_time;
        this->epoch += shift;
        this->app_limit_start_time = kuic::special_clock({ 0, 0 });
    }

    kuic::kuic_time_t elapsed_time = kuic::kuic_time_t(
        ((current_time + delay_min - this->epoch) / kuic::clock_microsecond) << 10) / kuic::clock_millisecond;
    kuic::kuic_time_t offset = std::abs<kuic::kuic_time_t>(
        kuic::kuic_time_t(this->time_to_origin_point) - elapsed_time);

    kuic::packet_number_t delta_congestion_window = kuic::packet_number_t(
        (kuic::congestion::cubic_congestion_window_scale * offset * offset * offset) >> kuic::congestion::cubic_scale);
    kuic::packet_number_t target_congestion_window;
    if (elapsed_time > kuic::kuic_time_t(this->time_to_origin_point)) {
        target_congestion_window = this->origin_point_congestion_window + delta_congestion_window;
    }
    else {
        target_congestion_window = this->origin_point_congestion_window -
        delta_congestion_window;
    }

    while (true) {
        kuic::packet_number_t required_ack_count = kuic::packet_number_t(
            float(this->estimated_tcp_congestion_window) / this->alpha());
        if (this->acked_packets_count < required_ack_count) {
            break;
        }
        this->acked_packets_count -= required_ack_count;
        this->estimated_tcp_congestion_window++;
    }

    this->last_target_congestion_window = target_congestion_window;

    return std::max<kuic::packet_number_t>(target_congestion_window, this->estimated_tcp_congestion_window);
}

void kuic::congestion::cubic::set_connections_count(size_t count) {
    this->connections_count = count;
}