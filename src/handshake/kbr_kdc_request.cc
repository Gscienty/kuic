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

void kuic::handshake::kbr_kdc_request_body::support_encrypt_type(
        kuic::kbr_encryption_type_t encryption_type) {
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

    // serialize nonce
    temporary_msg.insert<unsigned int, eys::bigendian_serializer<kuic::byte_t, unsigned int>>(
            kuic::handshake::tag_nonce, this->nonce);
    // serialize from timestamp
    temporary_msg.insert(kuic::handshake::tag_time_from, this->from);
    // serialize till
    temporary_msg.insert(kuic::handshake::tag_time_till, this->till);
    // serialize renew time
    temporary_msg.insert(kuic::handshake::tag_renew_till_time, this->renew_time);
    // serialize encryption types
    temporary_msg.insert_elements<
        kuic::kbr_encryption_type_t, kuic::handshake::kbr_encryption_type_serializer>(
            kuic::handshake::tag_encrypt_type, this->encrypt_types);
    
    // AS request
    if (this->message_type == kuic::handshake::kbr_kdc_as_request) {
        // serialize client principal
        temporary_msg.insert(kuic::handshake::tag_client_principal_name, this->client_name);
        // serialize realm (client realm)
        temporary_msg.insert(kuic::handshake::tag_client_realm, this->realm);
    }
    // TGS request
    else if (this->message_type == kuic::handshake::kbr_kdc_tgs_request) {
        // serialize server principal
        temporary_msg.insert(kuic::handshake::tag_server_principal_name, this->server_name);
        // serialize realm (server realm)
        temporary_msg.insert(kuic::handshake::tag_server_realm, this->realm);
        // serialize authorization data
        temporary_msg.insert(kuic::handshake::tag_authorization_data, this->authorization_data);
    }

    return temporary_msg.serialize();
}

kuic::handshake::kbr_kdc_request_body
kuic::handshake::kbr_kdc_request_body::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::handshake_message temporary_msg = 
        kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_kdc_request_body(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_kbr_kdc_request_body) {
        return kuic::handshake::kbr_kdc_request_body(kuic::not_expect);
    }
    
    // declare result
    kuic::handshake::kbr_kdc_request_body result;

    // deserialize nonce
    temporary_msg.assign<unsigned int, eys::bigendian_serializer<kuic::byte_t, unsigned int>>(
            result.nonce, kuic::handshake::tag_nonce);
    // deserialize from
    temporary_msg.assign(result.from, kuic::handshake::tag_time_from);
    // deserialize till
    temporary_msg.assign(result.till, kuic::handshake::tag_time_till);
    // deserialize renew
    temporary_msg.assign(result.renew_time, kuic::handshake::tag_renew_till_time);
    // deserialize encryption types
    temporary_msg.assign_elements<
        kuic::kbr_encryption_type_t, kuic::handshake::kbr_encryption_type_serializer>(
                result.encrypt_types, kuic::handshake::tag_encrypt_type);
    // deserialize client principal
    temporary_msg.assign(result.client_name, kuic::handshake::tag_client_principal_name);
    // deserialize server principal
    temporary_msg.assign(result.server_name, kuic::handshake::tag_server_principal_name);
    // deserialize realm (client || server)
    temporary_msg.assign(result.realm, kuic::handshake::tag_server_realm);
    temporary_msg.assign(result.realm, kuic::handshake::tag_client_realm);
    // deserialize authorization data
    temporary_msg.assign(result.authorization_data, kuic::handshake::tag_authorization_data);

    return result;
}

// split (body | request)

kuic::handshake::kbr_kdc_request::kbr_kdc_request() { }

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::build_as_request(
        kuic::handshake::kbr_principal_name client_name,
        std::string realm,
        unsigned int nonce) {
    
    kuic::handshake::kbr_kdc_request as_req;

    as_req.version = kuic::kbr_current_protocol_version;
    as_req.message_type = kuic::handshake::kbr_kdc_as_request;
    as_req.body = kuic::handshake::kbr_kdc_request_body(
            kuic::handshake::kbr_kdc_as_request, client_name, realm, nonce);

    return as_req;
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::build_tgs_request(
        kbr_principal_name server_name,
        std::string realm,
        unsigned int nonce) {
    
    kuic::handshake::kbr_kdc_request tgs_req;
    
    tgs_req.version = kuic::kbr_current_protocol_version;
    tgs_req.message_type = kuic::handshake::kbr_kdc_tgs_request;
    tgs_req.body = kuic::handshake::kbr_kdc_request_body(
            kuic::handshake::kbr_kdc_tgs_request, server_name, realm, nonce);

    return tgs_req;
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

    // serialize versioin
    msg.insert<kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
            kuic::handshake::tag_protocol_version, this->version);
    // serialize message type
    msg.insert<kuic::kbr_message_type_t, kuic::handshake::kbr_message_type_serializer>(
            kuic::handshake::tag_message_type, this->message_type);
    // serialize body
    msg.insert(kuic::handshake::tag_kbr_kdc_request_body, this->body);

    return msg;
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::__deserialize(kuic::handshake::handshake_message &msg) { 
    kuic::handshake::kbr_kdc_request result;
    // check msg tag (AS | TGS)
    if (msg.get_tag() != kuic::handshake::tag_kbr_as_request) {
        return result;
    }

    // deserialize protocol version
    msg.assign<kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
            result.version, kuic::handshake::tag_protocol_version);
    // deserialize message type
    msg.assign<kuic::kbr_message_type_t, kuic::handshake::kbr_message_type_serializer>(
            result.message_type, kuic::handshake::tag_message_type);
    // deserialize request body
    msg.assign(result.body, kuic::handshake::tag_kbr_kdc_request_body);

    return result;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_kdc_request::serialize() const {
    return this->__serialize().serialize();
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::deserialize(
        const kuic::byte_t *buffer, const size_t len, size_t &seek) {

    kuic::handshake::handshake_message msg =
        kuic::handshake::handshake_message::deserialize(buffer, len, seek);
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
