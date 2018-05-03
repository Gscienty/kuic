#ifndef _KUIC_HANDSHAKE_TAG_
#define _KUIC_HANDSHAKE_TAG_

#include "type.h"
#include <memory>

namespace kuic {
    namespace handshake {
        const kuic::tag_t tag_kbr_as_request = 'ASRQ';
        const kuic::tag_t tag_kbr_tgs_request = 'TSRQ';
        const kuic::tag_t tag_kdc_request_body = 'KRBY';
        const kuic::tag_t tag_protocol_version = 'PVNO';
        const kuic::tag_t tag_message_type = 'MTYP';
        const kuic::tag_t tag_padata = 'PADA';
        const kuic::tag_t tag_client_principal_name = 'CNAM';
        const kuic::tag_t tag_client_realm = 'CMAN';
        const kuic::tag_t tag_server_realm = 'SMAN';

        const kuic::tag_t tag_server_principal_name = 'SNAM';
        const kuic::tag_t tag_time_from = 'TFRM';
        const kuic::tag_t tag_time_till = 'TTIL';
        const kuic::tag_t tag_renew_till_time = 'TRNW';
        const kuic::tag_t tag_nonce = 'NONC';
        const kuic::tag_t tag_encrypt_type = 'ETYP';
        const kuic::tag_t tag_address = 'CADR';
        const kuic::tag_t tag_encrypted_data = 'EDAT';
        const kuic::tag_t tag_additional_tickets = 'ADTK';

        const kuic::tag_t tag_kbr_tgt = 'TGT\0';
        const kuic::tag_t tag_tgt_key = 'TGTK';
        const kuic::tag_t tag_time_auth = 'TAUT';
        const kuic::tag_t tag_time_start = 'TSTR';
        const kuic::tag_t tag_time_end = 'TEND';
        const kuic::tag_t tag_time_key_expiration = 'TKEP';

        struct tag_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::tag_t e) {
                kuic::byte_t *buffer(new kuic::byte_t[4]);
                if ((e & 0xFF000000) == 0) {
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
                return std::pair<kuic::byte_t *, size_t>(buffer, 4);
            }

            static kuic::tag_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                if (seek + 4> len) {
                    return kuic::tag_t(0);
                }
                kuic::tag_t result = (buffer[seek] << 24) | (buffer[seek + 1] << 16) | (buffer[seek + 2] << 8) | (buffer[seek + 3]);
                if ((result & 0xFF000000) == 0) {
                    result <<= 8;
                }
                seek += 4;
                return result;
            } 
        };
    }
}

#endif
