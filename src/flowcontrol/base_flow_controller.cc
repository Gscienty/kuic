#include "flowcontrol/base_flow_controller.h"
#include "define.h"
#include <limits>

namespace kuic {
    BaseFlowController::BaseFlowController(RoundTripStatistics &rtt,
                                        unsigned long receiveWindowSize = 0UL,
                                        unsigned long maxReceiveWindowSize = 0UL)
        : rtt(rtt)
        , rwLock(false)
        , sentBytesCount(0UL)
        , sendWindow(0UL)
        , readedBytesCount(0UL)
        , highestReceived(0UL)
        , receiveWindow(receiveWindowSize)
        , receiveWindowSize(receiveWindowSize)
        , maxReceiveWindowSize(maxReceiveWindowSize) { }
    
    void BaseFlowController::setEpochStartTime(SpecialClock clock) {
        this->epochStartTime = clock;
    }

    void BaseFlowController::setEpochStartOffset(unsigned long offset) {
        this->epochStartOffset = offset;
    }

    void BaseFlowController::setReceiveWindowSize(unsigned long receiveWindowSize) {
        this->receiveWindowSize = receiveWindowSize;
    }

    void BaseFlowController::setReceiveWindow(unsigned long receiveWindow) {
        this->receiveWindow = receiveWindow;
    }

    unsigned long BaseFlowController::getReceiveWindowSize() const {
        return this->receiveWindowSize;
    }

    void BaseFlowController::setMaxReceiveWindowSize(unsigned long maxReceiveWindowSize) {
        this->maxReceiveWindowSize = maxReceiveWindowSize;
    }

    unsigned long BaseFlowController::getMaxReceiveWindowSize() const {
        return this->maxReceiveWindowSize;
    }

    void BaseFlowController::setHighestReceived(unsigned long highestReceived) {
        this->highestReceived = highestReceived;
    }
    
    unsigned long BaseFlowController::getHighestReceived() const {
        return this->highestReceived;
    }

    unsigned long BaseFlowController::getReceiveWindow() const {
        return this->receiveWindow;
    }

    void BaseFlowController::setReadedBytesCount(unsigned long readedBytesCount) {
        this->readedBytesCount = readedBytesCount;
    }

    unsigned long BaseFlowController::getReadedBytesCount() const {
        return this->readedBytesCount;
    }

    void BaseFlowController::addSentBytesCount(unsigned long n) {
        this->sentBytesCount += n;
    }

    unsigned long BaseFlowController::getSendWindow() const {
        return this->sendWindow;
    }

    void BaseFlowController::setSendWindow(unsigned long offset) {
        if (offset > this->sendWindow) {
            this->sendWindow = offset;
        }
    }

    unsigned long BaseFlowController::getSendWindowSize() const {
        if (this->sentBytesCount > this->sendWindow) {
            return 0UL;
        }
        return this->sendWindow - this->sentBytesCount;
    }

    void BaseFlowController::startNewAutoTuningEpoch() {
        CurrentClock current;

        this->epochStartOffset = this->readedBytesCount;
        this->epochStartTime = current.now();
    }

    void BaseFlowController::addReadedBytesCount(unsigned long n) {
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

        double fraction = ((double) bytesReadInEpoch) / ((double) (this->receiveWindowSize));
        
        CurrentClock current;
        if (current.since(this->epochStartTime) < ((long) (4 * fraction * ((double) rtt)))) {
            this->receiveWindowSize = std::min<unsigned long>(2 * this->receiveWindowSize, this->maxReceiveWindowSize);
        }

        this->startNewAutoTuningEpoch();
    }

    unsigned long BaseFlowController::receiveWindowUpdate() {
        if (this->receiveWindowHasUpdate() == false) {
            return 0;
        }

        this->tryAdjustWindowSize();
        this->receiveWindow = this->readedBytesCount + this->receiveWindowSize;
        return this->receiveWindow;
    }

    bool BaseFlowController::receiveWindowHasUpdate() const {
        unsigned long bytesRemaining = this->receiveWindow - this->readedBytesCount;
        return bytesRemaining <= ((unsigned long) (((double) this->receiveWindowSize) * ((double) (1 - WINDOW_UPDATE_THRESHOLD))));
    }

    bool BaseFlowController::checkFlowControlViolation() const {
        return this->highestReceived > this->receiveWindow;
    }
}