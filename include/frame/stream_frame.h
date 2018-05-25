#ifndef _KUIC_FRAME_STREAM_FRAME_
#define _KUIC_FRAME_STREAM_FRAME_

#include "frame/frame.h"
#include "type.h"
#include <vector>

namespace kuic {
    namespace frame {
        class stream_frame
            : public frame {
        private:
            kuic::stream_id_t stream_id;
            bool fin_bit;
            bool data_length_present;
            kuic::bytes_count_t offset;
            std::vector<kuic::byte_t> data;

            stream_frame() { }
            stream_frame(kuic::error_t error) : frame(error) { }
        public:
            static stream_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek);

            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;

            kuic::stream_id_t &get_stream_id();
            bool &get_fin_bit();
            kuic::bytes_count_t &get_offset();
            std::vector<kuic::byte_t> &get_data();


            kuic::bytes_count_t max_data_length(kuic::bytes_count_t max_size) const;
            stream_frame maybe_split_offset_frame(kuic::bytes_count_t max_size);
        };
    }
}

#endif

