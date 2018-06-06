#include "stream/streams_map.h"
#include "define.h"
#include <algorithm>

kuic::stream::streams_map::streams_map(
        kuic::stream::stream_sender &sender,
        std::function<kuic::flowcontrol::stream_flow_controller *(kuic::stream_id_t)> new_flow_controller,
        int max_incoming_streams,
        int max_incoming_uni_streams,
        bool is_client)
    : is_client(is_client)
    , new_flow_controller(new_flow_controller)
    , sender(sender)
    , in_uni(
            is_client ? 3 : 2,
            4 * (max_incoming_uni_streams - 1) + (is_client ? 3 : 2),
            max_incoming_streams,
            [this] (kuic::frame::frame &frame) -> void {
                this->sender.queue_control_frame(frame);
            },
            [this] (kuic::stream_id_t stream_id) -> kuic::stream::receive_stream * {
                return new kuic::stream::receive_stream(
                        stream_id,
                        this->sender,
                        std::shared_ptr<kuic::flowcontrol::stream_flow_controller>(
                            this->new_flow_controller(stream_id)));
            })
    , out_uni(
            is_client ? 2 : 3,
            [this] (kuic::frame::stream_id_blocked_frame &frame) -> void {
                this->sender.queue_control_frame(frame);
            },
            [this] (kuic::stream_id_t stream_id) -> kuic::stream::send_stream * {
                return new kuic::stream::send_stream(
                        stream_id,
                        this->sender,
                        std::shared_ptr<kuic::flowcontrol::stream_flow_controller>(
                            this->new_flow_controller(stream_id)));
            }) { }

kuic::stream_type_t
kuic::stream::streams_map::get_stream_type(const kuic::stream_id_t stream_id) const {
    if (this->is_client) {
        switch (stream_id % 4) {
        case 2:
            return kuic::stream_type_out_unicast;
        case 3:
            return kuic::stream_type_in_unicast;
        }
    }
    else {
        switch (stream_id % 4) {
        case 2:
            return kuic::stream_type_in_unicast;
        case 3:
            return kuic::stream_type_in_unicast;
        }
    }
    return kuic::stream_type_unknow;
}

kuic::nullable<kuic::stream::receive_stream>
kuic::stream::streams_map::accept_unicast_stream() {
    return this->in_uni.accept_stream();
}

kuic::nullable<kuic::stream::send_stream>
kuic::stream::streams_map::open_unicast_stream() {
    return this->out_uni.open_stream();
}

bool kuic::stream::streams_map::delete_stream(kuic::stream_id_t stream_id) {
    switch (this->get_stream_type(stream_id)) {
    case kuic::stream_type_in_unicast:
        return this->in_uni.delete_stream(stream_id);
    case kuic::stream_type_out_unicast:
        return this->out_uni.delete_stream(stream_id);
    }

    return false;
}

kuic::nullable<kuic::stream::receive_stream>
kuic::stream::streams_map::get_or_open_receive_stream(const kuic::stream_id_t stream_id) {
    switch (this->get_stream_type(stream_id)) {
    case kuic::stream_type_in_unicast:
        return this->in_uni.get_or_open_stream(stream_id);
    }
    return kuic::nullable<kuic::stream::receive_stream>(nullptr);
}

kuic::nullable<kuic::stream::send_stream>
kuic::stream::streams_map::get_or_open_send_stream(const kuic::stream_id_t stream_id) {
    switch (this->get_stream_type(stream_id)) {
    case kuic::stream_type_out_unicast:
        return this->out_uni.get_stream(stream_id);
    }

    return kuic::nullable<kuic::stream::send_stream>(nullptr);
}

bool kuic::stream::streams_map::handle_max_stream_id_frame(kuic::frame::max_stream_id_frame &frame) {
    switch (this->get_stream_type(frame.get_stream_id())) {
    case kuic::stream_type_out_unicast:
        this->out_uni.set_max_stream(frame.get_stream_id());
        return true;
    }
    return false;
}

void kuic::stream::streams_map::update_limit(kuic::handshake::transport_parameters &parameters) {
    bool peer_is_client = !this->is_client;

    this->out_uni.set_max_stream(
            parameters.get_max_unicast_streams() == 0
            ? 0
            : (peer_is_client ? 3 : 2) + 4 * (parameters.get_max_unicast_streams() - 1));
}

void kuic::stream::streams_map::close_with_error(kuic::error_t error) {
    this->in_uni.close_with_error(error);
    this->out_uni.close_with_error(error);
}
