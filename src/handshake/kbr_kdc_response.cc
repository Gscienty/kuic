#include "handshake/kbr_kdc_response.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"
#include "define.h"
#include "error.h"
#include <utility>
#include <vector>
#include <algorithm>

#include <iostream>

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

unsigned int 
kuic::handshake::kbr_kdc_response_part::get_nonce() const {
    return this->nonce;
}

kuic::handshake::kbr_encryption_key
kuic::handshake::kbr_kdc_response_part::get_encryption_key() const {
    return this->key;
}

kuic::special_clock
kuic::handshake::kbr_kdc_response_part::get_key_expiration() const {
    return this->key_expiration;
}

std::string
kuic::handshake::kbr_kdc_response_part::get_server_realm() const {
    return this->server_realm;
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_kdc_response_part::get_server_name() const {
    return this->server_name;
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

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_kdc_response_part::serialize() const {
    // declare temporary handshake_message 
    // machine will encrypte it & embedded kdc response package
    kuic::handshake::handshake_message msg(
            kuic::handshake::tag_kbr_tgt);
    
    // declare serialize buffer ptr
    kuic::byte_t *serialized_buffer_ptr = nullptr;

    // declare serialize buffer, 
    // used to store middle serialize value
    std::unique_ptr<kuic::byte_t> serialized_buffer;

    // declare serialize buffer length
    size_t serialized_size = 0;

    // serialize key
    std::tie(serialized_buffer_ptr, serialized_size) = this->key.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_tgt_key, serialized_buffer.get(),
            serialized_size);

    // serialize nonce
    std::tie(serialized_buffer_ptr, serialized_size) = eys::bigendian_serializer<kuic::byte_t, unsigned int>::serialize(this->nonce);
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_nonce, serialized_buffer.get(), serialized_size);

    // serialize auth time
    serialized_size = 0;
    std::tie(serialized_buffer_ptr, serialized_size) = this->auth_time.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_time_auth, serialized_buffer.get(), serialized_size);

    // serialize start time
    serialized_size = 0;
    std::tie(serialized_buffer_ptr, serialized_size) = this->start_time.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_time_start, serialized_buffer.get(), serialized_size);

    // serialize end time
    serialized_size = 0;
    std::tie(serialized_buffer_ptr, serialized_size) = this->end_time.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_time_end, serialized_buffer.get(), serialized_size);

    // serialize time renew till
    serialized_size = 0;
    std::tie(serialized_buffer_ptr, serialized_size) = this->renew_till.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_renew_till_time, serialized_buffer.get(), serialized_size);

    // serialize key expiration
    serialized_size = 0;
    std::tie(serialized_buffer_ptr, serialized_size) = this->key_expiration.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_time_key_expiration, serialized_buffer.get(), serialized_size);

    // serialize server realm
    serialized_buffer = std::unique_ptr<kuic::byte_t>(
            new kuic::byte_t[this->server_realm.size()]);
    std::copy(this->server_realm.begin(), this->server_realm.end(), serialized_buffer.get());
    msg.insert(kuic::handshake::tag_server_realm, serialized_buffer.get(), this->server_realm.size());

    // serialize server name
    serialized_size = 0;
    std::tie(serialized_buffer_ptr, serialized_size) = this->server_name.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_server_principal_name, serialized_buffer.get(), serialized_size);

    // serialize temporary handshake_message
    // you should encrypt the result
    // (byte-string) in other function.
    return msg.serialize();
}

kuic::handshake::kbr_kdc_response_part
kuic::handshake::kbr_kdc_response_part::deserialize(kuic::byte_t *buffer, size_t size, size_t &seek) {
    kuic::handshake::handshake_message temporary_msg = kuic::handshake::handshake_message::deserialize(buffer, size, seek);

    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_kdc_response_part(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_kbr_tgt) {
        // TODO it is not tag_kbr_tgt 
        return kuic::handshake::kbr_kdc_response_part();
    }

    // declare result
    kuic::handshake::kbr_kdc_response_part result;
    // deserialize secret key
    if (temporary_msg.exist(kuic::handshake::tag_tgt_key)) {
        size_t segment_seek = 0;
        result.key = kuic::handshake::kbr_encryption_key::deserialize(
                temporary_msg.get(kuic::handshake::tag_tgt_key).data(),
                temporary_msg.get(kuic::handshake::tag_tgt_key).size(),
                segment_seek);
    }

    // deserialize nonce
    if (temporary_msg.exist(kuic::handshake::tag_nonce)) {
        size_t segment_seek = 0;
        result.nonce = eys::bigendian_serializer<kuic::byte_t, unsigned int>::deserialize(
                temporary_msg.get(kuic::handshake::tag_nonce).data(),
                temporary_msg.get(kuic::handshake::tag_nonce).size(),
                segment_seek);
    }
    
    // deserialize auth time
    if (temporary_msg.exist(kuic::handshake::tag_time_auth)) {
        size_t seek = 0;
        result.auth_time = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_time_auth).data(),
                temporary_msg.get(kuic::handshake::tag_time_auth).size(),
                seek);
    }
    
    // deserialize start time
    if (temporary_msg.exist(kuic::handshake::tag_time_start)) {
        size_t seek = 0;
        result.start_time = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_time_start).data(),
                temporary_msg.get(kuic::handshake::tag_time_start).size(),
                seek);
    }

    // deserialize end time
    if (temporary_msg.exist(kuic::handshake::tag_time_end)) {
        size_t seek = 0;
        result.end_time = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_time_end).data(),
                temporary_msg.get(kuic::handshake::tag_time_end).size(),
                seek);
    }

    // deserialize renew till
    if (temporary_msg.exist(kuic::handshake::tag_renew_till_time)) {
        size_t seek = 0;
        result.renew_till = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_renew_till_time).data(),
                temporary_msg.get(kuic::handshake::tag_renew_till_time).size(),
                seek);
    }

    // deserialize key expiration
    if (temporary_msg.exist(kuic::handshake::tag_time_key_expiration)) {
        size_t seek = 0;
        result.key_expiration = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_time_key_expiration).data(),
                temporary_msg.get(kuic::handshake::tag_time_key_expiration).size(),
                seek);
    }

    // deserialize server realm
    if (temporary_msg.exist(kuic::handshake::tag_server_realm)) {
        result.server_realm = std::string(
                temporary_msg.get(kuic::handshake::tag_server_realm).begin(),
                temporary_msg.get(kuic::handshake::tag_server_realm).end());
    }

    // deserialize server name
    if (temporary_msg.exist(kuic::handshake::tag_server_principal_name)) {
        size_t seek = 0;
        result.server_name = kuic::handshake::kbr_principal_name::deserialize(
                temporary_msg.get(kuic::handshake::tag_server_principal_name).data(),
                temporary_msg.get(kuic::handshake::tag_server_principal_name).size(),
                seek);
    }

    return result;
}

// split ( kbr_kdc_response_part | kbr_kdc_response )

kuic::handshake::kbr_kdc_response::kbr_kdc_response() { }

kuic::handshake::kbr_kdc_response::kbr_kdc_response(kuic::error_t err)
    : lawful_package(err) { }

kuic::handshake::kbr_kdc_response
kuic::handshake::kbr_kdc_response::build_as_response(
        std::string realm,
        kuic::kbr_encryption_type_t encryption_type,
        kuic::byte_t *secret_key,
        size_t secret_key_size,
        kuic::handshake::kbr_kdc_response_part &part) {
    kuic::handshake::kbr_kdc_response ret;
    
    ret.version = kuic::kbr_current_protocol_version;
    ret.message_type = kuic::handshake::kbr_kdc_as_response;
    ret.realm = realm;

    // prepare part buffer
    size_t serialized_part_buffer_size = 0;
    kuic::byte_t *serialized_part_buffer_ptr = nullptr;
    std::tie(serialized_part_buffer_ptr, serialized_part_buffer_size) = part.serialize();
    std::unique_ptr<kuic::byte_t> serialized_part_buffer(serialized_part_buffer_ptr);

    // construct encrypted data (a field in TGT response body)
    kuic::handshake::kbr_encrypted_data encrypted_data(
            0x00000000,
            encryption_type);
    encrypted_data.set_plain_message(serialized_part_buffer.get(), serialized_part_buffer_size, secret_key, secret_key_size);
    ret.encrypted_data = encrypted_data;

    return ret;
}

kuic::handshake::handshake_message
kuic::handshake::kbr_kdc_response::__serialize() const {
    // TODO consider tgs
    kuic::handshake::handshake_message result(kuic::handshake::tag_kbr_tgt);
    // declare temporary var
    std::unique_ptr<kuic::byte_t> serialized_buffer;
    size_t serialized_size = 0;
    kuic::byte_t *serialized_buffer_ptr = nullptr;

    // serialize version
    std::tie(serialized_buffer_ptr, serialized_size) = kuic::handshake::kbr_protocol_version_serializer::serialize(this->version);
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    result.insert(kuic::handshake::tag_protocol_version, serialized_buffer.get(), serialized_size);

    // serialize message type
    std::tie(serialized_buffer_ptr, serialized_size) = kuic::handshake::kbr_message_type_serializer::serialize(this->message_type);
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    result.insert(kuic::handshake::tag_message_type, serialized_buffer.get(), serialized_size);

    // serialize realm
    serialized_buffer = std::unique_ptr<kuic::byte_t>(new kuic::byte_t[this->realm.size()]);
    std::copy(this->realm.begin(), this->realm.end(), serialized_buffer.get());
    result.insert(kuic::handshake::tag_client_realm, serialized_buffer.get(), this->realm.size());

    // serialize client name
    std::tie(serialized_buffer_ptr, serialized_size) = this->client_name.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    result.insert(kuic::handshake::tag_client_principal_name, serialized_buffer.get(), serialized_size);

    // serialize encrypted data
    std::tie(serialized_buffer_ptr, serialized_size) = this->encrypted_data.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
    result.insert(kuic::handshake::tag_encrypted_data, serialized_buffer.get(), serialized_size);

    return result;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_kdc_response::serialize() const {
    return this->__serialize().serialize();
}

kuic::handshake::kbr_kdc_response
kuic::handshake::kbr_kdc_response::deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::handshake_message msg = kuic::handshake::handshake_message::deserialize(buffer, len, seek);

    if (msg.is_lawful() == false) {
        return kuic::handshake::kbr_kdc_response(msg.get_error());
    }
    if (msg.get_tag() != kuic::handshake::tag_kbr_tgt) {
        return kuic::handshake::kbr_kdc_response(kuic::not_expect);
    }
    
    kuic::handshake::kbr_kdc_response result;
    
    // deserialize version
    if (msg.exist(kuic::handshake::tag_protocol_version)) {
        size_t seek = 0;
        result.version = kuic::handshake::kbr_protocol_version_serializer::deserialize(
                msg.get(kuic::handshake::tag_protocol_version).data(),
                msg.get(kuic::handshake::tag_protocol_version).size(),
                seek);
    }

    // deserialize message type
    if (msg.exist(kuic::handshake::tag_message_type)) {
        size_t seek = 0;
        result.message_type = kuic::handshake::kbr_message_type_serializer::deserialize(
                msg.get(kuic::handshake::tag_message_type).data(),
                msg.get(kuic::handshake::tag_message_type).size(),
                seek);
    }

    // deserialize realm
    if (msg.exist(kuic::handshake::tag_client_realm)) {
        result.realm = std::string(
                msg.get(kuic::handshake::tag_client_realm).begin(),
                msg.get(kuic::handshake::tag_client_realm).end());
    }

    // deserialize client name
    if (msg.exist(kuic::handshake::tag_client_principal_name)) {
        size_t seek = 0;
        result.client_name = kuic::handshake::kbr_principal_name::deserialize(
                msg.get(kuic::handshake::tag_client_principal_name).data(),
                msg.get(kuic::handshake::tag_client_principal_name).size(),
                seek);
    }

    // encrypted data
    if (msg.exist(kuic::handshake::tag_encrypted_data)) {
        size_t seek = 0;
        result.encrypted_data = kuic::handshake::kbr_encrypted_data::deserialize(
                msg.get(kuic::handshake::tag_encrypted_data).data(),
                msg.get(kuic::handshake::tag_encrypted_data).size(),
                seek);
    }

    return result;
}

kuic::kbr_protocol_version_t
kuic::handshake::kbr_kdc_response::get_version() const {
    return this->version;
}

kuic::kbr_message_type_t
kuic::handshake::kbr_kdc_response::get_message_type() const {
    return this->message_type;
}

std::string
kuic::handshake::kbr_kdc_response::get_realm() const {
    return this->realm;
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_kdc_response::get_client_name() const {
    return this->client_name;
}

kuic::handshake::kbr_encrypted_data
kuic::handshake::kbr_kdc_response::get_encryption_key() const {
    return this->encrypted_data;
}
