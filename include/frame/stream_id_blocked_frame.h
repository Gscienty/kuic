#ifndef _KUIC_FRAME_STREAM_ID_BLOCKED_FRAME_
#define _KUIC_FRAME_STREAM_ID_BLOCKED_FRAME_

#include "frame.h"
#include "type.h"

namespace kuic {
    namespace frame {
        class stream_id_blocked_frame
            : public frame {
        private:
            kuic::stream_id_t stream_id;
            stream_id_blocked_frame(kuic::error_t error) : frame(error) { }
        public:
            stream_id_blocked_frame() { }
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            static stream_id_blocked_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek);
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;

            kuic::stream_id_t &get_stream_id();
        };
    }
}

#endif

