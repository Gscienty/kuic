#include "crypt/aead_sm4_gcm.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <iostream>

TEST(aead_sm4_gcm, enc) {
    kuic::byte_t key[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    kuic::byte_t iv[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    std::string key_str(key, key + sizeof(key));
    std::string iv_str(iv, iv + sizeof(iv));

    kuic::crypt::aead_sm4_gcm aead(key_str, iv_str);

    kuic::byte_t plain[] = {
        0x00, 0x00, 0x00, 0x00
    };

    kuic::byte_t a_data[] = {
        0xFF, 0xFF, 0xFF, 0xFF
    };

    std::string plain_str(plain, plain + sizeof(plain));
    std::string a_data_str(a_data, a_data + sizeof(a_data));

    std::string ciphe_str = aead.seal(plain_str, 1, a_data_str);

    std::for_each(ciphe_str.begin(), ciphe_str.end(),
            [] (const kuic::byte_t b) -> void {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) b << ' ';
            });

    *ciphe_str.rbegin() = 0x00;
    std::string rplain_str = aead.open(ciphe_str, 1, a_data_str);

    std::cout << "NEXT:" << std::endl;
    std::for_each(rplain_str.begin(), rplain_str.end(),
            [] (const kuic::byte_t b) -> void {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int) b;
            });
    std::cout << "FINISHED" << std::endl;
}

int main() {
    return RUN_ALL_TESTS();
}
