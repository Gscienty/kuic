#include "handshake/kbr_kdc_response.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "define.h"
#include <utility>
#include <vector>
#include <algorithm>

kuic::handshake::kbr_kdc_response_part::kbr_kdc_response_part() { }

kuic::handshake::kbr_kdc_response_part::kbr_kdc_response_part(
        unsigned int nonce)
    : nonce(nonce)
    , auth_time(kuic::current_clock())
    , start_time(kuic::current_clock())
    , end_time(kuic::special_clock(kuic::current_clock()) + 
            60 * kuic::clock_second) { }

void kuic::handshake::kbr_kdc_response_part::set_nonce(
        unsigned int nonce) {
    this->nonce = nonce;
}

void kuic::handshake::kbr_kdc_response_part::set_key(
        kuic::handshake::kbr_encryption_key key) {
    this->key = key;
}

void kuic::handshake::kbr_kdc_response_part::set_key_expiration(
        kuic::special_clock &clock) {
    this->key_expiration = clock;
}

void kuic::handshake::kbr_kdc_response_part::set_server_realm(
        std::string realm) {
    this->server_realm = realm;
}

void kuic::handshake::kbr_kdc_response_part::set_server_name(
        kuic::handshake::kbr_principal_name server_name) {
    this->server_name = server_name;
}

char *kuic::handshake::kbr_kdc_response_part::serialize(
        size_t &size) {
    // declare temporary handshake_message 
    // machine will encrypte it & embedded kdc response package
    kuic::handshake::handshake_message msg(
            kuic::handshake::tag_kbr_tgt);
 
    // declare serialize buffer, 
    // used to store middle serialize value
    std::unique_ptr<kuic::byte_t> seialize_buffer;

    // TODO serialize kbr_kdc_response_part fields

    // serialize temporary handshake_message
    // you should encrypt the result
    // (byte-string) in other function.
    std::vector<kuic::byte_t> serialized_vec = msg.serialize();
    size = serialized_vec.size();
    char *result_buffer = new char[size];
    std::copy_n(
            serialized_vec.begin(),
            size,
            result_buffer);
    return result_buffer;
}

// split ( kbr_kdc_response_part | kbr_kdc_response )

kuic::handshake::kbr_kdc_response::kbr_kdc_response() { }

kuic::handshake::kbr_kdc_response
kuic::handshake::kbr_kdc_response::build_as_response() {
    kuic::handshake::kbr_kdc_response ret;
    
    ret.version = kuic::kbr_current_protocol_version;


    return ret;
}
