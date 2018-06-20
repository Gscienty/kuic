#include "ackhandler/send_packet_handler.h"
#include "frame/stream_frame.h"
#include "frame/ping_frame.h"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>

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
        packet.send_time = kuic::special_clock(kuic::current_clock());
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
        EXPECT_FALSE(handler.get_send_packet_history().get_packet(expected[i]) == nullptr);
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

void update_rtt(kuic::ackhandler::send_packet_handler &handler, kuic::kuic_time_t rtt) {
    handler.get_rtt().update_rtt(rtt, 0);
}

TEST(send_packet_handler, ack_processing) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        handler.sent_packet(retransmittable_packet(p));
    }

    update_rtt(handler, kuic::clock_second * 60 * 60);
    EXPECT_EQ(10, handler.get_bytes_in_flight());
}

TEST(send_packet_handler, ack_validation__accepts_acks_sent_in_packet_0) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        handler.sent_packet(retransmittable_packet(p));
    }


    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::pair<kuic::packet_number_t, kuic::packet_number_t>(0, 5));
    EXPECT_TRUE(handler.received_ack(ack, 0, false, kuic::special_clock(kuic::current_clock())));
}

TEST(send_packet_handler, ack_validation__rejects_duplicate_acks) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        handler.sent_packet(retransmittable_packet(p));
    }

    kuic::frame::ack_frame ack1;
    ack1.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(3)));
    kuic::frame::ack_frame ack2;
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(4)));

    EXPECT_TRUE(handler.received_ack(ack1, 1337, false, kuic::special_clock(kuic::current_clock())));
    EXPECT_EQ(handler.get_largest_acked(), 3);

    EXPECT_TRUE(handler.received_ack(ack2, 1337, false, kuic::special_clock(kuic::current_clock())));
    EXPECT_EQ(handler.get_largest_acked(), 3);
}

TEST(send_packet_handler, ack_validation__rejects_out_of_order_acks) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        handler.sent_packet(retransmittable_packet(p));
    }

    kuic::frame::ack_frame ack1;
    ack1.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(3)));
    kuic::frame::ack_frame ack2;
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(4)));
    EXPECT_TRUE(handler.received_ack(ack1, 1337, false, kuic::special_clock(kuic::current_clock())));
    EXPECT_TRUE(handler.received_ack(ack2, 1337 - 1, false, kuic::special_clock(kuic::current_clock())));
    EXPECT_EQ(handler.get_largest_acked(), 3);
}

TEST(send_packet_handler, ack_validation__rejects_acks_with_a_too_high_largest_acked_packet_number) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        handler.sent_packet(retransmittable_packet(p));
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(9999)));
    EXPECT_FALSE(handler.received_ack(ack, 0, false, kuic::special_clock(kuic::current_clock())));
    EXPECT_EQ(handler.get_bytes_in_flight(), 10);
}


TEST(send_packet_handler, ack_validation__ignores_repeated_acks) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();

    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(3)));
    kuic::current_clock current;
    kuic::special_clock now(current);

    EXPECT_TRUE(handler.received_ack(ack, 1337, false, now));
    EXPECT_EQ(handler.get_bytes_in_flight(), 7);
    EXPECT_TRUE(handler.received_ack(ack, 1337 + 1, false, now));
    EXPECT_EQ(handler.get_largest_acked(), 3);
    EXPECT_EQ(handler.get_bytes_in_flight(), 7);
}

TEST(send_packet_handler, adjuests_largestacked_adjuests_the_bytes_in_flight) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(5)));
    EXPECT_TRUE(handler.received_ack(ack, 1, false, kuic::current_clock()));

    EXPECT_EQ(handler.get_largest_acked(), 5);
    kuic::packet_number_t ps[] = { 6, 7, 8, 9 };
    expect_in_packet_history(ps, 4, handler);
    EXPECT_EQ(handler.get_bytes_in_flight(), 4);
}

TEST(send_packet_handler, acks_packet_0) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(0)));
    EXPECT_TRUE(handler.received_ack(ack, 1, false, kuic::current_clock()));
    EXPECT_TRUE(handler.get_send_packet_history().get_packet(0) == nullptr);
    kuic::packet_number_t ps[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    expect_in_packet_history(ps, 9, handler);
}

TEST(send_packet_handler, handles_ack_frame_with_one_missing_packet_range) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(6), kuic::packet_number_t(9)));
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(3)));

    EXPECT_TRUE(handler.received_ack(ack, 1, false, kuic::current_clock()));
    kuic::packet_number_t ps[] = { 0, 4, 5 };
    expect_in_packet_history(ps, 3, handler);
}

TEST(send_packet_handler, dose_not_ack_packets_below_the_lowestacked) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(3), kuic::packet_number_t(8)));
    EXPECT_TRUE(handler.received_ack(ack, 1, false, kuic::current_clock()));
    kuic::packet_number_t ps[] = { 0, 1, 2, 9 };
    expect_in_packet_history(ps, 4, handler);
}

TEST(send_packet_handler, handles_an_ack_with_multiple_missing_packet_ranges) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }
    
    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(9), kuic::packet_number_t(9)));
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(6), kuic::packet_number_t(7)));
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(3), kuic::packet_number_t(3)));
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(1)));

    EXPECT_TRUE(handler.received_ack(ack, 1, false, kuic::current_clock()));
    kuic::packet_number_t ps[] = { 0, 2, 4, 5, 8 };
    expect_in_packet_history(ps, 5, handler);
}

TEST(send_packet_handler, processes_an_ack_frame_that_would_be_sent_after_a_late_arrival_of_a_packet) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }
    
    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(4), kuic::packet_number_t(6)));
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(2)));

    EXPECT_TRUE(handler.received_ack(ack, 1, false, time));
    kuic::packet_number_t ps[] = { 0, 3, 7, 8, 9 };
    expect_in_packet_history(ps, 5, handler);

    kuic::frame::ack_frame ack2;
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(6)));

    EXPECT_TRUE(handler.received_ack(ack2, 2, false, time));
    kuic::packet_number_t ps2[] = { 0, 7, 8, 9 };
    expect_in_packet_history(ps2, 4, handler);
    EXPECT_EQ(handler.get_bytes_in_flight(), 4);
}

TEST(send_packet_handler, processes_an_ack_frame_that_would_be_sent_after_a_late_arrival_of_a_packet_and_another_packet) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }
    
    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(4), kuic::packet_number_t(6)));
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(0), kuic::packet_number_t(2)));

    EXPECT_TRUE(handler.received_ack(ack, 1, false, time));
    kuic::packet_number_t ps[] = { 3, 7, 8, 9 };
    expect_in_packet_history(ps, 4, handler);
    EXPECT_EQ(handler.get_bytes_in_flight(), 4);

    kuic::frame::ack_frame ack2;
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(7)));
    EXPECT_TRUE(handler.received_ack(ack2, 2, false, time));
    EXPECT_EQ(handler.get_bytes_in_flight(), 2);
    kuic::packet_number_t ps2[] = { 8, 9 };
    expect_in_packet_history(ps2, 2, handler);
}

TEST(send_packet_handler, processes_an_ack_that_contains_old_ack_ranges) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }
    
    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(6)));
    EXPECT_TRUE(handler.received_ack(ack, 1, false, time));
    kuic::packet_number_t ps[] = { 0, 7, 8, 9 };
    expect_in_packet_history(ps, 4, handler);

    kuic::frame::ack_frame ack2;
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(8), kuic::packet_number_t(8)));
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(3), kuic::packet_number_t(3)));
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(1), kuic::packet_number_t(1)));

    EXPECT_TRUE(handler.received_ack(ack2, 2, false, time));
    kuic::packet_number_t ps2[] = { 0, 7, 9 };
    expect_in_packet_history(ps2, 3, handler);
}

TEST(send_packet_handler, determining_which_acks_we_have_received_an_ack_for) {
    kuic::congestion::rtt _rtt;
    kuic::ackhandler::send_packet_handler handler(_rtt);
    handler.set_handshake_complete();
    
    kuic::special_clock time = kuic::current_clock();
    for (int i = 0; i < 10; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        auto inp = retransmittable_packet(p);
        inp.send_time = time;
        handler.sent_packet(inp);
    }

    kuic::frame::stream_frame str;
    str.get_stream_id() = 5;
    kuic::byte_t data[] = { 0x13, 0x37 };
    str.get_data().assign(data, data + sizeof(data));

    kuic::frame::ack_frame ack1;
    ack1.get_ranges().push_back(std::make_pair(kuic::packet_number_t(80), kuic::packet_number_t(100)));
    kuic::frame::ack_frame ack2;
    ack2.get_ranges().push_back(std::make_pair(kuic::packet_number_t(50), kuic::packet_number_t(200)));
    kuic::ackhandler::packet mock_packets[3];

    mock_packets[0].packet_number = 13;
    mock_packets[0].frames.push_back(std::make_shared<kuic::frame::frame>(ack1));
    mock_packets[0].frames.push_back(std::make_shared<kuic::frame::frame>(str));
    mock_packets[0].length = 1;

    mock_packets[1].packet_number = 14;
    mock_packets[1].frames.push_back(std::make_shared<kuic::frame::frame>(ack2));
    mock_packets[1].frames.push_back(std::make_shared<kuic::frame::frame>(str));
    mock_packets[1].length = 1;

    mock_packets[2].packet_number = 15;
    mock_packets[2].frames.push_back(std::make_shared<kuic::frame::frame>(str));
    mock_packets[2].length = 1;

    for (int i = 0; i < 3; i++) {
        handler.sent_packet(mock_packets[i]);
    }

    kuic::frame::ack_frame ack;
    ack.get_ranges().push_back(std::make_pair(kuic::packet_number_t(13), kuic::packet_number_t(15)));

    EXPECT_TRUE(handler.received_ack(ack, 1, false, time));
    EXPECT_EQ(handler.get_lowest_packet_not_confirmd_acked(), 201);
}

int main() {
    return RUN_ALL_TESTS();
}
