#include "packet_unpacker.h"
#include "frame/frame.h"

kuic::packet_unpacker::packet_unpacker(kuic::unpacked_aead &aead)
    : aead(aead) { }

std::unique_ptr<kuic::unpacked_packet>
kuic::packet_unpacker::unpack(
        std::basic_string<kuic::byte_t> &header_binary,
        kuic::frame::header &header,
        std::basic_string<kuic::byte_t> &data) {
    std::basic_string<kuic::byte_t> decrypted;
    bool is_handshake = false;
    if (header.get_is_long()) {
        decrypted = this->aead.open_handshake(data, header.get_packet_number(), header_binary);
        is_handshake = true;
    }
    else {
        decrypted = this->aead.open(data, header.get_packet_number(), header_binary);
        is_handshake = false;
    }

    std::unique_ptr<kuic::unpacked_packet> unpacked_packet(new kuic::unpacked_packet());
    unpacked_packet->get_frames() = this->parse_frames(decrypted);
    return unpacked_packet;
}

std::vector<std::shared_ptr<kuic::frame::frame>>
kuic::packet_unpacker::parse_frames(
        std::basic_string<kuic::byte_t> &decrypted) {
    std::vector<std::shared_ptr<kuic::frame::frame>> result; 

    size_t seek = 0;
    while (true) {
        std::shared_ptr<kuic::frame::frame> frame = kuic::frame::frame::parse_next_frame(decrypted, seek);
        if (bool(frame) == false) {
            break;
        }
        if (frame->is_lawful()) {
            result.push_back(frame);
        }
    }

    return result;
}
