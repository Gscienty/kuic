#include "frame/reset.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"

std::basic_string<kuic::byte_t> kuic::frame::reset::write_public_reset(
        kuic::connection_id conn_id, kuic::packet_number_t rejected_packet_number, unsigned long nonce_proof) {
    std::basic_string<kuic::byte_t> result;
    result.push_back(0x0A);
    result.append(conn_id.bytes(), conn_id.bytes() + 8);

    kuic::handshake::handshake_message msg(kuic::handshake::tag_public_reset);

    msg.insert(kuic::handshake::tag_rejected_packet_number, rejected_packet_number);
    msg.insert(kuic::handshake::tag_public_reset_nonce, nonce_proof);

    result.append(msg.serialize());

    return result; 
}

kuic::frame::reset kuic::frame::reset::parse_public_reset(std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::frame::reset pr;
    seek += 9; // ignore type flag & connection id

    kuic::handshake::handshake_message msg = kuic::handshake::handshake_message::deserialize(buffer, seek);

    if (msg.get_tag() != kuic::handshake::tag_public_reset_nonce) {
        return kuic::frame::reset(kuic::not_expect);
    }

    msg.assign<kuic::packet_number_t>(pr.rejected_packet_number, kuic::handshake::tag_rejected_packet_number);
    msg.assign<unsigned long>(pr.nonce, kuic::handshake::tag_public_reset_nonce);

    return pr;
}
