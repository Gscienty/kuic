#include "window_update_queue.h"
#include "frame/max_stream_data_frame.h"
#include <algorithm>

kuic::window_update_queue::window_update_queue(
        kuic::stream::stream_getter &stream_getter,
        kuic::stream::crypto_stream &crypto_stream,
        kuic::flowcontrol::connection_flow_controller &conn_flow_controller,
        std::function<void (std::shared_ptr<kuic::frame::frame>)> callback)
    : queued_conn(false)
    , crypto_stream(crypto_stream)
    , stream_getter(stream_getter)
    , conn_flow_controller(conn_flow_controller)
    , callback(callback) { }

void kuic::window_update_queue::add_stream(kuic::stream_id_t stream_id) {
    std::lock_guard<std::mutex> lock(this->mutex);
    this->queue.insert(stream_id);
}

void kuic::window_update_queue::add_connection() {
    std::lock_guard<std::mutex> lock(this->mutex);
    this->queued_conn = true;
}

void kuic::window_update_queue::queue_all() {
    std::lock_guard<std::mutex> lock(this->mutex);
    if (this->queued_conn) {
        kuic::frame::max_stream_data_frame frame;
        frame.get_byte_offset() = this->conn_flow_controller.get_window_update();
        this->callback(frame);
        this->queued_conn = false;
    }

    kuic::bytes_count_t offset = 0;
    
    for (auto id_itr = this->queue.begin(); id_itr != this->queue.end(); id_itr++) {
        if (*id_itr == this->crypto_stream.get_stream_id()) {
            offset = this->crypto_stream.get_window_update();
        }
        else {
            std::shared_ptr<kuic::stream::receive_stream> str = this->stream_getter.get_or_open_receive_stream(*id_itr);
            if (bool(str) == false) {
                continue;
            }
            offset = const_cast<kuic::stream::receive_stream *>(str.get())->get_window_update();
        }

        if (offset == 0) {
            continue;
        }
        
        kuic::frame::max_stream_data_frame frame;
        frame.get_stream_id() = *id_itr;
        frame.get_byte_offset() = offset;

        this->callback(frame);

        id_itr = this->queue.erase(id_itr);
    }
}
