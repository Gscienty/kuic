#ifndef _KUIC_CUBIC_SENDER_
#define _KUIC_CUBIC_SENDER_

#include "define.h"
#include "congestion/slow_start.h"
#include "congestion/proportional_rate_reduction.h"
#include "congestion/round_trip_statistics.h"
#include "congestion/cubic.h"

namespace kuic {
    struct ConnectionState {
        unsigned long slowStartPacketsLost;
        unsigned long slowStartBytesLost;
    };

    class CubicSender {
    private:
        SlowStart slowStart;
        ProportionalRateReduction prr;
        RoundTripStatistics& rtt;
        ConnectionState state;
        Cubic cubic;

        unsigned long largestSentPacketNumber;
        unsigned long largestAckedPacketNumber;
        unsigned long largestSentAtLastCutback;
        unsigned long congestionWindow;
        unsigned long slowStartThreshold;
        bool lastCutbackExitedSlowstart;
        bool slowStartLargeReduction;
        unsigned long minCongestionWindow;
        unsigned long maxTCPCongestionWindow;
        int numConnections;
        unsigned long congestionWindowCount;
        unsigned long initialCongestionWindow;
        unsigned long initialMaxCongestionWindow;

        bool isCongestionWindowLimited(unsigned long bytesInFlight);
        void tryIncreaseCongrestionWindow(unsigned long ackedPacketNumber, unsigned long ackedBytes, unsigned long bytesInFlight);

    public:
        CubicSender(RoundTripStatistics &rtt, unsigned long initialCongestionWindow, unsigned long initialMaxCongestionWindow);
        ~CubicSender();

        bool inRecovery() const;
        bool inSlowStart() const;
        unsigned long getCongestionWindow() const;
        unsigned long getSlowStartThreshold() const;
        void exitSlowStart();
        long timeUntilSend(unsigned long bytesInFlight);
        bool onPacketSent(long sentTime, unsigned long bytesInFlight, unsigned long packetNumber, unsigned long bytes, bool isRetransmittable);
        void onPacketAcked(unsigned long ackedPacketNumber, unsigned long ackedBytes, unsigned long bytesInFlight);
        void onPacketLost(unsigned long packetNumber, unsigned long lostBytes, unsigned long bytesInFlight);
        void tryExitSlowStart();
        unsigned long bandwidthEstimate();

        SlowStart &getSlowStart() const;

        void setNumEmulatedConnections(int n);
        void onRetransmissionTimeout(bool packetsRetransmitted);
        void onConnectionMigration();
        void setSlowStartLargeReduction(bool enable);
        long retransmissionDelay();
    };

    const unsigned long CUBIC_SENDER_MAX_BURST_BYTES                    = 3 * DEFAULT_TCP_MSS;
    const unsigned long CUBIC_SENDER_DEFAULT_MINIMUM_CONGESTION_WINDOW  = 2;
}

#endif