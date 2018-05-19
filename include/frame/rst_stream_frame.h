#ifndef _KUIC_FRAME_RST_STREAM_FRAME_
#define _KUIC_FRAME_RST_STREAM_FRAME_

#include "frame/frame.h"
#include "type.h"

namespace kuic {
    namespace frame {
        class rst_stream_frame
           : public frame {
        private:
            kuic::stream_id_t stream_id;
            kuic::application_error_code_t error_code;
            kuic::bytes_count_t offset;

            rst_stream_frame(kuic::error_t error) : frame(error) { }
            rst_stream_frame() { }
        public:
            static rst_stream_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek);
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
        };
    }
}

#endif

