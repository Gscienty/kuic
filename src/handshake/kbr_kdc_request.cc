#include "handshake/kbr_kdc_request.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"
#include "little_endian_serializer.h"
#include "clock.h"
#include "define.h"
#include <memory>

kuic::handshake::kbr_kdc_request::kbr_kdc_request() {
}

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
    as_req.till = as_req.from + kuic::clock_second;
    as_req.nonce = nonce;

    return as_req;
}

kuic::handshake::handshake_message
kuic::handshake::kbr_kdc_request::serialize() {
    if (this->message_type == kuic::handshake::kbr_kdc_as_request) {
        kuic::handshake::handshake_message msg(kuic::handshake::tag_kbr_as_request);
        size_t size;

        // version
        std::unique_ptr<kuic::byte_t> serialize_buffer(
                reinterpret_cast<kuic::byte_t *>(kuic::handshake::kbr_protocol_version_serializer::serialize(this->version, size)));
        msg.insert(kuic::handshake::tag_protocol_version, serialize_buffer.get(), size);

        // message type
        serialize_buffer = std::unique_ptr<kuic::byte_t>(
                reinterpret_cast<kuic::byte_t *>(kuic::handshake::kbr_message_type_serializer::serialize(this->message_type, size)));
        msg.insert(kuic::handshake::tag_message_type, serialize_buffer.get(), size);
        
        // client princpal
        serialize_buffer = std::unique_ptr<kuic::byte_t>(
                reinterpret_cast<kuic::byte_t *>(this->client_name.serialize(size)));
        msg.insert(kuic::handshake::tag_client_principal_name, serialize_buffer.get(), size);

        // realm
        serialize_buffer = std::unique_ptr<kuic::byte_t>(new kuic::byte_t[this->realm.length()]);
        std::copy(this->realm.begin(), this->realm.end(), serialize_buffer.get());
        msg.insert(kuic::handshake::tag_realm, serialize_buffer.get(), this->realm.length());

        // encryption types
        serialize_buffer = std::unique_ptr<kuic::byte_t>(new kuic::byte_t[this->encrypt_types.size() * sizeof(kuic::kbr_encryption_type_t)]);
        std::copy(this->encrypt_types.begin(), this->encrypt_types.end(), reinterpret_cast<kuic::kbr_encryption_type_t *>(serialize_buffer.get()));
        msg.insert(kuic::handshake::tag_encrypt_type, serialize_buffer.get(), this->encrypt_types.size() * sizeof(kuic::kbr_encryption_type_t));
        
        // from timestamp
        serialize_buffer = std::unique_ptr<kuic::byte_t>(
                reinterpret_cast<kuic::byte_t *>(this->from.timestamp_serialize(size)));
        msg.insert(kuic::handshake::tag_time_from, serialize_buffer.get(), size);

        // till timestamp
        serialize_buffer = std::unique_ptr<kuic::byte_t>(
                reinterpret_cast<kuic::byte_t *>(this->till.timestamp_serialize(size)));
        msg.insert(kuic::handshake::tag_time_till, serialize_buffer.get(), size);

        // nonce
        serialize_buffer = std::unique_ptr<kuic::byte_t>(
                reinterpret_cast<kuic::byte_t *>(eys::serializer<unsigned int>::serialize(this->nonce, size)));
        msg.insert(kuic::handshake::tag_nonce, serialize_buffer.get(), size);

        return msg;
    }
    else {

        return kuic::handshake::handshake_message();
    }

    return kuic::handshake::handshake_message();
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::deserialize(kuic::handshake::handshake_message &msg) { 
    kuic::handshake::kbr_kdc_request ret;
    // check msg tag (AS | TGS)
    if (msg.get_tag() != kuic::handshake::tag_kbr_as_request) {
        return ret;
    }
    
    ssize_t seek = 0;
    // deserialize protocol version
    if (msg.exist(kuic::handshake::tag_protocol_version)) {
        ret.version = kuic::handshake::kbr_protocol_version_serializer::deserialize(
                reinterpret_cast<char *>(msg.get(kuic::handshake::tag_protocol_version).data()),
                msg.get(kuic::handshake::tag_protocol_version).size(),
                seek);
    }

    // deserialize message type
    if (msg.exist(kuic::handshake::tag_message_type)) {
        seek = 0;
        ret.message_type = kuic::handshake::kbr_message_type_serializer::deserialize(
                reinterpret_cast<char *>(msg.get(kuic::handshake::tag_message_type).data()),
                msg.get(kuic::handshake::tag_message_type).size(),
                seek);
    }

    // deserialize client principal name
    if (msg.exist(kuic::handshake::tag_client_principal_name)) {
        ret.client_name = kuic::handshake::kbr_principal_name::deserialize(
                reinterpret_cast<char *>(
                    msg.get(kuic::handshake::tag_client_principal_name).data()),
                msg.get(kuic::handshake::tag_client_principal_name).size());
    }

    // deserialize realm
    if (msg.exist(kuic::handshake::tag_realm)) {
        ret.realm = std::string(
                msg.get(kuic::handshake::tag_realm).begin(),
                msg.get(kuic::handshake::tag_realm).end());
    }

    // deserialize encrypt type
    if (msg.exist(kuic::handshake::tag_encrypt_type)) {
        ret.encrypt_types.assign(
                msg.get(kuic::handshake::tag_encrypt_type).begin(),
                msg.get(kuic::handshake::tag_encrypt_type).end());
    }

    // deserialize from
    if (msg.exist(kuic::handshake::tag_time_from)) {
        seek = 0;
        ret.from = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    msg.get(kuic::handshake::tag_time_from).data()),
                msg.get(kuic::handshake::tag_time_from).size(),
                seek);
    }

    // deserialize till
    if (msg.exist(kuic::handshake::tag_time_till)) {
        seek = 0;
        ret.till = kuic::special_clock::deserialize(
                reinterpret_cast<char *>(
                    msg.get(kuic::handshake::tag_time_till).data()),
                msg.get(kuic::handshake::tag_time_till).size(),
                seek);
    }

    // deserialize nonce
    if (msg.exist(kuic::handshake::tag_nonce)) {
        seek = 0;
        ret.nonce = eys::deserializer<unsigned int>::deserialize(
                reinterpret_cast<char *>(
                    msg.get(kuic::handshake::tag_nonce).data()),
                msg.get(kuic::handshake::tag_nonce).size(),
                seek);
    }

    return ret;
}

kuic::kbr_protocol_version_t 
kuic::handshake::kbr_kdc_request::get_version() const {
    return this->version;
}

kuic::kbr_message_type_t
kuic::handshake::kbr_kdc_request::get_message_type() const {
    return this->message_type;
}

kuic::handshake::kbr_principal_name 
kuic::handshake::kbr_kdc_request::get_client_name() const {
    return this->client_name;
}

std::string 
kuic::handshake::kbr_kdc_request::get_realm() const {
    return this->realm;
}

kuic::handshake::kbr_principal_name 
kuic::handshake::kbr_kdc_request::get_server_name() const {
    return this->server_name;
}

kuic::special_clock
kuic::handshake::kbr_kdc_request::get_from() const {
    return this->from;
}

kuic::special_clock
kuic::handshake::kbr_kdc_request::get_till() const {
    return this->till;
}

kuic::special_clock 
kuic::handshake::kbr_kdc_request::get_rtime() const {
    return this->rtime;
}

unsigned int
kuic::handshake::kbr_kdc_request::get_nonce() const {
    return this->nonce;
}

void kuic::handshake::kbr_kdc_request::support_encrypt_type(
        kuic::kbr_encryption_type_t encryption_type) {
    this->encrypt_types.push_back(encryption_type);
}
