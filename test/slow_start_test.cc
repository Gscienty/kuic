#include "congestion/slow_start.h"
#include "define.h"
#include "gtest/gtest.h"

namespace kuic {
    namespace test {
        TEST(slow_start, works_in_a_simple_case) {
            kuic::SlowStart slowStart;

            unsigned long packetNumber = 1;
            unsigned long endPacketNumber = 3;
            slowStart.startReceiveRound(endPacketNumber);

            packetNumber++;
            EXPECT_FALSE(slowStart.isEndOfRound(packetNumber));
            EXPECT_FALSE(slowStart.isEndOfRound(packetNumber));
            packetNumber++;
            EXPECT_FALSE(slowStart.isEndOfRound(packetNumber));
            packetNumber++;
            EXPECT_TRUE(slowStart.isEndOfRound(packetNumber));

            endPacketNumber = 20;
            slowStart.startReceiveRound(endPacketNumber);
            while (packetNumber < endPacketNumber) {
                packetNumber++;
                EXPECT_FALSE(slowStart.isEndOfRound(packetNumber));
            }

            packetNumber++;
            EXPECT_TRUE(slowStart.isEndOfRound(packetNumber));
        }

        TEST(slow_start, work_with_delay) {
            kuic::SlowStart slowStart;
            long rtt = 60 * 1000 * 1000;
            int minSamples = 8;

            unsigned long endPacketNumber = 1UL;
            endPacketNumber++;
            slowStart.startReceiveRound(endPacketNumber);

            for (int i = 0; i < minSamples; i++) {
                EXPECT_FALSE(slowStart.shouldExitSlowStart(rtt + i * 1000 * 1000, rtt, 100));
            }

            endPacketNumber++;
            slowStart.startReceiveRound(endPacketNumber);
            for (int i = 1; i < minSamples; i++) {
                EXPECT_FALSE(slowStart.shouldExitSlowStart(rtt + (i + 10) * 1000 * 1000, rtt, 100));
            }

            EXPECT_TRUE(slowStart.shouldExitSlowStart(rtt * 10 * 1000 * 1000, rtt, 100));
        }
    }
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}