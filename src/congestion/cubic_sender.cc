#include "congestion/cubic_sender.h"
#include <algorithm>

namespace kuic {
    CubicSender::CubicSender(
        RoundTripStatistics &rtt,
        unsigned long initialCongestionWindow,
        unsigned long initialMaxCongestionWindow) : rtt { rtt } {
        this->initialCongestionWindow = initialCongestionWindow;
        this->initialMaxCongestionWindow = initialMaxCongestionWindow;
        this->congestionWindow = initialCongestionWindow;
        this->minCongestionWindow = CUBIC_SENDER_DEFAULT_MINIMUM_CONGESTION_WINDOW;
        this->slowStartThreshold = initialMaxCongestionWindow;
        this->maxTCPCongestionWindow = initialMaxCongestionWindow;
        this->numConnections = CUBIC_DEFAULT_NUM_CONNECTION;
    }

    CubicSender::~CubicSender() {
        delete &this->cubic;
    }

    bool CublicSender::inRecovery() const {
        return this->largestAckedPacketNumber <= this->largestSentPacketNumber && this->largestAckedPacketNumber != 0;
    }
    bool CublicSender::inSlowStart() const {
        return this->getCongestionWindow() < this->getSlowStartThreshold();
    }
    unsigned long CublicSender::getCongestionWindow() const { return this->congestionWindow * DEFAULT_TCP_MSS; }
    unsigned long CublicSender::getSlowStartThreshold() const { return this->slowStartThreshold * DEFAULT_TCP_MSS; }
    void CublicSender::exitSlowStart() { this->slowStartThreshold = this->congestionWindow; }


    long CubicSender::timeUntilSend(unsigned long bytesInFlight) {
        if (this->inRecovery() && this->prr.timeUntilSend(this->getCongestionWindow(), bytesInFlight, this->getSlowStartThreshold()) == 0) {
            return 0L;
        }

        long delay = this->rtt->getSmoothedRTT() / (2 * this->congestionWindow);
        if (this->inSlowStart() == false) {
            delay = delay * 8 / 5;
        }
        return delay;
    }

    bool CubicSender::onPacketSent(long sentTime, unsigned long bytesInFlight, unsigned long packetNumber, unsigned long bytes, bool isRetransmittable) {
        if (isRetransmittable == false) { return false; }

        if (this->inRecovery()) {
            this->prr.onPacketSent(bytes);
        }

        this->largestSentPacketNumber = packetNumber;
        this->slowStart.onPacketSent(packetNumber);

        return true;
    }

    void CubicSender::onPacketAcked(unsigned long ackedPacketNumber, unsigned long ackedBytes, unsigned long bytesInFlight) {
        this->largestAckedPacketNumber = std::max<unsigned long>(this->largestAckedPacketNumber, ackedPacketNumber);
        if (this->inRecovery()) {
            this->prr.onPacketAcked(ackedBytes);
        }
        else {
            this->tryIncreaseCongrestionWindow(ackedPacketNumber, ackedBytes, bytesInFlight);

            if (this->inSlowStart()) {
                this->slowStart.onPacketAcked(ackedPacketNumber);
            }
        }
    }

    void CubicSender::tryExitSlowStart() {
        if (this->inSlowStart() && this->slowStart.shouldExitSlowStart(this->rtt.getLatestRTT(), this->rtt.getMinRTT(), this->congestionWindow)) {
            this->exitSlowStart();
        }
    }

    bool CublicSender::isCongestionWindowLimited(unsigned long bytesInFlight) {
        unsigned long congestionWindow = this->getCongestionWindow();
        if (bytesInFlight >= congestionWindow) { return true; }

        unsigned long availableBytes = congestionWindow - bytesInFlight;
        bool slowStartLimited = this->inSlowStart() && bytesInFlight > congestionWindow / 2;
        return slowStartLimited || availableBytes <= CUBIC_SENDER_MAX_BURST_BYTES;
    }

    void CubicSender::tryIncreaseCongrestionWindow(unsigned long ackedPacketNumber, unsigned long ackedBytes, unsigned long bytesInFlight) {
        if (this->isCongestionWindowLimited(bytesInFlight) == false) {
            this->cubic.onApplicationLimited();
        }
        else if (this->congestionWindow >= this->maxTCPCongestionWindow) {
            return;
        }
        else if (this->inSlowStart()) {
            this->congestionWindow++;
        }
        else {
            this->congestionWindow = std::max<unsigned long>(
                this->maxTCPCongestionWindow,
                this->cubic.congestionWindowAfterAck(this->congestionWindow, this->rtt.getMinRTT())
            );
        }
    }

    void CubicSender::onPacketLost(unsigned long packetNumber, unsigned long lostBytes, unsigned long bytesInFlight) {
        if (packetNumber <= this->largestSentAtLastCutback) {
            if (this->lastCutbackExitedSlowstart) {
                this->state.slowStartPacketsLost++;
                this->state.slowStartBytesLost += lostBytes;

                if (this->slowStartLargeReduction) {
                    if (this->state.slowStartPacketsLost == 1 || (this->state.slowStartBytesLost) > (this->state.slowStartBytesLost - lostBytes)) {
                        this->congestionWindow = std::max<unsigned long>(this->congestionWindow - 1, this->minCongestionWindow);
                    }
                    this->slowStartThreshold = this->congestionWindow;
                }
            }
            return;
        }

        this->lastCutbackExitedSlowstart = this->inSlowStart();
        if (this->inSlowStart()) {
            this->state.slowStartPacketsLost++;
        }

        this->prr.onPacketLost(bytesInFlight);

        if (this->slowStartLargeReduction && this->inSlowStart()) {
            this->congestionWindow -= 1;
        }
        else {
            this->congestionWindow = this->cubic.congestionWindowAfterPacketLoss(this->congestionWindow);
        }

        this->congestionWindow = std::max<unsigned long>(this->congestionWindow, this->minCongestionWindow);
        this->slowStartThreshold = this->congestionWindow;
        this->largestSentAtLastCutback = this->largestSentPacketNumber;

        this->congestionWindowCount = 0;
    }

    unsigned long inline __bandwidthFromDelta(unsigned long bytes, unsigned long delta) {
        return bytes * 1000 * 1000 * 1000 / delta * 8 * 1;
    }

    unsigned long CubicSender::bandwidthEstimate() {
        unsigned long smoothedRTT = this->rtt.getSmoothedRTT();
        if (smoothedRTT == 0) { return 0; }
        return __bandwidthFromDelta(this->getCongestionWindow(), smoothedRTT);
    }

    SlowStart& CubicSender::getSlowStart() const { return this->slowStart; }

    void CubicSender::setNumEmulatedConnections(int n) {
        this->numConnections = std::max<int>(n, 1);
        this->cubic.setNumConnections(this->numConnections);
    }

    void CubicSender::onRetransmissionTimeout(bool packetsRetransmitted) {
        this->largestSentAtLastCutback = 0;
        if (this->packetsRetransmitted == false) {
            return;
        }
        this->slowStart.restart();
        this->slowStartThreshold = this->congestionWindow / 2;
        this->congestionWindow = this->minCongestionWindow;
    }

    void CubicSender::onConnectionMigration() {
        this->slowStart.restart();
        this->prr.initialize();
        this->largestSentPacketNumber = 0;
        this->largestAckedPacketNumber = 0;
        this->largestSentAtLastCutback = 0;
        this->lastCutbackExitedSlowstart = false;
        this->cubic.reset();
        this->congestionWindowCount = 0;
        this->congestionWindow = this->initialCongestionWindow;
        this->slowStartThreshold = this->initialMaxCongestionWindow;
        this->maxTCPCongestionWindow = this->initialMaxCongestionWindow;
    }

    void CubicSender::setSlowStartLargeReduction(bool enable) {
        this->slowStartLargeReduction = enable;
    }

    long CubicSender::retransmissionDelay() {
        if (this->rtt.getSmoothedRTT() == 0) {
            return 0L;
        }
        return this->rtt.getSmoothedRTT() + this->rtt.getMeanDeviation() * 4;
    }
}