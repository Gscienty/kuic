#ifndef _KUIC_FRAME_MAX_STREAM_DATA_FRAME_
#define _KUIC_FRAME_MAX_STREAM_DATA_FRAME_

#include "frame/frame.h"
#include "type.h"

namespace kuic {
    namespace frame {
        class max_stream_data_frame
            : public frame {
        private:
            kuic::stream_id_t stream_id;
            kuic::bytes_count_t byte_offset;

            max_stream_data_frame(kuic::error_t error) : frame(error) { }
        public:
            max_stream_data_frame() { }

            kuic::stream_id_t &get_stream_id();
            kuic::bytes_count_t &get_byte_offset();

            static max_stream_data_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
        };
    }
}

#endif

