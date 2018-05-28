#ifndef _KUIC_STREAM_OUT_UNICAST_STREAM_
#define _KUIC_STREAM_OUT_UNICAST_STREAM_

#include "rw_lock.h"
#include <condition_variable>
#include <map>

namespace kuic {
    namespace stream {
        class out_unicast_stream {
        private:
            kuic::rw_lock mutex;
            std::condition_variable cond;


        };
    }
}

#endif

