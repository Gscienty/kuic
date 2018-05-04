#include "handshake/kbr_authorization_data.h"
#include "gtest/gtest.h"
#include <utility>

TEST(ad, sample_serialize) {
    kuic::handshake::kbr_authorization_data ad;

    std::pair<kuic::byte_t *, size_t> tuple = ad.serialize();

    EXPECT_EQ(4, tuple.second);
}

TEST(ad, serialize_one_item) {
    kuic::handshake::kbr_authorization_data ad;
    
    kuic::byte_t if_relevant_buffer[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    kuic::handshake::if_relevant_ad_item element(if_relevant_buffer, sizeof(if_relevant_buffer));
    ad.add_element(kuic::handshake::kbr_authorization_data_item(&element));

    std::pair<kuic::byte_t *, size_t> tuple = ad.serialize();

    // elements count (4) + one element offset (4) + element type (4) + value (8)
    EXPECT_EQ(4 + 4 + 4 + 8, tuple.second);
}

TEST(ad, serialize_two_item) {
    kuic::handshake::kbr_authorization_data ad;

    kuic::byte_t if_relevant_buffer_1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    kuic::byte_t if_relevant_buffer_2[] = { 0x01, 0x02, 0x03, 0x04 };

    kuic::handshake::if_relevant_ad_item element_1(if_relevant_buffer_1, sizeof(if_relevant_buffer_1));
    kuic::handshake::if_relevant_ad_item element_2(if_relevant_buffer_2, sizeof(if_relevant_buffer_2));

    ad.add_element(kuic::handshake::kbr_authorization_data_item(&element_1));
    ad.add_element(kuic::handshake::kbr_authorization_data_item(&element_2));

    std::pair<kuic::byte_t *, size_t> tuple = ad.serialize();

    // elements count (4) + two element offset (8)
    // + first element type (4) + first element value (8) 
    // + second element type (4) + second element value (4)
    EXPECT_EQ(4 + 8 + 4 + 8 + 4 + 4, tuple.second);
}

TEST(ad, deserialize_two_item) {
    kuic::handshake::kbr_authorization_data ad;

    kuic::byte_t if_relevant_buffer_1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    kuic::byte_t if_relevant_buffer_2[] = { 0x01, 0x02, 0x03, 0x04 };

    kuic::handshake::if_relevant_ad_item element_1(if_relevant_buffer_1, sizeof(if_relevant_buffer_1));
    kuic::handshake::if_relevant_ad_item element_2(if_relevant_buffer_2, sizeof(if_relevant_buffer_2));

    ad.add_element(kuic::handshake::kbr_authorization_data_item(&element_1));
    ad.add_element(kuic::handshake::kbr_authorization_data_item(&element_2));

    std::pair<kuic::byte_t *, size_t> tuple = ad.serialize();

    // elements count (4) + two element offset (8)
    // + first element type (4) + first element value (8) 
    // + second element type (4) + second element value (4)
    EXPECT_EQ(4 + 8 + 4 + 8 + 4 + 4, tuple.second);
    
    size_t seek = 0;
    kuic::handshake::kbr_authorization_data dad = kuic::handshake::kbr_authorization_data::deserialize(
            tuple.first, tuple.second, seek);
    EXPECT_EQ(2, dad.get_elements().size());
    EXPECT_EQ(kuic::handshake::ad_type_if_relevant, dad.get_elements()[0].get_type());

    kuic::handshake::if_relevant_ad_item *item = reinterpret_cast<kuic::handshake::if_relevant_ad_item *>(dad.get_elements()[0].get_item());
    for (size_t i = 0; i < sizeof(if_relevant_buffer_1); i++) {
        EXPECT_EQ(item->get_data()[i], if_relevant_buffer_1[i]);
    }
}

int main () {
    return RUN_ALL_TESTS();
}
