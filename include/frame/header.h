#ifndef _KUIC_FRAME_HEADER_
#define _KUIC_FRAME_HEADER_

#include "connection_id.h"
#include "type.h"
#include "lawful_package.h"
#include <vector>
#include <utility>
#include <string>

namespace kuic {
    namespace frame {
        class header {
        private:
            std::vector<kuic::byte_t> raw;
            
            kuic::connection_id dest_conn_id;
            kuic::connection_id src_conn_id;
            bool omit_connection_id;

            int packet_number_length;
            kuic::packet_number_t packet_number;

            kuic::packet_type_t packet_type;
            bool is_long;
            kuic::bytes_count_t payload_length;

            static header deserialize_long_header(const kuic::byte_t *buffer, size_t len, size_t &seek, kuic::byte_t type_byte);
            static header deserialize_short_header(const kuic::byte_t *buffer, size_t len, size_t &seek, kuic::byte_t type_byte);
            
            static int decode_single_connection_id_length(kuic::byte_t enc);
            
            std::pair<kuic::byte_t *, size_t> serialize_long_header() const;
            std::pair<kuic::byte_t *, size_t> serialize_short_header() const;
        public:
            header()
                : omit_connection_id(false)
                , packet_number_length(0)
                , packet_number(0)
                , packet_type(0)
                , is_long(false)
                , payload_length(0) { }

            kuic::connection_id &get_dest_conn_id();
            kuic::connection_id &get_src_conn_id();
            kuic::packet_number_t &get_packet_number();
            int &get_packet_number_length();
            kuic::packet_type_t &get_packet_type();
            bool &get_is_long();
            kuic::bytes_count_t &get_payload_length();

            static header deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek);
            std::string serialize() const;
            size_t length() const;
        };
    }
}

#endif

