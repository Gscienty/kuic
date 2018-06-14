#ifndef _KUIC_STREAM_OUT_UNICAST_STREAM_
#define _KUIC_STREAM_OUT_UNICAST_STREAM_

#include "stream/send_stream.h"
#include "frame/stream_id_blocked_frame.h"
#include "rw_lock.h"
#include "type.h"
#include <mutex>
#include <condition_variable>
#include <map>
#include <functional>
#include <memory>

namespace kuic {
    namespace stream {
        class out_unicast_stream {
        private:
            kuic::rw_lock mutex;
            
            std::map<kuic::stream_id_t, std::shared_ptr<send_stream>> streams;

            kuic::stream_id_t next_stream;
            kuic::stream_id_t max_stream;
            kuic::stream_id_t hightst_blocked;

            std::function<send_stream *(kuic::stream_id_t)> new_stream;
            std::function<void (kuic::frame::stream_id_blocked_frame &)> queue_stream_id_blocked;

            kuic::error_t close_error;
        public:
            out_unicast_stream(
                    kuic::stream_id_t stream_id,
                    std::function<void (kuic::frame::stream_id_blocked_frame &)> queue_stream_id_blocked,
                    std::function<send_stream *(kuic::stream_id_t)> new_stream);
            std::shared_ptr<send_stream> open_stream();
            std::shared_ptr<send_stream> open_stream_implement();
            std::shared_ptr<send_stream> get_stream(kuic::stream_id_t stream_id);
            bool delete_stream(kuic::stream_id_t stream_id);
            void set_max_stream(kuic::stream_id_t stream_id);
            void close_with_error(kuic::error_t error);
        };
    }
}

#endif

