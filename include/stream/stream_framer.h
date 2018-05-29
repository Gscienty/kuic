#ifndef _KUIC_STREAM_STREAM_FRAMER_
#define _KUIC_STREAM_STREAM_FRAMER_

#include "stream/stream_getter.h"
#include "stream/stream.h"
#include "type.h"
#include <list>
#include <mutex>
#include <set>

namespace kuic {
    namespace stream {
        class stream_framer{
        private:
            stream_getter &_stream_getter;
            crypto_stream &_crypto_stream;

            std::mutex mutex;
            std::set<kuic::stream_id_t> active_streams;
            std::list<kuic::stream_id_t> stream_queue;

            bool has_crypto_stream_data;
        public:
            stream_framer(crypto_stream &_crypto_stream, stream_getter &_stream_getter);
            void add_active_stream(kuic::stream_id_t stream_id);
            bool get_has_crypto_stream_data();

            kuic::frame::stream_frame &pop_crypto_stream_frame(kuic::bytes_count_t max_len);
            std::list<kuic::frame::stream_frame &> pop_stream_frames(kuic::bytes_count_t max_total_len);
        };
    }
}

#endif

