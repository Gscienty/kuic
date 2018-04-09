#include "flowcontrol/base_flow_controller.h"
#include "gtest/gtest.h"
#include "define.h"

namespace kuic {
    namespace test {
        TEST(flowcontrol_base_flow_controller, update_size_window) {
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, 0, 0);

            ctr.addSentBytesCount(5);
            ctr.setSendWindow(15);

            EXPECT_EQ(15 - 5, ctr.getSendWindowSize());
        }

        TEST(flowcontrol_base_flow_controller, zero_send_window) {
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, 0, 0);

            ctr.addSentBytesCount(15);
            ctr.setSendWindow(10);

            EXPECT_EQ(0, ctr.getSendWindowSize());
        }

        TEST(flowcontrol_base_flow_controller, dose_not_decrease_flow_control_window) {
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, 0, 0);

            ctr.setSendWindow(20);
            EXPECT_EQ(20, ctr.getSendWindowSize());
            ctr.setSendWindow(10);
            EXPECT_EQ(20, ctr.getSendWindowSize());
        }

        TEST(flowcontrol_base_flow_controller, add_bytes_read) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            ctr.setReadedBytesCount(5);
            ctr.addReadedBytesCount(6);

            EXPECT_EQ(5 + 6, ctr.getReadedBytesCount());
        }

        TEST(flowcontrol_base_flow_controller, trigger_window_update_necessary) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            double bytesConsumed = ((double) receiveWindowSize) * WINDOW_UPDATE_THRESHOLD + 1;
            unsigned long bytesRemaining = receiveWindowSize - ((unsigned long) bytesConsumed);
            unsigned long readPosition = receiveWindow - bytesRemaining;
            ctr.setReadedBytesCount(readPosition);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_EQ(readPosition + receiveWindowSize, offset);
            EXPECT_EQ(readPosition + receiveWindowSize, ctr.getReceiveWindow());
        }

        TEST(flowcontrol_base_flow_controller, dont_trigger_window_update_necessary) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            double bytesConsumed = ((double) receiveWindowSize) * WINDOW_UPDATE_THRESHOLD - 1;
            unsigned long bytesRemaining = receiveWindowSize - ((unsigned long) bytesConsumed);
            unsigned long readPosition = receiveWindow - bytesRemaining;
            ctr.setReadedBytesCount(readPosition);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_EQ(0, offset);
        }

        inline void setRTT(RoundTripStatistics &rtt, long t) {
            rtt.updateRTT(t, 0, CurrentClock().now());
            EXPECT_EQ(t, rtt.getSmoothedRTT());
        }

        TEST(flowcontrol_base_flow_controller, dosent_increase_window_size) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, 5000);
            ctr.setReceiveWindow(receiveWindow);

            ctr.tryAdjustWindowSize();
            EXPECT_EQ(receiveWindowSize, ctr.getReceiveWindowSize());
        }

        TEST(flowcontrol_base_flow_controller, dosent_increase_window_size_when_no_rtt_estimate_is_available) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, 5000);
            ctr.setReadedBytesCount(receiveWindow - receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);
            
            setRTT(rtt, 0);
            ctr.startNewAutoTuningEpoch();
            ctr.addReadedBytesCount(400);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_NE(0, offset);
            EXPECT_EQ(receiveWindowSize, ctr.getReceiveWindowSize());
        }

        inline long scaleDuration(long t) {
            long scaleFactor = 1;
            return t * scaleFactor;
        }

        TEST(flowcontrol_base_flow_controller, increases_window_size_if_read_so_fast_less_4_rtts) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, 5000);
            ctr.setReadedBytesCount(receiveWindow - receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);
            
            unsigned long bytesRead = ctr.getReadedBytesCount();
            long rttTime = scaleDuration(20 * 1000 * 1000);
            setRTT(rtt, rttTime);
            unsigned long dataRead = receiveWindowSize * 2 / 3 + 1;
            ctr.setEpochStartOffset(ctr.getReadedBytesCount());
            ctr.setEpochStartTime(CurrentClock().now() + (-rttTime * 4 * 2 / 3));
            ctr.addReadedBytesCount(dataRead);
            unsigned long offset = ctr.receiveWindowUpdate();

            EXPECT_NE(0, offset);

            unsigned long newWindowSize = ctr.getReceiveWindowSize();
            EXPECT_EQ(2 * receiveWindowSize, newWindowSize);
            EXPECT_EQ(bytesRead + dataRead + newWindowSize, offset);
        }

        TEST(flowcontrol_base_flow_controller, dosent_increases_window_size_if_read_so_fast_less_4_rtts_less_half_window_has_been_read) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, 5000);
            ctr.setReadedBytesCount(receiveWindow - receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            unsigned long bytesRead = ctr.getReadedBytesCount();
            long rttTime = scaleDuration(20 * 1000 * 1000);
            setRTT(rtt, rttTime);
            unsigned long dataRead = receiveWindowSize * 1 / 3 + 1;
            ctr.setEpochStartOffset(ctr.getReadedBytesCount());
            ctr.setEpochStartTime(CurrentClock().now() + (-rttTime * 4 * 1 / 3));
            ctr.addReadedBytesCount(dataRead);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_NE(0, offset);
            unsigned long newWindowSize = ctr.getReceiveWindowSize();
            EXPECT_EQ(newWindowSize, receiveWindowSize);
            EXPECT_EQ(offset, bytesRead + dataRead + newWindowSize);
        }

        TEST(flowcontrol_base_flow_controller, dosent_increase_window_size_if_read_too_slowly) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, 5000);
            ctr.setReadedBytesCount(receiveWindow - receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            unsigned long bytesRead = ctr.getReadedBytesCount();
            long rttTime = scaleDuration(20 * 1000 * 1000);
            setRTT(rtt, rttTime);
            unsigned long dataRead = receiveWindowSize * 2 / 3 - 1;
            ctr.setEpochStartOffset(ctr.getReadedBytesCount());
            ctr.setEpochStartTime(CurrentClock().now() + (-rttTime * 4 * 2 / 3));
            ctr.addReadedBytesCount(dataRead);
            unsigned long offset = ctr.receiveWindowUpdate();
            EXPECT_NE(0, offset);
            EXPECT_EQ(receiveWindowSize, ctr.getReceiveWindowSize());
            EXPECT_EQ(offset, bytesRead + dataRead + receiveWindowSize);
        }

        inline void resetEpoch(BaseFlowController &ctr) {
            ctr.setEpochStartTime(CurrentClock().now() + (-1000 * 1000));
            ctr.setEpochStartOffset(ctr.getReadedBytesCount());
            ctr.addReadedBytesCount(ctr.getReceiveWindowSize() / 2 + 1);
        }

        TEST(flowcontrol_base_flow_controller, dosent_increase_window_size_higher_than_max) {
            unsigned long receiveWindow = 10000;
            unsigned long receiveWindowSize = 1000;
            RoundTripStatistics rtt;
            BaseFlowController ctr(rtt, receiveWindowSize, 5000);
            ctr.setReadedBytesCount(receiveWindow - receiveWindowSize);
            ctr.setReceiveWindow(receiveWindow);

            setRTT(rtt, scaleDuration(20 * 1000 * 1000));
            resetEpoch(ctr);
            ctr.tryAdjustWindowSize();
            EXPECT_EQ(2 * receiveWindowSize, ctr.getReceiveWindowSize());

            resetEpoch(ctr);
            ctr.tryAdjustWindowSize();
            EXPECT_EQ(2 * 2 * receiveWindowSize, ctr.getReceiveWindowSize());

            resetEpoch(ctr);
            ctr.tryAdjustWindowSize();
            EXPECT_EQ(5000, ctr.getReceiveWindowSize());
            ctr.tryAdjustWindowSize();
            EXPECT_EQ(5000, ctr.getReceiveWindowSize());
        }
    }
}

int main () {
    RUN_ALL_TESTS();
    return 0;
}