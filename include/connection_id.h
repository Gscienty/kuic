#ifndef _KUIC_CONNECTION_ID_
#define _KUIC_CONNECTION_ID_

#include "type.h"
#include "lawful_package.h"
#include <cstddef>
#include <string>

namespace kuic {
    class connection_id 
        : public kuic::lawful_package {
    private:
        kuic::byte_t value[8];

        connection_id(kuic::error_t error) :lawful_package(error) { }
    public:
        connection_id() { }
        static connection_id generate_connection_id();
        static connection_id deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek, size_t connection_id_length);
        bool operator== (const connection_id &other_connection_id);
        const kuic::byte_t *bytes() const;
    };
}

#endif

