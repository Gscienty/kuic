#include "packed_packet.h"

kuic::ackhandler::packet
kuic::packed_packet::to_ack_handler_packet() {
    kuic::ackhandler::packet packet;
    packet.packet_number = this->header.get_packet_number();
    packet.packet_type = this->header.get_packet_type();
    packet.frames = this->frames;
    packet.length = this->raw.size();
    packet.is_handshake = this->is_handshake;
    packet.send_time = kuic::special_clock(kuic::current_clock());

    return packet;
}


