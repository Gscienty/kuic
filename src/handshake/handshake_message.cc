#include "define.h"
#include "error.h"
#include "little_endian_serializer.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "eys.h"
#include <memory>
#include <utility>
#include <algorithm>

kuic::handshake::handshake_message::handshake_message(
    kuic::tag_t tag, std::map<kuic::tag_t, std::vector<kuic::byte_t> > &data)
        : tag(tag)
        , data(std::move(data)) { }

kuic::handshake::handshake_message::handshake_message() { }

std::pair<kuic::handshake::handshake_message, kuic::error_t>
kuic::handshake::handshake_message::parse_handshake_message(
    eys::in_buffer &reader) {

    if (reader.remain() < 4) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(), kuic::reader_buffer_remain_not_enough);
    }

    kuic::tag_t tag;
    reader.get<kuic::tag_t, kuic::handshake::tag_serializer>(tag);
    unsigned int parameters_count;
    reader.get<unsigned int, kuic::little_endian_serializer<unsigned int>>(parameters_count);
    if (parameters_count > kuic::max_parameters_count) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(), kuic::handshake_too_many_entries);
    }

    char *index;
    size_t truth_size;
    std::tie<char *, size_t>(index, truth_size) = reader.get_range(parameters_count * 8);
    std::unique_ptr<char []> uni_index(index);

    if (truth_size != parameters_count * 8) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(), kuic::reader_buffer_remain_not_enough);
    }

    std::map<kuic::tag_t, std::vector<kuic::byte_t> > result_map;
    unsigned int seg_start = 0;
    for (ssize_t pos = 0; pos < parameters_count * 8; ) {
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(
            uni_index.get(), parameters_count * 8, pos);
        unsigned int seg_end = kuic::little_endian_serializer<unsigned int>::deserialize(
            uni_index.get(), parameters_count * 8, pos);
        
        unsigned int seg_len = seg_end - seg_start;
        if (seg_len > kuic::parameter_max_length) {
            return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(), kuic::handshake_invalid_value_length);
        }

        char *seg;
        size_t seg_truth_len;
        std::tie<char *, size_t>(seg, seg_truth_len) = reader.get_range(seg_len);
        std::unique_ptr<char []> uni_seg(seg);

        if (seg_truth_len != seg_len) {
            return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(), kuic::reader_buffer_remain_not_enough);
        }
        result_map.insert(std::pair<kuic::tag_t, std::vector<kuic::byte_t> >(
            tag, std::vector<kuic::byte_t>(seg, seg + seg_truth_len)));
        
        seg_start = seg_end;
    }

    return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
        kuic::handshake::handshake_message(tag, result_map), kuic::no_error);
}

std::vector<kuic::byte_t>
kuic::handshake::handshake_message::serialize() {
    std::vector<kuic::byte_t> result;
    
    size_t ser_size;
    std::unique_ptr<char []> ser_buffer(
        kuic::handshake::tag_serializer::serialize(this->tag, ser_size));
    result.insert(result.begin(), ser_buffer.get(), ser_buffer.get() + ser_size);

    ser_buffer = std::unique_ptr<char []>(kuic::little_endian_serializer<unsigned short>::serialize(
        static_cast<unsigned short>(this->data.size()), ser_size));
    result.insert(result.end(), ser_buffer.get(), ser_buffer.get() + ser_size);

    result.push_back(kuic::byte_t(0));
    result.push_back(kuic::byte_t(0));

    ser_buffer = std::unique_ptr<char []>(new char[this->data.size() * 8]);
    unsigned int offset = 0;
    
    std::vector<kuic::tag_t> tags_sorted = this->get_tags_sorted();
    std::vector<kuic::byte_t> data_buffer;

    int i = 0;
    std::for_each(tags_sorted.begin(), tags_sorted.end(), [&] (const kuic::tag_t &t) -> void {
        std::vector<kuic::byte_t> &value = this->data[t];
        data_buffer.insert(data_buffer.end(), value.begin(), value.end());

        offset += (unsigned int) (value.size());
        size_t size;
        std::unique_ptr<char []> t_ser(kuic::handshake::tag_serializer::serialize(t, size));
        std::uninitialized_copy_n(t_ser.get(), size, ser_buffer.get() + i * 8);
        t_ser = std::unique_ptr<char []>(kuic::little_endian_serializer<unsigned int>::serialize(offset, size));
        std::uninitialized_copy_n(t_ser.get(), size, ser_buffer.get() + i * 8 + 4);

        i++;
    });
    result.insert(result.end(), ser_buffer.get(), ser_buffer.get() + this->data.size() * 8);
    result.insert(result.end(), data_buffer.begin(), data_buffer.end());

    return result;
}

std::vector<kuic::tag_t>
kuic::handshake::handshake_message::get_tags_sorted() const {
    std::vector<kuic::tag_t> result;
    
    std::for_each(this->data.begin(), this->data.end(),
        [&] (const std::pair<kuic::tag_t, std::vector<kuic::byte_t> > &p) -> void {
            result.push_back(p.first);
        });
    std::sort(result.begin(), result.end(), [] (const kuic::tag_t &a, const kuic::tag_t &b) -> bool {
        return a < b;
    });

    return result;
}

std::vector<kuic::byte_t> &
kuic::handshake::handshake_message::get(kuic::tag_t tag) {
    return this->data[tag];
}