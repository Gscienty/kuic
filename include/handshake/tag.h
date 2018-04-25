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
        const kuic::tag_t tag_realm = 'DMAN';
        const kuic::tag_t tag_server_principal_name = 'SNAM';
        const kuic::tag_t tag_time_from = 'TFRM';
        const kuic::tag_t tag_time_till = 'TTIL';
        const kuic::tag_t tag_renew_till_time = 'TRNW';
        const kuic::tag_t tag_nonce = 'NONC';
        const kuic::tag_t tag_encrypt_type = 'ETYP';
        const kuic::tag_t tag_address = 'CADR';
        const kuic::tag_t tag_encrypted_data = 'EDAT';
        const kuic::tag_t tag_additional_tickets = 'ADTK';


        struct tag_serializer {
            static char *serialize(kuic::tag_t e, size_t &size) {
                char *buffer(new char[4]);
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
                size = 4;
                return buffer;
            }

            static kuic::tag_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                if (seek + 4> ssize_t(len)) {
                    return kuic::tag_t(0);
                }
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
