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

kuic::handshake::kbr_kdc_response_part::kbr_kdc_response_part(kuic::error_t err)
    : lawful_package(err) { }

kuic::handshake::kbr_kdc_response_part::kbr_kdc_response_part(unsigned int nonce)
    : nonce(nonce)
    , auth_time(kuic::current_clock())
    , start_time(kuic::current_clock())
    , end_time(kuic::special_clock(kuic::current_clock()) + 60 * kuic::clock_second) { }

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
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_kbr_response_body);

    // serialize key
    temporary_msg.insert(kuic::handshake::tag_key, this->key);
    // serialize nonce
    temporary_msg.insert<unsigned int, eys::bigendian_serializer<kuic::byte_t, unsigned int>>(
                kuic::handshake::tag_nonce, this->nonce);
    // serialize auth time
    temporary_msg.insert(kuic::handshake::tag_time_auth,                this->auth_time         );
    // serialize start time
    temporary_msg.insert(kuic::handshake::tag_time_start,               this->start_time        );
    // serialize end time
    temporary_msg.insert(kuic::handshake::tag_time_end,                 this->end_time          );
    // serialize time renew till
    temporary_msg.insert(kuic::handshake::tag_renew_till_time,          this->renew_till        );
    // serialize key expiration
    temporary_msg.insert(kuic::handshake::tag_time_key_expiration,      this->key_expiration    );
    // serialize server realm
    temporary_msg.insert(kuic::handshake::tag_server_realm,             this->server_realm      );
    // serialize server name
    temporary_msg.insert(kuic::handshake::tag_server_principal_name,    this->server_name       );

    // serialize temporary handshake_message
    // you should encrypt the result
    // (byte-string) in other function.
    return temporary_msg.serialize();
}

kuic::handshake::kbr_kdc_response_part
kuic::handshake::kbr_kdc_response_part::deserialize(
        const kuic::byte_t *buffer, size_t size, size_t &seek) {
    kuic::handshake::handshake_message temporary_msg =
        kuic::handshake::handshake_message::deserialize(buffer, size, seek);

    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_kdc_response_part(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_kbr_response_body) {
        return kuic::handshake::kbr_kdc_response_part(kuic::not_expect);
    }

    // declare result
    kuic::handshake::kbr_kdc_response_part result;
    // deserialize secret key
    temporary_msg.assign(result.key, kuic::handshake::tag_key);
    // deserialize nonce
    temporary_msg.assign<
        unsigned int,
        eys::bigendian_serializer<kuic::byte_t, unsigned int>>(
                result.nonce, kuic::handshake::tag_nonce);
    
    // deserialize auth time
    temporary_msg.assign(result.auth_time,      kuic::handshake::tag_time_auth              );
    // deserialize start time
    temporary_msg.assign(result.start_time,     kuic::handshake::tag_time_start             );
    // deserialize end time
    temporary_msg.assign(result.end_time,       kuic::handshake::tag_time_end               );
    // deserialize renew till
    temporary_msg.assign(result.renew_till,     kuic::handshake::tag_time_till              );
    // deserialize key expiration
    temporary_msg.assign(result.key_expiration, kuic::handshake::tag_time_key_expiration    );
    // deserialize server realm
    temporary_msg.assign(result.server_realm,   kuic::handshake::tag_server_realm           );
    // deserialize server name
    temporary_msg.assign(result.server_name,    kuic::handshake::tag_server_principal_name  );

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
        kuic::handshake::kbr_kdc_response_part &part,
        kuic::handshake::kbr_ticket &ticket) {
    kuic::handshake::kbr_kdc_response ret;
    
    // set version
    ret.version = kuic::kbr_current_protocol_version;
    // set message type (AS request)
    ret.message_type = kuic::handshake::kbr_kdc_as_response;
    // set realm
    ret.realm = realm;
    // set ticket
    ret.ticket = ticket;

    // crypt response enc-part
    size_t serialized_part_buffer_size = 0;
    kuic::byte_t *serialized_part_buffer_ptr = nullptr;
    std::tie(serialized_part_buffer_ptr, serialized_part_buffer_size) = part.serialize();
    std::unique_ptr<kuic::byte_t> serialized_part_buffer(serialized_part_buffer_ptr);

    // construct encrypted data
    kuic::handshake::kbr_encrypted_data encrypted_data(0x00000000, encryption_type);
    encrypted_data.set_plain_message(serialized_part_buffer.get(), serialized_part_buffer_size, secret_key, secret_key_size);
    ret.encrypted_data = encrypted_data;

    return ret;
}

kuic::handshake::handshake_message
kuic::handshake::kbr_kdc_response::__serialize() const {
    kuic::handshake::handshake_message result;

    if (this->message_type == kuic::handshake::kbr_kdc_as_response) {
        result.set_tag(kuic::handshake::tag_kbr_tgt);
    }
    else if (this->message_type == kuic::handshake::kbr_kdc_tgs_response) {
        result.set_tag(kuic::handshake::tag_kbr_tgs);
    }
    else {
        result.set_error(not_expect);
        return result;
    }

    // serialize version
    result.insert<kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
            kuic::handshake::tag_protocol_version, this->version);
    // serialize message type
    result.insert<kuic::kbr_message_type_t, kuic::handshake::kbr_message_type_serializer>(
            kuic::handshake::tag_message_type,                  this->message_type  );
    // serialize realm
    result.insert(kuic::handshake::tag_client_realm,            this->realm         );
    // serialize client name
    result.insert(kuic::handshake::tag_client_principal_name,   this->client_name   );
    // serialize encrypted data
    result.insert(kuic::handshake::tag_encrypted_data,          this->encrypted_data);

    return result;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_kdc_response::serialize() const {
    return this->__serialize().serialize();
}

kuic::handshake::kbr_kdc_response
kuic::handshake::kbr_kdc_response::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {

    kuic::handshake::handshake_message temporary_msg = 
        kuic::handshake::handshake_message::deserialize(buffer, len, seek);

    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_kdc_response(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_kbr_tgt &&
            temporary_msg.get_tag() != kuic::handshake::tag_kbr_tgs) {
        return kuic::handshake::kbr_kdc_response(kuic::not_expect);
    }
    
    kuic::handshake::kbr_kdc_response result;
    
    // deserialize version
    temporary_msg.assign<kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
            result.version, kuic::handshake::tag_protocol_version);
    // deserialize message type
    temporary_msg.assign<kuic::kbr_message_type_t, kuic::handshake::kbr_message_type_serializer>(
            result.message_type, kuic::handshake::tag_message_type);
    // deserialize realm
    temporary_msg.assign(result.realm,          kuic::handshake::tag_client_realm           );
    // deserialize client name
    temporary_msg.assign(result.client_name,    kuic::handshake::tag_client_principal_name  );
    // encrypted data
    temporary_msg.assign(result.encrypted_data, kuic::handshake::tag_encrypted_data         );

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
