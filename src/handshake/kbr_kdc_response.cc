#include "handshake/kbr_kdc_response.h"
#include "define.h"

kuic::handshake::kbr_kdc_response::kbr_kdc_response() {

}

kuic::handshake::kbr_kdc_response
kuic::handshake::kbr_kdc_response::build_as_response() {
    kuic::handshake::kbr_kdc_response ret;
    
    ret.version = kuic::kbr_current_protocol_version;


    return ret;
}
