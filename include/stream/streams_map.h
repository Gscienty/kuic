#ifndef _KUIC_STREAM_STREAMS_MAP_
#define _KUIC_STREAM_STREAMS_MAP_

#include "stream/stream_sender.h"
#include "stream/in_unicast_stream.h"
#include "stream/out_unicast_stream.h"
#include "stream/receive_stream.h"
#include "stream/send_stream.h"
#include "flowcontrol/stream_flow_controller.h"
#include "handshake/transport_parameters.h"
#include "type.h"
#include <functional>

namespace kuic {
    namespace stream {
        class streams_map {
        private:
            bool is_client;
            std::function<kuic::flowcontrol::stream_flow_controller *(kuic::stream_id_t)> new_flow_controller;
            stream_sender &sender;

            in_unicast_stream in_uni;
            out_unicast_stream out_uni;
        public:
            streams_map(
                    stream_sender &sender,
                    std::function<kuic::flowcontrol::stream_flow_controller *(kuic::stream_id_t)> new_flow_controller,
                    int max_incoming_streams,
                    int max_incoming_uni_streams,
                    bool is_client);

            kuic::stream_type_t get_stream_type(const kuic::stream_id_t stream_id) const;

            std::shared_ptr<receive_stream> accept_unicast_stream(); 
            std::shared_ptr<send_stream> open_unicast_stream();
            bool delete_stream(kuic::stream_id_t stream_id);
            std::shared_ptr<receive_stream> get_or_open_receive_stream(const kuic::stream_id_t stream_id);
            std::shared_ptr<send_stream> get_or_open_send_stream(const kuic::stream_id_t stream_id);
            bool handle_max_stream_id_frame(kuic::frame::max_stream_id_frame &frame);
            void update_limit(kuic::handshake::transport_parameters &parameters);
            void close_with_error(kuic::error_t error);
        };
    }
}

#endif

