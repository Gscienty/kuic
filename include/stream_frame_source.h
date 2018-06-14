#ifndef _KUIC_STREAM_FRAME_SOURCE_
#define _KUIC_STREAM_FRAME_SOURCE_

#include "frame/stream_frame.h"
#include "type.h"
#include <list>

namespace kuic {
    class stream_frame_source {
    public:
        virtual bool get_has_crypto_stream_data() = 0;
        virtual kuic::frame::stream_frame &pop_crypto_stream_frame(kuic::bytes_count_t max_len) = 0;
        virtual std::list<std::shared_ptr<kuic::frame::stream_frame>> pop_stream_frames(kuic::bytes_count_t max_total_len) = 0;
    };
}

#endif

