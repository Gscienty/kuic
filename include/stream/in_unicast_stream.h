#ifndef _KUIC_STREAM_IN_UNICAST_STREAM_
#define _KUIC_STREAM_IN_UNICAST_STREAM_

#include "stream/receive_stream.h"
#include "frame/max_stream_id_frame.h"
#include "nullable.h"
#include "rw_lock.h"
#include "type.h"
#include <map>
#include <condition_variable>
#include <functional>
#include <memory>

namespace kuic {
    namespace stream {
        class in_unicast_stream {
        private:
            kuic::rw_lock mutex;
            std::condition_variable cond;
            std::map<kuic::stream_id_t, std::unique_ptr<receive_stream>> streams;

            kuic::stream_id_t next_stream;
            kuic::stream_id_t highest_stream;
            kuic::stream_id_t max_stream;
            int max_num_streams;

            std::function<receive_stream *(kuic::stream_id_t)> new_stream;
            std::function<void (kuic::frame::max_stream_id_frame &)> queue_max_stream_id;

            kuic::error_t error;
            
        public:
            in_unicast_stream(
                    kuic::stream_id_t next_stream,
                    kuic::stream_id_t initial_max_stream_id,
                    int max_num_streams,
                    std::function<void (kuic::frame::frame &)> queue_control_frame,
                    std::function<receive_stream *(kuic::stream_id_t)> new_stream);
            kuic::nullable<kuic::stream::receive_stream> accept_stream();
            kuic::nullable<kuic::stream::receive_stream> get_or_open_stream(kuic::stream_id_t stream_id);
            bool delete_stream(kuic::stream_id_t stream_id);
            void close_with_error(kuic::error_t error);
        };
    }
}

#endif

