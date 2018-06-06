#ifndef _KUIC_HANDSHAKE_TRANSPORT_PARAMETERS_
#define _KUIC_HANDSHAKE_TRANSPORT_PARAMETERS_

#include "type.h"

namespace kuic {
    namespace handshake {
        class transport_parameters {
        private:
            kuic::bytes_count_t stream_flow_control_window;
            kuic::bytes_count_t connection_flow_control_window;
            kuic::bytes_count_t max_packet_size;
            unsigned short max_unicast_streams;
            kuic::kuic_time_t idle_timeout;
        public:
            kuic::bytes_count_t &get_stream_flow_control_window();
            kuic::bytes_count_t &get_connection_flow_control_window();
            kuic::bytes_count_t &get_max_packet_size();
            unsigned short &get_max_unicast_streams();
            kuic::kuic_time_t &get_idle_timeout();

        };
    }
}

#endif

