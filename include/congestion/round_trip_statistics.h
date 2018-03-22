#ifndef _KUIC_RTT_
#define _KUIC_RTT_
#include "extend_timespec.h"

namespace kuic {
    struct RoundTripStatisticsSample {
        long rtt;
        timespec t;
    };

    class RoundTripStatistics {
    private:
        long initialRTTus;
        long recentMinRTTWindow;
        long minRTT;
        long latestRTT;
        long smoothedRTT;
        long meanDeviation;

        unsigned int numMinRTTSamplesRemaining;

        RoundTripStatisticsSample newMinRTT;
        RoundTripStatisticsSample recentMinRTT;
        RoundTripStatisticsSample halfWindowRTT;
        RoundTripStatisticsSample quarterWindowRTT;

        void updateRecentMinRTT(long sample, timespec now);

    public:
        RoundTripStatistics();
        long getInitialRTTus() const { return this->initialRTTus; }
        long getMinRTT() const { return this->minRTT; }
        long getLatestRTT() const { return this->latestRTT; }
        long getRecentMinRTT() const { return this->recentMinRTT.rtt; }
        long getSmoothedRTT() const { return this->smoothedRTT; }
        long getQuarterWindowRTT() const { return this->quarterWindowRTT.rtt; }
        long getHalfWindowRTT() const { return this->halfWindowRTT.rtt; }
        long getMeanDeviation() const { return this->meanDeviation; }

        void setRecentMinRTTWindow(const long recentMinRTTWindow) { this->recentMinRTTWindow = recentMinRTTWindow; }

        void updateRTT(long sendDelta, long ackDelay, timespec now);
        void sampleNewRecentMinRTT(unsigned int numSamples);
        void onConnectionMigration();

        void expireSmoothedMetrics();
    };

    const long  RTT_INITIAL_RTT_US  = 100 * 1000;
    const float RTT_ALPHA           = 0.125;
    const float RTT_BETA            = 0.25;
    const float RTT_HALF_WINDOW     = 0.5;
    const float RTT_QUARTER_WINDOW  = 0.25;
}

#endif