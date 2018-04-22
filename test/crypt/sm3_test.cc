#include "crypt/sm3.h"
#include "gtest/gtest.h"

TEST(sm3, hasher) {
    kuic::byte_t msg[] = {
        97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100,
        97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100,
        97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100,
        97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100, 97, 98, 99, 100,
    };

    kuic::byte_t *hash_code_ptr = kuic::crypt::sm3::hash(msg, sizeof(msg));
    for (int i = 0; i < 32; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) hash_code_ptr[i];
    }
}

int main() {
    return RUN_ALL_TESTS();
}