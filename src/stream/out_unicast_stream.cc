#include "stream/out_unicast_stream.h"
#include "frame/stream_id_blocked_frame.h"
#include <algorithm>

kuic::stream::out_unicast_stream::out_unicast_stream(
        kuic::stream_id_t stream_id,
        std::function<void (kuic::frame::stream_id_blocked_frame &)> queue_stream_id_blocked,
        std::function<send_stream *(kuic::stream_id_t)> new_stream)
    : next_stream(stream_id)
    , max_stream(0)
    , hightst_blocked(0)
    , new_stream(new_stream)
    , queue_stream_id_blocked(queue_stream_id_blocked)
    , close_error(kuic::no_error) { }

std::shared_ptr<kuic::stream::send_stream>
kuic::stream::out_unicast_stream::open_stream_implement() {
    if (this->close_error != kuic::no_error) {
        return std::shared_ptr<kuic::stream::send_stream>();
    }
    if (this->next_stream > this->max_stream) {
        if (this->max_stream == 0 || this->hightst_blocked < this->max_stream) {
            kuic::frame::stream_id_blocked_frame frame;
            frame.get_stream_id() = this->max_stream;
            this->queue_stream_id_blocked(frame);
            this->hightst_blocked = this->max_stream;
        }
        return std::shared_ptr<kuic::stream::send_stream>();
    }
    std::shared_ptr<kuic::stream::send_stream> s(this->new_stream(this->next_stream));
    this->streams.insert(
            std::pair<kuic::stream_id_t, std::shared_ptr<kuic::stream::send_stream>>(
                this->next_stream, s));
    this->next_stream += 4;
    return s;
}

std::shared_ptr<kuic::stream::send_stream>
kuic::stream::out_unicast_stream::open_stream() {
    kuic::writer_lock_guard lock(this->mutex);
    return this->open_stream_implement();
}

std::shared_ptr<kuic::stream::send_stream>
kuic::stream::out_unicast_stream::get_stream(kuic::stream_id_t stream_id) {
    kuic::reader_lock_guard lock(this->mutex);
    if (stream_id >= this->next_stream) {
        return std::shared_ptr<kuic::stream::send_stream>();
    }
    return this->streams.find(stream_id)->second;
}

bool kuic::stream::out_unicast_stream::delete_stream(kuic::stream_id_t stream_id) {
    kuic::writer_lock_guard lock(this->mutex);

    if (this->streams.find(stream_id) == this->streams.end()) {
        return false;
    }

    this->streams.erase(this->streams.find(stream_id));
    return true;
}

void kuic::stream::out_unicast_stream::set_max_stream(kuic::stream_id_t stream_id) {
    kuic::writer_lock_guard lock(this->mutex);
    if (stream_id > this->max_stream) {
        this->max_stream = stream_id;
    }
}

void kuic::stream::out_unicast_stream::close_with_error(kuic::error_t error) {
    kuic::writer_lock_guard lock(this->mutex);
    this->close_error = error;
    std::for_each(
            this->streams.begin(), this->streams.end(),
            [&] (std::map<kuic::stream_id_t, std::shared_ptr<kuic::stream::send_stream>>::reference stream) -> void {
                stream.second->close_for_shutdown(error);
            });
}
