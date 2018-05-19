#ifndef _KUIC_FRAME_CONNECTION_CLOSE_FRAME_
#define _KUIC_FRAME_CONNECTION_CLOSE_FRAME_

#include "frame/frame.h"
#include "type.h"
#include <string>

namespace kuic {
    namespace frame {
        class connection_close_frame
            : public frame {
        private:
            kuic::application_error_code_t error_code;
            std::string reason_phrase;
        public:
            static connection_close_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek);
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
        };
    }
}

#endif

