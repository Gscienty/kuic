#include "clock.h"
#include "eys.h"
#include <memory>

kuic::kuic_time_t inline __inl_ttl(timespec t) {
    return t.tv_sec * kuic::clock_second + t.tv_nsec;
}

kuic::kuic_time_t kuic::clock::operator- (const kuic::clock &b) {
    return __inl_ttl(this->get()) - __inl_ttl(b.get());
}

kuic::kuic_time_t kuic::clock::operator- (const kuic::kuic_time_t b) {
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

bool kuic::clock::operator< (const kuic::kuic_time_t b) {
    return __inl_ttl(this->get()) < b;
}

bool kuic::clock::operator> (const kuic::kuic_time_t b) {
    return __inl_ttl(this->get()) > b;
}

bool kuic::clock::operator== (const kuic::kuic_time_t b) {
    return __inl_ttl(this->get()) == b;
}

bool kuic::clock::operator<= (const kuic::kuic_time_t b) {
    return __inl_ttl(this->get()) <= b;
}

bool kuic::clock::operator>= (const kuic::kuic_time_t b) {
    return __inl_ttl(this->get()) >= b;
}

bool kuic::clock::operator!= (const kuic::kuic_time_t b) {
    return __inl_ttl(this->get()) != b;
}

bool kuic::clock::is_zero() const {
    return __inl_ttl(this->get()) == 0L;
}

kuic::kuic_time_t kuic::clock::since(const kuic::clock &c) const {
    return __inl_ttl(this->get()) - __inl_ttl(c.get());
}

bool kuic::clock::before(const kuic::clock &c) const {
    return __inl_ttl(this->get()) < __inl_ttl(c.get());
}

timespec kuic::current_clock::get() const {
    timespec result;
    clock_gettime(CLOCK_REALTIME, &result);
    return result;
}

char *
kuic::clock::timestamp_serialize(size_t &size) {
    return eys::serializer<unsigned long>::serialize((unsigned long)(this->get().tv_sec), size);
}

char *
kuic::clock::timestamp_nano_serialize(size_t &size) {
    return eys::serializer<long>::serialize(long(__inl_ttl(this->get())), size);
}

kuic::special_clock
kuic::special_clock::deserialize(const char *buffer, size_t len, ssize_t &seek) {
    if (seek + ssize_t(sizeof(unsigned long)) < ssize_t(len)) {
        return kuic::special_clock();
    }
    
    timespec ts;

    ts.tv_sec = eys::deserializer<unsigned long>::deserialize(buffer, len, seek);
    ts.tv_nsec = 0; 
    
    return kuic::special_clock(ts);
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
    kuic::kuic_time_t nano_second = __inl_ttl(this->get()) + __inl_ttl(b.get());
    return kuic::special_clock({
        nano_second / kuic::clock_second,
        nano_second % kuic::clock_second
    });
}

kuic::special_clock kuic::special_clock::operator+ (const kuic::kuic_time_t b) {
    kuic::kuic_time_t nano_second = __inl_ttl(this->get()) + b;
    return kuic::special_clock({
        nano_second / kuic::clock_second,
        nano_second % kuic::clock_second
    });
}

kuic::special_clock &kuic::special_clock::operator+= (const kuic::kuic_time_t b) {
    kuic::kuic_time_t nano_second = __inl_ttl(this->get()) + b;
    this->special_time.tv_sec = nano_second / kuic::clock_second;
    this->special_time.tv_nsec = nano_second % kuic::clock_second;

    return *this;
}

kuic::special_clock &kuic::special_clock::operator+= (const clock &b) {
    kuic::kuic_time_t nano_second = __inl_ttl(this->get()) + __inl_ttl(b.get());
    this->special_time.tv_sec = nano_second / kuic::clock_second;
    this->special_time.tv_nsec = nano_second % kuic::clock_second;

    return *this;
}
