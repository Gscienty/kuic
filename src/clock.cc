#include "clock.h"
#include <time.h>

long inline __inl_ttl(timespec t) {
    return t.tv_sec * kuic::clock_second + t.tv_nsec;
}

long kuic::clock::operator- (const kuic::clock &b) {
    return __inl_ttl(this->get()) - __inl_ttl(b.get());
}

long kuic::clock::operator- (const long b) {
    return __inl_ttl(this->get()) - b;
}

bool kuic::clock::operator< (const kuic::clock &b) {
    return __inl_ttl(this->get()) < __inl_ttl(b.get());
}

bool kuic::clock::operator> (const kuic::clock &b) {
    return __inl_ttl(this->get()) > __inl_ttl(b.get());
}

bool kuic::clock::operator== (const kuic::clock &b) {
    return __inl_ttl(this->get()) == __inl_ttl(b.get());
}

bool kuic::clock::operator<= (const kuic::clock &b) {
    return __inl_ttl(this->get()) <= __inl_ttl(b.get());
}

bool kuic::clock::operator>= (const kuic::clock &b) {
    return __inl_ttl(this->get()) >= __inl_ttl(b.get());
}

bool kuic::clock::operator!= (const kuic::clock &b) {
    return __inl_ttl(this->get()) != __inl_ttl(b.get());
}

bool kuic::clock::operator< (const long b) {
    return __inl_ttl(this->get()) < b;
}

bool kuic::clock::operator> (const long b) {
    return __inl_ttl(this->get()) > b;
}

bool kuic::clock::operator== (const long b) {
    return __inl_ttl(this->get()) == b;
}

bool kuic::clock::operator<= (const long b) {
    return __inl_ttl(this->get()) <= b;
}

bool kuic::clock::operator>= (const long b) {
    return __inl_ttl(this->get()) >= b;
}

bool kuic::clock::operator!= (const long b) {
    return __inl_ttl(this->get()) != b;
}

timespec kuic::current_clock::get() const {
    timespec result;
    clock_gettime(CLOCK_REALTIME, &result);
    return result;
}

kuic::special_clock::special_clock()
    : special_time( { 0, 0 } ) { }

kuic::special_clock::special_clock(const timespec &t)
    : special_time(t) { }

kuic::special_clock::special_clock(const kuic::clock &clock)
    : special_time(clock.get())  { }
    
timespec kuic::special_clock::get() const {
    return this->special_time;
}

kuic::special_clock kuic::special_clock::operator+ (const clock &b) {
    long nano_second = __inl_ttl(this->get()) + __inl_ttl(b.get());
    return kuic::special_clock({
        nano_second / kuic::clock_second,
        nano_second % kuic::clock_second
    });
}

kuic::special_clock kuic::special_clock::operator+ (const long b) {
    long nano_second = __inl_ttl(this->get()) + b;
    return kuic::special_clock({
        nano_second / kuic::clock_second,
        nano_second % kuic::clock_second
    });
}

kuic::special_clock &kuic::special_clock::operator+= (const long b) {
    long nano_second = __inl_ttl(this->get()) + b;
    this->special_time.tv_sec = nano_second / kuic::clock_second;
    this->special_time.tv_nsec = nano_second % kuic::clock_second;

    return *this;
}

kuic::special_clock &kuic::special_clock::operator+= (const clock &b) {
    long nano_second = __inl_ttl(this->get()) + __inl_ttl(b.get());
    this->special_time.tv_sec = nano_second / kuic::clock_second;
    this->special_time.tv_nsec = nano_second % kuic::clock_second;

    return *this;
}