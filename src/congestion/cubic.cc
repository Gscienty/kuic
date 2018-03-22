#include "congestion/cubic.h"
#include "extend_timespec.h"
#include <algorithm>

namespace kuic {
    Cubic::Cubic() {
        this->numConnections = CUBIC_DEFAULT_NUM_CONNECTION;
        this->reset();
    }

    void Cubic::reset() {
        this->epoch = { 0, 0 };
        this->appLimitStartTime = { 0, 0 };
        this->lastUpdateTime = { 0, 0 };
        this->lastCongestionWindow = 0;
        this->lastMaxCongestionWindow = 0;
        this->ackedPacketsCount = 0;
        this->estimatedTCPCongestionWindow = 0;
        this->originPointCongestionWindow = 0;
        this->timeToOriginPoint = 0;
        this->lastTargetCongestionWindow = 0;
    }

    float Cubic::beta() const { return (((float) (this->numConnections)) - 1 + CUBIC_BETA) / ((float) (this->numConnections)); }

    float Cubic::alpha() const {
        float b = this->beta();
        return 3 * ((float) (this->numConnections)) * ((float) (this->numConnections)) * (1 - b) / (1 + b);
    }

    void Cubic::onApplicationLimited() {
        if (this->appLimitStartTime.tv_sec == 0 && this->appLimitStartTime.tv_nsec == 0) {
            clock_gettime(CLOCK_REALTIME, &this->appLimitStartTime);
        }
        else {
            this->epoch = { 0, 0 };
        }
    }

    unsigned long Cubic::congestionWindowAfterPacketLoss(unsigned long currentCongestionWindow) {
        if (currentCongestionWindow < this->lastMaxCongestionWindow) {
            this->lastMaxCongestionWindow = (unsigned long) (CUBIC_BETA_LAST_MAX * ((float) currentCongestionWindow));
        }
        else {
            this->lastMaxCongestionWindow = currentCongestionWindow;
        }

        this->epoch = { 0, 0 };

        return (unsigned long) (((float) currentCongestionWindow) * this->beta());
    }

    unsigned long Cubic::congestionWindowAfterAck(unsigned long currentCongestionWindow, timespec delayMin) {
        this->ackedPacketsCount++;
        timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);

        if (this->lastCongestionWindow == currentCongestionWindow && this->currentTime - this->lastUpdateTime <= CUBIC_MAX_TIME_INTERVAL) {
            return std::max<unsigned long>(this->lastTargetCongestionWindow, this->estimatedTCPCongestionWindow);
        }

        this->lastCongestionWindow = currentCongestionWindow;
        this->lastUpdateTime = currentTime;

        if (this->epoch.tv_sec == 0 && this->epoch->tv_nsec) {
            this->epoch = currentTime;
            this->ackedPacketsCount = 1;

            this->estimatedTCPCongestionWindow = currentCongestionWindow;
            if (this->lastMaxCongestionWindow <= currentCongestionWindow) {
                this->timeToOriginPoint = 0;
                this->originPointCongestionWindow = currentCongestionWindow;
            }
            else {
                this->timeToOriginPoint = (unsigned int) cbrt((float) (CUBIC_FACTOR * (this->lastMaxCongestionWindow - currentCongestionWindow)));
                this->originPointCongestionWindow = this->lastMaxCongestionWindow;
            }
        }
        else if (!(this->appLimitStartTime.tv_nsec == 0 && this->appLimitStartTime.tv_sec == 0)) {
            timespec shift = currentTime - this->appLimitStartTime;
            this->epoch += shift;
            this->appLimitStartTime = { 0, 0 };
        }

        long elapsedTime = ((long) (((currentTime.tv_nsec + delayMin - this->epoch.tv_nsec) / 1000) << 10)) / 1000000;
        long offset = std::abs<long>(((long) this->timeToOriginPoint) - elapsedTime);

        unsigned long deltaCongestionWindow = (unsigned long) ((CUBIC_CONGESTION_WINDOW_SCALE * offset * offset * offset) >> CUBIC_SCALE);
        unsigned long targetCongestionWindow = this->originPointCongestionWindow + (elapsedTime > ((long) (this->timeToOriginPoint)) ? 1 : -1) * deltaCongestionWindow;

        while (true) {
            unsigned long requiredAckCount = (unsigned long) (((float) (this->estimatedTCPCongestionWindow)) / this->alpha());
            if (this->ackedPacketsCount < requiredAckCount) {
                break;
            }
            this->ackedPacketsCount -= requiredAckCount;
            this->estimatedTCPCongestionWindow++;
        }
        this->lastTargetCongestionWindow = targetCongestionWindow;

        return std::max<unsigned long>(targetCongestionWindow, this->estimatedTCPCongestionWindow);
    }
}