#include "crypt/ecb_mode.h"
#include "gtest/gtest.h"
#include "crypt/sm4.h"
#include <iostream>
#include <iomanip>

TEST(ecb_mode, encrypt) {
    kuic::crypt::ecb_mode mode(new kuic::crypt::sm4());
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
     kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

     mode.set_message(plain, sizeof(plain));
     mode.set_secret_key(secret, sizeof(secret));

     std::pair<kuic::byte_t *, size_t> result = mode.encrypt();

     for (size_t i = 0; i < result.second; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int) (result.first[i]);
     }
}

TEST(ecb_mode, encrypt_2) {
    kuic::crypt::ecb_mode mode(new kuic::crypt::sm4());
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32 };
     kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

     mode.set_message(plain, sizeof(plain));
     mode.set_secret_key(secret, sizeof(secret));

     std::pair<kuic::byte_t *, size_t> result = mode.encrypt();

     for (size_t i = 0; i < result.second; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int) (result.first[i]);
     }
}

TEST(ecb_mode, encrypt_3) {
    kuic::crypt::ecb_mode mode(new kuic::crypt::sm4());
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01 };
     kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

     mode.set_message(plain, sizeof(plain));
     mode.set_secret_key(secret, sizeof(secret));

     std::pair<kuic::byte_t *, size_t> result = mode.encrypt();

     for (size_t i = 0; i < result.second; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int) (result.first[i]);
     }
}


TEST(ecb_mode, decrypt) {
    kuic::crypt::ecb_mode mode(new kuic::crypt::sm4());
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };
     kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

     mode.set_message(plain, sizeof(plain));
     mode.set_secret_key(secret, sizeof(secret));

     std::pair<kuic::byte_t *, size_t> cipher = mode.encrypt();
    
     mode.set_message(cipher.first, cipher.second);

     std::pair<kuic::byte_t *, size_t> plain_pair = mode.decrypt();
     delete cipher.first;

     for (size_t i = 0; i < plain_pair.second; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int) plain_pair.first[i];
     }
     delete plain_pair.first;
}

TEST(ecb_mode, decrypt_2) {
    kuic::crypt::ecb_mode mode(new kuic::crypt::sm4());
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32 };
     kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

     mode.set_message(plain, sizeof(plain));
     mode.set_secret_key(secret, sizeof(secret));

     std::pair<kuic::byte_t *, size_t> cipher = mode.encrypt();
    
     mode.set_message(cipher.first, cipher.second);

     std::pair<kuic::byte_t *, size_t> plain_pair = mode.decrypt();
     delete cipher.first;

     for (size_t i = 0; i < plain_pair.second; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int) plain_pair.first[i];
     }
     delete plain_pair.first;
}


TEST(ecb_mode, decrypt_3) {
    kuic::crypt::ecb_mode mode(new kuic::crypt::sm4());
    kuic::byte_t plain[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01 };
     kuic::byte_t secret[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

     mode.set_message(plain, sizeof(plain));
     mode.set_secret_key(secret, sizeof(secret));

     std::pair<kuic::byte_t *, size_t> cipher = mode.encrypt();
    
     mode.set_message(cipher.first, cipher.second);

     std::pair<kuic::byte_t *, size_t> plain_pair = mode.decrypt();
     delete cipher.first;

     for (size_t i = 0; i < plain_pair.second; i++) {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int) plain_pair.first[i];
     }
}

int main() {
    return RUN_ALL_TESTS();
}
