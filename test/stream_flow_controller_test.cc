#include "flowcontrol/stream_flow_controller.h"
#include "gtest/gtest.h"
#include "define.h"

#include <iostream>

namespace kuic {
    namespace test {
        TEST(stream_flow_controller, set_send_and_receive_windows) {
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, 0, 0);
            RoundTripStatistics rtt;
            unsigned long receiveWindow = 2000;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);

            EXPECT_EQ(5, ctr.getStreamID());
            EXPECT_EQ(receiveWindow, ctr.getReceiveWindowSize());
            EXPECT_EQ(maxReceiveWindow, ctr.getMaxReceiveWindowSize());
            EXPECT_EQ(sendWindow, ctr.getSendWindow());
            EXPECT_TRUE(ctr.getContributesToConnection());
        }

        TEST(stream_flow_controller, updates_the_highest_received) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindowSize, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);

            ctr.setHighestReceived(1337);
            ErrorCode err = ctr.updateHighestReceived(1338, false);
            EXPECT_EQ(0, err);
            EXPECT_EQ(1338, ctr.getHighestReceived());
        }

        TEST(stream_flow_controller, informs_connection_flow_controller_about_received_data) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindowSize, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);

            ctr.setHighestReceived(10);
            cc.setHighestReceived(100);
            ErrorCode err = ctr.updateHighestReceived(20, false);
            EXPECT_EQ(0, err);
            EXPECT_EQ(100 + 10, cc.getHighestReceived());
        }

        TEST(stream_flow_controller, dose_not_decrease_highest_received) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindowSize, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);

            ctr.setHighestReceived(1337);
            ErrorCode err = ctr.updateHighestReceived(1000, false);
            EXPECT_EQ(0, err);
            EXPECT_EQ(1337, ctr.getHighestReceived());
        }

        TEST(stream_flow_controller, dose_nothing_when_setting_the_same_byte_offset) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindowSize, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ctr.setHighestReceived(1337);
            ErrorCode err = ctr.updateHighestReceived(1337, false);
            EXPECT_EQ(0, err);
        }

        TEST(stream_flow_controller, dose_not_give_a_flow_control_violation_when_using_the_window_completely) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ErrorCode err = ctr.updateHighestReceived(receiveWindow, false);
            EXPECT_EQ(0, err);
        }

        TEST(stream_flow_controller, detects_a_flow_control_violation) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ErrorCode err = ctr.updateHighestReceived(receiveWindow + 1, false);
            EXPECT_EQ(FLOW_CONTROL_RECEIVED_TOO_MUCH_DATA, err);
        }

        TEST(stream_flow_controller, accepts_final_offset_higher_than_highest_received) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ctr.setHighestReceived(100);
            ErrorCode err = ctr.updateHighestReceived(101, true);
            EXPECT_EQ(0, err);
            EXPECT_EQ(101, ctr.getHighestReceived());
        }

        TEST(stream_flow_controller, error_final_offset_smaller_than_highest_offset) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ctr.setHighestReceived(100);
            ErrorCode err = ctr.updateHighestReceived(99, true);
            EXPECT_EQ(STREAM_DATA_AFTER_TERMINATION, err);
        }

        TEST(stream_flow_controller, accepts_delayed_data_after_receiving_final_offset) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ErrorCode err = ctr.updateHighestReceived(300, true);
            EXPECT_EQ(0, err);
            err = ctr.updateHighestReceived(250, false);
            EXPECT_EQ(0, err);
        }

        TEST(stream_flow_controller, errors_when_receiving_higher_offset_after_final_offset) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ErrorCode err = ctr.updateHighestReceived(200, true);
            EXPECT_EQ(0, err);
            err = ctr.updateHighestReceived(250, false);
            EXPECT_EQ(STREAM_DATA_AFTER_TERMINATION, err);
        }

        TEST(stream_flow_controller, accept_duplicate_final_offsets) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ErrorCode err = ctr.updateHighestReceived(200, true);
            EXPECT_EQ(0, err);
            err = ctr.updateHighestReceived(200, true);
            EXPECT_EQ(0, err);
            EXPECT_EQ(200, ctr.getHighestReceived());
        }

        TEST(stream_flow_controller, errors_receiving_inconsistent_final_offset) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            
            ErrorCode err = ctr.updateHighestReceived(200, true);
            EXPECT_EQ(0, err);
            err = ctr.updateHighestReceived(201, true);
            EXPECT_EQ(STREAM_DATA_AFTER_TERMINATION, err);
        }

        TEST(stream_flow_controller, save_when_data_read_on_stream_not_contributing_to_connection) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, false, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);

            ctr.addReadedBytesCount(100);
            EXPECT_EQ(100, ctr.getReadedBytesCount());
            EXPECT_EQ(0, cc.getReadedBytesCount());
        }

        TEST(stream_flow_controller, save_when_data_read_on_stream_contributing_to_connection) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 600;
            unsigned long maxReceiveWindow = 3000;
            unsigned long sendWindow = 4000;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);

            ctr.addReadedBytesCount(100);
            EXPECT_EQ(100, ctr.getReadedBytesCount());
            EXPECT_EQ(100, cc.getReadedBytesCount());
        }

        inline void setRTT(RoundTripStatistics &rtt, long t) {
            rtt.updateRTT(t, 0, CurrentClock().now());
            EXPECT_EQ(t, rtt.getSmoothedRTT());
        }

        inline long scaleDuration(long t) {
            long scaleFactor = 1;
            return t * scaleFactor;
        }

        TEST(stream_flow_controller, tell_if_it_has_window_updates) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 100;
            unsigned long sendWindow = 100;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, false, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            EXPECT_FALSE(ctr.receiveWindowHasUpdate());
            ctr.addReadedBytesCount(30);
            EXPECT_TRUE(ctr.receiveWindowHasUpdate());
            EXPECT_NE(0, ctr.receiveWindowUpdate());
            EXPECT_FALSE(ctr.receiveWindowHasUpdate());
        }

        TEST(stream_flow_controller, tell_connection_flow_controller_when_window_autotuned) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 200;
            unsigned long sendWindow = 100;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            unsigned long oldOffset = ctr.getReadedBytesCount();
            setRTT(rtt, scaleDuration(20 * 1000 * 1000));
            ctr.setEpochStartOffset(oldOffset);
            ctr.setEpochStartTime(CurrentClock().now() + (-1000 * 1000));
            ctr.addReadedBytesCount(55);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_EQ(oldOffset + 55 + 2 * receiveWindowSize, offset);
            EXPECT_EQ(2 * receiveWindowSize, ctr.getReceiveWindowSize());
            EXPECT_EQ((unsigned long) (((double) ctr.getReceiveWindowSize()) * CONNECTION_FLOW_CONTROL_MULTIPLIER), cc.getReceiveWindowSize());
        }

        TEST(stream_flow_controller, dose_not_tell_connection_flow_controller_if_it_dose_not_contribute) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 200;
            unsigned long sendWindow = 100;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, false, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            unsigned long oldOffset = ctr.getReadedBytesCount();
            setRTT(rtt, scaleDuration(20 * 1000 * 1000));
            ctr.setEpochStartOffset(oldOffset);
            ctr.setEpochStartTime(CurrentClock().now() + (-1000 * 1000));
            ctr.addReadedBytesCount(55);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_NE(0, offset);
            EXPECT_EQ(2 * receiveWindowSize, ctr.getReceiveWindowSize());
            EXPECT_EQ(2 * receiveWindowSize, cc.getReceiveWindowSize());
        }

        TEST(stream_flow_controller, dose_not_increase_window_after_final_offset_was_already_received) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 200;
            unsigned long sendWindow = 100;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, false, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            ctr.addReadedBytesCount(30);
            ErrorCode err = ctr.updateHighestReceived(90, true);
            EXPECT_EQ(0, err);
            EXPECT_FALSE(ctr.receiveWindowHasUpdate());
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_EQ(0, offset);
        }

        TEST(stream_flow_controller, dose_not_care_about_connection_level_window_dose_not_contribute) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 200;
            unsigned long sendWindow = 15;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, false, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            cc.setSendWindow(1);
            ctr.addSentBytesCount(5);
            EXPECT_EQ(10, ctr.getSendWindowSize());
        }

        TEST(stream_flow_controller, make_sure_it_dose_not_overflow_connection_level_window) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 200;
            unsigned long sendWindow = 15;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            cc.setSendWindow(12);
            ctr.setSendWindow(20);
            ctr.addSentBytesCount(10);
            EXPECT_EQ(2, ctr.getSendWindowSize());
        }

        TEST(stream_flow_controller, dose_not_say_it_is_blocked_if_only_connection_is_blocked) {
            unsigned long receiveWindow = 100;
            unsigned long receiveWindowSize = 60;
            unsigned long maxReceiveWindow = 200;
            unsigned long sendWindow = 15;
            RoundTripStatistics crtt;
            ConnectionFlowController cc(crtt, receiveWindow, maxReceiveWindow);
            RoundTripStatistics rtt;
            StreamFlowController ctr(5, true, cc, receiveWindow, maxReceiveWindow, sendWindow, rtt);
            ctr.setReceiveWindow(receiveWindow);
            ctr.setReceiveWindowSize(receiveWindowSize);
            ctr.setReadedBytesCount(100 - 60);
            cc.setReceiveWindowSize(120);
            
            cc.setSendWindow(50);
            ctr.setSendWindow(100);
            ctr.addSentBytesCount(50);
            bool blocked = cc.isNewlyBlocked();
            cc.getNewlyBlockedAt();
            EXPECT_TRUE(blocked);
            EXPECT_FALSE(ctr.isBlocked());
        }
    }
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}