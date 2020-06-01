#include "hash_table.hpp"

auto compute_element_length(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity,
                            size_t remainder_length) -> size_t {
    auto a = (size_t) ceil(log2(number_of_pd));
    auto b = (size_t) ceil(log2(quotient_range));
    auto c = (size_t) ceil(log2(remainder_length));
    return a + b + c;
}

auto compute_max_capacity(size_t max_capacity, double max_load_factor) -> size_t {
    return ((size_t) std::ceil(max_capacity / max_load_factor)) << 4u;

}
