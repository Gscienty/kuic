#include "stream/streams_map.h"
#include <algorithm>

kuic::stream::streams_map::streams_map(
        kuic::stream::stream_sender &sender,
        std::function<kuic::flowcontrol::stream_flow_controller *(kuic::stream_id_t)> &new_flow_controller,
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
                        std::make_shared<kuic::flowcontrol::stream_flow_controller>(
                            this->new_flow_controller(stream_id)));
            })
    , out_uni(
            is_client ? 2 : 3,
            [this] (kuic::stream_id_t stream_id) -> kuic::stream::send_stream * {
                return new kuic::stream::send_stream(
                        stream_id,
                        this->sender,
                        std::make_shared<kuic::flowcontrol::stream_flow_controller>(
                            this->new_flow_controller(stream_id)));
            },
            [this] (kuic::frame::stream_id_blocked_frame &frame) -> void {
                this->sender.queue_control_frame(frame);
            }) { }
