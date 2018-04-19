#include "congestion/rtt.h"
#include <limits>
#include <algorithm>

kuic::congestion::rtt()
    : initial_rtt_us(0L)
    , recent_min_rtt_window(std::numeric_limits<kuic::kuic_time_t>::max())
    , min_rtt(0L)
    , latest_rtt(0L)
    , smoothed_rtt(0L)
    , mean_deviation(0L)
    , new_min_rtt({ 0, kuic::special_clock({ 0, 0 }) })
    , recent_min_rtt({ 0, kuic::special_clock({ 0, 0 }) })
    , half_window_rtt({ 0, kuic::special_clock({ 0, 0 }) })
    , quarter_window_rtt({ 0, kuic::special_clock({ 0, 0 }) }) { }

void kuic::congestion::rtt::update_recent_min_rtt(kuic::kuic_time_t sample, kuic::clock now) {
    if (this->min_rtt_samples_remaining_count > 0) {
        this->min_rtt_samples_remaining_count--;
        if (this->new_min_rtt.rtt == 0 || sample <= this->new_min_rtt.rtt) {
            this->new_min_rtt = { sample, now };
        }
        if (this->min_rtt_samples_remaining_count == 0) {
            this->recent_min_rtt = this->new_min_rtt;
            this->half_window_rtt = this->new_min_rtt;
            this->quarter_window_rtt = this->new_min_rtt;
        }
    }

    if (this->recent_min_rtt.rtt == 0 || sample <= this->recent_min_rtt.rtt) {
        this->recent_min_rtt = { sample, now };
        this->half_window_rtt = this->recent_min_rtt;
        this->quarter_window_rtt = this->recent_min_rtt;
    }
    else if (sample <= this->half_window_rtt.rtt) {
        this->half_window_rtt = { sample, now };
        this->quarter_window_rtt = this->half_window_rtt;
    }
    else if (sample <= this->quarter_window_rtt.rtt) {
        this->quarter_window_rtt = { sample, now };
    }

    if (this->recent_min_rtt.t.before(now + (-this->recent_min_rtt_window))) {
        this->recent_min_rtt = this->half_window_rtt;
        this->half_window_rtt = this->quarter_window_rtt;
        this->quarter_window_rtt = { sample, now };
    }
    else if (this->recent_min_rtt.t.before(now + (-kuic::kuic_time_t(float(this->recent_min_rtt_window / kuic::clock_microsecond) * kuic::congestion::rtt_half_window) * kuic::clock_microsecond))) {
        this->half_window_rtt = this->quarter_window_rtt;
        this->quarter_window_rtt = { sample, now };
    }
    else if (this->recent_min_rtt.t.before(now + (-kuic::kuic_time_t(float(this->recent_min_rtt_window / kuic::clock_microsecond) * kuic::congestion::rtt_quarter_window) * kuic::clock_microsecond))) {
        this->quarter_window_rtt = { sample, now };
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_initial_rtt_us() const {
        return this->initial_rtt_us;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_min_rtt() const {
        return this->min_rtt;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_latest_rtt() const {
        return this->latest_rtt;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_recent_min_rtt() const {
        return this->recent_min_rtt.rtt;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_smoothed_rtt() const {
        return this->smoothed_rtt;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_quarter_window_rtt() const {
        return this->quarter_window_rtt.rtt;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_half_window_rtt() const {
        return this->half_window_rtt.rtt;
    }

    kuic::kuic_time_t kuic::congestion::rtt::get_mean_deviation() const {
        return this->mean_deviation;
    }

    void kuic::congestion::rtt::set_recent_min_rtt_window(const kuic::kuic_time_t recent_min_rtt_window) {
        this->recent_min_rtt_window = recent_min_rtt_window;
    }

    void kuic::congestion::rtt::update_rtt(kuic::kuic_time_t send_delta, kuic::kuic_time_t ack_delay, kuic::clock now) {
        if (send_delta == std::numeric_limits<kuic::kuic_time_t>::max() || send_delta <= 0) {
            return 0;
        }

        if (this->min_rtt == 0 || this->min_rtt > send_delta) {
            this->min_rtt = send_delta;
        }

        this->update_recent_min_rtt(send_delta, now);

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
                kuic::congestion::rtt_beta * float(
                    std::abs<kuic::kuic_time_t>(this->smoothed_rtt - sample) / kuic::clock_microsecond)) * kuic::clock_microsecond;
            this->smoothed_rtt = kuic::kuic_time_t(
                (1 - kuic::congestion::rtt_alpha) * float(this->smoothed_rtt / kuic::clock_microsecond) + 
                kuic::congestion::rtt_alpha * float(sample / kuic::clock_microsecond)) * kuic::clock_microsecond;
            
        }
    }

    void kuic::congestion::rtt::sample_new_recent_min_rtt(unsigned int samples_count) {
        this->min_rtt_samples_remaining_count = samples_count;
        this->new_min_rtt = { 0, kuic::special_clock({ 0, 0 }) };
    }

    void kuic::congestion::rtt::on_connection_migration() {
        this->latest_rtt = 0;
        this->min_rtt = 0;
        this->smoothed_rtt = 0;
        this->mean_deviation = 0;
        this->initial_rtt_us = kuic::congestion::rtt_initial_rtt_us;
        this->min_rtt_samples_remaining_count = 0;
        this->recent_min_rtt_window = std::numeric_limits<kuic::kuic_time_t>::max();
        this->recent_min_rtt = { 0, kuic::special_clock({ 0, 0 }) };
        this->half_window_rtt = { 0, kuic::special_clock({ 0, 0 }) };
        this->quarter_window_rtt = { 0, kuic::special_clock({ 0, 0 }) };
    }

    void kuic::congestion::rtt::expire_smoothed_metrics() {
        this->mean_deviation = std::max<kuic::kuic_time_t>(
            this->mean_deviation, std::abs<kuic::kuic_time_t>(this->smoothed_rtt - this->latest_rtt));
        this->smoothed_rtt = std::max<kuic::kuic_time_t>(this->smoothed_rtt, this->latest_rtt);
    }    
}