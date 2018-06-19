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
            static ping_frame deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
                kuic::byte_t type_byte = buffer[seek++];
                if (type_byte != kuic::frame_type_ping) {
                    return ping_frame(kuic::not_expect);
                }
                return ping_frame();
            }
            virtual std::basic_string<kuic::byte_t> serialize() const override {
                std::basic_string<kuic::byte_t> result;
                result.push_back(0x07);

                return result;
            }
            virtual size_t length() const override { return 1; }
            virtual kuic::frame_type_t type() const override { return kuic::frame_type_ping; }
        };
    }
}

#endif

