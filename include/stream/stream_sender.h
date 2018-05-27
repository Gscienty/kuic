#ifndef _KUIC_STREAM_STREAM_SENDER_
#define _KUIC_STREAM_STREAM_SENDER_

#include "frame/frame.h"
#include <functional>

namespace kuic {
    namespace stream {
        class stream_sender {
        public:
            virtual void queue_control_frame(kuic::frame::frame &) = 0;
            virtual void on_has_stream_data(kuic::stream_id_t) = 0;
            virtual void on_stream_completed(kuic::stream_id_t) = 0;
        };

        class unicast_stream_sender
            : public stream_sender {
        private:
            std::function<void ()> on_stream_completed_implement;
        public:
            virtual void on_stream_completed() override {
                this->on_stream_completed_implement();
            }
        };
    }
}

#endif

