#include "congestion/rtt.h"
#include "clock.h"
#include <limits>
#include <algorithm>

kuic::congestion::rtt::rtt()
    : min_rtt(0L)
    , latest_rtt(0L)
    , smoothed_rtt(0L)
    , mean_deviation(0L) { }

kuic::kuic_time_t kuic::congestion::rtt::get_min_rtt() const {
    return this->min_rtt;
}

kuic::kuic_time_t kuic::congestion::rtt::get_latest_rtt() const {
    return this->latest_rtt;
}

kuic::kuic_time_t kuic::congestion::rtt::get_smoothed_rtt() const {
    return this->smoothed_rtt;
}

kuic::kuic_time_t kuic::congestion::rtt::get_mean_deviation() const {
    return this->mean_deviation;
}

kuic::kuic_time_t kuic::congestion::rtt::get_smoothed_or_initial_rtt() const {
    if (this->smoothed_rtt != 0) {
        return this->smoothed_rtt;
    }

    return kuic::congestion::default_initial_rtt;
}

void kuic::congestion::rtt::update_rtt(kuic::kuic_time_t send_delta, kuic::kuic_time_t ack_delay) {
    if (send_delta == std::numeric_limits<kuic::kuic_time_t>::max() || send_delta <= 0) {
        return;
    }

    if (this->min_rtt == 0 || this->min_rtt > send_delta) {
        this->min_rtt = send_delta;
    }

    kuic::kuic_time_t sample = send_delta;

    if (sample - this->min_rtt >= ack_delay) {
        sample -= ack_delay;
    }
    this->latest_rtt = sample;

    if (this->smoothed_rtt == 0) {
        this->smoothed_rtt = sample;
        this->mean_deviation = sample / 2;
    }
    else {
        this->mean_deviation = kuic::kuic_time_t(
                (1 - kuic::congestion::rtt_beta) * float(this->mean_deviation / kuic::clock_microsecond) +
                kuic::congestion::rtt_beta * float(std::abs(this->smoothed_rtt - sample) / kuic::clock_microsecond)) *
            kuic::clock_microsecond;
        this->smoothed_rtt = kuic::kuic_time_t(
                (1 - kuic::congestion::rtt_alpha) * float(this->smoothed_rtt / kuic::clock_microsecond) +
                kuic::congestion::rtt_alpha * float(sample / kuic::clock_microsecond)) *
            kuic::clock_microsecond;
    }
}

void kuic::congestion::rtt::on_connection_migration() {
    this->latest_rtt = 0;
    this->min_rtt = 0;
    this->smoothed_rtt = 0;
    this->mean_deviation = 0;
}

void kuic::congestion::rtt::expire_smoothed_metrics() {
    this->mean_deviation = std::max<kuic::kuic_time_t>(
        this->mean_deviation, std::abs<kuic::kuic_time_t>(this->smoothed_rtt - this->latest_rtt));
    this->smoothed_rtt = std::max<kuic::kuic_time_t>(this->smoothed_rtt, this->latest_rtt);
}
