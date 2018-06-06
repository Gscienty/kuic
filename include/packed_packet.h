#ifndef _KUIC_PACKED_PACKET_
#define _KUIC_PACKED_PACKET_

#include "frame/header.h"
#include "frame/frame.h"
#include "ackhandler/packet.h"
#include "type.h"
#include <vector>

namespace kuic {
    class packed_packet {
    private:
        kuic::frame::header header;
        std::vector<kuic::byte_t> raw;
        std::vector<std::shared_ptr<kuic::frame::frame>> frames;
        bool is_handshake;
    public:
        kuic::ackhandler::packet to_ack_handler_packet();
    };
}

#endif

