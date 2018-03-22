#include "congestion/proportional_rate_reduction.h"
#include "define.h"
#include "gtest/gtest.h"
#include <limits>

namespace kuic {
    namespace test {
        TEST(prr, single_loss_results_in_send_on_every_other_ack) {
            kuic::ProportionalRateReduction prr;

            unsigned long numPacketsInFlight = 50UL;
            unsigned long bytesInFlight = numPacketsInFlight * kuic::DEFAULT_TCP_MSS;
            unsigned long sshthreshAfterLoss = numPacketsInFlight / 2;
            unsigned long congestionWindow = sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS;

            prr.onPacketLost(bytesInFlight);
            prr.onPacketAcked(kuic::DEFAULT_TCP_MSS);
            bytesInFlight -= kuic::DEFAULT_TCP_MSS;

            EXPECT_EQ(0, prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
            
            prr.onPacketSent(kuic::DEFAULT_TCP_MSS);

            EXPECT_EQ(
                std::numeric_limits<long>::max(),
                prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));

            for (unsigned long i = 0UL; i < sshthreshAfterLoss - 1; i++) {
                prr.onPacketAcked(kuic::DEFAULT_TCP_MSS);
                bytesInFlight -= kuic::DEFAULT_TCP_MSS;
                EXPECT_EQ(
                    std::numeric_limits<long>::max(),
                    prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
                prr.onPacketAcked(kuic::DEFAULT_TCP_MSS);
                bytesInFlight -= kuic::DEFAULT_TCP_MSS;
                prr.onPacketSent(kuic::DEFAULT_TCP_MSS);
                bytesInFlight += kuic::DEFAULT_TCP_MSS;
            }

            EXPECT_EQ(congestionWindow, bytesInFlight);

            for (int i = 0; i < 10; i++) {
                prr.onPacketAcked(kuic::DEFAULT_TCP_MSS);
                bytesInFlight -= kuic::DEFAULT_TCP_MSS;
                EXPECT_EQ(0, prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
                prr.onPacketSent(kuic::DEFAULT_TCP_MSS);
                bytesInFlight += kuic::DEFAULT_TCP_MSS;

                EXPECT_EQ(congestionWindow, bytesInFlight);
                EXPECT_EQ(
                    std::numeric_limits<long>::max(),
                    prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
            }
        }

        TEST(prr, burst_loss_results_in_slow_start) {
            kuic::ProportionalRateReduction prr;
            unsigned long bytesInFlight = 20 * kuic::DEFAULT_TCP_MSS;
            const int numPacketsLost = 13;
            const unsigned long sshthreshAfterLoss = 10;
            const unsigned long congestionWindow = sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS;

            bytesInFlight -= numPacketsLost * kuic::DEFAULT_TCP_MSS;
            prr.onPacketLost(bytesInFlight);

            for (int i = 0; i < 3; i++) {
                prr.onPacketAcked(kuic::DEFAULT_TCP_MSS);
                bytesInFlight -= kuic::DEFAULT_TCP_MSS;
                for (int j = 0; j < 2; j++) {
                    EXPECT_EQ(0, prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
                    prr.onPacketSent(kuic::DEFAULT_TCP_MSS);
                    bytesInFlight += kuic::DEFAULT_TCP_MSS;
                }
                EXPECT_EQ(
                    std::numeric_limits<long>::max(),
                    prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
            }

            for (int i = 0; i < 10; i++) {
                prr.onPacketAcked(kuic::DEFAULT_TCP_MSS);
                bytesInFlight -= kuic::DEFAULT_TCP_MSS;
                EXPECT_EQ(0, prr.timeUntilSend(congestionWindow, bytesInFlight, sshthreshAfterLoss * kuic::DEFAULT_TCP_MSS));
                prr.onPacketSent(kuic::DEFAULT_TCP_MSS);
                bytesInFlight += kuic::DEFAULT_TCP_MSS;
            }
        }
    }
}

int main() {
    int result = RUN_ALL_TESTS();
    return 0;
}