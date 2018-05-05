#include "handshake/kbr_ticket.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "handshake/serializer.h"

kuic::handshake::kbr_ticket_body::kbr_ticket_body() { }

kuic::handshake::kbr_ticket_body::kbr_ticket_body(kuic::error_t err)
    : lawful_package(err) { }

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_ticket_body::serialize() const {
    // declare temporary msg
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_ticket_body);
    
    // serialize key
    temporary_msg.insert(kuic::handshake::tag_key,                      this->key           );
    // serialize client name
    temporary_msg.insert(kuic::handshake::tag_client_principal_name,    this->client_name   );
    // serialize realm
    temporary_msg.insert(kuic::handshake::tag_client_realm,             this->client_realm  );
    // serialize auth time
    temporary_msg.insert(kuic::handshake::tag_time_auth,                this->auth_time     );
    // serialize start time
    temporary_msg.insert(kuic::handshake::tag_time_start,               this->start_time    );
    // serialize end time
    temporary_msg.insert(kuic::handshake::tag_time_end,                 this->end_time      );
    // serialize renew till
    temporary_msg.insert(kuic::handshake::tag_renew_till_time,          this->renew_till    );

    return temporary_msg.serialize();
}

kuic::handshake::kbr_ticket_body
kuic::handshake::kbr_ticket_body::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::handshake_message temporary_msg =
        kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    
    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_ticket_body(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_ticket_body) {
        return kuic::handshake::kbr_ticket_body(kuic::not_expect);
    }

    kuic::handshake::kbr_ticket_body result;

    // deserialize key
    temporary_msg.assign(result.key,            kuic::handshake::tag_key                    );
    // deserialize client name
    temporary_msg.assign(result.client_name,    kuic::handshake::tag_client_principal_name  );
    // deserialize realm
    temporary_msg.assign(result.client_realm,   kuic::handshake::tag_client_realm           );
    // deserialize auth time
    temporary_msg.assign(result.auth_time,      kuic::handshake::tag_time_auth              );
    // deserialize start time
    temporary_msg.assign(result.start_time,     kuic::handshake::tag_time_start             );
    // deserialize end time
    temporary_msg.assign(result.end_time,       kuic::handshake::tag_time_end               );
    // deserialize renew till
    temporary_msg.assign(result.renew_till,     kuic::handshake::tag_renew_till_time        );

    return result;
}

kuic::handshake::kbr_ticket::kbr_ticket() { }

kuic::handshake::kbr_ticket::kbr_ticket(kuic::error_t err)
    : lawful_package(err) { }

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_ticket::serialize() const {
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_ticket);

    // serialize version
    temporary_msg.insert<
        kuic::kbr_key_version_t, kuic::handshake::kbr_key_version_serializer>(
                kuic::handshake::tag_protocol_version, this->version);
    // serialize realm
    temporary_msg.insert(kuic::handshake::tag_issue_realm,  this->realm         );
    // serialize server name
    temporary_msg.insert(kuic::handshake::tag_server_realm, this->server_name   );
    // serialize encrypted data
    temporary_msg.insert(kuic::handshake::tag_encrypt_type, this->encrypted_data);

    return temporary_msg.serialize();
}

kuic::handshake::kbr_ticket
kuic::handshake::kbr_ticket::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::handshake_message temporary_msg = 
        kuic::handshake::handshake_message::deserialize(buffer, len, seek);

    if (temporary_msg.is_lawful() == false) {
        return kuic::handshake::kbr_ticket(temporary_msg.get_error());
    }
    if (temporary_msg.get_tag() != kuic::handshake::tag_ticket) {
        return kuic::handshake::kbr_ticket(kuic::not_expect);
    }

    kuic::handshake::kbr_ticket result;
    
    // deserialize version
    temporary_msg.assign<
        kuic::kbr_protocol_version_t, kuic::handshake::kbr_protocol_version_serializer>(
                result.version, kuic::handshake::tag_protocol_version);
    // deserialize realm
    temporary_msg.assign(result.realm, kuic::handshake::tag_issue_realm);
    // deserialize server name
    temporary_msg.assign(result.server_name, kuic::handshake::tag_server_principal_name);
    // deserialize encrypted data
    temporary_msg.assign(result.encrypted_data, kuic::handshake::tag_encrypted_data);

    return result;
}

