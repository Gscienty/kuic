#ifndef _KUIC_PACKED_PACKET_
#define _KUIC_PACKED_PACKET_

#include "frame/header.h"
#include "frame/frame.h"
#include "ackhandler/packet.h"
#include "type.h"
#include <string>
#include <memory>

namespace kuic {
    class packed_packet {
    private:
        std::shared_ptr<kuic::frame::header> header;
        std::string raw;
        std::vector<std::shared_ptr<kuic::frame::frame>> frames;
        bool is_handshake;
    public:
        std::shared_ptr<kuic::frame::header> &get_header();
        std::string &get_raw();
        std::vector<std::shared_ptr<kuic::frame::frame>> &get_frames();
        bool &get_is_handshake();
        kuic::ackhandler::packet to_ack_handler_packet();
    };
}

#endif

