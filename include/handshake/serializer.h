#ifndef _KUIC_HANDSHAKE_SERIALIZER_
#define _KUIC_HANDSHAKE_SERIALIZER_

#include "type.h"
#include "eys.h"

namespace kuic {
    namespace handshake {
        struct kbr_protocol_version_serializer {
            static char *serialize(kuic::kbr_protocol_version_t e, size_t &size) {
                return eys::serializer<unsigned int>::serialize((unsigned int)(e), size);
            }
            
            static kuic::kbr_protocol_version_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                return kuic::kbr_protocol_version_t(eys::deserializer<unsigned int>::deserialize(buffer, len, seek));
            }
        };

        struct kbr_key_version_serializer {
            static char *serialize(kuic::kbr_key_version_t e, size_t &size) {
            return eys::serializer<unsigned int>::serialize((unsigned int)(e), size);
            }

            static kuic::kbr_key_version_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                return kuic::kbr_key_version_t(eys::deserializer<unsigned int>::deserialize(buffer, len, seek));
            }
        };

        struct kbr_message_type_serializer {
            static char *serialize(kuic::kbr_message_type_t e, size_t &size) {
                return eys::serializer<unsigned int>::serialize((unsigned int)(e), size);
            }
            
            static kuic::kbr_message_type_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                return kuic::kbr_message_type_t(eys::deserializer<unsigned int>::deserialize(buffer, len, seek));
            }
        };

        struct kbr_name_type_serializer {
            static char *serialize(kuic::kbr_name_type_t e, size_t &size) {
                return eys::serializer<unsigned int>::serialize((unsigned int)(e), size);
            }
            
            static kuic::kbr_name_type_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                return kuic::kbr_name_type_t(eys::deserializer<unsigned int>::deserialize(buffer, len, seek));
            }
        };

        struct kbr_encryption_type_serializer {
            static char *serialize(kuic::kbr_encryption_type_t e, size_t &size) {
                return eys::serializer<unsigned int>::serialize((unsigned int)(e), size);
            }

            static kuic::kbr_name_type_t deserialize(const char *buffer, size_t len, ssize_t &seek) {
                return kuic::kbr_encryption_type_t(eys::deserializer<unsigned int>::deserialize(buffer, len, seek));
            }
        };
    }
}

#endif
