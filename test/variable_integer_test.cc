#include "variable_integer.h"
#include "gtest/gtest.h"

TEST(varint, decoding_1_byte) {
    kuic::byte_t buffer[] = { 25 };
    size_t seek = 0;

    EXPECT_EQ(25, kuic::variable_integer::read(buffer, sizeof(buffer), seek));
}

TEST(varint, decoding_2_byte) {
    kuic::byte_t buffer[] = { 0x40, 0x25 };
    size_t seek = 0;

    EXPECT_EQ(37, kuic::variable_integer::read(buffer, sizeof(buffer), seek));

    kuic::byte_t buffer2[] = { 0x7B, 0xBD };
    seek = 0;
    EXPECT_EQ(15293, kuic::variable_integer::read(buffer2, sizeof(buffer2), seek));
}

TEST(varint, decoding_4_byte) {
    kuic::byte_t buffer[] = { 0x9D, 0x7F, 0x3E, 0x7D };
    size_t seek = 0;

    EXPECT_EQ(494878333, kuic::variable_integer::read(buffer, sizeof(buffer), seek));
}

TEST(varint, decoding_8_byte) {
    kuic::byte_t buffer[] = { 0xC2, 0x19, 0x7C, 0x5E, 0xFF, 0x14, 0xE8, 0x8C };
    size_t seek = 0;

    EXPECT_EQ(151288809941952652L, kuic::variable_integer::read(buffer, sizeof(buffer), seek));
}

TEST(varint, encoding_8_byte) {
    std::pair<kuic::byte_t *, size_t> buffer = kuic::variable_integer::write(151288809941952652L);
    EXPECT_EQ(0xC2, buffer.first[0]);
    EXPECT_EQ(0x19, buffer.first[1]);
    EXPECT_EQ(0x7C, buffer.first[2]);
    EXPECT_EQ(0x5E, buffer.first[3]);
    EXPECT_EQ(0xFF, buffer.first[4]);
    EXPECT_EQ(0x14, buffer.first[5]);
    EXPECT_EQ(0xE8, buffer.first[6]);
    EXPECT_EQ(0x8C, buffer.first[7]);
}

int main() {
    return RUN_ALL_TESTS();
}
