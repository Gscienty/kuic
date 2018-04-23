#ifndef _KUIC_CLOCK_
#define _KUIC_CLOCK_

#include "define.h"
#include "type.h"
#include <time.h>

namespace kuic {
    class clock {
    public:
        virtual timespec get() const = 0;

        bool is_zero() const;
        kuic_time_t since(const clock &) const;
        bool before(const clock &) const;

        kuic_time_t operator- (const clock &b);
        kuic_time_t operator- (const kuic_time_t t);

        bool operator< (const clock &b);
        bool operator> (const clock &b);
        bool operator== (const clock &b);
        bool operator<= (const clock &b);
        bool operator>= (const clock &b);
        bool operator!= (const clock &b);

        bool operator< (const kuic_time_t b);
        bool operator> (const kuic_time_t b);
        bool operator== (const kuic_time_t b);
        bool operator<= (const kuic_time_t b);
        bool operator>= (const kuic_time_t b);
        bool operator!= (const kuic_time_t b);
    };

    class current_clock : public clock {
    public:
        timespec get() const override;
    };

    class special_clock : public clock {
    private:
        timespec special_time;
    public:
        special_clock();
        special_clock(const timespec &);
        special_clock(const clock &c);

        timespec get() const override;

        special_clock operator+ (const clock &b);
        special_clock operator+ (const kuic_time_t b);
        special_clock &operator+= (const kuic_time_t b);
        special_clock &operator+= (const clock &b);
    };
}

#endif