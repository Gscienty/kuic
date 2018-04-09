#include "congestion/round_trip_statistics.h"
#include "extend_timespec.h"
#include <limits>
#include <algorithm>

namespace kuic {
    RoundTripStatistics::RoundTripStatistics() {
        this->initialRTTus = RTT_INITIAL_RTT_US;
        this->recentMinRTTWindow = std::numeric_limits<long>::max();

        this->minRTT = 0L;
        this->latestRTT = 0L;
        this->smoothedRTT = 0L;
        this->meanDeviation = 0L;

        this->newMinRTT = { 0, { 0, 0 } };
        this->recentMinRTT = { 0, { 0, 0 } };
        this->halfWindowRTT = { 0, { 0, 0 } };
        this->quarterWindowRTT = { 0, { 0, 0 } };
    }

    void RoundTripStatistics::updateRecentMinRTT(long sample, timespec now) {
        if (this->numMinRTTSamplesRemaining > 0) {
            this->numMinRTTSamplesRemaining--;
            if (this->newMinRTT.rtt == 0 || sample <= this->newMinRTT.rtt) {
                this->newMinRTT = { sample, now };
            }
            if (this->numMinRTTSamplesRemaining == 0) {
                this->recentMinRTT = this->newMinRTT;
                this->halfWindowRTT = this->newMinRTT;
                this->quarterWindowRTT = this->quarterWindowRTT;
            }
        }

        if (this->recentMinRTT.rtt == 0 || sample <= this->recentMinRTT.rtt) {
            this->recentMinRTT = { sample, now };
            this->halfWindowRTT = this->recentMinRTT;
            this->quarterWindowRTT = this->recentMinRTT;
        }
        else if (sample <= this->halfWindowRTT.rtt) {
            this->halfWindowRTT = { sample, now };
            this->quarterWindowRTT = this->halfWindowRTT;
        }
        else if (sample <= this->quarterWindowRTT.rtt) {
            this->quarterWindowRTT = { sample, now };
        }


        if (this->recentMinRTT.t < now - this->recentMinRTTWindow) {
            this->recentMinRTT = this->halfWindowRTT;
            this->halfWindowRTT = this->quarterWindowRTT;
            this->quarterWindowRTT = { sample, now };
        }
        else if (this->halfWindowRTT.t < now - ((long) (((float) (this->recentMinRTTWindow / 1000)) * RTT_HALF_WINDOW)) * 1000) {
            this->halfWindowRTT = this->quarterWindowRTT;
            this->quarterWindowRTT = { sample, now };
        }
        else if (this->quarterWindowRTT.t < now - ((long) (((float) (this->recentMinRTTWindow / 1000)) * RTT_QUARTER_WINDOW)) * 1000) {
            this->quarterWindowRTT = { sample, now };
        }
    }

    void RoundTripStatistics::updateRTT(long sendDelta, long ackDelay, timespec now) {
        if (sendDelta == std::numeric_limits<long>::max() || sendDelta <= 0) {
            return;
        }

        if (this->minRTT == 0 || this->minRTT > sendDelta) {
            this->minRTT = sendDelta;
        }
        this->updateRecentMinRTT(sendDelta, now);

        long sample = sendDelta;
        if (sample - this->minRTT >= ackDelay) {
            sample -= ackDelay;
        }
        this->latestRTT = sample;

        if (this->smoothedRTT == 0) {
            this->smoothedRTT = sample;
            this->meanDeviation = sample / 2;
        }
        else {
            this->meanDeviation = ((long) ((1 - RTT_BETA) * ((float) (this->meanDeviation / 1000)) + RTT_BETA * ((float) (std::abs<long>(this->smoothedRTT - sample) / 1000)))) * 1000;
            this->smoothedRTT = ((long) ((1 - RTT_ALPHA) * ((float) (this->smoothedRTT / 1000)) + RTT_ALPHA * ((float) (sample / 1000)))) * 1000;
        }
    }

    void RoundTripStatistics::sampleNewRecentMinRTT(unsigned int numSamples) {
        this->numMinRTTSamplesRemaining = numSamples;
        this->newMinRTT = { 0, 0 };
    }

    void RoundTripStatistics::onConnectionMigration() {
        this->latestRTT = 0;
        this->minRTT = 0;
        this->smoothedRTT = 0;
        this->meanDeviation = 0;
        this->initialRTTus = RTT_INITIAL_RTT_US;
        this->numMinRTTSamplesRemaining = 0;
        this->recentMinRTTWindow = std::numeric_limits<long>::max();
        this->recentMinRTT = { 0, 0 };
        this->halfWindowRTT = { 0, 0 };
        this->quarterWindowRTT = { 0, 0 };
    }

    void RoundTripStatistics::expireSmoothedMetrics() {
        this->meanDeviation = std::max<long>(this->meanDeviation, std::abs<long>(this->smoothedRTT - this->latestRTT));
        this->smoothedRTT = std::max<long>(this->smoothedRTT, this->latestRTT);
    }
}