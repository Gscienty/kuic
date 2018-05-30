#ifndef _KUIC_STREAM_RECEIVE_STREAM_
#define _KUIC_STREAM_RECEIVE_STREAM_

#include "stream/stream_frame_sorter.h"
#include "stream/stream_sender.h"
#include "frame/rst_stream_frame.h"
#include "flowcontrol/stream_flow_controller.h"
#include "type.h"
#include <mutex>
#include <condition_variable>

namespace kuic {
    namespace stream {
        class receive_stream {
        private:
            std::mutex mutex;
            
            kuic::stream_id_t stream_id;
            stream_sender &sender;

            stream_frame_sorter frame_queue;
            int read_position_in_frame;
            kuic::bytes_count_t read_offset;

            kuic::error_t close_for_shutdown_error;
            kuic::error_t cancel_read_error;
            kuic::error_t reset_remote_error;

            bool _close_for_shutdown;
            bool fin_read;
            bool canceled_read;
            bool reset_remotely;

            std::condition_variable read_cond;
            kuic::special_clock read_deadline;

            std::unique_ptr<kuic::flowcontrol::stream_flow_controller> flow_controller;
            
        public:
            receive_stream(
                    kuic::stream_id_t stream_id,
                    stream_sender &sender,
                    kuic::flowcontrol::stream_flow_controller *flow_controller);

            kuic::bytes_count_t &get_read_offset();

            kuic::stream_id_t get_stream_id();
            kuic::bytes_count_t read(kuic::byte_t *buffer, const kuic::bytes_count_t size);
            void cancel_read(kuic::application_error_code_t error);
            void signal_read();
            bool handle_stream_frame(kuic::frame::stream_frame &frame);
            bool handle_rst_stream_frame(kuic::frame::rst_stream_frame &frame);
            void close_remote(kuic::bytes_count_t offset);
            void set_read_deadline(kuic::special_clock clock);
            void close_for_shutdown(kuic::error_t error);
            kuic::bytes_count_t get_window_update();
            stream_frame_sorter &get_frame_queue();
        };
    }
}

#endif

