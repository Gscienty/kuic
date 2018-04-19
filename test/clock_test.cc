#include "clock.h"
#include "gtest/gtest.h"
#include <iostream>

TEST(clock, current_time) {
    kuic::current_clock now = kuic::current_clock();
    
    std::cout << now.get().tv_sec << ' ' << now.get().tv_nsec << std::endl;
}

int main() {
    return RUN_ALL_TESTS();
}