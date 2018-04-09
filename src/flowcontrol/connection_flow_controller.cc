#include "flowcontrol/connection_flow_controller.h"
#include "readwrite_lock.h"
#include <algorithm>

namespace kuic {
    ConnectionFlowController::ConnectionFlowController(RoundTripStatistics &rtt,
                                                    unsigned long receiveWindowSize,
                                                    unsigned long maxReceiveWindowSize)
        : BaseFlowController(rtt, receiveWindowSize, maxReceiveWindowSize), lastBlockedAt(0) { }

    bool ConnectionFlowController::isNewlyBlocked() const {
        return this->getSendWindowSize() == 0 && this->getSendWindow() != this->lastBlockedAt;
    }

    unsigned long ConnectionFlowController::getNewlyBlockedAt() {
        if (this->isNewlyBlocked() == false) {
            return 0;
        }
        this->lastBlockedAt = this->getSendWindow();
        return this->getSendWindow();
    }

    ErrorCode ConnectionFlowController::incrementHighestReceived(unsigned long increment) {
        KuicRWLockWriterLockGuard locker(this->rwLock);

        this->highestReceived += increment;
        if (this->checkFlowControlViolation()) {
            return FLOW_CONTROL_RECEIVED_TOO_MUCH_DATA;
        }
        return 0;
    }

    unsigned long ConnectionFlowController::receiveWindowUpdate() {
        KuicRWLockReaderLockGuard locker(this->rwLock);

        unsigned long oldWindowSize = this->getReceiveWindowSize();
        unsigned long offset = this->BaseFlowController::receiveWindowUpdate();
        return offset;
    }

    void ConnectionFlowController::ensureMinimumWindowSize(unsigned long inc) {
        KuicRWLockWriterLockGuard locker(this->rwLock);

        if (inc > this->getReceiveWindowSize()) {
            this->setReceiveWindowSize(std::min<unsigned long>(inc, this->getMaxReceiveWindowSize()));
            this->startNewAutoTuningEpoch();
        }
    }
}