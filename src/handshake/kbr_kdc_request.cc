#include "handshake/kbr_kdc_request.h"
#include "clock.h"
#include "define.h"

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::build_as_request(
    kuic::handshake::kbr_principal_name client_name,
    std::string realm,
    unsigned int nonce) {
    
    kuic::handshake::kbr_kdc_request as_req;

    as_req.version = kuic::kbr_current_protocol_version;
    as_req.message_type = kuic::handshake::kbr_kdc_as_request;
    as_req.client_name = client_name;
    as_req.realm = realm;
    as_req.from = kuic::current_clock();
    as_req.till = as_req.form + kuic::clock_second;
    as_req.nonce = nonce;

    return as_req;
}