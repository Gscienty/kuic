#include "packet_packer.h"
#include "define.h"
#include <algorithm>

kuic::packet_packer::packet_packer(
        kuic::connection_id dest_conn_id,
        kuic::connection_id src_conn_id,
        kuic::packet_number_t initial_packet_number,
        std::vector<kuic::byte_t> div_nonce,
        kuic::sealing_manager &crypto_setup,
        kuic::stream_frame_source &stream_framer,
        bool is_client)
    : dest_conn_id(dest_conn_id)
    , src_conn_id(src_conn_id)
    , is_client(is_client)
    , div_nonce(div_nonce)
    , sealing_manager(crypto_setup)
    , packet_number_generator(
            initial_packet_number,
            kuic::skip_packet_average_period_length)
    , streams(stream_framer)
    , ack_frame(nullptr)
    , omit_connection_id(false)
    , max_packet_size(kuic::max_packet_size_ipv4)
    , has_sent_packet(false)
    , new_non_retransmittable_acks(0) { }

kuic::nullable<kuic::packed_packet>
kuic::packet_packer::pack_connection_close(std::shared_ptr<kuic::frame::connection_close_frame> frame) {
    std::vector<std::shared_ptr<kuic::frame::frame>> frames;
    frames.push_back(frame);

    kuic::crypt::aead &sealer = this->sealing_manager.get_sealer();
    kuic::frame::header *header = this->get_header(false);
    std::string raw = this->write_and_seal_packet(*header, frames, sealer);

    kuic::packed_packet *packet = new kuic::packed_packet();

    packet->get_header() = std::make_shared<kuic::frame::header>(*header);
    packet->get_raw() = raw;
    packet->get_frames() = frames;
    packet->get_is_handshake() = false;

    return kuic::nullable<kuic::packed_packet>(packet);
}

kuic::frame::header *
kuic::packet_packer::get_header(bool is_handshake) {
    kuic::packet_number_t packet_number = this->packet_number_generator.peek();
    size_t packet_number_length = 4;

    kuic::frame::header *header = new kuic::frame::header();
    
    header->get_dest_conn_id() = this->dest_conn_id;
    header->get_src_conn_id() = this->src_conn_id;
    header->get_packet_number() = packet_number;
    header->get_packet_number_length() = packet_number_length;

    if (is_handshake) {
        header->get_packet_type() = this->is_client ? kuic::packet_type_initial : kuic::packet_type_handshake;
    }

    return header;
}

std::string kuic::packet_packer::write_and_seal_packet(
        kuic::frame::header &header,
        std::vector<std::shared_ptr<kuic::frame::frame>> &payload_frames,
        kuic::crypt::aead &sealer) {
    std::string result;
    
    if (header.get_is_long()) {
        if (header.get_packet_type() == kuic::packet_type_initial) {
            size_t header_length = header.length();
            header.get_payload_length() = kuic::min_initial_packet_size - header_length;
        }
        else {
            kuic::bytes_count_t payload_length = sealer.overhead();
            std::for_each(payload_frames.begin(), payload_frames.end(),
                    [&] (const std::shared_ptr<kuic::frame::frame> &frame) -> void {
                        payload_length += frame->length();
                    });
            header.get_payload_length() = payload_length;
        }
    }

    // serialize header
    std::pair<kuic::byte_t *, size_t> serialized_header = header.serialize();
    result.insert(result.end(), serialized_header.first, serialized_header.first + serialized_header.second);
    delete[] serialized_header.first;

    if (header.get_packet_type() == kuic::packet_type_initial) {
        std::shared_ptr<kuic::frame::frame> &last_frame = *payload_frames.rbegin();
        if (last_frame->type() == kuic::frame_type_stream) {
            reinterpret_cast<kuic::frame::stream_frame *>(last_frame.get())->get_data_length_present() = true;
        }
    }
    
    std::string seal_str;
    // serialize frames
    std::for_each(payload_frames.begin(), payload_frames.end(),
            [&] (std::shared_ptr<kuic::frame::frame> &frame) -> void {
                std::pair<kuic::byte_t *, size_t> serialized_frame = frame->serialize();
                seal_str.insert(result.end(), serialized_frame.first, serialized_frame.first + serialized_frame.second);
                delete[] serialized_frame.first;
            });

    // if the packet is initial_packet fill pendding
    if (header.get_packet_type() == kuic::packet_type_initial) {
        size_t padding_length = kuic::min_initial_packet_size - sealer.overhead() - result.size() - seal_str.size();
        if (padding_length > 0) {
            seal_str.append(std::string(padding_length, 0x00));
        }
    }

    if (result.size() + seal_str.size() + sealer.overhead() > this->max_packet_size) {
        return std::string(); 
    }

    std::string sealed_str = sealer.seal(seal_str, header.get_packet_number(), result);

    this->has_sent_packet = true;
    result.append(sealed_str);

    return result;
}
