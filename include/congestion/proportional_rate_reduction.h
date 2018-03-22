#ifndef _KUIC_PROPORTIONAL_RATE_REDUCTION_
#define _KUIC_PROPORTIONAL_RATE_REDUCTION_

namespace kuic {
    class ProportionalRateReduction {
    private:
        unsigned long bytesSentSinceLoss;
        unsigned long bytesDeliveredSinceLoss;
        unsigned long ackCountSinceLoss;
        unsigned long bytesInFlightBeforeLoss;

    public:
        void initialize();
        void onPacketSent(unsigned long sentBytes);
        void onPacketLost(unsigned long bytesInFlight);
        void onPacketAcked(unsigned long ackedBytes);
        long timeUntilSend(unsigned long congestionWindow, unsigned long bytesInFlight, unsigned long slowstartThreshold);
    };

}

#endif