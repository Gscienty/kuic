#include "flowcontrol/base_flow_controller.h"
#include "define.h"
#include <limits>

namespace kuic {
    BaseFlowController::BaseFlowController(RoundTripStatistics &rtt)
        : rtt { rtt } {
        
        this->sentBytesCount = 0;
        this->sendOffset = 0;
    }
    
    void BaseFlowController::addSentBytesCount(unsigned long n) {
        this->sentBytesCount += n;
    }

    void BaseFlowController::updateSendOffset(unsigned long offset) {
        if (offset > this->sendOffset) {
            this->sendOffset = offset;
        }
    }

    unsigned long BaseFlowController::getSendWindowSize() const {
        if (this->sentBytesCount > this->sendOffset) {
            return 0;
        }
        return this->sentBytesCount - this->sendOffset;
    }

    void BaseFlowController::startNewAutoTuningEpoch() {
        CurrentClock current;

        this->epochStartOffset = this->readedBytesCount;
        this->epochStartTime = current.now();
    }

    void BaseFlowController::addReadBytesCount(unsigned long n) {
        KuicRWLockWriterLockGuard locker(this->rwLock);

        if (this->readedBytesCount == 0) {
            this->startNewAutoTuningEpoch();
        }

        this->readedBytesCount += n;
    }

    void BaseFlowController::tryAdjustWindowSize() {
        unsigned long bytesReadInEpoch = this->readedBytesCount - this->epochStartOffset;
        if (bytesReadInEpoch <= this->receiveWindowSize / 2) {
            return;
        }

        long rtt = this->rtt.getSmoothedRTT();
        if (rtt == 0) {
            return;
        }

        double fraction = ((double) bytesReadInEpoch) / ((double) (this->reveiveWindowSize));
        
        CurrentClock current;
        if (current.since(this->epochStartTime) < ((long) (4 * fraction * ((double) rtt)))) {
            this->receiveWindowSize = std::min<unsigned long>(2 * this->receiveWindowSize, this->maxReceiveWindowSize);
        }

        this->startNewAutoTuningEpoch();
    }

    unsigned long BaseFlowController::receiveBytesCountUpdate() {
        if (this->reveiveHasWindowUpdate() == false) {
            return 0;
        }

        this->tryAdjustWindowSize();
        this->receivedBytesCount = this->readedBytesCount + this->receiveWindowSize;
        return this->receivedBytesCount;
    }

    bool BaseFlowController::receiveWindowHasUpdate() const {
        unsigned long bytesRemaining = this->receivedBytesCount - this->readedBytesCount;
        return bytesRemaining <= ((unsigned long) (((double) this->receiveWindowSize) * ((double) (1 - WINDOW_UPDATE_THRESHOLD))));
    }


}