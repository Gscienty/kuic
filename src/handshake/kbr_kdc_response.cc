#include "handshake/kbr_kdc_response.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"
#include "define.h"
#include "error.h"
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

kuic::handshake::kbr_kdc_response_part
kuic::handshake::kbr_kdc_response_part::deserialize(
        kuic::byte_t *buffer,
        size_t size) {
    ssize_t seek = 0;
    kuic::handshake::handshake_message temporary_msg;
    kuic::error_t err;
    std::tie(temporary_msg, err) =
        kuic::handshake::handshake_message::deserialize(
                buffer, size, seek);
    if (err != kuic::no_error) {
        return kuic::handshake::kbr_kdc_response_part();
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_kbr_tgt) {
        return kuic::handshake::kbr_kdc_response_part();
    }
    
    kuic::handshake::kbr_kdc_response_part result;
    
    // deserialize secret key
    if (temporary_msg.exist(kuic::handshake::tag_tgt_key)) {
        result.key = kuic::handshake::kbr_encryption_key::deserialize(
                temporary_msg.get(kuic::handshake::tag_tgt_key).data(),
                temporary_msg.get(kuic::handshake::tag_tgt_key).size());
    }

    // deserialize nonce
    if (temporary_msg.exist(kuic::handshake::tag_nonce)) {
        ssize_t seek = 0;
        result.nonce = eys::deserializer<unsigned int>::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(kuic::handshake::tag_nonce).data()),
                temporary_msg.get(kuic::handshake::tag_nonce).size(),
                seek);
    }
    
    // deserialize auth time
    if (temporary_msg.exist(kuic::handshake::tag_time_auth)) {
        ssize_t seek = 0;
        result.auth_time = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(kuic::handshake::tag_time_auth).data()),
                temporary_msg.get(kuic::handshake::tag_time_auth).size(),
                seek);
    }
    
    // deserialize start time
    if (temporary_msg.exist(kuic::handshake::tag_time_start)) {
        ssize_t seek = 0;
        result.start_time = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(kuic::handshake::tag_time_start).data()),
                temporary_msg.get(kuic::handshake::tag_time_start).size(),
                seek);
    }

    // deserialize end time
    if (temporary_msg.exist(kuic::handshake::tag_time_end)) {
        ssize_t seek = 0;
        result.end_time = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(kuic::handshake::tag_time_end).data()),
                temporary_msg.get(kuic::handshake::tag_time_end).size(),
                seek); 
    }

    // deserialize renew till
    if (temporary_msg.exist(kuic::handshake::tag_renew_till_time)) {
        ssize_t seek = 0;
        result.renew_till = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(
                        kuic::handshake::tag_renew_till_time).data()),
                temporary_msg.get(kuic::handshake::tag_renew_till_time).size(),
                seek);
    }

    // deserialize key expiration
    if (temporary_msg.exist(kuic::handshake::tag_time_key_expiration)) {
        ssize_t seek = 0;
        result.key_expiration = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(
                        kuic::handshake::tag_time_key_expiration).data()),
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
        result.server_name = kuic::handshake::kbr_principal_name::deserialize(
                reinterpret_cast<char *>(
                    temporary_msg.get(
                        kuic::handshake::tag_server_principal_name).data()),
                temporary_msg.get(
                    kuic::handshake::tag_server_principal_name).size());
    }

    return result;
}

// split ( kbr_kdc_response_part | kbr_kdc_response )

kuic::handshake::kbr_kdc_response::kbr_kdc_response() { }

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

kuic::handshake::handshake_message
kuic::handshake::kbr_kdc_response::serialize() {
    kuic::handshake::handshake_message result;
    
    std::unique_ptr<kuic::byte_t> serialized_buffer;
    size_t serialized_size;

    // serialize version
    serialized_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                kuic::handshake::kbr_protocol_version_serializer::serialize(this->version, serialized_size)));
    result.insert(
            kuic::handshake::tag_protocol_version,
            serialized_buffer.get(),
            serialized_size);

    // serialize message type 
    serialized_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                kuic::handshake::kbr_message_type_serializer::serialize(this->message_type, serialized_size)));
    result.insert(
            kuic::handshake::tag_message_type,
            serialized_buffer.get(),
            serialized_size);

    // serialize realm
    serialized_buffer = std::unique_ptr<kuic::byte_t>(
            new kuic::byte_t[this->realm.size()]);
    std::copy(
            this->realm.begin(),
            this->realm.end(),
            serialized_buffer.get());
    result.insert(
            kuic::handshake::tag_message_type,
            serialized_buffer.get(),
            this->realm.size());

    // serialize client name
    serialized_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->client_name.serialize(serialized_size)));
    result.insert(
            kuic::handshake::tag_client_principal_name,
            serialized_buffer.get(),
            serialized_size);

    // serialize encrypted data
    serialized_buffer = std::unique_ptr<kuic::byte_t>(
            reinterpret_cast<kuic::byte_t *>(
                this->encrypted_data.serialize(serialized_size)));
    result.insert(
            kuic::handshake::tag_encrypted_data,
            serialized_buffer.get(),
            serialized_size);

    return result;
}
