#ifndef _KUIC_PACKET_PACKER_
#define _KUIC_PACKET_PACKER_

#include "connection_id.h"
#include "packed_packet.h"
#include "packet_number_generator.h"
#include "sealing_manager.h"
#include "nullable.h"
#include "frame/header.h"
#include "frame/frame.h"
#include "frame/ack_frame.h"
#include "frame/connection_close_frame.h"
#include "stream_frame_source.h"
#include "type.h"
#include <memory>
#include <vector>
#include <mutex>
#include <string>

namespace kuic {
    class packet_packer {
    private:
        kuic::connection_id dest_conn_id;
        kuic::connection_id src_conn_id;

        bool is_client;
        std::vector<kuic::byte_t> div_nonce;
        kuic::sealing_manager &sealing_manager;

        kuic::packet_number_generator packet_number_generator;
        kuic::stream_frame_source &streams;
        
        std::mutex control_frame_mutex;
        std::vector<std::shared_ptr<kuic::frame::frame>> control_frames;

        kuic::nullable<kuic::frame::ack_frame> ack_frame;
        bool omit_connection_id;
        kuic::bytes_count_t max_packet_size;
        bool has_sent_packet;
        int new_non_retransmittable_acks;
    public:
        packet_packer(
                kuic::connection_id dest_conn_id,
                kuic::connection_id src_conn_id,
                kuic::packet_number_t initial_packet_number,
                std::vector<kuic::byte_t> div_nonce,
                kuic::sealing_manager &crypto_setup,
                kuic::stream_frame_source &stream_framer,
                bool is_client);
        
        kuic::frame::header *get_header(bool is_handshake);

        kuic::nullable<kuic::packed_packet> pack_connection_close(std::shared_ptr<kuic::frame::connection_close_frame> frame);
        std::string write_and_seal_packet(
                kuic::frame::header &header,
                std::vector<std::shared_ptr<kuic::frame::frame>> &payload_frames,
                kuic::crypt::aead &sealer);
    };
}

#endif

