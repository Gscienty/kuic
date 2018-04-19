#include "flowcontrol/base_flow_controller.h"

kuic::flowcontrol::base_flow_controller::base_flow_controller(
    kuic::congestion::rtt &rtt,
    kuic::bytes_count_t receive_window_size,
    kuic::bytes_count_t max_receive_window_size)
        : _rtt(rtt)
        , rw_m(false)
        , bytes_sent(0)
        , send_window(0)
        , bytes_read(0)
        , highest_received(0)
        , receive_window(receive_window_size)
        , receive_window_size(receive_window_size)
        , max_receive_window_size(max_receive_window_size) { }

void kuic::flowcontrol::base_flow_controller::add_bytes_sent(
    kuic::bytes_count_t n) {

    this->bytes_sent += n;
}

void kuic::flowcontrol::base_flow_controller::update_send_window(
    kuic::bytes_count_t offset) {
    
    if (offset > this->send_window) {
        this->send_window = offset;
    }
}

void kuic::flowcontrol::base_flow_controller::add_bytes_read(kuic::bytes_count_t n) {
    kuic::writer_lock_guard locker(this->rw_m);

    if (this->bytes_read == 0) {
        this->start_new_auto_tuning_epoch();
    }

    this->bytes_read += n;
}

bool kuic::flowcontrol::base_flow_controller::has_window_update() const {
    kuic::bytes_count_t bytes_remaining = this->receive_window - this->bytes_read;
    return bytes_remaining <= kuic::bytes_count_t(
        double(this->receive_window_size) * double(1 - kuic::window_update_threshold));
}

kuic::bytes_count_t
kuic::flowcontrol::base_flow_controller::get_window_update() {
    if (this->has_window_update() == false) {
        return 0;
    }

    this->try_adjust_window_size();
    this->receive_window = this->bytes_read + this->receive_window_size;
    return this->receive_window;
}

void kuic::flowcontrol::base_flow_controller::try_adjust_window_size() {
    kuic::bytes_count_t bytes_read_in_epoch = this->bytes_read - this->epoch_start_offset;
    if (bytes_read_in_epoch <= this->receive_window_size / 2) {
        return;
    }

    kuic::kuic_time_t rtt = this->_rtt.get_smoothed_rtt();
    if (rtt == 0) {
        return;
    }

    double fraction = double(bytes_read_in_epoch) / double(this->receive_window_size);
    if (kuic::current_clock().since(this->epoch_start_time) < kuic::kuic_time_t(4 * fraction *double(rtt))) {
        this->receive_window_size = std::min<kuic::bytes_count_t>(
            2 * this->receive_window_size, this->max_receive_window_size);
    }
    this->start_new_auto_tuning_epoch();
}

void kuic::flowcontrol::base_flow_controller::start_new_auto_tuning_epoch() {
    this->epoch_start_time = kuic::current_clock();
    this->epoch_start_offset = this->bytes_read;
}

bool kuic::flowcontrol::base_flow_controller::check_flow_control_violation() {
    return this->highest_received > this->receive_window;
}

kuic::bytes_count_t
kuic::flowcontrol::base_flow_controller::send_window_size() const {
    if (this->bytes_sent > this->send_window) {
        return 0;
    }
    return this->send_window - this->bytes_sent;
}