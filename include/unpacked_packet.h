#ifndef _KUIC_UNPACKED_PACKET_
#define _KUIC_UNPACKED_PACKET_

#include "frame/frame.h"
#include <vector>
#include <memory>

namespace kuic {
    class unpacked_packet {
    private:
        std::vector<std::shared_ptr<kuic::frame::frame>> frames;
    public:
        std::vector<std::shared_ptr<kuic::frame::frame>> &get_frames() {
            return this->frames;
        }
    };

    class unpacked_aead {
    public:
        std::basic_string<kuic::byte_t> open_handshake(
                std::basic_string<kuic::byte_t> &secret,
                kuic::packet_number_t &packet_number,
                std::basic_string<kuic::byte_t> &a_data) = 0; 

        std::basic_string<kuic::byte_t> open(
                std::basic_string<kuic::byte_t> &secret,
                kuic::packet_number_t &packet_number,
                std::basic_string<kuic::byte_t> &a_data) = 0;
    };
}

#endif

