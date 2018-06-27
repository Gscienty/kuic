#ifndef _KUIC_HANDSHAKE_PACKAGE_SERIALIZER_
#define _KUIC_HANDSHAKE_PACKAGE_SERIALIZER_

#include "type.h"
#include "eys.h"
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

    template <> struct package_serializer<std::basic_string<kuic::byte_t>> {
        static std::basic_string<kuic::byte_t> serialize(const std::basic_string<kuic::byte_t> &entity) {
            return entity;
        }

        static std::basic_string<kuic::byte_t> deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            std::basic_string<kuic::byte_t> result(buffer.begin() + seek, buffer.end());
            seek = buffer.size();
            return result;
        }
    };

    template <> struct package_serializer<std::string> {
        static std::basic_string<kuic::byte_t> serialize(const std::string &entity) {
            return std::basic_string<kuic::byte_t>(entity.begin(), entity.end());
        }

        static std::string deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            std::string result(buffer.begin() + seek, buffer.end());
            seek = buffer.size();
            return result;
        }
    };

    template <> struct package_serializer<int> {
        static std::basic_string<kuic::byte_t> serialize(const int &entity) {
            return eys::bigendian_serializer<kuic::byte_t, int>::serialize(entity);
        }
        
        static int deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            return eys::bigendian_serializer<kuic::byte_t, int>::deserialize(buffer, seek);
        }
    };

    template <> struct package_serializer<kuic::packet_number_t> {
        static std::basic_string<kuic::byte_t> serialize(const kuic::packet_number_t &entity) {
            return eys::bigendian_serializer<kuic::byte_t, kuic::packet_number_t>::serialize(entity);
        }

        static kuic::packet_number_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            return eys::bigendian_serializer<kuic::byte_t, kuic::packet_number_t>::deserialize(buffer, seek);
        }
    };

    template <> struct package_serializer<unsigned long> {
        static std::basic_string<kuic::byte_t> serialize(const unsigned long &entity) {
            return eys::bigendian_serializer<kuic::byte_t, unsigned long>::serialize(entity);
        }

        static unsigned long deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
            return eys::bigendian_serializer<kuic::byte_t, unsigned long>::deserialize(buffer, seek);
        }
    };
}

#endif

