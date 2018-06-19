#ifndef _KUIC_HANDSHAKE_SERIALIZER_
#define _KUIC_HANDSHAKE_SERIALIZER_

#include "type.h"
#include "eys.h"
#include <utility>
#include <string>

namespace kuic {
    namespace handshake {
        struct kbr_error_code_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::error_t e) {
                return eys::bigendian_serializer<
                    kuic::byte_t, kuic::error_t>::serialize(e);
            }

            static kuic::error_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<
                    kuic::byte_t, kuic::error_t>::deserialize(buffer, seek);
            }
        };

        struct kbr_protocol_version_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::kbr_protocol_version_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_protocol_version_t>::serialize(e);
            }
            
            static kuic::kbr_protocol_version_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_protocol_version_t>::deserialize(buffer, seek);
            }
        };

        struct kbr_key_version_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::kbr_key_version_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_key_version_t>::serialize(e);
            }

            static kuic::kbr_key_version_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_key_version_t>::deserialize(buffer, seek);
            }
        };

        struct kbr_message_type_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::kbr_message_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_message_type_t>::serialize(e);
            }
            
            static kuic::kbr_message_type_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_message_type_t>::deserialize(buffer, seek);
            }
        };

        struct kbr_name_type_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::kbr_name_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_name_type_t>::serialize(e);
            }
            
            static kuic::kbr_name_type_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_name_type_t>::deserialize(buffer, seek);
            }
        };

        struct crypt_mode_type_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::crypt_mode_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::crypt_mode_type_t>::serialize(e);
            }

            static kuic::kbr_name_type_t deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::crypt_mode_type_t>::deserialize(buffer, seek);
            }
        };

        struct kbr_authorization_data_type_serializer {
            static std::basic_string<kuic::byte_t> serialize(const kuic::kbr_authorization_data_item_type_t e) {
                return eys::bigendian_serializer<kuic::byte_t, kuic::kbr_authorization_data_item_type_t>::serialize(e);
            }

            static
            kuic::kbr_authorization_data_item_type_t
            deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                return eys::bigendian_serializer<
                    kuic::byte_t, kuic::kbr_authorization_data_item_type_t>::deserialize(buffer, seek);
            }
        };
    }
}

#endif
