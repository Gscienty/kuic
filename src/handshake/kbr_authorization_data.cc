#include "handshake/kbr_authorization_data.h"
#include "handshake/serializer.h"
#include "handshake/handshake_message.h"
#include "handshake/tag.h"
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

// split (if_relevant_ad_item | kdc_issued_ad_item)

kuic::kbr_authorization_data_item_type_t
kuic::handshake::kdc_issued_ad_item::get_type() const {
    return kuic::handshake::ad_type_kdc_issued;
}

// issued serialize
std::pair<kuic::byte_t *, size_t>
kuic::handshake::kdc_issued_ad_item::serialize() const {
    kuic::handshake::handshake_message temporary_msg(kuic::handshake::tag_ad_issued);

    kuic::byte_t *serialized_buffer_ptr = nullptr;
    std::unique_ptr<kuic::byte_t []> serialized_buffer;
    size_t size = 0;

    // serialize checksum
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(new kuic::byte_t[this->checksum.size()]);
    std::copy(this->checksum.begin(), this->checksum.end(), serialized_buffer.get());
    temporary_msg.insert(kuic::handshake::tag_checksum, serialized_buffer.get(), this->checksum.size());

    // serialize issue realm
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(new kuic::byte_t[this->issue_realm.size()]);
    std::copy(this->issue_realm.begin(), this->issue_realm.end(), serialized_buffer.get());
    temporary_msg.insert(kuic::handshake::tag_issue_realm, serialized_buffer.get(), this->issue_realm.size());

    // serialize issue name
    std::tie(serialized_buffer_ptr, size) = this->issue_name.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    temporary_msg.insert(kuic::handshake::tag_issud_principal_name, serialized_buffer.get(), size);
    
    // serialize elements
    std::tie(serialized_buffer_ptr, size) = this->elements.serialize();
    serialized_buffer = std::unique_ptr<kuic::byte_t []>(serialized_buffer_ptr);
    temporary_msg.insert(kuic::handshake::tag_authorization_data, serialized_buffer.get(), size);

    return temporary_msg.serialize();
};

// issued deserialize
kuic::handshake::kdc_issued_ad_item
kuic::handshake::kdc_issued_ad_item::deserialize(const kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::kdc_issued_ad_item result;
    kuic::handshake::handshake_message temporary_msg = kuic::handshake::handshake_message::deserialize(buffer, len, seek);
    
    // TODO
    
    return result;
}

// split ( | kbr_authorization_data_item)

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
        kuic::byte_t *buffer, size_t len, size_t &seek) {
    kuic::handshake::kbr_authorization_data_item result;

    result.type = kuic::handshake::kbr_authorization_data_type_serializer::deserialize(buffer, len, seek);
    result.data.assign(buffer + seek, buffer + len);
    seek = len;
    return result;
}


std::shared_ptr<kuic::handshake::ad_item>
kuic::handshake::kbr_authorization_data_item::deserialize_item() const {
    size_t seek = 0;

    switch (this->type) {
    case kuic::handshake::ad_type_if_relevant:
        return std::make_shared<kuic::handshake::ad_item>(
                kuic::handshake::if_relevant_ad_item::deserialize(
                    this->data.data(), this->data.size(), seek));
    default:
        return std::make_shared<kuic::handshake::ad_item>(
                kuic::handshake::not_expect_ad_item());
    }
}
