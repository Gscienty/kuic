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
        const std::basic_string<kuic::byte_t> &buffer) {
    this->data.assign(buffer.begin(), buffer.end());
}

kuic::kbr_authorization_data_item_type_t
kuic::handshake::if_relevant_ad_item::get_type() const {
    return kuic::handshake::ad_type_if_relevant;
}

std::basic_string<kuic::byte_t>
kuic::handshake::if_relevant_ad_item::serialize() const {
    return this->data;
}

kuic::handshake::if_relevant_ad_item
kuic::handshake::if_relevant_ad_item::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::if_relevant_ad_item result;
    result.data.assign(buffer.begin() + seek, buffer.end());
    seek = buffer.size();
    return result;
}

std::basic_string<kuic::byte_t>
kuic::handshake::if_relevant_ad_item::get_data() const {
    return this->data;
}

// split (if_relevant_ad_item | kdc_issued_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::kdc_issued_ad_item::get_type() const {
    return kuic::handshake::ad_type_kdc_issued;
}

// issued serialize
std::basic_string<kuic::byte_t>
kuic::handshake::kdc_issued_ad_item::serialize() const {
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_ad_issued);

    // serialize checksum
    temporary_msg.insert(kuic::handshake::tag_checksum              ,   this->checksum      );
    // serialize issue realm
    temporary_msg.insert(kuic::handshake::tag_issue_realm           ,   this->issue_realm   );
    // serialize issue name
    temporary_msg.insert(kuic::handshake::tag_issue_principal_name  ,   this->issue_name    );
    // serialize elements
    temporary_msg.insert(kuic::handshake::tag_authorization_data    ,   this->elements      );

    return temporary_msg.serialize();
};

// issued deserialize
kuic::handshake::kdc_issued_ad_item
kuic::handshake::kdc_issued_ad_item::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::kdc_issued_ad_item result;
    kuic::handshake::handshake_message temporary_msg = kuic::handshake::handshake_message::deserialize(buffer, seek);
    
    // deserialize checksum
    temporary_msg.assign(result.checksum    ,   kuic::handshake::tag_checksum               );
    // deserialize issue realm
    temporary_msg.assign(result.issue_realm ,   kuic::handshake::tag_issue_realm            );
    // deserialize issue name
    temporary_msg.assign(result.issue_name  ,   kuic::handshake::tag_issue_principal_name   );
    // deserialize elements
    temporary_msg.assign(result.elements    ,   kuic::handshake::tag_authorization_data     );
    
    return result;
}

// split (kdc_issued_ad_item | and_or_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::and_or_ad_item::get_type() const {
    return kuic::handshake::ad_type_and_or; 
}

std::basic_string<kuic::byte_t>
kuic::handshake::and_or_ad_item::serialize() const {
    // declare result;
    std::basic_string<kuic::byte_t> result;
    // serialize condition count
    result.append(eys::littleendian_serializer<kuic::byte_t, int>::serialize(this->condition_count));
    // serialize elements
    result.append(this->elements.serialize());

    return result;
}

kuic::handshake::and_or_ad_item
kuic::handshake::and_or_ad_item::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::and_or_ad_item result;

    // deserialize condition count
    result.condition_count =
        eys::littleendian_serializer<kuic::byte_t, int>::deserialize(buffer, seek);
    // deserialize elements
    result.elements = kuic::handshake::kbr_authorization_data::deserialize(buffer, seek);
   
   return result; 
}

// split (and_or_ad_item | mandatory_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::mandatory_ad_item::get_type() const {
    return kuic::handshake::ad_type_mandatory_for_kdc;
}

std::basic_string<kuic::byte_t>
kuic::handshake::mandatory_ad_item::serialize() const {
    return this->elements.serialize();
}

kuic::handshake::mandatory_ad_item
kuic::handshake::mandatory_ad_item::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::mandatory_ad_item result;
    result.elements = kuic::handshake::kbr_authorization_data::deserialize(buffer, seek);
    return result;
}

// split (mandatory_ad_item | kbr_authorization_data_item)

kuic::handshake::kbr_authorization_data_item::kbr_authorization_data_item() { }

kuic::handshake::kbr_authorization_data_item::kbr_authorization_data_item(ad_item *item)
    : type(item->get_type())
    , data(item->serialize()) { }

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_authorization_data_item::serialize() const {
    std::basic_string<kuic::byte_t> result;

    // serialize type
    result.append(kuic::handshake::kbr_authorization_data_type_serializer::serialize(this->type));
    // copy this->data to result buffer
    result.append(this->data);

    return result;
}

kuic::handshake::kbr_authorization_data_item
kuic::handshake::kbr_authorization_data_item::deserialize(
        const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::kbr_authorization_data_item result;

    result.type = kuic::handshake::kbr_authorization_data_type_serializer::deserialize(buffer, seek);
    result.data.assign(buffer.begin() + seek, buffer.end());

    seek = buffer.size();
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
                    this->data, seek));
    case kuic::handshake::ad_type_kdc_issued:
        return new kuic::handshake::kdc_issued_ad_item(
                kuic::handshake::kdc_issued_ad_item::deserialize(
                    this->data, seek));
    case kuic::handshake::ad_type_and_or:
        return new kuic::handshake::and_or_ad_item(
                kuic::handshake::and_or_ad_item::deserialize(
                    this->data, seek));
    case kuic::handshake::ad_type_mandatory_for_kdc:
        return new kuic::handshake::mandatory_ad_item(
                kuic::handshake::mandatory_ad_item::deserialize(
                    this->data, seek));
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

std::basic_string<kuic::byte_t>
kuic::handshake::kbr_authorization_data::serialize() const {
    std::basic_string<kuic::byte_t> result;

    // serialize elements count
    result.append(eys::littleendian_serializer<kuic::byte_t, unsigned int>::serialize(this->elements.size()));

    // declare index area & data area
    std::basic_string<kuic::byte_t> index;
    std::basic_string<kuic::byte_t> data;
    int offset = 0;
    std::for_each(this->elements.begin(), this->elements.end(),
            [&] (const kuic::handshake::kbr_authorization_data_item &item) -> void {
                // serialize current element
                std::basic_string<kuic::byte_t> serialized_item = item.serialize();
                data.append(serialized_item);
                // calculate offset (segment end position)
                offset += serialized_item.size();
                // serialize offset
                index.append(eys::littleendian_serializer<kuic::byte_t, int>::serialize(offset));
            });

    // copy index to result
    result.append(index);
    // copy data to result
    result.append(data);

    return result;
}

kuic::handshake::kbr_authorization_data
kuic::handshake::kbr_authorization_data::deserialize(const std::basic_string<kuic::byte_t> &buffer, size_t &seek) {
    kuic::handshake::kbr_authorization_data result;
    unsigned int elements_size =
        eys::littleendian_serializer<kuic::byte_t, unsigned int>::deserialize(buffer, seek);

    std::vector<int> offsets;
    for (unsigned int i = 0; i < elements_size; i++) {
        offsets.push_back(
                eys::littleendian_serializer<kuic::byte_t, int>::deserialize(buffer, seek));
    }

    for (unsigned int i = 0; i < elements_size; i++) {
        std::basic_string<kuic::byte_t> item(
                buffer.begin() + seek,
                buffer.begin() + seek + size_t(offsets[i]));
        seek += size_t(offsets[i]);

        size_t item_seek = 0;
        result.elements.push_back(
                kuic::handshake::kbr_authorization_data_item::deserialize(item, item_seek));
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

