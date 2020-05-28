#include <iostream>
#include "tests.hpp"
int main() {
    std::cout << "Hello, World!" << std::endl;
    ulong shift = 15u;
    size_t reps = 1u << (shift + 3u), max_distinct_capacity = 1u << shift;
    size_t remainder_length = 4;
    size_t l1_counter_size = 3, l2_counter_size = 7;
    double l1_LF = 0.95, l2_LF = 0.65;

    CF_rates_wrapper<s_dict32>(max_distinct_capacity, reps, remainder_length, l1_counter_size, l2_counter_size, l1_LF,
                               l2_LF, cout);
    return 0;
}
