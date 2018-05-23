#include "ackhandler/send_packet_handler.h"
#include "frame/stream_frame.h"
#include "frame/ping_frame.h"
#include "gtest/gtest.h"
#include <iostream>

TEST(send_packet_handler, determine_packet_number_length) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    handler.get_largest_acked() = 0x1337;

    EXPECT_EQ(2, handler.get_packet_number_length(0x1338));
    EXPECT_EQ(4, handler.get_packet_number_length(0xFFFFFFF));
}

kuic::ackhandler::packet &retransmittable_packet(kuic::ackhandler::packet &packet) {
    if (packet.length == 0) {
        packet.length = 1;
    }
    if (packet.send_time.is_zero()) {
        packet.send_time = kuic::current_clock();
    }
    
    packet.frames.push_back(std::make_shared<kuic::frame::ping_frame>(*(new kuic::frame::ping_frame())));

    return packet;
}

kuic::ackhandler::packet &non_retransmittable_packet(kuic::ackhandler::packet &packet) {
    packet = retransmittable_packet(packet);

    packet.frames.clear();
    
    kuic::frame::ack_frame *ack_frame = new kuic::frame::ack_frame();
    ack_frame->get_ranges().push_back(
            std::pair<kuic::packet_number_t, kuic::packet_number_t>(1 ,1));

    packet.frames.push_back(std::make_shared<kuic::frame::ack_frame>(*ack_frame));

    return packet;
}

void expect_in_packet_history(kuic::packet_number_t expected[], int size, kuic::ackhandler::send_packet_handler &handler) {
    for (int i = 0; i < size; i++) {
        EXPECT_FALSE(handler.get_send_packet_history().get_packet(expected[i]).is_null());
    }
}

TEST(send_packet_handler, accpets_two_consecutive_packets) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;

    p1.packet_number = 1;
    p2.packet_number = 2;
    
    handler.sent_packet(retransmittable_packet(p1));
    handler.sent_packet(retransmittable_packet(p2));

    kuic::packet_number_t nums[] = { 1, 2 };
    expect_in_packet_history(nums, 2, handler);

    EXPECT_EQ(2, handler.get_bytes_in_flight());
    EXPECT_TRUE(handler.get_skipped_packets().empty());
}

TEST(send_packet_handler, accepts_packet_number_0) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;

    p1.packet_number = 0;
    handler.sent_packet(retransmittable_packet(p1));
    EXPECT_EQ(0, handler.get_last_sent_packet_number());

    p2.packet_number = 1;
    handler.sent_packet(retransmittable_packet(p2));
    EXPECT_EQ(1, handler.get_last_sent_packet_number());

    kuic::packet_number_t nums[] = { 0, 1 };
    expect_in_packet_history(nums, 2, handler);

    EXPECT_EQ(2, handler.get_bytes_in_flight());
    EXPECT_TRUE(handler.get_skipped_packets().empty());
}

TEST(send_packet_handler, store_sent_time) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock send_time = kuic::special_clock(kuic::current_clock()) + (-kuic::clock_second * 60);

    kuic::ackhandler::packet p1;
    p1.packet_number = 1;
    p1.send_time = send_time;

    handler.sent_packet(retransmittable_packet(p1));

    EXPECT_EQ(send_time - 0, handler.get_last_sent_retransmittable_packet_time() - 0);
}

TEST(send_packet_handler, store_handshake_time) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock send_time = kuic::special_clock(kuic::current_clock()) + (-kuic::clock_second * 60);

    kuic::ackhandler::packet p1;
    p1.packet_number = 1;
    p1.send_time = send_time;
    p1.is_handshake = true;

    kuic::ackhandler::packet p2;
    p2.packet_number = 2;
    p2.send_time = send_time + 60 * 60 * kuic::clock_second;
    p2.is_handshake = false;

    handler.sent_packet(retransmittable_packet(p1));
    handler.sent_packet(retransmittable_packet(p2));

    EXPECT_EQ(send_time - 0, handler.get_last_sent_handshake_packet_time() - 0);
}

TEST(send_packet_handler, dose_not_store_non_retransmittable_packets) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::ackhandler::packet p1;
    p1.packet_number = 1;
    kuic::special_clock send_time = kuic::special_clock(kuic::current_clock()) + (-kuic::clock_second * 60);
    p1.send_time = send_time;

    handler.sent_packet(non_retransmittable_packet(p1));

    EXPECT_EQ(0, handler.get_send_packet_history().size());
    EXPECT_TRUE(handler.get_last_sent_handshake_packet_time().is_zero());
    EXPECT_EQ(0, handler.get_bytes_in_flight());
}

TEST(send_packet_handler, works1) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;

    p1.packet_number = 1;
    p2.packet_number = 3;
    handler.sent_packet(retransmittable_packet(p1));
    handler.sent_packet(retransmittable_packet(p2));

    EXPECT_EQ(3, handler.get_last_sent_packet_number());

    kuic::packet_number_t nums[] = { 1, 3 };
    expect_in_packet_history(nums, 2, handler);
    
    EXPECT_EQ(1, handler.get_skipped_packets().size());
    EXPECT_EQ(2, handler.get_skipped_packets().front());
}

TEST(send_packet_handler, work2) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;

    p1.packet_number = 1;
    p2.packet_number = 3;
    handler.sent_packet(non_retransmittable_packet(p1));
    handler.sent_packet(non_retransmittable_packet(p2));

    EXPECT_EQ(1, handler.get_skipped_packets().size());
    EXPECT_EQ(2, handler.get_skipped_packets().front());

}

TEST(send_packet_handler, work3) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;
    kuic::ackhandler::packet p3;
    kuic::ackhandler::packet p4;

    p1.packet_number = 1;
    p2.packet_number = 3;
    p3.packet_number = 5;
    p4.packet_number = 8;
    
    handler.sent_packet(retransmittable_packet(p1));
    handler.sent_packet(retransmittable_packet(p2));
    handler.sent_packet(retransmittable_packet(p3));
    handler.sent_packet(retransmittable_packet(p4));
    

    EXPECT_EQ(4, handler.get_skipped_packets().size());
    EXPECT_EQ(2, handler.get_skipped_packets().front());
    auto i = handler.get_skipped_packets().begin();
    i++;
    EXPECT_EQ(4, *i);
    i++;
    EXPECT_EQ(6, *i);
    i++;
    EXPECT_EQ(7, *i);

}

TEST(send_packet_handler, garbage) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
 
    handler.get_skipped_packets().push_back(2);
    handler.get_skipped_packets().push_back(5);
    handler.get_skipped_packets().push_back(8);
    handler.get_skipped_packets().push_back(10);

    handler.get_largest_acked() = 1;
    handler.garbage_collect_skipped_packets();

    EXPECT_EQ(4, handler.get_skipped_packets().size());

    handler.get_largest_acked() = 5;
    handler.garbage_collect_skipped_packets();

    EXPECT_EQ(2, handler.get_skipped_packets().size());

    handler.get_largest_acked() = 15;
    handler.garbage_collect_skipped_packets();

    EXPECT_EQ(0, handler.get_skipped_packets().size());
}

TEST(send_packet_handler, ack_handling) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;
    
    p1.packet_number = 10;
    p2.packet_number = 12;

    handler.sent_packet(retransmittable_packet(p1));
    handler.sent_packet(retransmittable_packet(p2));

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(
            std::pair<kuic::packet_number_t, kuic::packet_number_t>(10, 12));

    EXPECT_FALSE(
            handler.received_ack(
                ack,
                1337,
                false,
                kuic::special_clock(kuic::current_clock())));


}

TEST(send_packet_handler, ack_handling_2) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;
    
    p1.packet_number = 10;
    p2.packet_number = 12;

    handler.sent_packet(retransmittable_packet(p1));
    handler.sent_packet(retransmittable_packet(p2));

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::pair<kuic::packet_number_t, kuic::packet_number_t>(12, 12));
    ack.get_ranges().push_back(std::pair<kuic::packet_number_t, kuic::packet_number_t>(10, 10));

    EXPECT_TRUE(
            handler.received_ack(ack, 1337, false, kuic::special_clock(kuic::current_clock())));
    EXPECT_NE(0, handler.get_largest_acked());
}

#include <iostream>

int main() {
    return RUN_ALL_TESTS();
}
