#include "handshake/kbr_authorization_data.h"
#include "handshake/serializer.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
#include "eys.h"
#include <algorithm>
#include <memory>

kuic::handshake::ad_item::ad_item() { }

kuic::handshake::ad_item::ad_item(kuic::error_t err)
    : lawful_package(err) { }

// split (ad_item | not_expect_ad_item)

kuic::handshake::not_expect_ad_item::not_expect_ad_item()
    : ad_item(kuic::not_expect) { }

kuic::kbr_authorization_data_item_type_t
kuic::handshake::not_expect_ad_item::get_type() const {
    return kuic::handshake::ad_type_not_expect;
}

// split (not_expect_ad_item | if_relevant_ad_item)

kuic::handshake::if_relevant_ad_item::if_relevant_ad_item() { }

kuic::handshake::if_relevant_ad_item::if_relevant_ad_item(
        const kuic::byte_t *buffer, size_t len) {
    this->data.assign(buffer, buffer + len);
}

kuic::kbr_authorization_data_item_type_t
kuic::handshake::if_relevant_ad_item::get_type() const {
    return kuic::handshake::ad_type_if_relevant;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::if_relevant_ad_item::serialize() const {
    kuic::byte_t *buffer = new kuic::byte_t[this->data.size()];
    std::copy(this->data.begin(), this->data.end(), buffer);
    return std::pair<kuic::byte_t *, size_t>(buffer, this->data.size());
}

kuic::handshake::if_relevant_ad_item
kuic::handshake::if_relevant_ad_item::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::if_relevant_ad_item result;
    result.data.assign(buffer + seek, buffer + len);
    seek = len;
    return result;
}

std::vector<kuic::byte_t>
kuic::handshake::if_relevant_ad_item::get_data() const {
    return this->data;
}

// split (if_relevant_ad_item | kdc_issued_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::kdc_issued_ad_item::get_type() const {
    return kuic::handshake::ad_type_kdc_issued;
}

// issued serialize
std::pair<kuic::byte_t *, size_t>
kuic::handshake::kdc_issued_ad_item::serialize() const {
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_ad_issued);

    // serialize checksum
    temporary_msg.insert(kuic::handshake::tag_checksum, this->checksum);
    // serialize issue realm
    temporary_msg.insert(kuic::handshake::tag_issue_realm, this->issue_realm);
    // serialize issue name
    temporary_msg.insert(kuic::handshake::tag_issue_principal_name, this->issue_name);
    // serialize elements
    temporary_msg.insert(kuic::handshake::tag_authorization_data, this->elements);

    return temporary_msg.serialize();
};

// issued deserialize
kuic::handshake::kdc_issued_ad_item
kuic::handshake::kdc_issued_ad_item::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::kdc_issued_ad_item result;
    kuic::handshake::handshake_message temporary_msg = kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    
    // deserialize checksum
    temporary_msg.assign(result.checksum, kuic::handshake::tag_checksum);
    // deserialize issue realm
    temporary_msg.assign(result.issue_realm, kuic::handshake::tag_issue_realm);
    // deserialize issue name
    temporary_msg.assign(result.issue_name, kuic::handshake::tag_issue_principal_name);
    // deserialize elements
    temporary_msg.assign(result.elements, kuic::handshake::tag_authorization_data);
    
    return result;
}

// split (kdc_issued_ad_item | and_or_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::and_or_ad_item::get_type() const {
    return kuic::handshake::ad_type_and_or; 
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::and_or_ad_item::serialize() const {
    // serialize elements
    size_t elements_size = 0;
    kuic::byte_t *elements_buffer = nullptr;
    std::tie(elements_buffer, elements_size) = this->elements.serialize();
    
    // declare result
    size_t result_size = elements_size + sizeof(int);
    kuic::byte_t *result = new kuic::byte_t[result_size];
    
    // declare condition count serialize temporary buffer
    size_t condition_count_size = 0;
    kuic::byte_t *condition_count_buffer_ptr = nullptr;
    
    // serialize condition count
    std::tie(condition_count_buffer_ptr, condition_count_size) = eys::littleendian_serializer<kuic::byte_t, int>::serialize(this->condition_count);
    std::unique_ptr<kuic::byte_t []> condition_count_buffer(condition_count_buffer_ptr);
    
    // copy condition count to result
    std::copy(
            condition_count_buffer.get(),
            condition_count_buffer.get() + condition_count_size,
            result);
    // copy elements to result
    std::copy(
            elements_buffer,
            elements_buffer + elements_size,
            result + condition_count_size);

    delete[] elements_buffer;
    return std::pair<kuic::byte_t *, size_t>(result, result_size);
}

kuic::handshake::and_or_ad_item
kuic::handshake::and_or_ad_item::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::and_or_ad_item result;

    // deserialize condition count
    result.condition_count = eys::littleendian_serializer<kuic::byte_t, int>::deserialize(buffer, len, seek);
    // deserialize elements
    result.elements = kuic::handshake::kbr_authorization_data::deserialize(buffer, len, seek);
   
   return result; 
}

// split (and_or_ad_item | mandatory_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::mandatory_ad_item::get_type() const {
    return kuic::handshake::ad_type_mandatory_for_kdc;
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::mandatory_ad_item::serialize() const {
    return this->elements.serialize();
}

kuic::handshake::mandatory_ad_item
kuic::handshake::mandatory_ad_item::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::mandatory_ad_item result;
    result.elements = kuic::handshake::kbr_authorization_data::deserialize(buffer, len, seek);
    return result;
}

// split (mandatory_ad_item | kbr_authorization_data_item)

kuic::handshake::kbr_authorization_data_item::kbr_authorization_data_item() { }

kuic::handshake::kbr_authorization_data_item::kbr_authorization_data_item(ad_item *item)
    : type(item->get_type()) {
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    size_t serialized_buffer_size = 0;
    std::tie(serialized_buffer_ptr, serialized_buffer_size) = item->serialize();

    std::unique_ptr<kuic::byte_t []> serialized_buffer(serialized_buffer_ptr);
    this->data.assign(serialized_buffer.get(), serialized_buffer.get() + serialized_buffer_size);
}

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_authorization_data_item::serialize() const {
    size_t size = sizeof(kuic::kbr_authorization_data_item_type_t) + this->data.size();
    kuic::byte_t *buffer = new kuic::byte_t[size];
    
    // declare temporary buffer
    kuic::byte_t *type_buffer_ptr = nullptr;
    size_t type_buffer_size = 0;
    std::unique_ptr<kuic::byte_t []> type_buffer;

    // serialize type
    std::tie(type_buffer_ptr, type_buffer_size) =
        kuic::handshake::kbr_authorization_data_type_serializer::serialize(this->type);
    type_buffer = std::unique_ptr<kuic::byte_t []>(type_buffer_ptr);
    // copy serialized type to result buffer
    std::copy(type_buffer.get(), type_buffer.get() + type_buffer_size, buffer);
    
    // copy this->data to result buffer
    std::copy(this->data.begin(), this->data.end(), buffer + type_buffer_size);

    return std::pair<kuic::byte_t *, size_t>(buffer, size);
}

kuic::handshake::kbr_authorization_data_item
kuic::handshake::kbr_authorization_data_item::deserialize(
        const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::kbr_authorization_data_item result;
    result.type = kuic::handshake::kbr_authorization_data_type_serializer::deserialize(buffer, len, seek);
    result.data.assign(buffer + seek, buffer + len);
    seek = len;
    return result;
}

// TODO 'get_item()' need alloc a memory block to store special ad_item, future should change it :)
kuic::handshake::ad_item *
kuic::handshake::kbr_authorization_data_item::get_item() const {
    size_t seek = 0;

    switch (this->type) {
    case kuic::handshake::ad_type_if_relevant:
        return  new kuic::handshake::if_relevant_ad_item(
                kuic::handshake::if_relevant_ad_item::deserialize(
                    this->data.data(), this->data.size(), seek));
    case kuic::handshake::ad_type_kdc_issued:
        return new kuic::handshake::kdc_issued_ad_item(
                kuic::handshake::kdc_issued_ad_item::deserialize(
                    this->data.data(), this->data.size(), seek));
    case kuic::handshake::ad_type_and_or:
        return new kuic::handshake::and_or_ad_item(
                kuic::handshake::and_or_ad_item::deserialize(
                    this->data.data(), this->data.size(), seek));
    case kuic::handshake::ad_type_mandatory_for_kdc:
        return new kuic::handshake::mandatory_ad_item(
                kuic::handshake::mandatory_ad_item::deserialize(
                    this->data.data(), this->data.size(), seek));
    default:
        return new kuic::handshake::not_expect_ad_item();
    }
}

kuic::kbr_authorization_data_item_type_t
kuic::handshake::kbr_authorization_data_item::get_type() const {
    return this->type;
}

// split (item | data)

kuic::handshake::kbr_authorization_data::kbr_authorization_data() { }

std::pair<kuic::byte_t *, size_t>
kuic::handshake::kbr_authorization_data::serialize() const {
    std::vector<kuic::byte_t> result;

    // declare temporary buffer
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    size_t serialized_size = 0;
    std::unique_ptr<kuic::byte_t []> serialized_buffer;

    // serialize elements count
    std::tie(serialized_buffer_ptr, serialized_size) = eys::littleendian_serializer<kuic::byte_t, unsigned int>::serialize(this->elements.size());
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    // copy serialized elements count to result vector
    result.insert(result.begin(), serialized_buffer.get(), serialized_buffer.get() + sizeof(unsigned int));

    // declare index area & data area
    std::vector<kuic::byte_t> index;
    std::vector<kuic::byte_t> data;
    int offset = 0;
    std::for_each(this->elements.begin(), this->elements.end(),
            [&] (const kuic::handshake::kbr_authorization_data_item &item) -> void {
                // serialize current element
                std::tie(serialized_buffer_ptr, serialized_size) = item.serialize();
                serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
                // insert serialized current element to data area
                data.insert(data.end(), serialized_buffer.get(), serialized_buffer.get() + serialized_size);
                // calculate offset (segment end position)
                offset += serialized_size;
                // serialize offset
                std::tie(serialized_buffer_ptr, serialized_size) = eys::littleendian_serializer<kuic::byte_t, int>::serialize(offset);
                serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
                // insert serialized offset to index area
                index.insert(index.end(), serialized_buffer.get(), serialized_buffer.get() + serialized_size);
            });

    // copy index to result vector
    result.insert(result.end(), index.begin(), index.end());
    // copy data to result vector
    result.insert(result.end(), data.begin(), data.end());

    kuic::byte_t *result_buffer = new kuic::byte_t[result.size()];
    std::copy(result.begin(), result.end(), result_buffer);
    return std::pair<kuic::byte_t *, size_t>(result_buffer, result.size());
}

kuic::handshake::kbr_authorization_data
kuic::handshake::kbr_authorization_data::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::kbr_authorization_data result;
    unsigned int elements_size = eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(buffer, len, seek);
    std::vector<int> offsets;
    for (unsigned int i = 0; i < elements_size; i++) {
        offsets.push_back(
                eys::littleendian_serializer<kuic::byte_t, int>::deserialize(buffer, len, seek));
    }
    size_t data_start_position = seek;
    for (unsigned int i = 0; i < elements_size; i++) {
        result.elements.push_back(
                kuic::handshake::kbr_authorization_data_item::deserialize(
                    buffer, data_start_position + size_t(offsets[i]), seek));
    }

    return result;
}

void kuic::handshake::kbr_authorization_data::add_element(
        kuic::handshake::kbr_authorization_data_item element) {
    this->elements.push_back(element);
}

std::vector<kuic::handshake::kbr_authorization_data_item>
kuic::handshake::kbr_authorization_data::get_elements() const {
    return this->elements;
}

