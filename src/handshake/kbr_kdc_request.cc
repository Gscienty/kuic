#include "handshake/kbr_kdc_request.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"
#include "clock.h"
#include "define.h"
#include <memory>

kuic::handshake::kbr_kdc_request::kbr_kdc_request() {
}

kuic::handshake::kbr_kdc_request
kuic::handshake::kbr_kdc_request::build_as_request(kuic::handshake::kbr_principal_name client_name, std::string realm, unsigned int nonce) {
    
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
kuic::handshake::kbr_kdc_request::__serialize() const {
    if (this->message_type == kuic::handshake::kbr_kdc_as_request) {
        kuic::handshake::handshake_message msg(kuic::handshake::tag_kbr_as_request);
        
        // declare temporary
        size_t size = 0;
        kuic::byte_t *serialized_buffer_ptr = nullptr;
        std::unique_ptr<kuic::byte_t> serialized_buffer;

        // version
        std::tie(serialized_buffer_ptr, size) = kuic::handshake::kbr_protocol_version_serializer::serialize(this->version);
        serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
        msg.insert(kuic::handshake::tag_protocol_version, serialized_buffer.get(), size);

        // message type
        std::tie(serialized_buffer_ptr, size) = kuic::handshake::kbr_message_type_serializer::serialize(this->message_type);
        serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
        msg.insert(kuic::handshake::tag_message_type, serialized_buffer.get(), size);
        
        // client princpal
        std::tie(serialized_buffer_ptr, size) = this->client_name.serialize();
        serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
        msg.insert(kuic::handshake::tag_client_principal_name, serialized_buffer.get(), size);

        // realm
        serialized_buffer = std::unique_ptr<kuic::byte_t>(new kuic::byte_t[this->realm.length()]);
        std::copy(this->realm.begin(), this->realm.end(), serialized_buffer.get());
        msg.insert(kuic::handshake::tag_client_realm, serialized_buffer.get(), this->realm.length());

        // encryption types
        serialized_buffer = std::unique_ptr<kuic::byte_t>(new kuic::byte_t[this->encrypt_types.size() * sizeof(kuic::kbr_encryption_type_t)]);
        std::copy(this->encrypt_types.begin(), this->encrypt_types.end(), reinterpret_cast<kuic::kbr_encryption_type_t *>(serialized_buffer.get()));
        msg.insert(kuic::handshake::tag_encrypt_type, serialized_buffer.get(), this->encrypt_types.size() * sizeof(kuic::kbr_encryption_type_t));
        
        // from timestamp
        std::tie(serialized_buffer_ptr, size) = this->from.serialize();
        serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
        msg.insert(kuic::handshake::tag_time_from, serialized_buffer.get(), size);

        // till timestamp
        std::tie(serialized_buffer_ptr, size) = this->till.serialize();
        serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
        msg.insert(kuic::handshake::tag_time_till, serialized_buffer.get(), size);

        // nonce
        std::tie(serialized_buffer_ptr, size) = eys::bigendian_serializer<kuic::byte_t, unsigned int>::serialize(this->nonce);
        serialized_buffer = std::unique_ptr<kuic::byte_t>(serialized_buffer_ptr);
        msg.insert(kuic::handshake::tag_nonce, serialized_buffer.get(), size);

        return msg;
    }
    else {
        // TODO
        return kuic::handshake::handshake_message();
    }

    return kuic::handshake::handshake_message(kuic::not_expect);
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

    // deserialize client principal name
    if (msg.exist(kuic::handshake::tag_client_principal_name)) {
        seek = 0;
        ret.client_name = kuic::handshake::kbr_principal_name::deserialize(
                msg.get(kuic::handshake::tag_client_principal_name).data(),
                msg.get(kuic::handshake::tag_client_principal_name).size(),
                seek);
    }

    // deserialize realm
    if (msg.exist(kuic::handshake::tag_client_realm)) {
        ret.realm = std::string(
                msg.get(kuic::handshake::tag_client_realm).begin(),
                msg.get(kuic::handshake::tag_client_realm).end());
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
                msg.get(kuic::handshake::tag_time_from).data(),
                msg.get(kuic::handshake::tag_time_from).size(),
                seek);
    }

    // deserialize till
    if (msg.exist(kuic::handshake::tag_time_till)) {
        seek = 0;
        ret.till = kuic::special_clock::deserialize(
                msg.get(kuic::handshake::tag_time_till).data(),
                msg.get(kuic::handshake::tag_time_till).size(),
                seek);
    }

    // deserialize nonce
    if (msg.exist(kuic::handshake::tag_nonce)) {
        seek = 0;
        ret.nonce = eys::bigendian_serializer<kuic::byte_t, unsigned int>::deserialize( 
                msg.get(kuic::handshake::tag_nonce).data(),
                msg.get(kuic::handshake::tag_nonce).size(),
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
    kuic::handshake::handshake_message msg = 
        kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    return kuic::handshake::kbr_kdc_request::__deserialize(msg);
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
