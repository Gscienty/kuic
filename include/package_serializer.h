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
        virtual std::basic_string<kuic::byte_t> serialize() const;
    };

    template <typename EntityType>
    struct package_serializer {
        static std::basic_string<kuic::byte_t> serialize(const EntityType &entity) {
            return entity.serialize();
        }

        static EntityType deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            return EntityType::deserialize(buffer, seek);
        }
    };

    template <>
    struct package_serializer<std::basic_string<kuic::byte_t>> {
        static std::basic_string<kuic::byte_t> serialize(const std::basic_string<kuic::byte_t> &entity) {
            return entity;
        }

        static std::basic_string<kuic::byte_t> deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            std::basic_string<kuic::byte_t> result(buffer.begin() + seek, buffer.end());
            seek = buffer.size();
            return result;
        }
    };

    template <>
    struct package_serializer<std::string> {
        static std::basic_string<kuic::byte_t> serialize(const std::string &entity) {
            return std::basic_string<kuic::byte_t>(entity.begin(), entity.end());
        }

        static std::string deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            std::string result(buffer.begin() + seek, buffer.end());
            seek = buffer.size();
            return result;
        }
    };
}

#endif

