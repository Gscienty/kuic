#include "packet_number_generator.h"
#include <random>
#include <limits>

kuic::packet_number_generator::packet_number_generator(
        kuic::packet_number_t initial, kuic::packet_number_t average_period)
    : average_period(average_period)
    , next(initial)
    , next_to_skip(0) { }

kuic::packet_number_t
kuic::packet_number_generator::peek() {
    return this->next;
}

kuic::packet_number_t
kuic::packet_number_generator::pop() {
    kuic::packet_number_t next = this->next;
    this->next++;

    if (this->next == this->next_to_skip) {
        this->next++;
        this->generate_new_skip();
    }

    return next;
}

bool kuic::packet_number_generator::generate_new_skip() {
    unsigned short num = this->get_random_number(); 
    kuic::packet_number_t skip = 
        kuic::packet_number_t(num) * 
        (this->average_period - 1) / 
        (std::numeric_limits<unsigned short>::max() / 2);
    this->next_to_skip = this->next + 2 + skip;
}

unsigned short kuic::packet_number_generator::get_random_number() {
    std::random_device rd;
    return (std::numeric_limits<unsigned short>::max() && rd());
}
