#ifndef _KUIC_BASE_FLOW_CONTROLLER_
#define _KUIC_BASE_FLOW_CONTROLLER_

#include "readwrite_lock.h"
#include "congestion/round_trip_statistics.h"
#include "extend_timespec.h"

namespace kuic {
    class BaseFlowController {
    private:
        unsigned long sentBytesCount;
        unsigned long sendWindow;
        
        unsigned long readedBytesCount;
        unsigned long receiveWindow;
        unsigned long receiveWindowSize;
        unsigned long maxReceiveWindowSize;

        SpecialClock epochStartTime;
        unsigned long epochStartOffset;
        RoundTripStatistics& rtt;

    protected:
        unsigned long highestReceived;
        KuicRWLock rwLock;

        bool checkFlowControlViolation() const;

    public:
        BaseFlowController(RoundTripStatistics &rtt,
                        unsigned long receiveWindowSize,
                        unsigned long maxReceiveWindowSize);

        void startNewAutoTuningEpoch();
        void tryAdjustWindowSize();

        void setEpochStartTime(SpecialClock clock);
        void setEpochStartOffset(unsigned long offset);

        void setMaxReceiveWindowSize(unsigned long maxReceiveWindowSize);
        unsigned long getMaxReceiveWindowSize() const;

        void setHighestReceived(unsigned long highestReceived);
        unsigned long getHighestReceived() const;

        void setReceiveWindowSize(unsigned long receiveWindowSize);
        unsigned long getReceiveWindowSize() const;

        void setReceiveWindow(unsigned long receiveWindow);
        unsigned long getReceiveWindow() const;

        void setSendWindow(unsigned long offset);
        unsigned long getSendWindow() const;

        void setReadedBytesCount(unsigned long readedBytesCount);
        unsigned long getReadedBytesCount() const;

        virtual unsigned long getSendWindowSize() const;

        virtual void addSentBytesCount(unsigned long n);
        virtual void addReadedBytesCount(unsigned long n);
        virtual unsigned long receiveWindowUpdate();
        virtual bool receiveWindowHasUpdate() const;
    };
}

#endif