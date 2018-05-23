#include "ackhandler/send_packet_history.h"
#include "gtest/gtest.h"
#include <vector>
#include <algorithm>

TEST(ackhandler, save_sent_packets) {
    kuic::ackhandler::send_packet_history hist;

    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;
    kuic::ackhandler::packet p3;

    p1.packet_number = 1;
    p2.packet_number = 2;
    p3.packet_number = 3;

    EXPECT_TRUE(hist.get_first_outstanding().is_null());

    hist.send_packet(p1);
    hist.send_packet(p2);
    hist.send_packet(p3);

    EXPECT_FALSE(hist.get_first_outstanding().is_null());
    EXPECT_EQ(1, hist.get_first_outstanding()->packet_number);

    int expect = 0;

    hist.iterate([&] (const kuic::ackhandler::packet &p) -> bool {
        EXPECT_EQ(++expect, p.packet_number);
        return true;
    });

    EXPECT_EQ(3, hist.size());

    EXPECT_EQ(2, hist.get_packet(2)->packet_number);
    EXPECT_TRUE(hist.get_packet(4).is_null());

    hist.remove(2);
    EXPECT_EQ(2, hist.size());
    EXPECT_TRUE(hist.get_packet(2).is_null());
}


TEST(ackhandler, gets_second_packet_if_first_is_retrainsmitted) {
    kuic::ackhandler::send_packet_history hist;

    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;
    kuic::ackhandler::packet p3;

    p1.packet_number = 1;
    p1.can_be_retransmitted = true;
    p2.packet_number = 3;
    p2.can_be_retransmitted = true;
    p3.packet_number = 4;
    p3.can_be_retransmitted = true;
    
    hist.send_packet(p1);
    hist.send_packet(p2);
    hist.send_packet(p3);

    kuic::nullable<kuic::ackhandler::packet> front = hist.get_first_outstanding();
    EXPECT_FALSE(front.is_null());
    EXPECT_EQ(1, front->packet_number);

    hist.mark_cannot_be_retransmitted(1);
    kuic::nullable<kuic::ackhandler::packet> front2 = hist.get_first_outstanding();

    EXPECT_FALSE(front2.is_null());
    EXPECT_EQ(3, front2->packet_number);
    
}



kuic::ackhandler::send_packet_history before() {
    kuic::ackhandler::send_packet_history hist;

    for (int i = 1; i <= 5; i++) {
        kuic::ackhandler::packet p;
        p.packet_number = i;
        hist.send_packet(p);
    }

    return hist;
}

TEST(ackhandler, retransmission_dosent_exist) {
    std::list<kuic::ackhandler::packet> ps;
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;

    p1.packet_number = 13;
    p2.packet_number = 15;

    ps.push_back(p1);
    ps.push_back(p2);

    kuic::ackhandler::send_packet_history hist = before();

    hist.send_packets_as_retrainsmission(ps.begin(), ps.end(), 2, 7);
    
    EXPECT_FALSE(hist.get_packet(13)->is_retransmission);
    EXPECT_EQ(0, hist.get_packet(13)->retransmission_of);
}

TEST(ackhandler, retransmission) {
    std::list<kuic::ackhandler::packet> ps;
    kuic::ackhandler::packet p1;
    kuic::ackhandler::packet p2;

    p1.packet_number = 13;
    p2.packet_number = 15;

    ps.push_back(p1);
    ps.push_back(p2);

    kuic::ackhandler::send_packet_history hist = before();

    hist.send_packets_as_retrainsmission(ps.begin(), ps.end(), 2, 2);

    int expects[] = { 1, 2, 3, 4, 5, 13, 15 };
    int i = 0;
    hist.iterate([&] (const kuic::ackhandler::packet &p) -> bool {
            EXPECT_EQ(expects[i++], p.packet_number);
            return true;
        });

    EXPECT_TRUE(hist.get_packet(13)->is_retransmission);
    EXPECT_EQ(2, hist.get_packet(13)->retransmission_of);
    EXPECT_EQ(2, hist.get_packet(15)->retransmission_of);

    EXPECT_EQ(2, hist.get_packet(2)->retransmitted_as.size());

    int expects2[] = { 13, 15 };
    i = 0;

    kuic::nullable<kuic::ackhandler::packet> p = hist.get_packet(2);
    std::for_each(p->retransmitted_as.begin(), p->retransmitted_as.end(),
            [&] (const kuic::packet_number_t &n) -> void {
            
            EXPECT_EQ(expects2[i++], n);
        });
}

int main() {
    return RUN_ALL_TESTS();
}
