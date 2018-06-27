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
#include "stream/stream_frame_source.h"
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
        std::basic_string<kuic::byte_t> div_nonce;
        kuic::sealing_manager &sealing_manager;

        kuic::packet_number_generator packet_number_generator;
        kuic::stream::stream_frame_source &streams;
        
        std::mutex control_frame_mutex;
        std::vector<std::shared_ptr<kuic::frame::frame>> control_frames;

        std::shared_ptr<kuic::frame::ack_frame> ack_frame;
        bool omit_connection_id;
        kuic::bytes_count_t max_packet_size;
        bool has_sent_packet;
        int num_non_retransmittable_acks;
    public:
        packet_packer(
                kuic::connection_id dest_conn_id,
                kuic::connection_id src_conn_id,
                kuic::packet_number_t initial_packet_number,
                std::basic_string<kuic::byte_t> div_nonce,
                kuic::sealing_manager &crypto_setup,
                kuic::stream::stream_frame_source &stream_framer,
                bool is_client);
        
        std::shared_ptr<kuic::frame::header> get_header(bool is_handshake);

        std::unique_ptr<kuic::packed_packet> pack_connection_close(std::shared_ptr<kuic::frame::connection_close_frame> frame);
        std::unique_ptr<kuic::packed_packet> pack_ack_packet();
        std::unique_ptr<kuic::packed_packet> pack_handshake_retransmission(kuic::ackhandler::packet &packet);
        std::vector<std::unique_ptr<kuic::packed_packet>> pack_retransmission(kuic::ackhandler::packet &ack_packet);
        std::unique_ptr<kuic::packed_packet> pack_crypto_packet();
        std::unique_ptr<kuic::packed_packet> pack_packet();

        std::basic_string<kuic::byte_t> write_and_seal_packet(
                kuic::frame::header &header,
                std::vector<std::shared_ptr<kuic::frame::frame>> &payload_frames,
                kuic::crypt::aead &sealer);

        std::vector<std::shared_ptr<kuic::frame::frame>> compose_next_packet(
                kuic::bytes_count_t max_frame_size, bool can_send_stream_frames);

        void queue_control_frame(std::shared_ptr<kuic::frame::frame> frame);

        bool can_send_data(bool is_handshake);

        void set_omit_connection_id();
        void change_dest_conn_id(kuic::connection_id &conn_id);
        void set_max_packet_size(kuic::bytes_count_t size);
    };
}

#endif

