#ifndef _KUIC_ERROR_
#define _KUIC_ERROR_

namespace kuic {
    typedef int error_t;
    const error_t no_error = 0;
    const error_t flow_control_received_too_much_data = 1001;
}

#endif