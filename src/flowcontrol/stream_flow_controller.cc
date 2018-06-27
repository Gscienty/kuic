#include "flowcontrol/stream_flow_controller.h"
#include <utility>

kuic::flowcontrol::stream_flow_controller::stream_flow_controller(
    kuic::stream_id_t stream_id,
    bool contributes_to_connection,
    connection_flow_controller &conn_ctrl,
    kuic::bytes_count_t receive_window,
    kuic::bytes_count_t max_receive_window,
    kuic::bytes_count_t initial_send_window,
    std::function<void (kuic::stream_id_t)> &&queue_window_update,
    kuic::congestion::rtt &rtt)
        : kuic::flowcontrol::base_flow_controller(
            rtt,
            receive_window,
            max_receive_window,
            initial_send_window)
        , stream_id(stream_id)
        , conn_ctrl(conn_ctrl)
        , contributes_to_connection(contributes_to_connection)
        , inner_queue_window_update(std::move(queue_window_update))
        , queue_window_update([this] () -> void {
                    this->inner_queue_window_update(this->stream_id);
                }) { }

kuic::error_t
kuic::flowcontrol::stream_flow_controller::update_highest_received(
    kuic::bytes_count_t byte_offset, bool is_final) {
    
    kuic::writer_lock_guard locker(this->rw_m);

    if (is_final && this->received_final_offset && byte_offset != this->highest_received) {
        return kuic::stream_data_after_termination;
    }
    if (this->received_final_offset && byte_offset > this->highest_received) {
        return kuic::stream_data_after_termination;
    }
    if (is_final) {
        this->received_final_offset = true;
    }
    if (byte_offset == this->highest_received) {
        return kuic::no_error;
    }
    if (byte_offset <= this->highest_received) {
        if (is_final) {
            return kuic::stream_data_after_termination;
        }
        return kuic::no_error;
    }

    kuic::bytes_count_t increment = byte_offset - this->highest_received;
    this->highest_received = byte_offset;
    if (this->check_flow_control_violation()) {
        return kuic::flow_control_received_too_much_data;
    }
    if (this->contributes_to_connection) {
        return this->conn_ctrl.increment_highest_received(increment);
    }
    return kuic::no_error;
}

void kuic::flowcontrol::stream_flow_controller::add_bytes_sent(kuic::bytes_count_t n) {
    this->kuic::flowcontrol::base_flow_controller::add_bytes_sent(n);
    if (this->contributes_to_connection) {
        this->conn_ctrl.add_bytes_sent(n);
    }
}

void kuic::flowcontrol::stream_flow_controller::add_bytes_read(kuic::bytes_count_t n) {
    this->kuic::flowcontrol::base_flow_controller::add_bytes_read(n);
    if (this->contributes_to_connection) {
        this->conn_ctrl.add_bytes_read(n);
    }
}

kuic::bytes_count_t
kuic::flowcontrol::stream_flow_controller::send_window_size() const {
    kuic::bytes_count_t window = this->kuic::flowcontrol::base_flow_controller::send_window_size();
    if (this->contributes_to_connection) {
        window = std::min<kuic::bytes_count_t>(window, this->conn_ctrl.send_window_size());
    }
    return window;
}

std::pair<bool, kuic::bytes_count_t>
kuic::flowcontrol::stream_flow_controller::is_blocked() const {
    if (this->send_window_size() != 0) {
        return std::pair<bool, kuic::bytes_count_t>(false, 0);
    }
    return std::pair<bool, kuic::bytes_count_t>(true, this->send_window);
}

bool kuic::flowcontrol::stream_flow_controller::has_window_update() {
    kuic::reader_lock_guard locker(this->rw_m);

    return this->received_final_offset == false &&
        this->kuic::flowcontrol::base_flow_controller::has_window_update();
}

kuic::bytes_count_t
kuic::flowcontrol::stream_flow_controller::get_window_update() {
    kuic::writer_lock_guard locker(this->rw_m);

    if (this->received_final_offset) {
        return 0;
    }
    
    kuic::bytes_count_t old_window_size = this->receive_window_size;
    kuic::bytes_count_t offset = this->kuic::flowcontrol::base_flow_controller::get_window_update();
    if (this->receive_window_size > old_window_size) {
        if (this->contributes_to_connection) {
            this->conn_ctrl.ensure_minimum_window_size(
                kuic::bytes_count_t(
                    double(this->receive_window_size) * kuic::connection_flow_control_multiplier));
        }
    }

    return offset;
}

void kuic::flowcontrol::stream_flow_controller::try_queue_window_update() {
    bool has_window_update = false;
    {
        kuic::reader_lock_guard lock(this->rw_m);
        has_window_update = this->received_final_offset == false && this->has_window_update();
    }
    if (has_window_update) {
        this->queue_window_update();
    }
    if (this->contributes_to_connection) {
        this->conn_ctrl.try_queue_window_update();
    }
}
