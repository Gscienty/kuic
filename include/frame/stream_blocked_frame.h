#ifndef _KUIC_FRAME_STREAM_BLOCKED_FRAME_
#define _KUIC_FRAME_STREAM_BLOCKED_FRAME_

#include "frame/frame.h"
#include "type.h"

namespace kuic {
    namespace frame {
        class stream_blocked_frame
            : public frame {
        private:
            kuic::stream_id_t stream_id;
            kuic::bytes_count_t offset;

            stream_blocked_frame(kuic::error_t error) : frame(error) { }
        public:
            kuic::stream_id_t &get_stream_id();
            kuic::bytes_count_t &get_offset();

            stream_blocked_frame() { }
            static stream_blocked_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            std::basic_string<kuic::byte_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
        };
    
    }
}

#endif

