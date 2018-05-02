#ifndef _KUIC_HANDSHAKE_HANDSHAKE_MESSAGE_
#define _KUIC_HANDSHAKE_HANDSHAKE_MESSAGE_

#include "type.h"
#include "handshake/lawful_package.h"
#include "handshake/package_serializer.h"
#include "eys.h"
#include <utility>
#include <map>
#include <vector>

namespace kuic {
    namespace handshake {
        class handshake_message : public package_serializer, public lawful_package {
        private:
            kuic::tag_t tag;
            std::map<kuic::tag_t, std::vector<kuic::byte_t> > data;

            handshake_message(kuic::tag_t tag, std::map<kuic::tag_t, std::vector<kuic::byte_t> > &data);
        public:
            handshake_message();
            handshake_message(kuic::error_t err);
            handshake_message(kuic::tag_t tag);

            void insert(kuic::tag_t tag, kuic::byte_t *data, size_t size);

            static handshake_message
                parse_handshake_message(eys::in_buffer &reader);
            static handshake_message
                deserialize(kuic::byte_t *buffer, size_t len, size_t &seek);

            virtual std::pair<kuic::byte_t *, size_t> serialize() const;
            
            std::vector<kuic::tag_t> get_tags_sorted() const;
            std::vector<kuic::byte_t> &get(kuic::tag_t tag);
            bool exist(kuic::tag_t tag) const;
            kuic::tag_t get_tag() const;
        };
    }
}

#endif
