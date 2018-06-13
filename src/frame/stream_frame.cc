#include "frame/stream_frame.h"
#include "variable_integer.h"
#include "define.h"
#include <algorithm>

kuic::frame::stream_frame
kuic::frame::stream_frame::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::frame::stream_frame frame;

    kuic::byte_t type_byte = buffer[seek++];
    if (seek >= len) {
        return kuic::frame::stream_frame(kuic::reader_buffer_remain_not_enough);
    }

    frame.fin_bit = (type_byte & 0x01) > 0;
    frame.data_length_present = (type_byte & 0x02) > 0;
    bool has_offset = (type_byte & 0x04) > 0;

    frame.stream_id = kuic::variable_integer::read(buffer, len, seek);

    if (has_offset) {
        frame.offset = kuic::variable_integer::read(buffer, len, seek);
    }
    else {
        frame.offset = 0;
    }

    kuic::bytes_count_t data_length = 0;
    if (frame.data_length_present) {
        data_length = kuic::variable_integer::read(buffer, len, seek);
        if (data_length > len - seek) {
            return kuic::frame::stream_frame(kuic::reader_buffer_remain_not_enough);
        }
    }
    else {
        data_length = len - seek;
    }
    
    if (data_length != 0) {
        frame.data.assign(buffer + seek, buffer + seek + data_length);
        seek += data_length;
    }

    return frame;
}

std::pair<kuic::byte_t *, size_t>
kuic::frame::stream_frame::serialize() const {
    if (this->data.empty() && this->fin_bit == false) {
        return std::pair<kuic::byte_t *, size_t>(nullptr, 0);
    }

    size_t size = this->length();
    size_t seek = 0;
    kuic::byte_t *result = new kuic::byte_t[size];

    kuic::byte_t type_byte = 0x10;

    if (this->fin_bit) {
        type_byte ^= 0x01;
    }
    if (this->data_length_present) {
        type_byte ^= 0x02;
    }
    if (this->offset != 0) {
        type_byte ^= 0x04;
    }

    result[seek++] = type_byte;
    kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->stream_id));
    if (this->offset != 0) {
        kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->offset));
    }
    if (this->data_length_present) {
        kuic::frame::frame::fill(result, size, seek, kuic::variable_integer::write(this->data.size()));
    }

    std::copy(this->data.begin(), this->data.end(), result + seek);

    return std::pair<kuic::byte_t *, size_t>(result, size);
}

size_t kuic::frame::stream_frame::length() const {
    size_t length = 1 + kuic::variable_integer::length(this->stream_id);
    if (this->offset != 0) {
        length += kuic::variable_integer::length(this->offset);
    }
    if (this->data_length_present) {
        length += kuic::variable_integer::length(this->data.size());
    }

    return length + this->data.size();
}

kuic::bytes_count_t
kuic::frame::stream_frame::max_data_length(kuic::bytes_count_t max_size) const {
    size_t header_length = 1 + kuic::variable_integer::length(this->stream_id);
    if (this->offset != 0) {
        header_length += kuic::variable_integer::length(this->offset);
    }
    if (this->data_length_present) {
        header_length += kuic::variable_integer::length(this->data.size());
    }
    if (header_length > max_size) {
        return 0;
    }

    kuic::bytes_count_t max_data_length = max_size - header_length;
    if (this->data_length_present && kuic::variable_integer::length(max_data_length) != 1) {
        max_data_length--;
    }

    return max_data_length;
}

std::shared_ptr<kuic::frame::stream_frame>
kuic::frame::stream_frame::maybe_split_offset_frame(kuic::bytes_count_t max_size) {
    if (max_size >= this->length()) {
        return std::shared_ptr<kuic::frame::stream_frame>();
    }

    kuic::bytes_count_t n = this->max_data_length(max_size);
    if (n == 0) {
        return std::shared_ptr<kuic::frame::stream_frame>();
    }

    std::shared_ptr<kuic::frame::stream_frame> new_frame(new kuic::frame::stream_frame());
    new_frame->fin_bit = false;
    new_frame->stream_id = this->stream_id;
    new_frame->offset = this->offset;
    new_frame->data.assign(this->data.begin(), this->data.begin() + n);
    new_frame->data_length_present = this->data_length_present;

    this->data.assign(this->data.begin() + n, this->data.end());
    this->offset += n;

    return new_frame;
}

std::vector<kuic::byte_t> &kuic::frame::stream_frame::get_data() {
    return this->data;
}

kuic::stream_id_t &kuic::frame::stream_frame::get_stream_id() {
    return this->stream_id;
}

kuic::frame_type_t kuic::frame::stream_frame::type() const {
    return kuic::frame_type_stream;
}

bool &kuic::frame::stream_frame::get_fin_bit() {
    return this->fin_bit;
}

kuic::bytes_count_t &kuic::frame::stream_frame::get_offset() {
    return this->offset;
}

bool &kuic::frame::stream_frame::get_data_length_present() {
    return this->data_length_present;
}
