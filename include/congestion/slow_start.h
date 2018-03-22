#ifndef _KUIC_SLOW_START_
#define _KUIC_SLOW_START_

namespace kuic {
    class SlowStart {
    private:
        unsigned long endPacketNumber;
        unsigned long lastSentPacketNumber;
        bool started;
        long currentMinRTT;
        unsigned int rttSampleCount;
        bool startFound;
    public:
        void startReceiveRound(unsigned long lastSent);
        bool isEndOfRound(unsigned long ack);
        bool shouldExitSlowStart(long lastestRTT, long minRTT, long congestionWindow);
        void onPacketSent(unsigned long packetNumber);
        void onPacketAcked(unsigned long packetNumber);
        bool isStarted() const;
        void restart();
    };

    const long SLOW_START_LOW_WINDOW                = 16;
    const unsigned int SLOW_START_MIN_SAMPLES       = 8;
    const int SLOW_START_DELAY_FACTOR_EXP           = 3;
    const long SLOW_START_DELAY_MIN_THRESHOLD_US    = 4000L;
    const long SLOW_START_DELAY_MAX_THRESHOLD_US    = 16000L;
}

#endif