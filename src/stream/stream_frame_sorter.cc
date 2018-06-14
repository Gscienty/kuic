#include "stream/stream_frame_sorter.h"
#include "define.h"

kuic::stream::stream_frame_sorter::stream_frame_sorter()
    : read_position(0) {
    
    this->gaps.push_front(std::pair<kuic::bytes_count_t, kuic::bytes_count_t>(0, kuic::max_byte_count));
}

bool kuic::stream::stream_frame_sorter::push(std::shared_ptr<kuic::frame::stream_frame> &frame) {
    if (frame->get_data().empty()) {
        if (frame->get_fin_bit()) {
            this->queued_frames.insert(
                    std::pair<kuic::bytes_count_t, std::shared_ptr<kuic::frame::stream_frame>>(
                        frame->get_offset(), frame));
            return true;
        }
        return false;
    }

    bool was_cut = false;

    if (this->queued_frames.find(frame->get_offset()) != this->queued_frames.end()) {
        std::shared_ptr<kuic::frame::stream_frame> &old_frame = this->queued_frames.find(frame->get_offset())->second;
        if (frame->get_data().size() <= old_frame->get_data().size()) {
            return false;
        }
        frame->get_data().assign(frame->get_data().begin() + old_frame->get_data().size(), frame->get_data().end());
        frame->get_offset() += old_frame->get_data().size();
        was_cut = true;
    }

    kuic::bytes_count_t start = frame->get_offset();
    kuic::bytes_count_t end = frame->get_offset() + frame->get_data().size();

    auto gap_itr = this->gaps.begin();
    for (; gap_itr != this->gaps.end(); gap_itr++) {
        if (end <= gap_itr->first) {
            return false;
        }
        if (end > gap_itr->first && start <= gap_itr->second) {
            break;
        }
    }

    if (gap_itr == this->gaps.end()) {
        return false;
    }

    if (start < gap_itr->first) {
        kuic::bytes_count_t add = gap_itr->first - start;
        frame->get_offset() += add;
        start += add;
        frame->get_data().assign(frame->get_data().begin() + add, frame->get_data().end());
        was_cut = true;
    }

    auto end_gap_itr = gap_itr;

    while (end >= end_gap_itr->second) {
        auto next_end_gap_itr = end_gap_itr;
        next_end_gap_itr++;

        if (next_end_gap_itr == this->gaps.end()) {
            return false;
        }

        if (end_gap_itr != gap_itr) {
            this->gaps.erase(end_gap_itr);
        }
        if (end <= next_end_gap_itr->first) {
            break;
        }

        this->queued_frames.erase(this->queued_frames.find(end_gap_itr->second));
        end_gap_itr = next_end_gap_itr;
    }

    if (end > end_gap_itr->second) {
        kuic::bytes_count_t cut_length = end - end_gap_itr->second;
        kuic::bytes_count_t length = frame->get_data().size() - cut_length;
        end -= cut_length;
        frame->get_data().assign(frame->get_data().begin(), frame->get_data().begin() + length);
        was_cut = true;
    }

    if (start == gap_itr->first) {
        if (end >= gap_itr->second) {
            this->gaps.erase(gap_itr);
        }
        if (end < end_gap_itr->second) {
            end_gap_itr->first = end;
        }
    }
    else if (end == end_gap_itr->second) {
        gap_itr->second = start;
    }
    else {
        if (gap_itr == end_gap_itr) {
            this->gaps.insert(gap_itr, std::pair<kuic::bytes_count_t, kuic::bytes_count_t>(end, gap_itr->second));
            gap_itr->second = start;
        }
        else {
            gap_itr->second = start;
            end_gap_itr->first = end;
        }
    }

    if (this->gaps.size() > kuic::max_stream_frame_sorter_gaps) {
        return false;
    }

    this->queued_frames.insert(
            std::pair<kuic::bytes_count_t, std::shared_ptr<kuic::frame::stream_frame>>(
                frame->get_offset(), frame));
    return true;
}

std::shared_ptr<kuic::frame::stream_frame>
kuic::stream::stream_frame_sorter::pop() {
    std::shared_ptr<kuic::frame::stream_frame> frame = this->head();
    if (bool(frame)) {
        this->read_position += frame->get_data().size();
        this->queued_frames.erase(this->queued_frames.find(frame->get_offset()));
    }
    return frame;
}

std::shared_ptr<kuic::frame::stream_frame>
kuic::stream::stream_frame_sorter::head() {
    if (this->queued_frames.find(this->read_position) != this->queued_frames.end()) {
        return this->queued_frames.find(this->read_position)->second;
    }
    return std::shared_ptr<kuic::frame::stream_frame>();
}

kuic::bytes_count_t &kuic::stream::stream_frame_sorter::get_read_position() {
    return this->read_position;
}
