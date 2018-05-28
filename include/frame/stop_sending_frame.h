#ifndef _KUIC_FRAME_STOP_SENDING_FRAME_
#define _KUIC_FRAME_STOP_SENDING_FRAME_

#include "frame/frame.h"
#include "type.h"

namespace kuic {
    namespace frame {
        class stop_sending_frame
            : public frame {
        private:
            kuic::stream_id_t stream_id;
            kuic::application_error_code_t error;
            stop_sending_frame(kuic::error_t error) : frame(error) { }
        public:
            stop_sending_frame() { }
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override;
            static stop_sending_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek);
            virtual size_t length() const override;
            virtual kuic::frame_type_t type() const override;

            kuic::stream_id_t &get_stream_id();
            kuic::application_error_code_t &get_application_error();
        };
    }
}

#endif

