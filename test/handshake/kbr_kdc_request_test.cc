#include "handshake/kbr_kdc_request.h"
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>

class sepical_in_buffer : public eys::in_buffer {
public:
    sepical_in_buffer(std::vector<kuic::byte_t> &bf)
        : in_buffer(bf.size()) {
        std::uninitialized_copy_n(bf.data(), bf.size(), this->buffer.get());
        this->data_size = bf.size();   
    }
};

TEST(kbr_kdc_request, as_request) {
    kuic::handshake::kbr_kdc_request req = kuic::handshake::kbr_kdc_request::build_as_request(
            kuic::handshake::kbr_principal_name(std::string("user")),
            std::string("local"),
            0x01234567);
   std::vector<kuic::byte_t> buff = req.serialize().serialize();

    sepical_in_buffer in_bufferer(buff);

    kuic::handshake::handshake_message msg;
    kuic::error_t err;
    std::tie(msg, err) = kuic::handshake::handshake_message::parse_handshake_message(in_bufferer);

    kuic::handshake::kbr_kdc_request rec_req = kuic::handshake::kbr_kdc_request::deserialize(msg);

    EXPECT_EQ(0x01234567, rec_req.get_nonce());
    EXPECT_EQ(0, rec_req.get_realm().compare("local"));
    EXPECT_EQ(0, rec_req.get_client_name().get_name().compare("user"));
}

int main() {
    return  RUN_ALL_TESTS();
}
