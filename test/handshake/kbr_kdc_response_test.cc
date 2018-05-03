#include "handshake/kbr_kdc_response.h"
#include "gtest/gtest.h"
#include <iostream>
#include <iomanip>

TEST(kb_kdc_response, sample_deserialize_part) {
    kuic::handshake::kbr_kdc_response_part part(0x12345678);
    
    kuic::byte_t secret_key[] = {
        0x67, 0x45, 0x23, 0x01,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    size_t seek = 0;
    part.set_key(kuic::handshake::kbr_encryption_key::deserialize(secret_key, sizeof(secret_key), seek));
    size_t size;
    kuic::byte_t *serialized_buffer_ptr = nullptr;
    std::tie(serialized_buffer_ptr, size) = part.serialize();

    seek = 0;
    kuic::handshake::kbr_kdc_response_part deserialized_part = kuic::handshake::kbr_kdc_response_part::deserialize(
            serialized_buffer_ptr, size, seek);
    delete[] serialized_buffer_ptr;

    EXPECT_EQ(0x12345678, deserialized_part.get_nonce());

    kuic::handshake::kbr_encryption_key secret_key_obj = deserialized_part.get_encryption_key();

    EXPECT_EQ(0x67452301, secret_key_obj.get_type());

    std::vector<kuic::byte_t> secret_key_vec = secret_key_obj.get_value();

    for (size_t i = 0; i < secret_key_vec.size(); i++) {
        EXPECT_EQ(secret_key[i + 4], secret_key_vec[i]);
    }
}

TEST(kbc_kdc_response, sample_deserialize) {
    kuic::byte_t client_secret_key[] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11
    };

    kuic::handshake::kbr_kdc_response_part part(0x12345678);
    kuic::byte_t secret_key[] = {
        0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    size_t seek = 0;
    part.set_key(kuic::handshake::kbr_encryption_key::deserialize(secret_key, sizeof(secret_key), seek));

    kuic::handshake::kbr_kdc_response response = kuic::handshake::kbr_kdc_response::build_as_response(
            "realm",
            kuic::kbr_encryption_type_sm4_ecb,
            client_secret_key,
            sizeof(client_secret_key),
            part);

    kuic::byte_t *serialized_response = nullptr;
    size_t size = 0;
    seek = 0;
    std::tie(serialized_response, size) = response.serialize();

    kuic::handshake::kbr_kdc_response deserialized_response = kuic::handshake::kbr_kdc_response::deserialize(serialized_response, size, seek);

    EXPECT_EQ(0, deserialized_response.get_realm().compare("realm"));
    EXPECT_EQ(kuic::handshake::kbr_kdc_as_response, deserialized_response.get_message_type());

    kuic::byte_t *plain_text = nullptr;
    size_t plain_text_size = 0;
    seek = 0;
    std::tie(plain_text, plain_text_size) = deserialized_response.get_encryption_key().get_plain_message(client_secret_key, sizeof(client_secret_key));

    kuic::handshake::kbr_kdc_response_part deserialized_part = kuic::handshake::kbr_kdc_response_part::deserialize(plain_text, plain_text_size, seek);
    EXPECT_EQ(0x12345678, deserialized_part.get_nonce());
    EXPECT_EQ(0x89abcdef, deserialized_part.get_encryption_key().get_type());

    for (size_t i = 0; i < 16; i++) {
        EXPECT_EQ(secret_key[i + 4], deserialized_part.get_encryption_key().get_value()[i]);
    }
}

int main() {
    return RUN_ALL_TESTS();
}
