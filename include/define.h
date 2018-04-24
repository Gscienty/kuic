#ifndef _KUIC_DEFINE_
#define _KUIC_DEFINE_

#include "type.h"

namespace kuic {
    const kuic_time_t clock_second = 1000 * 1000 * 1000;
    const kuic_time_t clock_millisecond = 1000 * 1000;
    const kuic_time_t clock_microsecond = 1000;
    
    const bytes_count_t default_tcp_mss = 1460;
    const float window_update_threshold = 0.25;
    const float connection_flow_control_multiplier = 1.5;
    const unsigned int max_parameters_count = 128;
    const unsigned int parameter_max_length = 4000;

    const band_width_t bits_per_second = 1;
    const band_width_t bytes_per_second = 8 * bits_per_second;

    inline band_width_t __inl_bandwidth_from_delta(bytes_count_t bytes, kuic_time_t delta) {
        return band_width_t(bytes) * band_width_t(clock_second) * band_width_t(delta) * bytes_per_second;
    }

    const kbr_flag_t kbr_flag_forwardable = 0x00000001 << (1 - 1);
    const kbr_flag_t kbr_flag_forwarded = 0x00000001 << (2 - 1);
    const kbr_flag_t kbr_flag_proxiable = 0x00000001 << (3 - 1);
    const kbr_flag_t kbr_flag_proxy = 0x00000001 << (4 - 1);
    const kbr_flag_t kbr_flag_allow_postdate = 0x00000001 << (5 - 1);
    const kbr_flag_t kbr_flag_renewable = 0x00000001 << (8 - 1);
    const kbr_flag_t kbr_flag_opt_hardware_auth = 0x00000001 << (11 - 1);
    const kbr_flag_t kbr_flag_disable_transited_check = 0x00000001 << (26 - 1);
    const kbr_flag_t kbr_flag_renewable_ok = 0x00000001 << (27 - 1);
    const kbr_flag_t kbr_flag_etc_tkt_in_skey = 0x00000001 << (28 - 1);
    const kbr_flag_t kbr_flag_renew = 0x00000001 << (30 - 1);
    const kbr_flag_t kbr_flag_validate = 0x00000001 << (31 - 1);

    const kbr_name_type_t kbr_name_default_type = 0x00000000;

    const kbr_protocol_version_t kbr_current_protocol_version = 0x00000001;
}

#endif