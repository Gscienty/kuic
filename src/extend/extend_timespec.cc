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

    timespec operator- (const timespec& a, const timespec& b) {
        __time_t second = a.tv_sec - b.tv_sec;
        __syscall_slong_t nanoSecond = a.tv_nsec - b.tv_nsec;
        if (nanoSecond < 0) {
            second--;
            nanoSecond += 1000 * 1000 * 1000;
        }

        return { second, nanoSecond };
    }

    timespec operator- (const timespec& a, const long b) {
        return a + (-b);
    }

    bool operator<= (const timespec& a, const long b) {
        return a.tv_sec * 1000 * 1000 * 1000 + a.tv_nsec <= b;
    }

    bool operator< (const timespec& a, const timespec& b) {
        return a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec);
    }
}