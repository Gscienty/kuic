#ifndef _KUIC_STREAM_STREAM_FRAME_SORDER_
#define _KUIC_STREAM_STREAM_FRAME_SORDER_

#include "frame/stream_frame.h"
#include <nullable.h>
#include "type.h"
#include <map>
#include <list>
#include <utility>

namespace kuic {
    namespace stream {
        class stream_frame_sorter {
        private:
            std::map<kuic::bytes_count_t, kuic::frame::stream_frame &> queued_frames;
            kuic::bytes_count_t read_position;
            std::list<std::pair<kuic::bytes_count_t, kuic::bytes_count_t>> gaps;
        public:
            stream_frame_sorter();

            bool push(kuic::frame::stream_frame &frame);
            kuic::nullable<kuic::frame::stream_frame> pop();
            kuic::nullable<kuic::frame::stream_frame> head();
        };
    }
}

#endif

