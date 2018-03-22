#ifndef _KUIC_TIMESPEC_
#define _KUIC_TIMESPEC_

#include <time.h>
namespace kuic {
    timespec operator+ (const timespec& a, const timespec& b);
    timespec operator+ (const timespec& a, const long b);
    timespec& operator+= (timespec& a, const long b);

    timespec operator- (const timespec& a, const timespec& b);
    timespec operator- (const timespec& a, const long b);

    bool operator<= (const timespec& a, const long b);
    bool operator< (const timespec& a, const timespec& b);
}

#endif