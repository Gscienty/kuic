#ifndef _KUIC_STREAM_STREAM_
#define _KUIC_STREAM_STREAM_

#include "stream/send_stream.h"
#include "stream/receive_stream.h"
#include "stream/stream_sender.h"
#include "flowcontrol/stream_flow_controller.h"
#include "frame/rst_stream_frame.h"
#include "type.h"
#include <mutex>
#include <memory>

namespace kuic {
    namespace stream {
        class stream {
        protected:
            kuic::stream::stream_sender &sender;
            std::unique_ptr<kuic::stream::stream_sender> send_sender;
            std::unique_ptr<kuic::stream::stream_sender> receive_sender;

            kuic::stream::send_stream send_stream;
            kuic::stream::receive_stream receive_stream;
            
            std::mutex completed_mutex;
            bool receive_stream_completed;
            bool send_stream_completed;
        public:
            stream(
                    kuic::stream_id_t stream_id,
                    kuic::stream::stream_sender &sender,
                    std::shared_ptr<kuic::flowcontrol::stream_flow_controller> flow_controller);
            
            kuic::bytes_count_t &get_write_offset();
            kuic::bytes_count_t write(const kuic::byte_t *p, const kuic::bytes_count_t size);
            kuic::bytes_count_t read(kuic::byte_t *buffer, const kuic::bytes_count_t size);


            void check_if_completed();

            kuic::stream_id_t get_stream_id() const;
            bool close();
            void set_deadline(kuic::special_clock clock);
            void close_for_shutdown(kuic::error_t error);
            bool handle_rst_stream_frame(kuic::frame::rst_stream_frame &frame);
            bool handle_stream_frame(kuic::frame::stream_frame &frame);
            void cancel_read(kuic::application_error_code_t error);
            std::pair<kuic::nullable<kuic::frame::stream_frame>, bool> pop_stream_frame(kuic::bytes_count_t max_bytes);
           stream_frame_sorter &get_frame_queue();
           kuic::bytes_count_t get_window_update();
        };

        class crypto_stream 
            : public stream {
        public:
            crypto_stream(kuic::stream::stream_sender &sender, std::shared_ptr<kuic::flowcontrol::stream_flow_controller> flow_controller)
                : stream(0, sender, flow_controller) { }

            void set_read_offset(kuic::bytes_count_t offset);
        };
    }
}

#endif

