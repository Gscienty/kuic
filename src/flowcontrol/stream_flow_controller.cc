#include "flowcontrol/stream_flow_controller.h"
#include "readwrite_lock.h"
#include "define.h"
#include <algorithm>

namespace kuic {
    StreamFlowController::StreamFlowController(unsigned long streamID,
                            bool contributesToConnection,
                            ConnectionFlowController &connection,
                            unsigned long receiveWindow,
                            unsigned long maxReceiveWindow,
                            unsigned long initialSendWindow,
                            RoundTripStatistics &rtt)
        : streamID(streamID)
        , connection(connection)
        , contributesToConnection(contributesToConnection)
        , receivedFinalOffset(false)
        , BaseFlowController(rtt, receiveWindow, maxReceiveWindow) {
        this->setSendWindow(initialSendWindow);
    }

    unsigned long StreamFlowController::getStreamID() const {
        return this->streamID;
    }

    bool StreamFlowController::getContributesToConnection() const {
        return this->contributesToConnection;
    }

    ErrorCode StreamFlowController::updateHighestReceived(unsigned long byteOffset, bool isFinal) {
        KuicRWLockWriterLockGuard locker(this->rwLock);

        if (isFinal && this->receivedFinalOffset && byteOffset != this->highestReceived) {
            return STREAM_DATA_AFTER_TERMINATION;
        }
        if (this->receivedFinalOffset && byteOffset > this->highestReceived) {
            return STREAM_DATA_AFTER_TERMINATION;
        }
        if (isFinal) {
            this->receivedFinalOffset = true;
        }
        if (byteOffset == this->highestReceived) {
            return 0;
        }
        if (byteOffset <= this->highestReceived) {
            if (isFinal) {
                return STREAM_DATA_AFTER_TERMINATION;
            }
            return 0;
        }

        unsigned long increment = byteOffset - this->highestReceived;
        this->highestReceived = byteOffset;
        if (this->checkFlowControlViolation()) {
            return FLOW_CONTROL_RECEIVED_TOO_MUCH_DATA;
        }

        if (this->contributesToConnection) {
            return this->connection.incrementHighestReceived(increment);
        }
        return 0;
    }

    void StreamFlowController::addReadedBytesCount(unsigned long n) {
        this->BaseFlowController::addReadedBytesCount(n);
        if (this->contributesToConnection) {
            this->connection.addReadedBytesCount(n);
        }
    }

    void StreamFlowController::addSentBytesCount(unsigned long n) {
        this->BaseFlowController::addSentBytesCount(n);
        if (this->contributesToConnection) {
            this->connection.addSentBytesCount(n);
        }
    }

    unsigned long StreamFlowController::getSendWindowSize() const {
        unsigned long window = this->BaseFlowController::getSendWindowSize();
        if (this->contributesToConnection) {
            window = std::min<unsigned long>(window, this->connection.getSendWindowSize());
        }
        return window;
    }

    bool StreamFlowController::isBlocked() const {
        return this->BaseFlowController::getSendWindowSize() == 0;
    }

    unsigned long StreamFlowController::blockedAt() const {
        return this->BaseFlowController::getSendWindowSize();
    }

    bool StreamFlowController::receiveWindowHasUpdate() {
        KuicRWLockReaderLockGuard locker(this->rwLock);
        return this->receivedFinalOffset == false && this->BaseFlowController::receiveWindowHasUpdate();
    }

    unsigned long StreamFlowController::receiveWindowUpdate() {
        KuicRWLockReaderLockGuard locker(this->rwLock);
        if (this->receivedFinalOffset) {
            return 0;
        }

        unsigned long oldWindowSize = this->getReceiveWindowSize();
        unsigned long offset = this->BaseFlowController::receiveWindowUpdate();

        if (this->getReceiveWindowSize() > oldWindowSize && this->contributesToConnection) {
            this->connection.ensureMinimumWindowSize((unsigned long) (((double) this->getReceiveWindowSize()) * CONNECTION_FLOW_CONTROL_MULTIPLIER));
        }

        return offset;
    }
}