#include "flowcontrol/connection_flow_controller.h"
#include "gtest/gtest.h"
#include "define.h"

namespace kuic {
    namespace test {
        inline void setRTT(RoundTripStatistics &rtt, long t) {
            rtt.updateRTT(t, 0, CurrentClock().now());
            EXPECT_EQ(t, rtt.getSmoothedRTT());
        }

        TEST(connection_flow_controller, set_send_and_receive_window) {
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindow, maxReceiveWindow);

            EXPECT_EQ(receiveWindow, ctr.getReceiveWindowSize());
            EXPECT_EQ(maxReceiveWindow, ctr.getMaxReceiveWindowSize());
        }

        TEST(connection_flow_controller, increase_highest_received_by_given_window_size) {
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindow, maxReceiveWindow);

            ctr.setHighestReceived(1337);
            ctr.incrementHighestReceived(123);

            EXPECT_EQ(1337 + 123, ctr.getHighestReceived());
        }

        TEST(connection_flow_controller, gets_a_window_update) {
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindow, maxReceiveWindow);
            ctr.setReceiveWindow(100);
            ctr.setReceiveWindowSize(60);
            ctr.setMaxReceiveWindowSize(1000);
            ctr.setReadedBytesCount(100 - 60);

            unsigned long windowSize = ctr.getReceiveWindowSize();
            unsigned long oldOffset = ctr.getReadedBytesCount();
            unsigned long dataRead = windowSize / 2 - 1;
            ctr.addReadedBytesCount(dataRead);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_EQ(offset, oldOffset + dataRead + 60);
        }

        inline long scaleDuration(long t) {
            long scaleFactor = 1;
            return t * scaleFactor;
        }

        TEST(connection_flow_controller, autotunes_the_window) {
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindow, maxReceiveWindow);
            ctr.setReceiveWindow(100);
            ctr.setReceiveWindowSize(60);
            ctr.setMaxReceiveWindowSize(1000);
            ctr.setReadedBytesCount(100 - 60);

            unsigned long oldOffset = ctr.getReadedBytesCount();
            unsigned long oldWindowSize = ctr.getReceiveWindowSize();
            long rttTime = scaleDuration(20 * 1000 * 1000);
            setRTT(rtt, rttTime);
            ctr.setEpochStartTime(CurrentClock().now() + (-1000 * 1000));
            ctr.setEpochStartOffset(oldOffset);
            unsigned long dataRead = oldWindowSize / 2 + 1;
            ctr.addReadedBytesCount(dataRead);
            unsigned long offset = ctr.receiveWindowUpdate();
            unsigned long newWindowSize = ctr.getReceiveWindowSize();
            EXPECT_EQ(2 * oldWindowSize, newWindowSize);
            EXPECT_EQ(offset, oldOffset + dataRead + newWindowSize);
        }

        TEST(connection_flow_controller, says_when_its_blocked) {
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindow, maxReceiveWindow);

            ctr.setSendWindow(100);
            EXPECT_FALSE(ctr.isNewlyBlocked());
            ctr.addSentBytesCount(100);
            bool blocked = ctr.isNewlyBlocked();
            unsigned long offset = ctr.getNewlyBlockedAt();
            EXPECT_TRUE(blocked);
            EXPECT_EQ(100, offset);
        }

        TEST(connection_flow_controller, dosent_say_newly_blocked_multiple_times_for_same_offset) {
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindow, maxReceiveWindow);

            ctr.setSendWindow(100);
            ctr.addSentBytesCount(100);
            EXPECT_TRUE(ctr.isNewlyBlocked());
            EXPECT_EQ(100, ctr.getNewlyBlockedAt());
            EXPECT_FALSE(ctr.isNewlyBlocked());
            ctr.setSendWindow(150);
            ctr.addSentBytesCount(150);
            EXPECT_TRUE(ctr.isNewlyBlocked());
        }

        TEST(connection_flow_controller, sets_minimum_window_window_size) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            unsigned long maxReceiveWindowSize = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindowSize, maxReceiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            ctr.ensureMinimumWindowSize(1800);
            EXPECT_EQ(1800, ctr.getReceiveWindowSize());
        }

        TEST(connection_flow_controller, dosent_reduce_window_window_size) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            unsigned long maxReceiveWindowSize = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindowSize, maxReceiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            ctr.ensureMinimumWindowSize(1);
            EXPECT_EQ(receiveWindowSize, ctr.getReceiveWindowSize());
        }

        TEST(connection_flow_controller, dosent_beyond_max_receive_window_size) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            unsigned long maxReceiveWindowSize = 3000;
            RoundTripStatistics rtt;
            ConnectionFlowController ctr(rtt, receiveWindowSize, maxReceiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            ctr.ensureMinimumWindowSize(2 * ctr.getMaxReceiveWindowSize());
            EXPECT_EQ(ctr.getMaxReceiveWindowSize(), ctr.getReceiveWindowSize());
        }
    }
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}