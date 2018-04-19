#include "clock.h"
#include "congestion/cubic.h"
#include "gtest/gtest.h"

const unsigned int connections_count = 2;
const float connection_beta = (float(connections_count) - 1 + kuic::congestion::cubic_beta) / float(connections_count);
const float connection_alpha = 3 * float(connections_count) * float(connections_count) * (1 - connection_beta) / (1 + connection_beta);

TEST(cubic, works_below_origin) {
    kuic::special_clock clock({ 0, 0 });
    kuic::congestion::cubic cubic(clock);

    kuic::kuic_time_t rtt_min = 100 * kuic::clock_millisecond;
    kuic::packet_number_t current_cwnd = 422;
    kuic::packet_number_t expected_cwnd = current_cwnd;

    clock += kuic::clock_millisecond;

    EXPECT_EQ(cubic.congestion_window_after_ack(current_cwnd, rtt_min), expected_cwnd);

    expected_cwnd = kuic::kuic_time_t(float(current_cwnd) * connection_beta);

    EXPECT_EQ(cubic.congestion_window_after_packet_loss(current_cwnd), expected_cwnd);

    current_cwnd = expected_cwnd;
    current_cwnd = cubic.congestion_window_after_ack(current_cwnd, rtt_min);

    for (int i = 0; i < 40; i++) {
        clock += 100 * kuic::clock_millisecond;
        current_cwnd = cubic.congestion_window_after_ack(current_cwnd, rtt_min);
    }

    expected_cwnd = 449;
    EXPECT_EQ(current_cwnd, expected_cwnd);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}