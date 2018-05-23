#ifndef _KUIC_FRAME_PING_FRAME_
#define _KUIC_FRAME_PING_FRAME_

#include "frame/frame.h"
#include "define.h"

namespace kuic {
    namespace frame {
        class ping_frame
            : public frame {
        private:
            ping_frame(kuic::error_t error) : frame(error) { }
        public:
            ping_frame() { }
            static ping_frame deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
                kuic::byte_t type_byte = buffer[seek++];
                if (type_byte != kuic::frame_type_ping) {
                    return ping_frame(kuic::not_expect);
                }
                return ping_frame();
            }
            virtual std::pair<kuic::byte_t *, size_t> serialize() const override {
                kuic::byte_t *result = new kuic::byte_t[1];
                result[0] = 0x07;

                return std::pair<kuic::byte_t *, size_t>(result, 1);
            }
            virtual size_t length() const override { return 1; }
            virtual kuic::frame_type_t type() const override { return kuic::frame_type_ping; }
        };
    }
}

#endif

