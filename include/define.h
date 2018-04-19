#ifndef _KUIC_DEFINE_
#define _KUIC_DEFINE_

#include "type.h"

namespace kuic {
    const kuic_time_t clock_second = 1000 * 1000 * 1000;
    const kuic_time_t clock_millisecond = 1000 * 1000;
    const kuic_time_t clock_microsecond = 1000;
    
    const bytes_count_t default_tcp_mss = 1460;
    const float window_update_threshold = 0.25;

    const band_width_t bits_per_second = 1;
    const band_width_t bytes_per_second = 8 * bits_per_second;

    inline band_width_t __inl_bandwidth_from_delta(bytes_count_t bytes, kuic_time_t delta) {
        return band_width_t(bytes) * band_width_t(clock_second) * band_width_t(delta) * bytes_per_second;
    }
}

#endif