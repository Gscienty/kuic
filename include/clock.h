#ifndef _KUIC_CLOCK_
#define _KUIC_CLOCK_

#include "package_serializer.h"
#include "define.h"
#include "type.h"
#include <time.h>
#include <sys/types.h> 

namespace kuic {
    class clock : public package_serializable {
    public:
        virtual timespec get() const = 0;

        bool is_zero() const;
        kuic_time_t since(const clock &) const;
        bool before(const clock &) const;

        virtual std::basic_string<kuic::byte_t> serialize() const override;

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
        static special_clock deserialize(
                const std::basic_string<kuic::byte_t> &buffer, size_t &seek);

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
