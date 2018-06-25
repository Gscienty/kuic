#ifndef _KUIC_FRAME_MAX_DATA_FRAME_
#define _KUIC_FRAME_MAX_DATA_FRAME_

#include "frame/frame.h"
#include "type.h"

namespace kuic {
    namespace frame {
        class max_data_frame
            : public frame {
        private:
            kuic::bytes_count_t byte_offset;

            max_data_frame(kuic::error_t error) : frame(error) { }
        public:
            max_data_frame() { }

            kuic::bytes_count_t &get_byte_offset();

            static max_data_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);
            virtual std::basic_string<kuic::byte_t> serialize() const override;
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
        };
    }
}

#endif

