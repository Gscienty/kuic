#include "flowcontrol/connection_flow_controller.h"

kuic::flowcontrol::connection_flow_controller::connection_flow_controller(
    kuic::bytes_count_t receive_window,
    kuic::bytes_count_t max_receive_window,
    kuic::congestion::rtt &rtt,
    std::function<void ()> &queue_window_update)
        : kuic::flowcontrol::base_flow_controller(
            rtt,
            receive_window,
            max_receive_window,
            0)
        , last_blocked_at(0) 
        , queue_window_update(queue_window_update) { }

kuic::bytes_count_t
kuic::flowcontrol::connection_flow_controller::send_window_size() const {
    return this->kuic::flowcontrol::base_flow_controller::send_window_size();
}

std::pair<bool, kuic::bytes_count_t>
kuic::flowcontrol::connection_flow_controller::is_newly_blocked() {
    if (this->send_window_size() != 0 || this->send_window == this->last_blocked_at) {
        return std::pair<bool, kuic::bytes_count_t>(false, 0);
    }

    this->last_blocked_at = this->send_window;
    return std::pair<bool, kuic::bytes_count_t>(true, this->send_window);
}

kuic::error_t
kuic::flowcontrol::connection_flow_controller::increment_highest_received(
    kuic::bytes_count_t increment) {
    
    kuic::writer_lock_guard locker(this->rw_m);

    this->highest_received += increment;

    if (this->kuic::flowcontrol::base_flow_controller::check_flow_control_violation()) {
        return kuic::flow_control_received_too_much_data;
    }
    return kuic::no_error;
}

kuic::bytes_count_t
kuic::flowcontrol::connection_flow_controller::get_window_update() {
    kuic::writer_lock_guard locker(this->rw_m);

    kuic::bytes_count_t old_window_size = this->receive_window_size;
    kuic::bytes_count_t offset = this->kuic::flowcontrol::base_flow_controller::get_window_update();
    if (old_window_size < this->receive_window_size) { }

    return offset;
}


void kuic::flowcontrol::connection_flow_controller::ensure_minimum_window_size(
    kuic::bytes_count_t inc) {
    kuic::writer_lock_guard locker(this->rw_m);

    if (inc > this->receive_window_size) {
        this->receive_window_size = std::min<kuic::bytes_count_t>(
            inc, this->max_receive_window_size);
        this->start_new_auto_tuning_epoch();
    }
}

void kuic::flowcontrol::connection_flow_controller::try_queue_window_update() {
    bool has_window_update = false;
    {
        kuic::reader_lock_guard lock(this->rw_m);
        has_window_update = this->has_window_update();
    }
    if (has_window_update) {
        this->queue_window_update();
    }
}
