#include "clock.h"
#include "congestion/cubic.h"
#include "gtest/gtest.h"

const unsigned int connections_count = 2;
const float connection_beta = (float(connections_count) - 1 + kuic::congestion::cubic_beta) / float(connections_count);
const double connection_alpha = 3 * double(connections_count) * double(connections_count) * (1 - connection_beta) / (1 + connection_beta);


TEST(cubic, works_below_origin) {
    kuic::congestion::cubic cubic;

    kuic::kuic_time_t rtt_min = 100 * kuic::clock_millisecond;
    kuic::bytes_count_t current_cwnd = 422 * kuic::default_tcp_mss;
    
    kuic::bytes_count_t expect_cwnd = current_cwnd + kuic::bytes_count_t(
            kuic::default_tcp_mss * connection_alpha * double(kuic::default_tcp_mss) / double(current_cwnd));

    kuic::current_clock current;
    kuic::special_clock c(current);
    EXPECT_EQ(expect_cwnd, 
            cubic.congestion_window_after_ack(kuic::default_tcp_mss, current_cwnd, rtt_min, c));

    expect_cwnd = kuic::bytes_count_t(double(current_cwnd) * connection_beta);
    EXPECT_EQ(expect_cwnd,
            cubic.congestion_window_after_packet_loss(current_cwnd));
}

int main() {
    return RUN_ALL_TESTS();
}
