#ifndef _KUIC_HANDSHAKE_PACKAGE_SERIALIZER_
#define _KUIC_HANDSHAKE_PACKAGE_SERIALIZER_

#include "type.h"
#include <utility>
#include <unistd.h>
#include <string>
#include <algorithm>

namespace kuic {
    class package_serializable {
    public:
        virtual std::pair<kuic::byte_t *, size_t> serialize() const;
    };

    template <typename EntityType>
    struct package_serializer {
        static
        std::pair<kuic::byte_t *, size_t>
        serialize(const EntityType &entity) {
            return entity.serialize();
        }

        static
        EntityType
        deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
            return EntityType::deserialize(buffer, len, seek);
        }
    };

    template <>
    struct package_serializer<std::string> {
        static
        std::pair<kuic::byte_t *, size_t>
        serialize(const std::string &entity) {
            kuic::byte_t *buffer = new kuic::byte_t[entity.length()];
            return std::pair<kuic::byte_t *, size_t>(buffer, entity.length());
        }

        static
        std::string
        deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
            std::string result(buffer + seek, buffer + len);
            seek = len;
            return result;
        }
    };

    template <>
    struct package_serializer<std::vector<kuic::byte_t>> {
        static
        std::pair<kuic::byte_t *, size_t>
        serialize(const std::vector<kuic::byte_t> &entity) {
            kuic::byte_t *buffer = new kuic::byte_t[entity.size()];
            std::copy(entity.begin(), entity.end(), buffer);
            return std::pair<kuic::byte_t *, size_t>(buffer, entity.size());
        }

        static
        std::vector<kuic::byte_t>
        deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
            std::vector<kuic::byte_t> result;
            result.assign(buffer + seek, buffer + len);
            seek = len;
            return result;
        }
    };
}

#endif

