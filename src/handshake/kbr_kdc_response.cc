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
    std::unique_ptr<kuic::byte_t> serialize_buffer;

    // declare serialize buffer length
    size_t serialize_size = 0;

    // serialize key
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            this->key.serialize(serialize_size));
    msg.insert(
            kuic::handshake::tag_tgt_key,
            serialize_buffer.get(),
            serialize_size);

    // serialize nonce
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                eys::serializer<unsigned int>::serialize(
                this->nonce,
                serialize_size)));
    msg.insert(
            kuic::handshake::tag_nonce,
            serialize_buffer.get(),
            serialize_size);

    // serialize auth time
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->auth_time.timestamp_serialize(
                    serialize_size)));
    msg.insert(
            kuic::handshake::tag_time_auth,
            serialize_buffer.get(),
            serialize_size);

    // serialize start time
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->start_time.timestamp_serialize(
                    serialize_size)));
    msg.insert(
            kuic::handshake::tag_time_start,
            serialize_buffer.get(),
            serialize_size);

    // serialize end time
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->end_time.timestamp_serialize(
                    serialize_size)));
    msg.insert(
            kuic::handshake::tag_time_end,
            serialize_buffer.get(),
            serialize_size);

    // serialize time renew till
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->renew_till.timestamp_serialize(
                    serialize_size)));
    msg.insert(
            kuic::handshake::tag_renew_till_time,
            serialize_buffer.get(),
            serialize_size);

    // serialize key expiration
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->key_expiration.timestamp_serialize(
                    serialize_size)));
    msg.insert(
            kuic::handshake::tag_time_key_expiration,
            serialize_buffer.get(),
            serialize_size);

    // serialize server realm
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            new kuic::byte_t[this->server_realm.size()]);
    std::copy(
            this->server_realm.begin(),
            this->server_realm.end(),
            serialize_buffer.get());
    msg.insert(
            kuic::handshake::tag_server_realm,
            serialize_buffer.get(),
            this->server_realm.size());

    // serialize server name
    serialize_size = 0;
    serialize_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->server_name.serialize(serialize_size)));
    msg.insert(
            kuic::handshake::tag_server_principal_name,
            serialize_buffer.get(),
            serialize_size);

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
kuic::handshake::kbr_kdc_response::build_as_response(
        std::string server_realm,
        kuic::kbr_encryption_type_t encryption_type,
        kuic::byte_t *secret_key,
        size_t secret_key_size,
        kuic::handshake::kbr_kdc_response_part &part) {
    kuic::handshake::kbr_kdc_response ret;
    
    ret.version = kuic::kbr_current_protocol_version;
    ret.message_type = kuic::handshake::kbr_kdc_as_response;
    ret.realm = server_realm;

    // prepare part buffer
    size_t serialized_part_size = 0;
    std::unique_ptr<kuic::byte_t> serialized_part (
            reinterpret_cast<kuic::byte_t *>(
                part.serialize(
                serialized_part_size)));
    // construct encrypted data (a field in TGT response body)
    kuic::handshake::kbr_encrypted_data encrypted_data(
            0x00000000,
            encryption_type);
    encrypted_data.set_plain_message(
            serialized_part.get(),
            serialized_part_size,
            secret_key,
            secret_key_size);
    ret.encrypted_data = encrypted_data;

    return ret;
}
