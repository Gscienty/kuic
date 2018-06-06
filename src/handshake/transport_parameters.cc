#include "handshake/transport_parameters.h"

kuic::bytes_count_t &
kuic::handshake::transport_parameters::get_stream_flow_control_window() {
    return this->stream_flow_control_window;
}

kuic::bytes_count_t &
kuic::handshake::transport_parameters::get_connection_flow_control_window() {
    return this->connection_flow_control_window;
}

kuic::bytes_count_t &
kuic::handshake::transport_parameters::get_max_packet_size() {
    return this->max_packet_size;
}

unsigned short &
kuic::handshake::transport_parameters::get_max_unicast_streams() {
    return this->max_unicast_streams;
}

kuic::kuic_time_t &
kuic::handshake::transport_parameters::get_idle_timeout() {
    return this->idle_timeout;
}
