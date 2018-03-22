#include "congestion/slow_start.h"

namespace kuic {
    void SlowStart::startReceiveRound(unsigned long lastSent) {
        this->endPacketNumber = lastSent;
        this->currentMinRTT = 0;
        this->rttSampleCount = 0;
        this->isStarted = true;
    }

    bool SlowStart::isEndOfRound(unsigned long ack) {
        return this->endPacketNumber < ack;
    }

    bool SlowStart::shouldExitSlowStart(long latestRTT, long minRTT, long congestionWindow) {
        if (!this->isStarted) {
            this->startReceiveRound(this->lastSentPacketNumber);
        }
        if (this->startFound) {
            return true;
        }

        this->rttSampleCount++;
        if (this->rttSampleCount <= SLOW_START_MIN_SAMPLES && (this->currentMinRTT == 0 || this->currentMinRTT > latestRTT)) {
            this->currentMinRTT = latestRTT;
        }

        if (this->rttSampleCount == SLOW_START_MIN_SAMPLES) {
            long minRTTIncreaseThreshold = (long) (minRTT / 1000 >> SLOW_START_DELAY_FACTOR_EXP);
            if (minRTTIncreaseThreshold > SLOW_START_DELAY_MAX_THRESHOLD_US) {
                minRTTIncreaseThreshold = SLOW_START_DELAY_MAX_THRESHOLD_US;
            }
            if (minRTTIncreaseThreshold < SLOW_START_DELAY_MIN_THRESHOLD_US) {
                minRTTIncreaseThreshold = SLOW_START_DELAY_MIN_THRESHOLD_US;
            }
            minRTTIncreaseThreshold *= 1000;

            if (this->currentMinRTT > (minRTT + minRTTIncreaseThreshold)) {
                this->startFound = true;
            }
        }

        return congestionWindow >= SLOW_START_LOW_WINDOW && this->startFound;
    }

    void SlowStart::onPacketSent(unsigned long packetNumber) {
        this->lastSentPacketNumber = packetNumber;
    }

    void SlowStart::onPacketAcked(unsigned long packetNumber) {
        if (this->isEndOfRound(packetNumber)) {
            this->isStarted = false;
        }
    }

    bool SlowStart::isStarted() const {
        return this->isStarted;
    }

    void SlowStart::restart() {
        this->isStarted = false;
        this->startFound = false;
    }
}