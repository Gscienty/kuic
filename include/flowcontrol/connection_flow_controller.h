#ifndef _KUIC_FLOWCONTROL_CONNECTION_FLOW_CONTROLLER_
#define _KUIC_FLOWCONTROL_CONNECTION_FLOW_CONTROLLER_

#include "error.h"
#include "flowcontrol/base_flow_controller.h"
#include "congestion/rtt.h"
#include <utility>
#include <functional>

namespace kuic {
    namespace flowcontrol {
        class connection_flow_controller : public base_flow_controller {
        private:
            kuic::bytes_count_t last_blocked_at;
            std::function<void ()> queue_window_update;

        public:
            connection_flow_controller(
                kuic::bytes_count_t receive_window,
                kuic::bytes_count_t max_receive_window,
                kuic::congestion::rtt &rtt,
                std::function<void ()> &&queue_window_update);
            
            kuic::bytes_count_t send_window_size() const;
            std::pair<bool, kuic::bytes_count_t> is_newly_blocked();
            kuic::error_t increment_highest_received(kuic::bytes_count_t increment);
            kuic::bytes_count_t get_window_update();
            void ensure_minimum_window_size(kuic::bytes_count_t inc);
            void try_queue_window_update();
        };
    }
}

#endif
