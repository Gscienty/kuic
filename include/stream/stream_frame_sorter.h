#ifndef _KUIC_STREAM_STREAM_FRAME_SORDER_
#define _KUIC_STREAM_STREAM_FRAME_SORDER_

#include "frame/stream_frame.h"
#include "type.h"
#include <map>
#include <list>
#include <utility>
#include <memory>

namespace kuic {
    namespace stream {
        class stream_frame_sorter {
        private:
            std::map<kuic::bytes_count_t, std::shared_ptr<kuic::frame::stream_frame>> queued_frames;
            kuic::bytes_count_t read_position;
            std::list<std::pair<kuic::bytes_count_t, kuic::bytes_count_t>> gaps;
        public:
            stream_frame_sorter();

            kuic::bytes_count_t &get_read_position();

            bool push(std::shared_ptr<kuic::frame::stream_frame> &frame);
            std::shared_ptr<kuic::frame::stream_frame> pop();
            std::shared_ptr<kuic::frame::stream_frame> head();
        };
    }
}

#endif

