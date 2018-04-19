#ifndef _KUIC_CONGESTION_BASE_FLOW_CONTROLLER_
#define _KUIC_CONGESTION_BASE_FLOW_CONTROLLER_

#include "type.h"
#include "rw_lock.h"
#include "clock.h"
#include "congestion/rtt.h"

namespace kuic {
    namespace flowcontrol {
        class base_flow_controller {
        private:
            kuic::bytes_count_t bytes_sent;
            kuic::bytes_count_t send_window;

            kuic::rw_lock rw_m;
            kuic::bytes_count_t bytes_read;
            kuic::bytes_count_t highest_received;
            kuic::bytes_count_t receive_window;
            kuic::bytes_count_t receive_window_size;
            kuic::bytes_count_t max_receive_window_size;

            kuic::special_clock epoch_start_time;
            kuic::bytes_count_t epoch_start_offset;
            
            kuic::congestion::rtt _rtt;
        
        public:
            void add_bytes_sent(kuic::bytes_count_t n);
            void update_send_window(kuic::bytes_count_t offset);
            kuic::bytes_count_t send_window_size();
            void add_bytes_read(kuic::bytes_count_t n);
            bool has_window_update();
            kuic::bytes_count_t get_window_update();
            void try_adjust_window_size();
            void start_new_auto_tuning_epoch();
            bool check_flow_control_violation();
        };
    }
}

#endif