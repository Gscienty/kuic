#ifndef _KUIC_HANDSHAKE_HANDSHAKE_MESSAGE_
#define _KUIC_HANDSHAKE_HANDSHAKE_MESSAGE_

#include "type.h"
#include "eys.h"
#include <utility>
#include <map>
#include <vector>

namespace kuic {
    namespace handshake {
        class handshake_message {
        private:
            kuic::tag_t tag;
            std::map<kuic::tag_t, std::vector<kuic::byte_t> > data;

            handshake_message(kuic::tag_t tag, std::map<kuic::tag_t, std::vector<kuic::byte_t> > &data);
        public:
            handshake_message();
            
            static std::pair<handshake_message, kuic::error_t>
            parse_handshake_message(eys::in_buffer &reader);
            std::vector<kuic::byte_t> serialize();
            std::vector<kuic::tag_t> get_tags_sorted() const;

            std::vector<kuic::byte_t> &get(kuic::tag_t tag);
        };
    }
}

#endif