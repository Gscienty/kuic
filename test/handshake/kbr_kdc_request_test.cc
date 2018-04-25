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

   std::for_each(buff.begin(), buff.end(), [] (const kuic::byte_t &b) -> void {
           std::cout 
                << std::hex 
                << std::setw(2) 
                << std::setfill('0')
                << int(b);
    });

    sepical_in_buffer in_bufferer(buff);

    kuic::handshake::handshake_message msg;
    kuic::error_t err;
    std::tie(msg, err) = kuic::handshake::handshake_message::parse_handshake_message(in_bufferer);

    kuic::handshake::kbr_kdc_request rec_req = kuic::handshake::kbr_kdc_request::deserialize(msg);
}

int main() {
    return  RUN_ALL_TESTS();
}
