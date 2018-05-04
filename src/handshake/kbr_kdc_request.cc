#include "handshake/kbr_kdc_request.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"
#include "clock.h"
#include "define.h"
#include <memory>

kuic::handshake::kbr_kdc_request_body::kbr_kdc_request_body() { }

kuic::handshake::kbr_kdc_request_body::kbr_kdc_request_body(kuic::error_t err)
    : lawful_package(err) { }

kuic::handshake::kbr_kdc_request_body::kbr_kdc_request_body(
        kuic::kbr_message_type_t message_type,
        kuic::handshake::kbr_principal_name name,
        std::string realm,
        unsigned int nonce)
    : message_type(message_type)
    , client_name(name)
    , server_name(name)
    , realm(realm)
    , from(kuic::current_clock())
    , till(kuic::special_clock(kuic::current_clock()) + kuic::clock_second)
    , nonce(nonce) { }

void kuic::handshake::kbr_kdc_request_body::support_encrypt_type(kuic::kbr_encryption_type_t encryption_type) {
    this->encrypt_types.push_back(encryption_type);
}

unsigned int kuic::handshake::kbr_kdc_request_body::get_nonce() const {
    return this->nonce;
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_kdc_request_body::get_client_name() const {
    return this->client_name;
}

kuic::handshake::kbr_principal_name
kuic::handshake::kbr_kdc_request_body::get_server_name() const {
    return this->server_name;
}

std::string
kuic::handshake::kbr_kdc_request_body::get_realm() const {
    return this->realm;
}

kuic::special_clock
kuic::handshake::kbr_kdc_request_body::get_from() const {
    return this->from;
}

kuic::special_clock
kuic::handshake::kbr_kdc_request_body::get_till() const {
    return this->till;
}

kuic::special_clock
kuic::handshake::kbr_kdc_request_body::get_renew_time() const {
    return this->renew_time;
}

kuic::handshake::kbr_encrypted_data
kuic::handshake::kbr_kdc_request_body::get_authorization_data() const {
    return this->authorization_data;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_kdc_request_body::serialize() const {
    // declare temporary handshake_message
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_kbr_kdc_request_body);

    // declare temporary buffer & size
    size_t size = 0;
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    std::unique_ptr<kuic::byte_t []> serialized_buffer;
    
    // serialize nonce
    std::tie(serialized_buffer_ptr, size) = eys::bigendian_serializer<kuic::byte_t, unsigned int>::serialize(this->nonce);
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    temporary_msg.insert(kuic::handshake::tag_nonce, serialized_buffer.get(), size);

    // serialize from timestamp
    std::tie(serialized_buffer_ptr, size) = this->from.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    temporary_msg.insert(kuic::handshake::tag_time_from, serialized_buffer.get(), size);

    // serialize till
    std::tie(serialized_buffer_ptr, size) = this->till.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    temporary_msg.insert(kuic::handshake::tag_time_till, serialized_buffer.get(), size);

    // serialize renew time
    std::tie(serialized_buffer_ptr, size) = this->renew_time.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    temporary_msg.insert(kuic::handshake::tag_renew_till_time, serialized_buffer.get(), size);

    // serialize encryption types
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(new kuic::byte_t[this->encrypt_types.size() * sizeof(kuic::kbr_encryption_type_t)]);
    std::copy(this->encrypt_types.begin(), this->encrypt_types.end(), reinterpret_cast<kuic::kbr_encryption_type_t *>(serialized_buffer.get()));
    temporary_msg.insert(kuic::handshake::tag_encrypt_type, serialized_buffer.get(), this->encrypt_types.size() * sizeof(kuic::kbr_encryption_type_t));
    
    // AS request
    if (this->message_type == kuic::handshake::kbr_kdc_as_request) {
        // serialize client principal
        std::tie(serialized_buffer_ptr, size) = this->client_name.serialize();
        serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
        temporary_msg.insert(kuic::handshake::tag_client_principal_name, serialized_buffer.get(), size);

        // serialize realm (client realm)
        serialized_buffer = std::unique_ptr<kuic::byte_t []>(new kuic::byte_t[this->realm.length()]);
        std::copy(this->realm.begin(), this->realm.end(), serialized_buffer.get());
        temporary_msg.insert(kuic::handshake::tag_client_realm, serialized_buffer.get(), this->realm.length());
    }
    // TGS request
    else if (this->message_type == kuic::handshake::kbr_kdc_tgs_request) {
        // serialize server principal
        std::tie(serialized_buffer_ptr, size) = this->server_name.serialize();
        serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
        temporary_msg.insert(kuic::handshake::tag_server_principal_name, serialized_buffer.get(), size);

        // serialize realm (server realm)
        serialized_buffer = std::unique_ptr<kuic::byte_t []>(new kuic::byte_t[this->realm.length()]);
        std::copy(this->realm.begin(), this->realm.end(), serialized_buffer.get());
        temporary_msg.insert(kuic::handshake::tag_server_realm, serialized_buffer.get(), this->realm.length());

        // serialize authorization data
        std::tie(serialized_buffer_ptr, size) = this->authorization_data.serialize();
        serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
        temporary_msg.insert(kuic::handshake::tag_authorization_data, serialized_buffer.get(), size);
    }

    return temporary_msg.serialize();
}

kuic::handshake::kbr_kdc_request_body
kuic::handshake::kbr_kdc_request_body::deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::handshake_message temporary_msg = kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_kdc_request_body(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_kbr_kdc_request_body) {
        return kuic::handshake::kbr_kdc_request_body(kuic::not_expect);
    }
    
    // declare result
    kuic::handshake::kbr_kdc_request_body result;

    // deserialize nonce
    if (temporary_msg.exist(kuic::handshake::tag_nonce)) {
        size_t seek = 0;
        result.nonce = eys::bigendian_serializer<kuic::byte_t, unsigned int>::deserialize(
                temporary_msg.get(kuic::handshake::tag_nonce).data(),
                temporary_msg.get(kuic::handshake::tag_nonce).size(),
                seek);
    }

    // deserialize from
    if (temporary_msg.exist(kuic::handshake::tag_time_from)) {
        size_t seek = 0;
        result.from = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_time_from).data(),
                temporary_msg.get(kuic::handshake::tag_time_from).size(),
                seek);
    }

    // deserialize till
    if (temporary_msg.exist(kuic::handshake::tag_time_till)) {
        size_t seek = 0;
        result.till = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_time_till).data(),
                temporary_msg.get(kuic::handshake::tag_time_till).size(),
                seek);
    }

    // deserialize renew
    if (temporary_msg.exist(kuic::handshake::tag_renew_till_time)) {
        size_t seek = 0;
        result.renew_time = kuic::special_clock::deserialize(
                temporary_msg.get(kuic::handshake::tag_renew_till_time).data(),
                temporary_msg.get(kuic::handshake::tag_renew_till_time).size(),
                seek);
    }

    // deserialize encryption types
    if (temporary_msg.exist(kuic::handshake::tag_encrypt_type)) {
        result.encrypt_types.assign(
                temporary_msg.get(kuic::handshake::tag_encrypt_type).begin(),
                temporary_msg.get(kuic::handshake::tag_encrypt_type).end());
    }
    
    // deserialize client principal
    if (temporary_msg.exist(kuic::handshake::tag_client_principal_name)) {
        size_t seek = 0;
        result.client_name = kuic::handshake::kbr_principal_name::deserialize(
                temporary_msg.get(kuic::handshake::tag_client_principal_name).data(),
                temporary_msg.get(kuic::handshake::tag_client_principal_name).size(),
                seek);
    }

    // deserialize server principal
    if (temporary_msg.exist(kuic::handshake::tag_server_principal_name)) {
        size_t seek = 0;
        result.server_name = kuic::handshake::kbr_principal_name::deserialize(
                temporary_msg.get(kuic::handshake::tag_server_principal_name).data(),
                temporary_msg.get(kuic::handshake::tag_server_principal_name).size(),
                seek);
    }

    // deserialize realm (client || server)
    if (temporary_msg.exist(kuic::handshake::tag_server_realm)) {
        result.realm.assign(
                temporary_msg.get(kuic::handshake::tag_server_realm).begin(),
                temporary_msg.get(kuic::handshake::tag_server_realm).end());
    }
    if (temporary_msg.exist(kuic::handshake::tag_client_realm)) {
        result.realm.assign(
                temporary_msg.get(kuic::handshake::tag_client_realm).begin(),
                temporary_msg.get(kuic::handshake::tag_client_realm).end());
    }

    // deserialize authorization data
    if (temporary_msg.exist(kuic::handshake::tag_authorization_data)) {
        size_t seek = 0;
        result.authorization_data = kuic::handshake::kbr_encrypted_data::deserialize(
                temporary_msg.get(kuic::handshake::tag_authorization_data).data(),
                temporary_msg.get(kuic::handshake::tag_authorization_data).size(),
                seek);
    }

    return result;
}

// split (body | request)

kuic::handshake::kbr_kdc_request::kbr_kdc_request() { }

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::build_as_request(kuic::handshake::kbr_principal_name client_name, std::string realm, unsigned int nonce) {
    
    kuic::handshake::kbr_kdc_request as_req;

    as_req.version = kuic::kbr_current_protocol_version;
    as_req.message_type = kuic::handshake::kbr_kdc_as_request;
    as_req.body = kuic::handshake::kbr_kdc_request_body(
            kuic::handshake::kbr_kdc_as_request, client_name, realm, nonce);

    return as_req;
}

kuic::handshake::handshake_message
kuic::handshake::kbr_kdc_request::__serialize() const {        
    // declare result msg
    kuic::handshake::handshake_message msg;
    
    // set tag
    if (this->message_type == kuic::handshake::kbr_kdc_as_request) {
        msg.set_tag(kuic::handshake::tag_kbr_as_request);
    }
    else if (this->message_type == kuic::handshake::tag_kbr_tgs_request) {
        msg.set_tag(kuic::handshake::tag_kbr_tgs_request);
    }
    else {
        return handshake::handshake_message(kuic::not_expect);
    }

    
    // declare temporary
    size_t size = 0;
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    std::unique_ptr<kuic::byte_t []> serialized_buffer;
    
    // serialize version
    std::tie(serialized_buffer_ptr, size) = kuic::handshake::kbr_protocol_version_serializer::serialize(this->version);
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_protocol_version, serialized_buffer.get(), size);

    // serialize message type
    std::tie(serialized_buffer_ptr, size) = kuic::handshake::kbr_message_type_serializer::serialize(this->message_type);
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_message_type, serialized_buffer.get(), size);
    
    // serialize body
    std::tie(serialized_buffer_ptr, size) = this->body.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    msg.insert(kuic::handshake::tag_kbr_kdc_request_body, serialized_buffer.get(), size);

    return msg;
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::__deserialize(kuic::handshake::handshake_message &msg) { 
    kuic::handshake::kbr_kdc_request ret;
    // check msg tag (AS | TGS)
    if (msg.get_tag() != kuic::handshake::tag_kbr_as_request) {
        return ret;
    }

    size_t seek = 0;
    // deserialize protocol version
    if (msg.exist(kuic::handshake::tag_protocol_version)) {
        ret.version = kuic::handshake::kbr_protocol_version_serializer::deserialize(
                msg.get(kuic::handshake::tag_protocol_version).data(),
                msg.get(kuic::handshake::tag_protocol_version).size(),
                seek);
    }

    // deserialize message type
    if (msg.exist(kuic::handshake::tag_message_type)) {
        seek = 0;
        ret.message_type = kuic::handshake::kbr_message_type_serializer::deserialize(
                msg.get(kuic::handshake::tag_message_type).data(),
                msg.get(kuic::handshake::tag_message_type).size(),
                seek);
    }
    
    if (msg.exist(kuic::handshake::tag_kbr_kdc_request_body)) {
        seek = 0;
        ret.body = kuic::handshake::kbr_kdc_request_body::deserialize(
                msg.get(kuic::handshake::tag_kbr_kdc_request_body).data(),
                msg.get(kuic::handshake::tag_kbr_kdc_request_body).size(),
                seek);
    }

    return ret;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_kdc_request::serialize() const {
    return this->__serialize().serialize();
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::deserialize(kuic::byte_t *buffer, const size_t len, size_t &seek) {
    kuic::handshake::handshake_message msg = kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    return kuic::handshake::kbr_kdc_request::__deserialize(msg);
}

kuic::handshake::kbr_kdc_request_body
kuic::handshake::kbr_kdc_request::get_body() const {
    return this->body;
}

kuic::kbr_message_type_t
kuic::handshake::kbr_kdc_request::get_message_type() const {
    return this->message_type;
}
