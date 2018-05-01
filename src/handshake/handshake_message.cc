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

kuic::handshake::handshake_message::handshake_message(kuic::tag_t tag)
    : tag(tag) { }

void kuic::handshake::handshake_message::insert(
    kuic::tag_t tag, kuic::byte_t *data, size_t size) {
    
    this->data.insert(
        std::pair<kuic::tag_t, std::vector<kuic::byte_t> >(
            tag, std::vector<kuic::byte_t>(data, data + size)));
}

std::pair<kuic::handshake::handshake_message, kuic::error_t>
kuic::handshake::handshake_message::parse_handshake_message(
    eys::in_buffer &reader) {

    // check reader remain lawful handshake message
    if (reader.remain() < 4) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(), kuic::reader_buffer_remain_not_enough);
    }

    // get main tag name
    kuic::tag_t tag;
    reader.get<kuic::tag_t, kuic::handshake::tag_serializer>(tag);
    // get current message parameters count
    unsigned int parameters_count;
    reader.get<unsigned int, kuic::little_endian_serializer<unsigned int>>(parameters_count);
    // if current message parameters count too many
    // then return error
    if (parameters_count > kuic::max_parameters_count) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(), kuic::handshake_too_many_entries);
    }

    // get handshake message index area
    // use unique_ptr store it
    // each parameter has 8 bytes
    // first 4 bytes store child-tag 
    // next 4 bytes store segment length
    char *index;
    size_t truth_size;
    std::tie<char *, size_t>(index, truth_size) = reader.get_range(parameters_count * 8);
    std::unique_ptr<char []> uni_index(index);
    
    // check current handshake message remain bytes is enough
    if (truth_size != parameters_count * 8) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(), kuic::reader_buffer_remain_not_enough);
    }
    
    // declare tag-value map to store current serialized bytes-string
    std::map<kuic::tag_t, std::vector<kuic::byte_t> > result_map;
    unsigned int seg_start = 0;
    for (ssize_t pos = 0; pos < parameters_count * 8; ) {
        // get current segment tag name
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(
            uni_index.get(), parameters_count * 8, pos);
        unsigned int seg_end = kuic::little_endian_serializer<unsigned int>::deserialize(
            uni_index.get(), parameters_count * 8, pos);

        // calculate current segment length
        unsigned int seg_len = seg_end - seg_start;
        // check current segment length
        if (seg_len > kuic::parameter_max_length) {
            return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(), kuic::handshake_invalid_value_length);
        }
        
        // get current segment
        // use unique_ptr to store temporary bytes-string
        char *seg;
        size_t seg_truth_len;
        std::tie<char *, size_t>(seg, seg_truth_len) = reader.get_range(seg_len);
        std::unique_ptr<char []> uni_seg(seg);

        if (seg_truth_len != seg_len) {
            return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(), kuic::reader_buffer_remain_not_enough);
        }

        // insert current segment to result_map
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
    
    // declare segment size
    size_t ser_size;
    // declare segment temporary buffer
    // current ser_buffer store main tag
    std::unique_ptr<char []> ser_buffer(
        kuic::handshake::tag_serializer::serialize(this->tag, ser_size));
    // copy current ser_buffer to result (byte vector)
    result.insert(result.begin(), ser_buffer.get(), ser_buffer.get() + ser_size);
    
    // reset ser_buffer to store segment count
    ser_buffer = std::unique_ptr<char []>(kuic::little_endian_serializer<unsigned short>::serialize(
        static_cast<unsigned short>(this->data.size()), ser_size));
    // copy current ser_buffer to result (only set short (max is 65535))
    result.insert(result.end(), ser_buffer.get(), ser_buffer.get() + ser_size);
    // fill zero (like int)
    result.push_back(kuic::byte_t(0));
    result.push_back(kuic::byte_t(0));

    // reset ser_buffer to store index area
    ser_buffer = std::unique_ptr<char []>(new char[this->data.size() * 8]);
    unsigned int offset = 0;
    // get sorted_tags
    std::vector<kuic::tag_t> tags_sorted = this->get_tags_sorted();
    // declare temporary data_buffer to store data area
    std::vector<kuic::byte_t> data_buffer;

    // enum each tag to stored index & data
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

std::pair<kuic::handshake::handshake_message, kuic::error_t>
kuic::handshake::handshake_message::deserialize(
        kuic::byte_t *buffer, size_t len, ssize_t &seek) {
    // check it is a lawful handshake message
    // if it not, return reader buffer remain not enough 
    if (len - seek < 4) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(),
                kuic::reader_buffer_remain_not_enough);
    }

    // get global tag name
    kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(reinterpret_cast<char *>(buffer), len, seek);
    // get handshake message parameters count
    // index part's length is parameters count * 8
    // because tag occupy 4 bytes 
    // and content length occupy 4bytes
    unsigned int parameters_count = kuic::little_endian_serializer<unsigned int>::deserialize(
            reinterpret_cast<char *>(buffer), len, seek);
    // if remain bytes have not enough 
    // or too many parameters count
    // then return them errors
    if (parameters_count > kuic::max_parameters_count) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(),
                kuic::handshake_too_many_entries);
    }

    if (len - seek < parameters_count * 8) {
        return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                kuic::handshake::handshake_message(),
                kuic::reader_buffer_remain_not_enough);
    }

    // declare child tag-value map
    std::map<kuic::tag_t, std::vector<kuic::byte_t> > result_map;
    // declare index point to index start position
    kuic::byte_t *index = buffer + seek;
    // set seek to value area start position
    seek += parameters_count * 8;
    unsigned int seg_start = 0;
    for (ssize_t pos = 0; pos < parameters_count * 8; ) {
        // get child tag name
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(
                reinterpret_cast<char *>(index),
                parameters_count * 8,
                pos);
        // get end of current segment
        unsigned int seg_end = kuic::little_endian_serializer<unsigned int>::deserialize(
                reinterpret_cast<char *>(index),
                parameters_count * 8,
                pos);
        // calculate current segment length
        unsigned int seg_len = seg_end - seg_start;
        
        // if current segment length great than 
        // parameter max length, then return error  
        if (seg_len > kuic::parameter_max_length) {
            return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                    kuic::handshake::handshake_message(),
                    kuic::handshake_invalid_value_length);
        }
        // check remain bytes is enough current segment bytes
        if (len - seek < seg_len) {
            return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
                    kuic::handshake::handshake_message(),
                    kuic::handshake_invalid_value_length);
        }
        // insert current tag & value
        result_map.insert(
                std::pair<kuic::tag_t, std::vector<kuic::byte_t> >(
                    tag,
                    std::vector<kuic::byte_t>(
                        buffer + seek,
                        buffer + seek + seg_len)));
        
        // add seg_len to seek
        seek += seg_len;
        seg_start = seg_end;

    }
    
    return std::pair<kuic::handshake::handshake_message, kuic::error_t>(
            kuic::handshake::handshake_message(tag, result_map),
            kuic::no_error); 
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

bool kuic::handshake::handshake_message::exist(kuic::tag_t tag) const {
    return this->data.find(tag) != this->data.end();
}

kuic::tag_t kuic::handshake::handshake_message::get_tag() const {
    return this->tag;
}
