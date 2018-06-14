#include "stream/stream_framer.h"
#include "define.h"

kuic::stream::stream_framer::stream_framer(
        kuic::stream::crypto_stream &_crypto_stream,
        kuic::stream::stream_getter &_stream_getter)
    : _stream_getter(_stream_getter)
    , _crypto_stream(_crypto_stream) { }

void kuic::stream::stream_framer::add_active_stream(kuic::stream_id_t stream_id) {
    std::lock_guard<std::mutex> lock(this->mutex);
    if (stream_id == 0) {
        this->has_crypto_stream_data = true;
        return;
    }

    if (this->active_streams.find(stream_id) == this->active_streams.end()) {
        this->stream_queue.push_back(stream_id);
        this->active_streams.insert(stream_id);
    }
}

bool kuic::stream::stream_framer::get_has_crypto_stream_data() {
    bool result = false;
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        result = this->has_crypto_stream_data;
    }

    return result;
}

kuic::frame::stream_frame &
kuic::stream::stream_framer::pop_crypto_stream_frame(kuic::bytes_count_t max_len) {
    std::lock_guard<std::mutex> lock(this->mutex);
    std::pair<std::shared_ptr<kuic::frame::stream_frame>, bool> result =
        this->_crypto_stream.pop_stream_frame(max_len);
    this->has_crypto_stream_data = result.second;

    return *result.first;
}

std::list<std::shared_ptr<kuic::frame::stream_frame>>
kuic::stream::stream_framer::pop_stream_frames(kuic::bytes_count_t max_total_len) {
    kuic::bytes_count_t current_length = 0;
    std::list<std::shared_ptr<kuic::frame::stream_frame>> frames;

    std::lock_guard<std::mutex> lock(this->mutex);
    
    int num_active_streams = this->stream_queue.size();
    for (int i = 0; i < num_active_streams; i++) {
        if (max_total_len - current_length < kuic::min_stream_frame_size) {
            break;
        }

        kuic::stream_id_t id = this->stream_queue.front();
        this->stream_queue.pop_front();

        const kuic::stream::send_stream *send_stream = this->_stream_getter.get_or_open_send_stream(id);
        if (send_stream == nullptr) {
            this->active_streams.erase(this->active_streams.find(id));
            continue;
        }

        auto poped_stream_frame = const_cast<kuic::stream::send_stream *>(send_stream)
            ->pop_stream_frame(max_total_len - current_length);
        if (poped_stream_frame.second) {
            this->stream_queue.push_back(id);
        }
        else {
            this->active_streams.erase(this->active_streams.find(id));
        }

        if (bool(poped_stream_frame.first)) {
            continue;
        }

        frames.push_back(poped_stream_frame.first);
        current_length += poped_stream_frame.first->length();
    }

    return frames;
}
