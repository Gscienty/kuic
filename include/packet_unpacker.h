#ifndef _KUIC_PACKET_UNPACKER_
#define _KUIC_PACKET_UNPACKER_

#include "unpacked_packet.h"
#include "crypt/aead.h"
#include "frame/header.h"
#include <memory>
#include <string>

namespace kuic {
    class packet_unpacker {
    private:
        kuic::unpacked_aead &aead;
    public:
        packet_unpacker(kuic::unpacked_aead &aead);

        std::unique_ptr<kuic::unpacked_packet> unpack(
                std::basic_string<kuic::byte_t> &header_binary,
                kuic::frame::header &header,
                std::basic_string<kuic::byte_t> &data);

        std::vector<std::shared_ptr<kuic::frame::frame>> parse_frames(
                std::basic_string<kuic::byte_t> &decrypted);
    };
}

#endif

