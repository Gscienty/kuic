#ifndef _KUIC_FRAME_APPLICATION_CLOSE_FRAME_
#define _KUIC_FRAME_APPLICATION_CLOSE_FRAME_

#include "frame/frame.h"
#include "type.h"
#include <string>

namespace kuic {
    namespace frame {
        class application_close_frame
            : public frame {
        private:
            kuic::application_error_code_t error_code;
            std::string reason_phrase;
        public:
            static application_close_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
        };
    }
}

#endif

