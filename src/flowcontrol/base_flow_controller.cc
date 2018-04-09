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
        , sendOffset(0UL)
        , readedBytesCount(0UL)
        , highestReceived(0UL)
        , receivedBytesCount(0UL)
        , receiveWindowSize(receiveWindowSize)
        , maxReceiveWindowSize(std::max<unsigned long>(maxReceiveWindowSize, receiveWindowSize)) { }
    
    void BaseFlowController::setEpochStartTime(SpecialClock clock) {
        this->epochStartTime = clock;
    }

    void BaseFlowController::setEpochStartOffset(unsigned long offset) {
        this->epochStartOffset = offset;
    }

    void BaseFlowController::setReceiveWindowSize(unsigned long receiveWindowSize) {
        this->receiveWindowSize = receiveWindowSize;
    }

    void BaseFlowController::setReceivedBytesCount(unsigned long receivedBytesCount) {
        this->receivedBytesCount = receivedBytesCount;
    }

    unsigned long BaseFlowController::getReceiveWindowSize() const {
        return this->receiveWindowSize;
    }

    unsigned long BaseFlowController::getReceivedBytesCount() const {
        return this->receivedBytesCount;
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

    void BaseFlowController::updateSendOffset(unsigned long offset) {
        if (offset > this->sendOffset) {
            this->sendOffset = offset;
        }
    }

    unsigned long BaseFlowController::getSendWindowSize() const {
        if (this->sentBytesCount > this->sendOffset) {
            return 0UL;
        }
        return this->sendOffset - this->sentBytesCount;
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

    unsigned long BaseFlowController::receiveBytesCountUpdate() {
        if (this->receiveWindowHasUpdate() == false) {
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