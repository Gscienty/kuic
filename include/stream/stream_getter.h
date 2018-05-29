#ifndef _KUIC_STREAM_STREAM_GETTER_
#define _KUIC_STREAM_STREAM_GETTER_

#include "stream/receive_stream.h"
#include "stream/send_stream.h"
#include "nullable.h"
#include "type.h"

namespace kuic {
    namespace stream {
        class stream_getter {
        public:
            virtual kuic::nullable<receive_stream> get_or_open_receive_stream(kuic::stream_id_t) = 0;
            virtual kuic::nullable<send_stream> get_or_open_send_stream(kuic::stream_id_t) = 0;
        };
    }
}

#endif

