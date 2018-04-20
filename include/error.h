#ifndef _KUIC_ERROR_
#define _KUIC_ERROR_

namespace kuic {
    typedef int error_t;

    const error_t no_error = 0;
    const error_t flow_control_received_too_much_data = 1001;
    const error_t stream_data_after_termination = 2001;
    const error_t reader_buffer_remain_not_enough = 3001;
    const error_t handshake_too_many_entries = 4001;
    const error_t handshake_invalid_value_length = 4002;
}

#endif