#include "packed_packet.h"

kuic::ackhandler::packet
kuic::packed_packet::to_ack_handler_packet() {
    kuic::ackhandler::packet packet;
    packet.packet_number = this->header->get_packet_number();
    packet.packet_type = this->header->get_packet_type();
    packet.frames = this->frames;
    packet.length = this->raw.size();
    packet.is_handshake = this->is_handshake;
    packet.send_time = kuic::special_clock(kuic::current_clock());

    return packet;
}

std::shared_ptr<kuic::frame::header> &
kuic::packed_packet::get_header() {
    return this->header;
}

std::basic_string<kuic::byte_t>&
kuic::packed_packet::get_raw() {
    return this->raw;
}

std::vector<std::shared_ptr<kuic::frame::frame>> &
kuic::packed_packet::get_frames() {
    return this->frames;
}

bool &
kuic::packed_packet::get_is_handshake() {
    return this->is_handshake;
}

