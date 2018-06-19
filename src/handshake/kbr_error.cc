#include "handshake/kbr_error.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_error::serialize() const {
    kuic::handshake::handshake_message temporary_message(kuic::handshake::tag_error);
    
    // serialize protocol version
    temporary_message.insert<
        kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
                kuic::handshake::tag_protocol_version, this->version);
    // serialize message type
    temporary_message.insert<
        kuic::kbr_message_type_t, kuic::handshake::kbr_message_type_serializer>(
                kuic::handshake::tag_message_type, this->message_type);
    // serialize current clock
    temporary_message.insert(kuic::handshake::tag_current_clock, this->current_clock);
    // serialize server clock
    temporary_message.insert(kuic::handshake::tag_server_clock, this->server_clock);
    // serialize error code
    temporary_message.insert(kuic::handshake::tag_error_code, this->error);
    // serialize client name
    temporary_message.insert(kuic::handshake::tag_client_principal_name, this->client_name);
    // serialize client realm
    temporary_message.insert(kuic::handshake::tag_client_realm, this->client_realm);
    // serialize server realm
    temporary_message.insert(kuic::handshake::tag_server_realm, this->server_realm);
    // serialize server name
    temporary_message.insert(kuic::handshake::tag_server_principal_name, this->server_name);
    // serialize error string
    temporary_message.insert(kuic::handshake::tag_error_string, this->error_string);

    return temporary_message.serialize();
}

kuic::handshake::kbr_error
kuic::handshake::kbr_error::deserialize(kuic::handshake::handshake_message &msg) {
    kbr_error result;
    
    // deserialize version
    msg.assign<
        kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
                result.version, kuic::handshake::tag_protocol_version);
    // deserialize message type
    msg.assign<
        kuic::kbr_message_type_t, kuic::handshake::kbr_message_type_serializer>(
                result.message_type, kuic::handshake::tag_message_type);
    // deserialize current clock
    msg.assign(result.current_clock, kuic::handshake::tag_current_clock);
    // deserialize server clock
    msg.assign(result.server_clock, kuic::handshake::tag_server_clock);
    // deserialize error
    msg.assign<
        kuic::error_t, kuic::handshake::kbr_error_code_serializer>(
                result.error, kuic::handshake::tag_error_code);
    // deserialize client name
    msg.assign(result.client_name, kuic::handshake::tag_client_principal_name);
    // deserialize client realm
    msg.assign(result.client_realm, kuic::handshake::tag_client_realm);
    // deserialize server realm
    msg.assign(result.server_realm, kuic::handshake::tag_server_realm);
    // deserialize error string
    msg.assign(result.error_string, kuic::handshake::tag_error_string);

    return result;
}

