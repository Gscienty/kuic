#include "crypt/sm4.h"
#include "gtest/gtest.h"
#include <memory>
#include <iostream>
#include <iomanip>

TEST(sm4, sample) {
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    
    kuic::crypt::sm4 sm4er;

    kuic::byte_t *cipher = sm4er.encrypt(plain, secret);
    for (int i = 0; i < 16; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) cipher[i];
    }
}
TEST(sm4, sample2) {
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };

    kuic::crypt::sm4 sm4er;

    kuic::byte_t *cipher = sm4er.encrypt(plain, secret);
    kuic::byte_t *text = sm4er.decrypt(cipher, secret);

    for (int i = 0; i < 16; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) text[i];
    }
}

int main() {
    return RUN_ALL_TESTS();
}
