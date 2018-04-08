#include "extend_timespec.h"
#include <iostream>

namespace kuic {
    timespec operator+ (const timespec& a, const timespec& b) {
        __time_t second = a.tv_sec + b.tv_sec;
        __syscall_slong_t nanoSecond = a.tv_nsec + b.tv_nsec;

        if (nanoSecond >= 1000 * 1000 * 1000) {
            second++;
            nanoSecond -= 1000 * 1000 * 1000;
        }
        else if (nanoSecond < 0) {
            second--;
            nanoSecond += 1000 * 1000 * 1000;
        }

        return { second, nanoSecond };
    }

    timespec operator+ (const timespec& a, const long b) {
        __time_t second = a.tv_sec + b / (1000 * 1000 * 1000);
        __syscall_slong_t nanoSecond = a.tv_nsec + b % (1000 * 1000 * 1000);
        
        if (nanoSecond >= 1000 * 1000 * 1000) {
            second++;
            nanoSecond -= 1000 * 1000 * 1000;
        }
        else if (nanoSecond < 0) {
            second--;
            nanoSecond += 1000 * 1000 * 1000;
        }

        return { second, nanoSecond };
    }

    timespec& operator+= (timespec& a, const long b) {
        a = a + b;
        return a;
    }

    timespec& operator+= (timespec& a, const timespec& b) {
        a.tv_sec += b.tv_sec + (a.tv_nsec + b.tv_nsec) / (1000 * 1000 * 1000);
        a.tv_nsec += (a.tv_nsec + b.tv_nsec) % (1000 * 1000 * 1000);
        return a;
    }

    long operator- (const timespec& a, const timespec& b) {
        return (a.tv_sec * 1000 * 1000 * 1000 + a.tv_nsec) - (b.tv_sec * 1000 * 1000 * 1000 + b.tv_nsec);
    }

    bool operator<= (const timespec& a, const long b) {
        return a.tv_sec * 1000 * 1000 * 1000 + a.tv_nsec <= b;
    }

    bool operator< (const timespec& a, const timespec& b) {
        return a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec);
    }

    long Clock::since(const Clock &clock) {
        return this->now() - clock.now();
    }

    timespec CurrentClock::now() const {
        timespec ret;
        clock_gettime(CLOCK_REALTIME, &ret);
        return ret;
    }

    SpecialClock::SpecialClock()
        : Clock(), specialTime ( { 0, 0 } ) { }

    SpecialClock::SpecialClock(const timespec &specialTime)
        : Clock(), specialTime { specialTime } { }

    timespec SpecialClock::now() const {
        return this->specialTime;
    }
}