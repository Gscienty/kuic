#include "flowcontrol/stream_flow_controller.h"
#include "gtest/gtest.h"
#include "define.h"

namespace kuic {
    namespace test {
        TEST(stream_flow_controller, updates_the_highest_received) {
            RoundTripStatistics rtt;
            StreamFlowController(
                10,
                ConnectionFlowController(RoundTripStatistics(), 1000, 1000),
                2000,
                3000,
                4000,
                rtt);
        }
    }
}