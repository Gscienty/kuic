#include "congestion/round_trip_statistics.h"
#include "gtest/gtest.h"
#include <limits>
#include <vector>

namespace kuic {
    namespace test {
        TEST(rtt, default_before_update) {
            kuic::RoundTripStatistics rtt;
            EXPECT_LT(0, rtt.getInitialRTTus());
            EXPECT_EQ(0, rtt.getMinRTT());
            EXPECT_EQ(0, rtt.getSmoothedRTT());
        }

        TEST(rtt, smoothed_rtt) {
            kuic::RoundTripStatistics rtt;

            rtt.updateRTT(300 * 1000 * 1000, 100 * 1000 * 1000, { 0, 0 });
            EXPECT_EQ(300 * 1000 * 1000, rtt.getLatestRTT());
            EXPECT_EQ(300 * 1000 * 1000, rtt.getSmoothedRTT());

            rtt.updateRTT(300 * 1000 * 1000, 50 * 1000 * 1000, { 0, 0 });
            EXPECT_EQ(300 * 1000 * 1000, rtt.getLatestRTT());
            EXPECT_EQ(300 * 1000 * 1000, rtt.getSmoothedRTT());

            rtt.updateRTT(200 * 1000 * 1000, 300 * 1000 * 1000, { 0, 0 });
            EXPECT_EQ(200 * 1000 * 1000, rtt.getLatestRTT());
            EXPECT_EQ(287500 * 1000, rtt.getSmoothedRTT());
        }

        TEST(rtt, min_rtt) {
            kuic::RoundTripStatistics rtt;

            rtt.updateRTT(200 * 1000 * 1000, 0, { 0, 0 });
            EXPECT_EQ(200 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(200 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.updateRTT(10 * 1000 * 1000, 0, ((timespec) { 0, 0 }) + (10L * 1000L * 1000L * 1000L));
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.updateRTT(50 * 1000 * 1000, 0, ((timespec) { 0, 0 }) + (20L * 1000L * 1000L * 1000L));
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.updateRTT(50 * 1000 * 1000, 0, ((timespec) { 0, 0 }) + (30L * 1000L * 1000L * 1000L));
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.updateRTT(50 * 1000 * 1000, 0, ((timespec) { 0, 0 }) + (40L * 1000L * 1000L * 1000L));
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.updateRTT(7 * 1000 * 1000, 0, ((timespec) { 0, 0 }) + (50L * 1000L * 1000L * 1000L));
            EXPECT_EQ(7 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(7 * 1000 * 1000, rtt.getRecentMinRTT());
        }

        TEST(rtt, recent_min_rtt) {
            kuic::RoundTripStatistics rtt;

            rtt.updateRTT(10 * 1000 * 1000, 0, { 0, 0 });
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.sampleNewRecentMinRTT(4);

            for (int i = 0; i < 3; i++) {
                rtt.updateRTT(50 * 1000 * 1000, 0, { 0, 0 });
                EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
                EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());
            }

            rtt.updateRTT(50 * 1000 * 1000, 0, { 0, 0 });
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(50 * 1000 * 1000, rtt.getRecentMinRTT());
        }

        TEST(rtt, windowed_recent_min_rtt) {
            kuic::RoundTripStatistics rtt;

            rtt.setRecentMinRTTWindow(99 * 1000 * 1000);

            timespec now = { 0, 0 };
            long rttSample = 10 * 1000 * 1000;
            rtt.updateRTT(rttSample, 0, now);
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());

            for (int i = 0; i < 8; i++) {
                now += 25L * 1000 * 1000;
                rttSample += 10L * 1000 * 1000;

                rtt.updateRTT(rttSample, 0, now);
                EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
                EXPECT_EQ(rttSample, rtt.getQuarterWindowRTT());
                EXPECT_EQ(rttSample - (10 * 1000 * 1000), rtt.getHalfWindowRTT());

                if (i < 3) {
                    EXPECT_EQ(10 * 1000 * 1000, rtt.getRecentMinRTT());
                }
                else if (i < 5) {
                    EXPECT_EQ(30 * 1000 * 1000, rtt.getRecentMinRTT());
                }
                else if (i < 7) {
                    EXPECT_EQ(50 * 1000 * 1000, rtt.getRecentMinRTT());
                }
                else {
                    EXPECT_EQ(70 * 1000 * 1000, rtt.getRecentMinRTT());
                }
            }

            rttSample -= 5 * 1000 * 1000;
            rtt.updateRTT(rttSample, 0, now);
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(rttSample, rtt.getQuarterWindowRTT());
            EXPECT_EQ(rttSample - 5 * 1000 * 1000, rtt.getHalfWindowRTT());
            EXPECT_EQ(70 * 1000 * 1000, rtt.getRecentMinRTT());

            rttSample = 65 * 1000 * 1000;
            rtt.updateRTT(rttSample, 0, now);
            EXPECT_EQ(10 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(rttSample, rtt.getQuarterWindowRTT());
            EXPECT_EQ(rttSample, rtt.getHalfWindowRTT());
            EXPECT_EQ(rttSample, rtt.getRecentMinRTT());

            rttSample = 5 * 1000 * 1000;
            rtt.updateRTT(rttSample, 0, now);
            EXPECT_EQ(rttSample, rtt.getMinRTT());
            EXPECT_EQ(rttSample, rtt.getQuarterWindowRTT());
            EXPECT_EQ(rttSample, rtt.getHalfWindowRTT());
            EXPECT_EQ(rttSample, rtt.getRecentMinRTT());
        }

        TEST(rtt, expire_smoothed_metrics) {
            kuic::RoundTripStatistics rtt;
            long initialRTT = 10 * 1000 * 1000;
            rtt.updateRTT(initialRTT, 0, { 0, 0 });
            EXPECT_EQ(initialRTT, rtt.getMinRTT());
            EXPECT_EQ(initialRTT, rtt.getMinRTT());
            EXPECT_EQ(initialRTT, rtt.getSmoothedRTT());
            EXPECT_EQ(initialRTT / 2, rtt.getMeanDeviation());


            long doubleRTT = initialRTT * 2;
            rtt.updateRTT(doubleRTT, 0, { 0, 0 });
            EXPECT_EQ((long) (initialRTT * 1.125), rtt.getSmoothedRTT());

            rtt.expireSmoothedMetrics();
            EXPECT_EQ(doubleRTT, rtt.getSmoothedRTT());
            EXPECT_EQ((long) (initialRTT * 0.875), rtt.getMeanDeviation());

            long halfRTT = initialRTT / 2;
            rtt.updateRTT(halfRTT, 0, { 0, 0 });
            EXPECT_GT(doubleRTT, rtt.getSmoothedRTT());
            EXPECT_LT(initialRTT, rtt.getMeanDeviation());
        }

        TEST(rtt, update_rtt_with_bad_send_deltas) {
            kuic::RoundTripStatistics rtt;
            long initialRTT = 10 * 1000 * 1000;
            rtt.updateRTT(initialRTT, 0, { 0, 0 });

            EXPECT_EQ(initialRTT, rtt.getMinRTT());
            EXPECT_EQ(initialRTT, rtt.getRecentMinRTT());
            EXPECT_EQ(initialRTT, rtt.getSmoothedRTT());

            std::vector<long> badSendDeltas({ 0, std::numeric_limits<long>::max(), -1000 * 1000 });

            for (long badSendDelta : badSendDeltas) {
                rtt.updateRTT(badSendDelta, 0, { 0, 0 });
                EXPECT_EQ(initialRTT, rtt.getMinRTT());
                EXPECT_EQ(initialRTT, rtt.getRecentMinRTT());
                EXPECT_EQ(initialRTT, rtt.getSmoothedRTT());
            }
        }

        TEST(rtt, reset_after_connection_migrations) {
            kuic::RoundTripStatistics rtt;
            rtt.updateRTT(200 * 1000 * 1000, 0, { 0, 0 });
            EXPECT_EQ(200 * 1000 * 1000, rtt.getLatestRTT());
            EXPECT_EQ(200 * 1000 * 1000, rtt.getSmoothedRTT());
            EXPECT_EQ(200 * 1000 * 1000, rtt.getMinRTT());

            rtt.updateRTT(300 * 1000 * 1000, 100 * 1000 * 1000, { 0, 0 });
            EXPECT_EQ(200 * 1000 * 1000, rtt.getLatestRTT());
            EXPECT_EQ(200 * 1000 * 1000, rtt.getSmoothedRTT());
            EXPECT_EQ(200 * 1000 * 1000, rtt.getMinRTT());
            EXPECT_EQ(200 * 1000 * 1000, rtt.getRecentMinRTT());

            rtt.onConnectionMigration();
            EXPECT_EQ(0, rtt.getLatestRTT());
            EXPECT_EQ(0, rtt.getSmoothedRTT());
            EXPECT_EQ(0, rtt.getMinRTT());
            EXPECT_EQ(0, rtt.getRecentMinRTT());

        }
    }
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}