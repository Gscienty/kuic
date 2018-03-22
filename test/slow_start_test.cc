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
        }
    }
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}