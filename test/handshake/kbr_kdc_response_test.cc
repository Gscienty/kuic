#include "handshake/kbr_kdc_response.h"
#include "gtest/gtest.h"
#include <iostream>
#include <iomanip>

TEST(kbr_kdc_response, sample_serialize_part) {
    kuic::handshake::kbr_kdc_response_part part(0x12345678);
    
    kuic::byte_t secret_key[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    
    part.set_key(kuic::handshake::kbr_encryption_key::deserialize(secret_key, sizeof(secret_key)));
    size_t size;
    kuic::byte_t *serialized_buffer = reinterpret_cast<kuic::byte_t *>(part.serialize(size));

    for (size_t i = 0; i < size; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << int(serialized_buffer[i]);
    }

    delete[] serialized_buffer;
}

TEST(kb_kdc_response, sample_deserialize_part) {
    kuic::handshake::kbr_kdc_response_part part(0x12345678);
    
    kuic::byte_t secret_key[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    
    part.set_key(kuic::handshake::kbr_encryption_key::deserialize(secret_key, sizeof(secret_key)));
    size_t size;
    kuic::byte_t *serialized_buffer = reinterpret_cast<kuic::byte_t *>(part.serialize(size));

    kuic::handshake::kbr_kdc_response_part deserialize_part = kuic::handshake::kbr_kdc_response_part::deserialize(
            serialized_buffer, size);

    delete[] serialized_buffer;
}

int main() {
    return RUN_ALL_TESTS();
}