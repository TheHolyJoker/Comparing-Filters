#ifndef FILTERS_Wrap_Fixed_pd_TESTS_HPP
#define FILTERS_Wrap_Fixed_pd_TESTS_HPP

#include "Wrap_Fixed_pd.hpp"

namespace fpd_tests {
    
    // using namespace Wrap_Fixed_pd;
    
    auto insert_find_single(int64_t quot, uint8_t rem, Wrap_Fixed_pd::packed_fpd *pd) -> bool;

    auto insert_find_single_with_capacity(int64_t quot, uint8_t rem, Wrap_Fixed_pd::packed_fpd *pd) -> bool;

    auto insert_find_all(Wrap_Fixed_pd::packed_fpd *pd) -> bool;

    auto insert_find_all() -> bool;

    auto rand_test1() -> bool;
    
    auto rand_test2() -> bool;

    auto determ_no_false_positive()->bool;

    
}// namespace fpd_tests

#endif//FILTERS_Wrap_Fixed_pd_TESTS_HPP