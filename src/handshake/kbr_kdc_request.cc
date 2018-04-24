#include "handshake/kbr_kdc_request.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"
#include "little_endian_serializer.h"
#include "clock.h"
#include "define.h"
#include <memory>

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

kuic::handshake::handshake_message
kuic::handshake::kbr_kdc_request::serialize() {
    if (this->message_type == kuic::handshake::kbr_kdc_as_request) {
        kuic::handshake::handshake_message msg(kuic::handshake::tag_kbr_as_request);
        size_t size;
        std::unique_ptr<kuic::byte_t []> serialize_buffer(
            kuic::handshake::kbr_protocol_version_serializer::serialize(this->version, size));

        msg.insert(kuic::handshake::tag_protocol_version, serialize_buffer.get(), size);

        serialize_buffer = std::unique_ptr<kuic::byte_t[]>(
            kuic::handshake::kbr_message_type_serializer::serialize(this->message_type, size));
        msg.insert(kuic::handshake::tag_message_type, serialize_buffer.get(), size);

        serialize_buffer = std::unique_ptr<kuic::byte_t[]>(this->client_name.serialize(size));
        msg.insert(kuic::handshake::tag_client_principal_name, serialize_buffer.get(), size);

        serialize_buffer = std::unique_ptr<kuic::byte_t[]>(new kuic::byte_t[this->realm.length()]);
        std::copy(this->realm.begin(), this->realm.end(), serialize_buffer.get());
        msg.insert(kuic::handshake::tag_realm, serialize_buffer.get(), this->realm.length());

        

        return msg;
    }
    else {

        return kuic::handshake:handshake_message();
    }

    return kuic::handshake:handshake_message();
}