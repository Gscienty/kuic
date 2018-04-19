#ifndef _KUIC_FLOWCONTROL_STREAM_FLOW_CONTROLLER
#define _KUIC_FLOWCONTROL_STREAM_FLOW_CONTROLLER

#include "flowcontrol/base_flow_controller.h"
#include "flowcontrol/connection_flow_controller.h"
#include "type.h"
#include "error.h"
#include <utility>

namespace kuic {
    namespace flowcontrol {
        class stream_flow_controller : public base_flow_controller {
        private:
            kuic::stream_id_t stream_id;
            connection_flow_controller &conn_ctrl;
            bool contributes_to_connection;
            bool received_final_offset;
        public:
            stream_flow_controller(
                kuic::stream_id_t stream_id,
                bool contributes_to_connection,
                connection_flow_controller &conn_ctrl,
                kuic::bytes_count_t receive_window,
                kuic::bytes_count_t max_receive_window,
                kuic::bytes_count_t initial_send_window,
                kuic::congestion::rtt &rtt);

            kuic::error_t update_highest_received(kuic::bytes_count_t byte_offset, bool is_final);
            void add_bytes_sent(kuic::bytes_count_t n);
            void add_bytes_read(kuic::bytes_count_t n);
            kuic::bytes_count_t send_window_size() const;
            std::pair<bool, kuic::bytes_count_t> is_blocked() const;
            bool has_window_update();
            kuic::bytes_count_t get_window_update();
        };
    }
}

#endif