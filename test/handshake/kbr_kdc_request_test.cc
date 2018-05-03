#include "handshake/kbr_kdc_request.h"
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>

TEST(kbr_kdc_request, as_request) {
    kuic::handshake::kbr_kdc_request req = kuic::handshake::kbr_kdc_request::build_as_request(
            kuic::handshake::kbr_principal_name(std::string("user")),
            std::string("local"),
            0x01234567);
    kuic::byte_t *buff;
    size_t size;
    std::tie(buff, size) = req.serialize();

    size_t seek = 0;
    kuic::handshake::kbr_kdc_request rec_req = kuic::handshake::kbr_kdc_request::deserialize(buff, size, seek);

    EXPECT_EQ(0x01234567, rec_req.get_body().get_nonce());
    EXPECT_EQ(0, rec_req.get_body().get_realm().compare("local"));
    EXPECT_EQ(0, rec_req.get_body().get_client_name().get_name().compare("user"));
}

int main() {
    return  RUN_ALL_TESTS();
}
