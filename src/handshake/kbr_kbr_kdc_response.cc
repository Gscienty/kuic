#include "handshake/kbr_kdc_response.h"

kuic::handshake::kbr_kdc_response_part::kbr_kdc_response_part(
        std::string server_name,
        std::string server_realm,
        unsigned int nonce)
    : nonce(nonce)
    , auth_time(kuic::current_clock())
    , key_expiration(kuic::special_clock(kuic::current_clock()) + 60 * kuic::clock_second)
    , server_realm(server_realm)
    , server_name(kuic::handshake::kbr_principal_name(server_name)) { }

void kuic::handshake::kbr_kdc_response_part::set_key_expiration(
        kuic::special_clock &clock) {
    this->key_expiration = clock;
}
