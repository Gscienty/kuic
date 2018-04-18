#ifndef _KUIC_CLOCK_
#define _KUIC_CLOCK_

#include <sys/time.h>

namespace kuic {
    const long clock_second = 1000 * 1000 * 1000;

    class clock {
    public:
        virtual timespec get() const = 0;

        long operator- (const clock &b);
        long operator- (const long t);

        bool operator< (const clock &b);
        bool operator> (const clock &b);
        bool operator== (const clock &b);
        bool operator<= (const clock &b);
        bool operator>= (const clock &b);
        bool operator!= (const clock &b);

        bool operator< (const long b);
        bool operator> (const long b);
        bool operator== (const long b);
        bool operator<= (const long b);
        bool operator>= (const long b);
        bool operator!= (const long b);
    };

    class current_clock : public clock {
    public:
        timespec get() const;
    };

    class special_clock : public clock {
    private:
        timespec special_time;
    public:
        special_clock();
        special_clock(const timespec &);
        special_clock(const clock &c);

        timespec get() const;

        special_clock operator+ (const clock &b);
        special_clock operator+ (const long b);
        special_clock &operator+= (const long b);
        special_clock &operator+= (const clock &b);
    };
}

#endif