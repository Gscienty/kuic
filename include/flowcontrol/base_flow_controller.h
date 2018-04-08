#ifndef _KUIC_BASE_FLOW_CONTROLLER_
#define _KUIC_BASE_FLOW_CONTROLLER_

#include "readwrite_lock.h"
#include "congestion/round_trip_statistics.h"
#include "extend_timespec.h"

namespace kuic {
    class BaseFlowController {
    private:
        unsigned long sentBytesCount;
        unsigned long sendOffset;
        
        KuicRWLock rwLock;
        unsigned long readedBytesCount;
        unsigned long highestReceived;
        unsigned long receivedBytesCount;
        unsigned long receiveWindowSize;
        unsigned long maxReceiveWindowSize;

        SpecialClock epochStartTime;
        unsigned long epochStartOffset;
        RoundTripStatistics& rtt;

        void startNewAutoTuningEpoch();

    public:
        BaseFlowController(RoundTripStatistics &rtt);

        void addSentBytesCount(unsigned long n);
        void addReadBytesCount(unsigned long n);
        void updateSendOffset(unsigned long offset);
        unsigned long getSendWindowSize() const;
        void tryAdjustWindowSize();
        unsigned long receiveBytesCountUpdate();
        bool receiveWindowHasUpdate() const;
    };
}

#endif