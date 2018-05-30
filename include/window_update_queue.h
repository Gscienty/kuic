#ifndef _KUIC_WINDOW_UPDATE_QUEUE_
#define _KUIC_WINDOW_UPDATE_QUEUE_

#include "stream/stream.h"
#include "stream/stream_getter.h"
#include "flowcontrol/stream_flow_controller.h"
#include "type.h"
#include <mutex>
#include <set>
#include <algorithm>

namespace kuic {
    class window_update_queue {
    private:
        std::mutex mutex;

        std::set<kuic::stream_id_t> queue;
        bool queued_conn;

        kuic::stream::crypto_stream &crypto_stream;
        kuic::stream::stream_getter &stream_getter;

        kuic::flowcontrol::connection_flow_controller &conn_flow_controller;
        std::function<void (kuic::frame::frame &)> callback;
    public:
        window_update_queue(
                kuic::stream::stream_getter &stream_getter,
                kuic::stream::crypto_stream &crypto_stream,
                kuic::flowcontrol::connection_flow_controller &conn_flow_controller,
                std::function<void (kuic::frame::frame &)> callback);

        void add_stream(kuic::stream_id_t stream_id);
        void add_connection();
        void queue_all();
    };
}

#endif

