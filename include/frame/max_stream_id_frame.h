#ifndef _KUIC_FRAME_MAX_STREAM_ID_FRAME_
#define _KUIC_FRAME_MAX_STREAM_ID_FRAME_

#include "frame/frame.h"
#include "variable_integer.h"
#include "type.h"
#include "define.h"

namespace kuic {
    namespace frame {
        class max_stream_id_frame
            : public frame {
        private:
            kuic::stream_id_t stream_id;

            max_stream_id_frame(kuic::error_t error) : frame(error) { }
        public:
            max_stream_id_frame() { }

            kuic::stream_id_t &get_stream_id() {
                return this->stream_id;
            }

            static max_stream_id_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
                seek++;
                if (seek >= len) {
                    return max_stream_id_frame(kuic::reader_buffer_remain_not_enough);
                }
                max_stream_id_frame frame;
                frame.stream_id = kuic::variable_integer::read(buffer, len, seek);
                return frame;
            }

            virtual std::pair<kuic::byte_t *, size_t> serialize() const override {
                size_t size = this->length();
                size_t seek = 0;
                kuic::byte *result = new kuic::byte_t[size];
                result[seek++] = 0x06;
                kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->stream_id));
                return std::pair<kuic::byte_t *, size_t>(result, size);
            }

            virtual size_t length() const override {
                return 1 + kuic::variable_integer::length(this->stream_id);
            }

            virtual kuic::frame_type_t type() const override {
                return kuic::frame_type_stream_id;
            }
        };
    }
}

#endif

