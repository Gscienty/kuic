#include "packet_packer.h"
#include "define.h"
#include <algorithm>
#include <queue>

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
    , omit_connection_id(false)
    , max_packet_size(kuic::max_packet_size_ipv4)
    , has_sent_packet(false)
    , new_non_retransmittable_acks(0) { }

std::unique_ptr<kuic::packed_packet>
kuic::packet_packer::pack_connection_close(std::shared_ptr<kuic::frame::connection_close_frame> frame) {
    std::vector<std::shared_ptr<kuic::frame::frame>> frames;
    frames.push_back(frame);

    kuic::crypt::aead &sealer = this->sealing_manager.get_sealer();
    kuic::frame::header *header = this->get_header(false);
    std::string raw = this->write_and_seal_packet(*header, frames, sealer);

    std::unique_ptr<kuic::packed_packet> packet(new kuic::packed_packet());

    packet->get_header() = std::make_shared<kuic::frame::header>(*header);
    packet->get_raw() = raw;
    packet->get_frames() = frames;
    packet->get_is_handshake() = false;

    return packet;
}

std::unique_ptr<kuic::packed_packet>
kuic::packet_packer::pack_ack_packet() {
    if (bool(this->ack_frame) == false) {
        return std::unique_ptr<kuic::packed_packet>();
    }
    std::vector<std::shared_ptr<kuic::frame::frame>> frames;
    frames.push_back(this->ack_frame);

    kuic::crypt::aead &sealer = this->sealing_manager.get_sealer();
    kuic::frame::header *header = this->get_header(false);
    std::string raw = this->write_and_seal_packet(*header, frames, sealer);

    std::unique_ptr<kuic::packed_packet> packet(new kuic::packed_packet());

    packet->get_header() = std::make_shared<kuic::frame::header>(*header);
    packet->get_raw() = raw;
    packet->get_frames() = frames;
    packet->get_is_handshake() = false;

    return packet;
}

std::vector<std::unique_ptr<kuic::packed_packet>>
kuic::packet_packer::pack_retransmission(kuic::ackhandler::packet &ack_packet) {
    if (ack_packet.is_handshake) {
        std::vector<std::unique_ptr<kuic::packed_packet>> packets;
        packets.push_back(this->pack_handshake_retransmission(ack_packet));

        return packets;
    }

    std::queue<std::shared_ptr<kuic::frame::frame>> control_frames;
    std::queue<std::shared_ptr<kuic::frame::frame>> stream_frames;

    std::for_each(ack_packet.frames.begin(), ack_packet.frames.end(),
            [&] (std::shared_ptr<kuic::frame::frame> &frame) -> void {
                if (frame->type() == kuic::frame_type_stream) {
                    stream_frames.push(frame);
                }
                else {
                    control_frames.push(frame);
                }
            });

    kuic::crypt::aead &sealer = this->sealing_manager.get_sealer();
    std::vector<std::unique_ptr<kuic::packed_packet>> packets;

    while (control_frames.empty() == false || stream_frames.empty() == false) {
        std::vector<std::shared_ptr<kuic::frame::frame>> frames;   
        kuic::bytes_count_t payload_length = 0;

        kuic::frame::header *header = this->get_header(false);
        kuic::bytes_count_t header_length = header->length();

        kuic::bytes_count_t max_size = this->max_packet_size - sealer.overhead() - header_length;

        while (control_frames.empty() == false) {
            std::shared_ptr<kuic::frame::frame> &frame = control_frames.front();
            kuic::bytes_count_t length = frame->length();
            if (payload_length + length > max_size) {
                break;
            }
            payload_length += length;
            frames.push_back(frame);
            control_frames.pop();
        }

        max_size++;

        while (stream_frames.empty() == false && payload_length + kuic::min_stream_frame_size < max_size) {
            std::shared_ptr<kuic::frame::frame> &frame = stream_frames.front();
            std::shared_ptr<kuic::frame::stream_frame> sf = reinterpret_cast<kuic::frame::stream_frame *>(frame.get())
                    ->maybe_split_offset_frame(max_size - payload_length);
            if (bool(sf)) {
                frames.push_back(sf);
                payload_length += sf->length();
            }
            else {
                frames.push_back(frame);
                payload_length += frame->length();
                stream_frames.pop();
            }
        }

        if ((*frames.rbegin())->type() == kuic::frame_type_stream) {
            reinterpret_cast<kuic::frame::stream_frame *>(frames.rbegin()->get())->get_data_length_present() = false;
        }

        std::string raw = this->write_and_seal_packet(*header, frames, sealer);

        std::unique_ptr<kuic::packed_packet> packet(new kuic::packed_packet());

        packet->get_header() = std::make_shared<kuic::frame::header>(header);
        packet->get_raw() = raw;
        packet->get_frames() = frames;
        packet->get_is_handshake() = false;
        
        packets.push_back(packet);
    }

    return packets;
}

std::unique_ptr<kuic::packed_packet>
kuic::packet_packer::pack_handshake_retransmission(kuic::ackhandler::packet &packet) {
    kuic::crypt::aead &sealer = this->sealing_manager.get_sealer();
    if (packet.packet_type == kuic::packet_type_initial) {
        this->has_sent_packet = false;
    }
    kuic::frame::header *header = this->get_header(false);
    header->get_packet_type() = packet.packet_type;
    std::vector<std::shared_ptr<kuic::frame::frame>> frames = packet.frames;

    std::string raw = this->write_and_seal_packet(*header, frames, sealer);

    std::unique_ptr<kuic::packed_packet> result_packet(new kuic::packed_packet());

    result_packet->get_header() = std::make_shared<kuic::frame::header>(*header);
    result_packet->get_raw() = raw;
    result_packet->get_frames() = frames;
    result_packet->get_is_handshake() = true;

    return result_packet;
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
