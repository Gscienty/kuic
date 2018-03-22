#ifndef _KUIC_CUBIC_
#define _KUIC_CUBIC_

#include <sys/time.h>

namespace kuic {
    class Cubic {
    private:
        int numConnections;
        timespec epoch;
        timespec appLimitStartTime;
        timespec lastUpdateTime;
        unsigned long lastCongestionWindow;
        unsigned long lastMaxCongestionWindow;
        unsigned long ackedPacketsCount;
        unsigned long estimatedTCPCongestionWindow;
        unsigned long originPointCongestionWindow;
        unsigned int timeToOriginPoint;
        unsigned long lastTargetCongestionWindow;

    public:
        Cubic();

        void reset();
        float alpha() const;
        float beta() const;
        void onApplicationLimited();
        unsigned long congestionWindowAfterPacketLoss(unsigned long currentCongestionWindow);
        unsigned long congestionWindowAfterAck(unsigned long currentCongestionWindow, timespec delayMin);
        void setNumConnections(int n) { this->numConnections = n; }
    };

    const int CUBIC_SCALE                   = 40;
    const int CUBIC_CONGESTION_WINDOW_SCALE = 410;
    const unsigned long CUBIC_FACTOR        = 1 << CUBIC_SCALE / CUBIC_CONGESTION_WINDOW_SCALE;
    const int CUBIC_DEFAULT_NUM_CONNECTION  = 2;

    const float CUBIC_BETA                  = 0.7;
    const float CUBIC_BETA_LAST_MAX         = 0.85;
    const long CUBIC_MAX_TIME_INTERVAL      = 30 * 1000 * 1000;
}

#endif