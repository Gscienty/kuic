#include "define.h"
#include "error.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "eys.h"
#include <memory>
#include <utility>
#include <algorithm>

kuic::handshake::handshake_message::handshake_message(kuic::error_t err)
    : lawful_package(err) { }

kuic::handshake::handshake_message::handshake_message(
    kuic::tag_t tag, std::map<kuic::tag_t, std::basic_string<kuic::byte_t> > &data)
        : tag(tag)
        , data(std::move(data)) { }

kuic::handshake::handshake_message::handshake_message() { }

kuic::handshake::handshake_message::handshake_message(kuic::tag_t tag)
    : tag(tag) { }

void kuic::handshake::handshake_message::set_tag(kuic::tag_t tag) {
    this->tag = tag;
}

void kuic::handshake::handshake_message::insert(kuic::tag_t tag, std::basic_string<kuic::byte_t> data) {
    this->data.insert(std::pair<kuic::tag_t, std::basic_string<kuic::byte_t>>(tag, data));
}

kuic::handshake::handshake_message
kuic::handshake::handshake_message::parse_handshake_message(std::basic_string<kuic::byte_t> &buffer) {

    // check reader remain lawful handshake message
    if (buffer.size() < 4) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }
    size_t seek = 0;

    // get main tag name
    kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(buffer, seek);
    // get current message parameters count
    unsigned int parameters_count = 
        eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(buffer, seek);

    // if current message parameters count too many
    // then return error
    if (parameters_count > kuic::max_parameters_count) {
        return kuic::handshake::handshake_message(kuic::handshake_too_many_entries);
    }

    // get handshake message index area
    // each parameter has 8 bytes
    // first 4 bytes store child-tag 
    // next 4 bytes store segment length
    
    // check current handshake message remain bytes is enough
    if (seek + parameters_count * 8 > buffer.size()) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }
    std::basic_string<kuic::byte_t> index(
            buffer.begin() + seek,
            buffer.begin() + seek + (parameters_count * 8));
    seek += parameters_count * 8;

    // declare tag-value map to store current serialized bytes-string
    std::map<kuic::tag_t, std::basic_string<kuic::byte_t>> result_map;
    unsigned int seg_start = 0;
    for (size_t pos = 0; pos < parameters_count * 8; ) {
        // get current segment tag name
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(index, pos);
        // get current segment end position
        unsigned int seg_end = eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(index, pos);

        // calculate current segment length
        unsigned int seg_len = seg_end - seg_start;
        // check current segment length
        if (seg_len > kuic::parameter_max_length) {
            return kuic::handshake::handshake_message(kuic::handshake_invalid_value_length);
        }
        
        // get current segment
        // use unique_ptr to store temporary bytes-string
        if (seek + seg_len > buffer.size()) {
            return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
        }
        std::basic_string<kuic::byte_t> seg(
                buffer.begin() + seek,
                buffer.begin() + seek + seg_len);
        seek += seg_len;

        if (seg.size() != seg_len) {
            return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
        }

        // insert current segment to result_map
        result_map.insert(
                std::pair<kuic::tag_t, std::basic_string<kuic::byte_t>>(tag, seg));
        
        seg_start = seg_end;
    }

    return kuic::handshake::handshake_message(tag, result_map);
}

std::basic_string<kuic::byte_t>
kuic::handshake::handshake_message::serialize() const {
    
    std::basic_string<kuic::byte_t> result;

    // current ser_buffer store main tag
    result.append(kuic::handshake::tag_serializer::serialize(this->tag));
    // reset ser_buffer to store segment count
    result.append(eys::littleendian_serializer<kuic::byte_t, unsigned short>::serialize(this->data.size()));
    // fill zero (like int)
    result.push_back(kuic::byte_t(0));
    result.push_back(kuic::byte_t(0));

    // reset serialized_buffer to store index area
    unsigned int offset = 0;
    // get sorted_tags
    std::vector<kuic::tag_t> tags_sorted = this->get_tags_sorted();
    // declare temporary data_buffer to store data area
    std::basic_string<kuic::byte_t> data_buffer;

    std::for_each(tags_sorted.begin(), tags_sorted.end(),
            [&, this] (const kuic::tag_t &t) -> void {
                // find the tag's data
                auto t_ptr = this->data.find(t);
                // copy segment data to data_buffer
                data_buffer.append(t_ptr->second);
                offset += (unsigned int) (t_ptr->second.size());

                // serialize current tag
                result.append(kuic::handshake::tag_serializer::serialize(t));
                // serialize current segment end position
                result.append(eys::littleendian_serializer<kuic::byte_t, unsigned int>::serialize(offset));
            });
    // insert data area
    result.append(data_buffer);

    return result;
}

kuic::handshake::handshake_message
kuic::handshake::handshake_message::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    // check it is a lawful handshake message
    // if it not, return reader buffer remain not enough 
    if (buffer.size() - seek < 4) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }

    // get global tag name
    kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(buffer, seek);
    // get handshake message parameters count
    // index part's length is parameters count * 8
    // because tag occupy 4 bytes 
    // and content length occupy 4bytes
    unsigned int parameters_count =
        eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(buffer, seek);
    // if remain bytes have not enough 
    // or too many parameters count
    // then return them errors
    if (parameters_count > kuic::max_parameters_count) {
        return kuic::handshake::handshake_message(kuic::handshake_too_many_entries);
    }

    if (buffer.size() - seek < parameters_count * 8) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }

    // declare child tag-value map
    std::map<kuic::tag_t, std::basic_string<kuic::byte_t> > result_map;
    // declare index point to index start position
    std::basic_string<kuic::byte_t> index(
            buffer.begin() + seek,
            buffer.begin() + seek + (parameters_count * 8));
    // set seek to value area start position
    seek += parameters_count * 8;
    unsigned int seg_start = 0;
    for (size_t pos = 0; pos < parameters_count * 8; ) {
        // get child tag name
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(index, pos);
        // get end of current segment
        unsigned int seg_end = 
            eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(index, pos);
        // calculate current segment length
        unsigned int seg_len = seg_end - seg_start;
        
        // if current segment length great than 
        // parameter max length, then return error  
        if (seg_len > kuic::parameter_max_length) {
            return kuic::handshake::handshake_message(kuic::handshake_invalid_value_length);
        }
        // check remain bytes is enough current segment bytes
        if (buffer.size() - seek < seg_len) {
            return kuic::handshake::handshake_message(kuic::handshake_invalid_value_length);
        }
        // insert current tag & value
        result_map.insert(
                std::pair<kuic::tag_t, std::basic_string<kuic::byte_t>>(
                    tag, std::basic_string<kuic::byte_t>(
                        buffer.begin() + seek,
                        buffer.begin() + seek + seg_len)));
        
        // add seg_len to seek
        seek += seg_len;
        seg_start = seg_end;

    }
    
    return kuic::handshake::handshake_message(tag, result_map);
}

std::vector<kuic::tag_t>
kuic::handshake::handshake_message::get_tags_sorted() const {
    std::vector<kuic::tag_t> result;
    
    std::for_each(this->data.begin(), this->data.end(),
        [&] (const std::pair<kuic::tag_t, std::basic_string<kuic::byte_t> > &p) -> void {
            result.push_back(p.first);
        });
    std::sort(result.begin(), result.end(), [] (const kuic::tag_t &a, const kuic::tag_t &b) -> bool {
        return a < b;
    });

    return result;
}

std::basic_string<kuic::byte_t> &
kuic::handshake::handshake_message::get_serialized_buffer(kuic::tag_t tag) {
    return this->data[tag];
}

bool kuic::handshake::handshake_message::exist(kuic::tag_t tag) const {
    return this->data.find(tag) != this->data.end();
}

kuic::tag_t kuic::handshake::handshake_message::get_tag() const {
    return this->tag;
}
