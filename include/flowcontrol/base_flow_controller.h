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


    public:
        BaseFlowController(RoundTripStatistics &rtt,
                        unsigned long receiveWindowSize,
                        unsigned long maxReceiveWindowSize);

        void startNewAutoTuningEpoch();
        void tryAdjustWindowSize();

        void setEpochStartTime(SpecialClock clock);
        void setEpochStartOffset(unsigned long offset);
        void setReceiveWindowSize(unsigned long receiveWindowSize);
        unsigned long getReceiveWindowSize() const;
        void setReceivedBytesCount(unsigned long receivedBytesCount);
        unsigned long getReceivedBytesCount() const;

        void setReadedBytesCount(unsigned long readedBytesCount);
        unsigned long getReadedBytesCount() const;

        void addSentBytesCount(unsigned long n);
        void addReadedBytesCount(unsigned long n);
        void updateSendOffset(unsigned long offset);
        unsigned long getSendWindowSize() const;
        unsigned long receiveBytesCountUpdate();
        bool receiveWindowHasUpdate() const;
    };
}

#endif