#include "congestion/cubic.h"
#include "extend_timespec.h"
#include "gtest/gtest.h"
#include <cmath>
#include <iostream>

namespace kuic {
    namespace test {
        class MockClock : public Clock {
        private:
            timespec& clock;

        public:
            MockClock(timespec& clock) : clock(clock) { }
            timespec now() const { return clock; }
        };

        const unsigned int numConnections = 2;
        const float nConnectionBeta = (((float) numConnections) - 1 + kuic::CUBIC_BETA) / ((float) numConnections);
        const float nConnectionAlpha = 3 * ((float) numConnections) * ((float) numConnections) * (1 - nConnectionBeta) / (1 + nConnectionBeta);

        TEST(cubic, works_above_origin) {
            timespec clock = { 0, 0 };
            MockClock mockClock(clock);
            kuic::Cubic cubic(mockClock);

            long rttMin = 100 * 1000 * 1000;
            float rttMinS = (rttMin / (1000 * 1000)) / 1000.0;
            unsigned long currentCWND = 10UL;
            unsigned long expectCWND = currentCWND;

            clock += (long) (1000 * 1000);
            timespec initialTime = clock;
            currentCWND = cubic.congestionWindowAfterAck(currentCWND, rttMin);
            EXPECT_EQ(expectCWND, currentCWND);
            currentCWND = expectCWND;
            unsigned long initialCWND = currentCWND;

            int maxRenoRTTs = (int) (sqrt((double) (nConnectionAlpha / (0.4 * rttMinS * rttMinS * rttMinS))) - 1);
            for (int i = 0; i < maxRenoRTTs; i++) {
                unsigned long maxPerAckCWND = currentCWND;
                for (unsigned long n = 1UL; n < ((unsigned long) (((double) maxPerAckCWND) / nConnectionAlpha)); n++) {
                    unsigned long nextCWND = cubic.congestionWindowAfterAck(currentCWND, rttMin);
                    EXPECT_EQ(nextCWND, currentCWND);
                }
                clock += (long) (100 * 1000 * 1000);
                currentCWND = cubic.congestionWindowAfterAck(currentCWND, rttMin);
                expectCWND++;
                EXPECT_EQ(currentCWND, expectCWND);
            }

            for (int i = 0; i < 52; i++) {
                for (unsigned long n = 1UL; n < currentCWND; n++) {
                    EXPECT_EQ(cubic.congestionWindowAfterAck(currentCWND, rttMin), currentCWND);
                }
                clock += 100 * 1000 * 1000;
                currentCWND = cubic.congestionWindowAfterAck(currentCWND, rttMin);
            }

            float elapsedTimeS = ((float) (clock - initialTime + rttMin)) / ((float) 1000 * 1000 * 1000);
            expectCWND = initialCWND + ((unsigned long) ((elapsedTimeS * elapsedTimeS * elapsedTimeS * 410) / 1024));

            EXPECT_EQ(currentCWND, expectCWND);
        }

        TEST(cubic, manages_loss_events) {
            timespec clock;
            MockClock mockClock(clock);
            kuic::Cubic cubic(mockClock);

            long rttMin = 100 * 1000 * 1000;
            unsigned long currentCWND = 422UL;
            unsigned long expectedCWND = currentCWND;

            clock += 1000 * 1000;

            EXPECT_EQ(cubic.congestionWindowAfterAck(currentCWND, rttMin), expectedCWND);
            expectedCWND = (unsigned long) (((float) currentCWND) * nConnectionBeta);
            EXPECT_EQ(cubic.congestionWindowAfterPacketLoss(currentCWND), expectedCWND);
            expectedCWND = (unsigned long) (((float) currentCWND) * nConnectionBeta);
            EXPECT_EQ(cubic.congestionWindowAfterPacketLoss(currentCWND), expectedCWND);
        }

        TEST(cubic, works_below_origin) {
            timespec clock;
            MockClock mockClock(clock);
            kuic::Cubic cubic(mockClock);

            long rttMin = 100 * 1000 * 1000;
            unsigned long currentCWND = 422UL;
            unsigned long expectedCWND = currentCWND;

            clock += (long) (1000 * 1000);

            EXPECT_EQ(cubic.congestionWindowAfterAck(currentCWND, rttMin), expectedCWND);
            expectedCWND = (unsigned long) (((float) currentCWND) * nConnectionBeta);
            EXPECT_EQ(cubic.congestionWindowAfterPacketLoss(currentCWND), expectedCWND);
            currentCWND = expectedCWND;
            currentCWND = cubic.congestionWindowAfterAck(currentCWND, rttMin);

            for (int i = 0; i < 40; i++) {
                clock += 100 * 1000 * 1000;
                currentCWND = cubic.congestionWindowAfterAck(currentCWND, rttMin);
            }

            expectedCWND = 449UL;
            EXPECT_EQ(currentCWND, expectedCWND);
        }
    }
}

int main() {
    RUN_ALL_TESTS();

    return 0;
}