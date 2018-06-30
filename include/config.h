#ifndef _KUIC_CONFIG_
#define _KUIC_CONFIG_

#include "type.h"
#include <functional>

namespace kuic {
    struct config {
        bool request_connection_ido_mission;
        kuic::kuic_time_t handshake_timeout;
        kuic::kuic_time_t idle_timeout;
        unsigned long max_receive_stream_flow_control_window;
        unsigned long max_receive_connection_flow_control_window;
        int max_incoming_streams;
        int max_incoming_unicast_streams;
        bool keep_alive;
    };
}

#endif

