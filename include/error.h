#ifndef _KUIC_ERROR_
#define _KUIC_ERROR_

namespace kuic {
    typedef int ErrorCode;
    const ErrorCode FLOW_CONTROL_RECEIVED_TOO_MUCH_DATA = -1001;

    const ErrorCode STREAM_DATA_AFTER_TERMINATION = -2001;
}

#endif