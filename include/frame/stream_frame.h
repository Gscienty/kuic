#ifndef _KUIC_FRAME_STREAM_FRAME_
#define _KUIC_FRAME_STREAM_FRAME_

#include "frame/frame.h"
#include "type.h"
#include <vector>
#include <memory>

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

            stream_frame(kuic::error_t error) : frame(error) { }
        public:
            stream_frame() { }
            static stream_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek);

            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;
            virtual std::basic_string<kuic::byte_t> serialize() const override;

            kuic::stream_id_t &get_stream_id();
            bool &get_fin_bit();
            kuic::bytes_count_t &get_offset();
            std::vector<kuic::byte_t> &get_data();
            bool &get_data_length_present(); 

            kuic::bytes_count_t max_data_length(kuic::bytes_count_t max_size) const;
            std::shared_ptr<stream_frame> maybe_split_offset_frame(kuic::bytes_count_t max_size);
        };
    }
}

#endif

