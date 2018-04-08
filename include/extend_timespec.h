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
        virtual long since(const Clock &);
    };

    class CurrentClock : public Clock {
    public:
        timespec now() const;
        long since(const Clock &);
    };

    class SpecialClock : public Clock {
    private:
        timespec specialTime;
    public:
        SpecialClock();
        SpecialClock(const timespec &);
        timespec now() const;
        long since(const Clock &);
    };
}

#endif