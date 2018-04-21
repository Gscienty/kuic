#ifndef _KUIC_HANDSHAKE_TAG_
#define _KUIC_HANDSHAKE_TAG_

#include "type.h"
#include <memory>

namespace kuic {
    namespace handshake {
        const kuic::tag_t tag_client_hello = 'CHLO';
        const kuic::tag_t tag_server_rejection = 'REJ';
        const kuic::tag_t tag_server_config = 'SCFG';

        const kuic::tag_t tag_padding = 'PAD';
        const kuic::tag_t tag_server_name_indication = 'SNI';
        const kuic::tag_t tag_version = 'VER';
        const kuic::tag_t tag_max_stream_per_connection = 'MSPC';
        const kuic::tag_t tag_max_incoming_dyanamic_stream = 'MIDS';
        const kuic::tag_t tag_user_agent_id = 'UAID';
        const kuic::tag_t tag_truncation_connection_id = 'TCID';
        const kuic::tag_t tag_proof_demand = 'PDMD';
        const kuic::tag_t tag_socket_receive_buffer = 'SRBF';
        const kuic::tag_t tag_idle_connection_state_lifetime = 'ICSL';
        const kuic::tag_t tag_client_proof_nonce = 'NONP';
        const kuic::tag_t tag_silently_close_timeout = 'SCLS';
        const kuic::tag_t tag_cert_timestamp_leaf_cert = 'CSCT';
        const kuic::tag_t tag_connection_options = 'COPT';
        const kuic::tag_t tag_initial_connection_flow_control_receive_window = 'CFCW';
        const kuic::tag_t tag_initial_stream_flow_control_receive_window = 'SFCW';
        
        const kuic::tag_t tag_source_address_token = 'STK';
        const kuic::tag_t tag_server_nonce = 'SNO';
        const kuic::tag_t tag_server_proof = 'PROF';

        const kuic::tag_t tag_client_nonce = 'NONC';
        
        const kuic::tag_t tag_server_config_id = 'SCID';
        const kuic::tag_t tag_list_aead_algo = 'AEAD';
        
        const kuic::tag_t tag_server_hello = 'SHLO';
        const kuic::tag_t tag_public_reset_tag = 'PRST';

        struct tag_serializer {
            static char *serialize(kuic::tag_t e, size_t &size) {
                char *buffer(new char[4]);
                if (e & 0xFF000000 == 0) {
                    buffer[0] = (e & 0x00FF0000) >> 16;
                    buffer[1] = (e & 0x0000FF00) >> 8;
                    buffer[2] = (e & 0x000000FF);
                    buffer[3] = 0x00;
                }
                else {
                    buffer[0] = (e & 0xFF000000) >> 24;
                    buffer[1] = (e & 0x00FF0000) >> 16;
                    buffer[2] = (e & 0x0000FF00) >> 8;
                    buffer[3] = (e & 0x000000FF);
                }
                size = 4;
                return buffer;
            }

            static kuic::tag_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                kuic::tag_t result = (buffer[seek] << 24) | (buffer[seek + 1] << 16) | (buffer[seek + 2] << 8) | (buffer[seek + 3]);
                if ((result & 0x000000FF) == 0) {
                    result >>= 8;
                }
                seek += 4;
                return result;
            } 
        };
    }
}

#endif