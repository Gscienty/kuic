#ifndef _KUIC_HANDSHAKE_SERIALIZER_
#define _KUIC_HANDSHAKE_SERIALIZER_

#include "type.h"
#include "eys.h"
#include <utility>

namespace kuic {
    namespace handshake {
        struct kbr_protocol_version_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::kbr_protocol_version_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_protocol_version_t>::serialize(e);
            }
            
            static kuic::kbr_protocol_version_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_protocol_version_t>::deserialize(buffer, len, seek);
            }
        };

        struct kbr_key_version_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::kbr_key_version_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_key_version_t>::serialize(e);
            }

            static kuic::kbr_key_version_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_key_version_t>::deserialize(buffer, len, seek);
            }
        };

        struct kbr_message_type_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::kbr_message_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_message_type_t>::serialize(e);
            }
            
            static kuic::kbr_message_type_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_message_type_t>::deserialize(buffer, len, seek);
            }
        };

        struct kbr_name_type_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::kbr_name_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_name_type_t>::serialize(e);
            }
            
            static kuic::kbr_name_type_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_name_type_t>::deserialize(buffer, len, seek);
            }
        };

        struct kbr_encryption_type_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::kbr_encryption_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_encryption_type_t>::serialize(e);
            }

            static kuic::kbr_name_type_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_encryption_type_t>::deserialize(buffer, len, seek);
            }
        };

        struct kbr_authorization_data_type_serializer {
            static std::pair<kuic::byte_t *, size_t> serialize(kuic::kbr_authorization_data_item_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_authorization_data_item_type_t>::serialize(e);
            }

            static kuic::kbr_authorization_data_item_type_t deserialize(kuic::byte_t *buffer, size_t len, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_authorization_data_item_type_t>::deserialize(buffer, len, seek);
            }
        };
    }
}

#endif
