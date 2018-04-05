#ifndef _KUIC_TIMESPEC_
#define _KUIC_TIMESPEC_

#include <sys/time.h>
namespace kuic {
    timespec operator+ (const timespec& a, const timespec& b);
    timespec operator+ (const timespec& a, const long b);
    timespec& operator+= (timespec& a, const long b);
    timespec& operator+= (timespec& a, const timespec& b);

    long operator- (const timespec& a, const timespec& b);

    bool operator<= (const timespec& a, const long b);
    bool operator< (const timespec& a, const timespec& b);

    class Clock {
    public:
        virtual timespec now() const = 0;
    };

    class DefaultClock : public Clock {
    public:
        timespec now() const;
    };
}

#endif