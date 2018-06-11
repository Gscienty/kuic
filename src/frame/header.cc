#include "frame/header.h"
#include "frame/frame.h"
#include "variable_integer.h"
#include "eys.h"
#include <algorithm>

kuic::frame::header kuic::frame::header::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    // TODO deserialize error
    if (len - seek <= 0) { return kuic::frame::header(); }

    kuic::byte_t type_byte = buffer[seek++];
    
    if ((type_byte & 0x80) != 0) {
        return kuic::frame::header::deserialize_long_header(buffer, len, seek, type_byte);
    }
    return kuic::frame::header::deserialize_short_header(buffer, len, seek, type_byte);
}

kuic::frame::header
kuic::frame::header::deserialize_long_header(const kuic::byte_t *buffer, size_t len, size_t &seek, kuic::byte_t type_byte) {
    // ignore version
    eys::bigendian_serializer<kuic::byte_t, kuic::version_t>::deserialize(buffer, len, seek);

    kuic::byte_t conn_id_length = buffer[seek++];
    int dst_conn_id_length = kuic::frame::header::decode_single_connection_id_length(conn_id_length >> 4);
    int src_conn_id_length = kuic::frame::header::decode_single_connection_id_length(conn_id_length & 0x0F);

    kuic::connection_id dest_conn_id = kuic::connection_id::deserialize(buffer, len, seek, dst_conn_id_length);
    kuic::connection_id src_conn_id = kuic::connection_id::deserialize(buffer, len, seek, src_conn_id_length);

    kuic::frame::header header;

    header.is_long = true;
    header.dest_conn_id = dest_conn_id;
    header.src_conn_id = src_conn_id;
    
    // ignore version negotation
    
    header.payload_length = kuic::variable_integer::read(buffer, len, seek);
    header.packet_number = eys::bigendian_serializer<kuic::byte_t, kuic::packet_number_t>::deserialize(
            buffer, len, seek);
    header.packet_number_length = 4;
    header.packet_type = kuic::packet_type_t(type_byte & 0x7F);

    return header;
}

kuic::frame::header
kuic::frame::header::deserialize_short_header(const kuic::byte_t *buffer, size_t len, size_t &seek, kuic::byte_t type_byte) {
    kuic::connection_id conn_id = kuic::connection_id::deserialize(buffer, len, seek, 8);

    int packet_number_length = 0;
    kuic::packet_number_t packet_number = 0;
    switch (type_byte & 0x03) {
    case 0x00:
        packet_number_length = 1;
        packet_number = eys::bigendian_serializer<kuic::byte_t, unsigned char>::deserialize(buffer, len, seek);
        break;
    case 0x01:
        packet_number_length = 2;
        packet_number = eys::bigendian_serializer<kuic::byte_t, unsigned short>::deserialize(buffer, len, seek);
        break;
    case 0x02:
        packet_number_length = 4;
        packet_number = eys::bigendian_serializer<kuic::byte_t, unsigned int>::deserialize(buffer, len, seek);
        break;
    }

    kuic::frame::header header;
    header.dest_conn_id = conn_id;
    header.packet_number = packet_number;
    header.packet_number_length = packet_number_length;

    return header;
}

int kuic::frame::header::decode_single_connection_id_length(kuic::byte_t enc) {
    if (enc == 0) {
        return 0;
    }
    return int(enc) + 3;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::header::serialize_long_header() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *buffer = new kuic::byte_t[size];

    buffer[seek++] = this->packet_type | 0x80;

    // serialize version (zero)
    kuic::frame::frame::fill(buffer, size, seek,
            eys::bigendian_serializer<kuic::byte_t, unsigned int>::serialize(0));
    // serialize connection id length
    buffer[seek++] = 0x55;
    // serialize destation connection id
    std::copy(this->dest_conn_id.bytes(), this->dest_conn_id.bytes() + 8, buffer + seek);
    seek += 8;
    // serialize source connection id
    std::copy(this->src_conn_id.bytes(), this->src_conn_id.bytes() + 8, buffer + seek);
    seek += 8;
    // serialize payload length
    kuic::frame::frame::fill(buffer, size, seek,
            kuic::variable_integer::write(this->payload_length));
    // serialize packet number
    kuic::frame::frame::fill(buffer, size, seek,
            eys::bigendian_serializer<kuic::byte_t, kuic::packet_number_t>::serialize(this->packet_number));

    return std::pair<kuic::byte_t *, size_t>(buffer, size);
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::header::serialize_short_header() const {
    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *buffer = new kuic::byte_t[size];

    kuic::byte_t type_byte = 0x30;
    switch (this->packet_number_length) {
    case 1:
        type_byte |= 0x00;
        break;
    case 2:
        type_byte |= 0x01;
        break;
    case 4:
        type_byte |= 0x02;
        break;
    }
    buffer[seek++] = type_byte;

    std::copy(this->dest_conn_id.bytes(), this->dest_conn_id.bytes() + 8, buffer + seek);
    seek += 8;

    switch (this->packet_number_length) {
    case 1:
        buffer[seek++] = this->packet_number;
        break;
    case 2:
        kuic::frame::frame::fill(buffer, size, seek,
                eys::bigendian_serializer<kuic::byte_t, unsigned short>::serialize(this->packet_number));
        break;
    case 4:
        kuic::frame::frame::fill(buffer, size, seek,
                eys::bigendian_serializer<kuic::byte_t, unsigned int>::serialize(this->packet_number));
        break;
    }

    return std::pair<kuic::byte_t *, size_t>(buffer, size);
}

size_t
kuic::frame::header::length() const {
    if (this->is_long) {
        return 1 +  // type byte
            4 +     // version
            1 +     // connection id length
            8 + 8 + // connection length
            kuic::variable_integer::length(this->payload_length) + // payload length
            4;      // packet number
    }

    return 1 +  // type byte
        8 +     // connection id
        this->packet_number_length; // packet number
}

kuic::connection_id &
kuic::frame::header::get_dest_conn_id() {
    return this->dest_conn_id;
}

kuic::connection_id &
kuic::frame::header::get_src_conn_id() {
    return this->src_conn_id;
}

kuic::packet_number_t &
kuic::frame::header::get_packet_number() {
    return this->packet_number;
}

kuic::packet_type_t &
kuic::frame::header::get_packet_type() {
    return this->packet_type;
}

int &
kuic::frame::header::get_packet_number_length() {
    return this->packet_number_length;
}

bool &
kuic::frame::header::get_is_long() {
    return this->is_long;
}

kuic::bytes_count_t &
kuic::frame::header::get_payload_length() {
    return this->payload_length;
}
