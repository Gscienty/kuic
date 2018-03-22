#include "define.h"
#include "congestion/proportional_rate_reduction.h"
#include <limits>

namespace kuic {
    void ProportionalRateReduction::onPacketSent(unsigned long sentBytes) {
        this->bytesSentSinceLoss += sentBytes;
    }

    void ProportionalRateReduction::onPacketLost(unsigned long bytesInFlight) {
        this->bytesSentSinceLoss = 0;
        this->bytesInFlightBeforeLoss = bytesInFlight;
        this->bytesDeliveredSinceLoss = 0;
        this->ackCountSinceLoss = 0;
    }

    void ProportionalRateReduction::onPacketAcked(unsigned long ackedBytes) {
        this->bytesDeliveredSinceLoss += ackedBytes;
        this->ackCountSinceLoss++;
    }

    long ProportionalRateReduction::timeUntilSend(unsigned long congestionWindow, unsigned long bytesInFlight, unsigned long slowstartThreshold) {
        if (this->bytesDeliveredSinceLoss == 0 || bytesInFlight < DEFAULT_TCP_MSS) {
            return 0;
        }
        if (congestionWindow > bytesInFlight) {
            if (this->bytesDeliveredSinceLoss + this->ackCountSinceLoss * DEFAULT_TCP_MSS <= this->bytesSentSinceLoss) {
                return std::numeric_limits<long>::max();
            }
            return 0;
        }
        if (this->bytesDeliveredSinceLoss * slowstartThreshold > this->bytesSentSinceLoss * this->bytesInFlightBeforeLoss) {
            return 0;
        }
        return std::numeric_limits<long>::max();
    }

    void ProportionalRateReduction::initialize() {
        this->bytesSentSinceLoss = 0;
        this->bytesInFlightBeforeLoss = 0;
        this->bytesDeliveredSinceLoss = 0;
        this->ackCountSinceLoss = 0;
    }
}