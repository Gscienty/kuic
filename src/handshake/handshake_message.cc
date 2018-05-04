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
    kuic::tag_t tag, std::map<kuic::tag_t, std::vector<kuic::byte_t> > &data)
        : tag(tag)
        , data(std::move(data)) { }

kuic::handshake::handshake_message::handshake_message() { }

kuic::handshake::handshake_message::handshake_message(kuic::tag_t tag)
    : tag(tag) { }

void kuic::handshake::handshake_message::set_tag(kuic::tag_t tag) {
    this->tag = tag;
}

void kuic::handshake::handshake_message::insert(
    kuic::tag_t tag, kuic::byte_t *data, size_t size) {
    this->data.insert(std::pair<kuic::tag_t, std::vector<kuic::byte_t> >(tag, std::vector<kuic::byte_t>(data, data + size)));
}

kuic::handshake::handshake_message
kuic::handshake::handshake_message::parse_handshake_message(eys::in_buffer &reader) {

    // check reader remain lawful handshake message
    if (reader.remain() < 4) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }

    // declare main tag name
    kuic::tag_t tag;
    // get main tag name
    reader.get<kuic::byte_t, kuic::tag_t, kuic::handshake::tag_serializer>(tag);
    // declare current message parameters count
    unsigned int parameters_count;
    // get main parameters count
    reader.get<kuic::byte_t, unsigned int, eys::littleendian_serializer<kuic::byte_t, unsigned int>>(parameters_count);

    // if current message parameters count too many
    // then return error
    if (parameters_count > kuic::max_parameters_count) {
        return kuic::handshake::handshake_message(kuic::handshake_too_many_entries);
    }

    // get handshake message index area
    // use unique_ptr store it
    // each parameter has 8 bytes
    // first 4 bytes store child-tag 
    // next 4 bytes store segment length
    kuic::byte_t *index = nullptr;
    size_t truth_size = 0;
    std::tie<kuic::byte_t *, size_t>(index, truth_size) = reader.get_range<kuic::byte_t>(parameters_count * 8);
    std::unique_ptr<kuic::byte_t []> uni_index(index);
    
    // check current handshake message remain bytes is enough
    if (truth_size != parameters_count * 8) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }
    
    // declare tag-value map to store current serialized bytes-string
    std::map<kuic::tag_t, std::vector<kuic::byte_t> > result_map;
    unsigned int seg_start = 0;
    for (size_t pos = 0; pos < parameters_count * 8; ) {
        // get current segment tag name
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(uni_index.get(), parameters_count * 8, pos);
        // get current segment end position
        unsigned int seg_end = eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(uni_index.get(), parameters_count * 8, pos);

        // calculate current segment length
        unsigned int seg_len = seg_end - seg_start;
        // check current segment length
        if (seg_len > kuic::parameter_max_length) {
            return kuic::handshake::handshake_message(kuic::handshake_invalid_value_length);
        }
        
        // get current segment
        // use unique_ptr to store temporary bytes-string
        kuic::byte_t *seg = nullptr;
        size_t seg_truth_len = 0;
        std::tie(seg, seg_truth_len) = reader.get_range<kuic::byte_t>(seg_len);
        std::unique_ptr<kuic::byte_t []> uni_seg(seg);

        if (seg_truth_len != seg_len) {
            return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
        }

        // insert current segment to result_map
        result_map.insert(std::pair<kuic::tag_t, std::vector<kuic::byte_t> >(tag, std::vector<kuic::byte_t>(seg, seg + seg_truth_len)));
        
        seg_start = seg_end;
    }

    return kuic::handshake::handshake_message(tag, result_map);
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::handshake_message::serialize() const {
    std::vector<kuic::byte_t> temporary_serialized_buffer;
    
    // declare segment size
    size_t serialized_size = 0;
    // declare segment temporary buffer
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    // declare serialized buffer
    std::unique_ptr<kuic::byte_t []> serialized_buffer;

    // current ser_buffer store main tag
    std::tie(serialized_buffer_ptr, serialized_size) = kuic::handshake::tag_serializer::serialize(this->tag);
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);

    // copy current ser_buffer to result (byte vector)
    temporary_serialized_buffer.insert(
            temporary_serialized_buffer.begin(), serialized_buffer.get(), serialized_buffer.get() + serialized_size);
    
    // reset ser_buffer to store segment count
    std::tie(serialized_buffer_ptr, serialized_size) = eys::littleendian_serializer<kuic::byte_t, unsigned short>::serialize(
            static_cast<unsigned short>(this->data.size()));
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    
    // copy current ser_buffer to result (only set short (max is 65535))
    temporary_serialized_buffer.insert(
            temporary_serialized_buffer.end(), serialized_buffer.get(), serialized_buffer.get() + serialized_size);
    // fill zero (like int)
    temporary_serialized_buffer.push_back(kuic::byte_t(0));
    temporary_serialized_buffer.push_back(kuic::byte_t(0));

    // reset serialized_buffer to store index area
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(new kuic::byte_t[this->data.size() * 8]);
    unsigned int offset = 0;
    // get sorted_tags
    std::vector<kuic::tag_t> tags_sorted = this->get_tags_sorted();
    // declare temporary data_buffer to store data area
    std::vector<kuic::byte_t> data_buffer;

    // enum each tag to stored index & data
    int i = 0;
    std::for_each(tags_sorted.begin(), tags_sorted.end(), [&, this] (const kuic::tag_t &t) -> void {
        // find the tag's data
        auto t_ptr = this->data.find(t);
        // copy segment data to data_buffer
        data_buffer.insert(data_buffer.end(), t_ptr->second.begin(), t_ptr->second.end());

        offset += (unsigned int) (t_ptr->second.size());
        // declare inner size
        size_t size = 0;
        // declare inner serialized buffer
        kuic::byte_t *inner_serialized_buffer_ptr = nullptr;
        std::unique_ptr<kuic::byte_t []> inner_serialized_buffer;

        // serialize current tag
        std::tie(inner_serialized_buffer_ptr, size) = kuic::handshake::tag_serializer::serialize(t);
        inner_serialized_buffer = std::unique_ptr<kuic::byte_t []>(inner_serialized_buffer_ptr);
        // copy serialized tag to serialized buffer
        std::copy(inner_serialized_buffer.get(), inner_serialized_buffer.get() + size, serialized_buffer.get() + i * 8);
        
        // serialize current segment end position
        std::tie(inner_serialized_buffer_ptr, size) = eys::littleendian_serializer<kuic::byte_t, unsigned int>::serialize(offset);
        inner_serialized_buffer = std::unique_ptr<kuic::byte_t []>(inner_serialized_buffer_ptr);
        // copy serialized segment end position to serialized buffer
        std::copy(inner_serialized_buffer.get(), inner_serialized_buffer.get() + size, serialized_buffer.get() + i * 8 + 4);

        i++;
    });
    // insert index area
    temporary_serialized_buffer.insert(
            temporary_serialized_buffer.end(),serialized_buffer.get(), serialized_buffer.get() + this->data.size() * 8);
    // insert data area
    temporary_serialized_buffer.insert(
            temporary_serialized_buffer.end(), data_buffer.begin(), data_buffer.end());

    // construct result
    kuic::byte_t *result = new kuic::byte_t[temporary_serialized_buffer.size()];
    std::copy(temporary_serialized_buffer.begin(), temporary_serialized_buffer.end(), result);

    return std::pair<kuic::byte_t *, size_t>(result, temporary_serialized_buffer.size());
}

kuic::handshake::handshake_message
kuic::handshake::handshake_message::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    // check it is a lawful handshake message
    // if it not, return reader buffer remain not enough 
    if (len - seek < 4) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }

    // get global tag name
    kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(buffer, len, seek);
    // get handshake message parameters count
    // index part's length is parameters count * 8
    // because tag occupy 4 bytes 
    // and content length occupy 4bytes
    unsigned int parameters_count = eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(buffer, len, seek);
    // if remain bytes have not enough 
    // or too many parameters count
    // then return them errors
    if (parameters_count > kuic::max_parameters_count) {
        return kuic::handshake::handshake_message(kuic::handshake_too_many_entries);
    }

    if (len - seek < parameters_count * 8) {
        return kuic::handshake::handshake_message(kuic::reader_buffer_remain_not_enough);
    }

    // declare child tag-value map
    std::map<kuic::tag_t, std::vector<kuic::byte_t> > result_map;
    // declare index point to index start position
    const kuic::byte_t *index = buffer + seek;
    // set seek to value area start position
    seek += parameters_count * 8;
    unsigned int seg_start = 0;
    for (size_t pos = 0; pos < parameters_count * 8; ) {
        // get child tag name
        kuic::tag_t tag = kuic::handshake::tag_serializer::deserialize(
                index, parameters_count * 8, pos);
        // get end of current segment
        unsigned int seg_end = eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(index, parameters_count * 8, pos);
        // calculate current segment length
        unsigned int seg_len = seg_end - seg_start;
        
        // if current segment length great than 
        // parameter max length, then return error  
        if (seg_len > kuic::parameter_max_length) {
            return kuic::handshake::handshake_message(kuic::handshake_invalid_value_length);
        }
        // check remain bytes is enough current segment bytes
        if (len - seek < seg_len) {
            return kuic::handshake::handshake_message(kuic::handshake_invalid_value_length);
        }
        // insert current tag & value
        result_map.insert(
                std::pair<kuic::tag_t, std::vector<kuic::byte_t> >(tag, std::vector<kuic::byte_t>(buffer + seek, buffer + seek + seg_len)));
        
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
