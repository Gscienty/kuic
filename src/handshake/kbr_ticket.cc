#include "handshake/kbr_ticket.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_ticket_body::serialize() const {
    // declare temporary msg
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_ticket);
    
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
