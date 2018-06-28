#ifndef _KUIC_STREAM_SEND_STREAM_
#define _KUIC_STREAM_SEND_STREAM_

#include "frame/stream_frame.h"
#include "frame/max_stream_data_frame.h"
#include "frame/stop_sending_frame.h"
#include "stream/stream_sender.h"
#include "flowcontrol/stream_flow_controller.h"
#include "clock.h"
#include "rw_lock.h"
#include "type.h"
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>

namespace kuic {
    namespace stream {
        class send_stream {
        private:
            std::mutex mutex;
            stream_sender &sender;
            kuic::stream_id_t stream_id;
            kuic::bytes_count_t write_offset;
            
            kuic::error_t cancel_write_error;
            kuic::error_t close_for_shutdown_error;

            bool _close_for_shutdown;
            bool finished_writing;
            bool _cancel_write;
            bool fin_sent;

            std::string data_for_waiting;
            kuic::special_clock write_deadline;
            std::condition_variable write_cond;
            std::shared_ptr<kuic::flowcontrol::stream_flow_controller> flow_controller;
        public:
            send_stream(
                    kuic::stream_id_t stream_id,
                    stream_sender &sender,
                    std::shared_ptr<kuic::flowcontrol::stream_flow_controller> flow_controller);

            kuic::bytes_count_t &get_write_offset();

            kuic::stream_id_t get_stream_id() const;
            kuic::bytes_count_t write(const std::string &data);
            std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool> pop_stream_frame(kuic::bytes_count_t max_bytes);
            void signal_write();
            bool close();
            bool cancel_write(kuic::application_error_code_t error);
            void handle_max_stream_data_frame(kuic::frame::max_stream_data_frame &frame);
            void set_write_deadline(kuic::special_clock clock);
            void close_for_shutdown(kuic::error_t error);
            void handle_stop_sending_frame(std::shared_ptr<kuic::frame::stop_sending_frame> &frame);
        };
    }
}

#endif

