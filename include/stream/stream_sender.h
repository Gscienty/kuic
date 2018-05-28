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
            stream_sender &origin_sender;
            std::function<void ()> on_stream_completed_implement;
        public:
            unicast_stream_sender(
                    stream_sender &stream_sender,
                    std::function<void ()> on_stream_completed_implement)
                : origin_sender(stream_sender)
                , on_stream_completed_implement(on_stream_completed_implement) { }

            virtual void queue_control_frame(kuic::frame::frame &frame) override {
                this->origin_sender.queue_control_frame(frame);
            }

            virtual void on_has_stream_data(kuic::stream_id_t stream_id) override {
                this->origin_sender.on_has_stream_data(stream_id);
            }

            virtual void on_stream_completed(kuic::stream_id_t) override {
                this->on_stream_completed_implement();
            }
        };
    }
}

#endif

