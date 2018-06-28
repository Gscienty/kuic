#ifndef _KUIC_FRAME_BLOCKED_FRAME_
#define _KUIC_FRAME_BLOCKED_FRAME_

#include "frame/frame.h"
#include "type.h"
#include <string>

namespace kuic {
    namespace frame {
        class blocked_frame 
            : public frame {
        private:
            kuic::bytes_count_t offset;
            blocked_frame(kuic::error_t error) : frame(error) { }
        public:
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            static blocked_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);

            blocked_frame() { }

            kuic::bytes_count_t &get_offset();
        };
    }
}

#endif

