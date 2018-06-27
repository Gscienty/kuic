#ifndef _KUIC_HANDSHAKE_TAG_
#define _KUIC_HANDSHAKE_TAG_

#include "type.h"
#include <string>
#include <memory>

namespace kuic {
    namespace handshake {
        const kuic::tag_t tag_kbr_as_request        = 'ASRQ';
        const kuic::tag_t tag_kbr_tgs_request       = 'TSRQ';
        const kuic::tag_t tag_kbr_kdc_request_body  = 'RQBY';
        const kuic::tag_t tag_protocol_version      = 'PVNO';
        const kuic::tag_t tag_message_type          = 'MTYP';
        const kuic::tag_t tag_padata                = 'PADA';
        const kuic::tag_t tag_client_principal_name = 'CNAM';
        const kuic::tag_t tag_client_realm          = 'CMAN';
        const kuic::tag_t tag_server_realm          = 'SMAN';
        const kuic::tag_t tag_authorization_data    = 'AUDA';
        const kuic::tag_t tag_ad_issued             = 'ADIS';
        const kuic::tag_t tag_checksum              = 'CKSM';
        const kuic::tag_t tag_issue_principal_name  = 'INAM';
        const kuic::tag_t tag_issue_realm           = 'IMAN';
    
        const kuic::tag_t tag_ticket                = 'TICK';
        const kuic::tag_t tag_ticket_body           = 'TKBD';

        const kuic::tag_t tag_server_principal_name = 'SNAM';
        const kuic::tag_t tag_time_from             = 'TFRM';
        const kuic::tag_t tag_time_till             = 'TTIL';
        const kuic::tag_t tag_renew_till_time       = 'TRNW';
        const kuic::tag_t tag_nonce                 = 'NONC';
        const kuic::tag_t tag_encrypt_type          = 'ETYP';
        const kuic::tag_t tag_address               = 'CADR';
        const kuic::tag_t tag_encrypted_data        = 'EDAT';
        const kuic::tag_t tag_additional_tickets    = 'ADTK';

        const kuic::tag_t tag_key                   = 'KEY\0';

        const kuic::tag_t tag_kbr_response_body     = 'KRPB';
        const kuic::tag_t tag_kbr_tgt               = 'TGT\0';
        const kuic::tag_t tag_time_auth             = 'TAUT';
        const kuic::tag_t tag_time_start            = 'TSTR';
        const kuic::tag_t tag_time_end              = 'TEND';
        const kuic::tag_t tag_time_key_expiration   = 'TKEP';

        const kuic::tag_t tag_kbr_tgs               = 'TGS\0';

        const kuic::tag_t tag_error                 = 'ERR\0';
        const kuic::tag_t tag_current_clock         = 'CURT';
        const kuic::tag_t tag_server_clock          = 'SCLK';
        const kuic::tag_t tag_error_code            = 'ERRC';
        const kuic::tag_t tag_error_string          = 'ERRS';

        const kuic::tag_t tag_public_reset          = 'PRST';
        const kuic::tag_t tag_rejected_packet_number    = 'RRPN';
        const kuic::tag_t tag_public_reset_nonce    = 'RNON';

        struct tag_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::tag_t e) {
                std::basic_string<kuic::byte_t> buffer;
                buffer.resize(4);
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
                return buffer;
            } 

            static kuic::tag_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                if (seek + 4 > buffer.size()) {
                    return kuic::tag_t(0);
                }
                kuic::tag_t result =
                    (buffer[seek] << 24) | (buffer[seek + 1] << 16) | (buffer[seek + 2] << 8) | (buffer[seek + 3]);
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
