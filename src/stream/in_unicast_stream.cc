#include "stream/in_unicast_stream.h"
#include <algorithm>

kuic::stream::in_unicast_stream::in_unicast_stream(
        kuic::stream_id_t next_stream,
        kuic::stream_id_t initial_max_stream_id,
        int max_num_streams,
        std::function<void (kuic::frame::frame &)> queue_control_frame,
        std::function<kuic::stream::receive_stream *(kuic::stream_id_t)> new_stream)
    : next_stream(next_stream)
    , max_stream(initial_max_stream_id)
    , max_num_streams(max_num_streams)
    , new_stream(new_stream)
    , queue_max_stream_id(
            [&] (kuic::frame::max_stream_id_frame &frame) -> void {
                queue_control_frame(frame); 
            })
    , error(kuic::no_error) { }

kuic::nullable<kuic::stream::receive_stream>
kuic::stream::in_unicast_stream::accept_stream() {
    std::unique_lock<std::mutex> lock(this->mutex.get_inner_mutex());

    kuic::stream::receive_stream *ptr = nullptr;
    while (true) {
        if (this->error != kuic::no_error) {
            return kuic::nullable<kuic::stream::receive_stream>(nullptr);
        }

        auto finded = this->streams.find(this->next_stream);

        if (finded != this->streams.end()) {
            ptr = finded->second.get();
            break;
        }
        
        this->cond.wait(lock);
    }

    this->next_stream += 4;
    return kuic::nullable<kuic::stream::receive_stream>(*ptr);
}

kuic::nullable<kuic::stream::receive_stream>
kuic::stream::in_unicast_stream::get_or_open_stream(kuic::stream_id_t stream_id) {
    {
        kuic::reader_lock_guard lock(this->mutex);
        if (stream_id > this->max_stream) {
            return kuic::nullable<kuic::stream::receive_stream>(nullptr);
        }
        if (stream_id <= this->highest_stream) {
            return kuic::nullable<kuic::stream::receive_stream>(*this->streams.find(stream_id)->second);
        }
    }

    {
        kuic::writer_lock_guard lock(this->mutex);
        kuic::stream_id_t start = 0;
        if (this->highest_stream == 0) {
            start = this->next_stream;
        }
        else {
            start = this->highest_stream + 4;
        }

        for (kuic::stream_id_t new_id = start; new_id <= stream_id; new_id += 4) {
            this->streams.insert(
                    std::pair<kuic::stream_id_t, std::unique_ptr<kuic::stream::receive_stream>>(
                        new_id, std::unique_ptr<kuic::stream::receive_stream>(this->new_stream(new_id))));
            this->cond.notify_one();
        }
        this->highest_stream = stream_id;
        return kuic::nullable<kuic::stream::receive_stream>(*this->streams.find(stream_id)->second);
    }
}

bool kuic::stream::in_unicast_stream::delete_stream(kuic::stream_id_t stream_id) {
    kuic::writer_lock_guard lock(this->mutex);

    if (this->streams.find(stream_id) == this->streams.end()) {
        return false;
    }

    this->streams.erase(this->streams.find(stream_id));

    int num_new_streams = this->max_num_streams - this->streams.size();
    if (num_new_streams > 0) {
        this->max_stream = this->highest_stream + num_new_streams * 4;
        kuic::frame::max_stream_id_frame frame;
        frame.get_stream_id() = this->max_stream;
        this->queue_max_stream_id(frame);
    }
    return true;
}

void kuic::stream::in_unicast_stream::close_with_error(kuic::error_t error) {
    {
        kuic::writer_lock_guard lock(this->mutex);

        this->error = error;
        std::for_each(
                this->streams.begin(), this->streams.end(),
                [&] (std::unique_ptr<kuic::stream::receive_stream> &stream) -> void {
                    stream->close_for_shutdown(error);
                });
    }
    this->cond.notify_all();
}
