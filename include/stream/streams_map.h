#ifndef _KUIC_STREAM_STREAMS_MAP_
#define _KUIC_STREAM_STREAMS_MAP_

#include "stream/stream_sender.h"
#include "stream/in_unicast_stream.h"
#include "stream/out_unicast_stream.h"
#include "flowcontrol/stream_flow_controller.h"
#include <functional>

namespace kuic {
    namespace stream {
        class streams_map {
        private:
            bool is_client;
            std::function<kuic::flowcontrol::stream_flow_controller *(kuic::stream_id_t)> &new_flow_controller;
            stream_sender &sender;

            in_unicast_stream in_uni;
            out_unicast_stream out_uni;
        public:
            streams_map(
                    stream_sender &sender,
                    std::function<kuic::flowcontrol::stream_flow_controller *(kuic::stream_id_t)> &new_flow_controller,
                    int max_incoming_streams,
                    int max_incoming_uni_streams,
                    bool is_client);
        };
    }
}

#endif

