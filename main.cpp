//
// Created by tomer on 4/18/20.
//

#include <iostream>
#include "Comparing/benchmarking_filters.hpp"
#include "Comparing/benchmark_integer.hpp"
//#include "Filter_PD/Filters/Dict/multi_dict.hpp"

void call_all_filters();

auto main() -> int {
    std::cout << "Hello!" << std::endl;


    size_t reps = 1u << 21u, max_distinct_capacity = 1u << 18u;
    size_t remainder_length = 13;
    size_t l1_counter_size = 3, l2_counter_size = 7;
    double l1_LF = 0.95, l2_LF = 0.65;

    name_compare::b_filter_wrapper_int<Filter_QF, uint32_t>(max_distinct_capacity >> 4u, reps >> 4u, remainder_length,
                                                            l1_counter_size,
                                                            l2_counter_size, 0.6, l2_LF, cout);


//    name_compare::b_filter_wrapper<Filter_QF>(max_distinct_capacity >> 4u, reps>> 4u, remainder_length, l1_counter_size,
//                                              l2_counter_size, 0.6, l2_LF, cout);

//    name_compare::b_filter_wrapper<GeneralBF>(max_distinct_capacity, reps, remainder_length, l1_counter_size,
//                                              l2_counter_size, l1_LF, l2_LF, cout);

//    name_compare::b_filter_wrapper<multi_dict64>(max_distinct_capacity, reps, remainder_length, l1_counter_size,
//                             l2_counter_size, l1_LF, l2_LF, cout);
    for (int i = 0; i < 4; ++i) {
        call_all_filters();

    }
//    CPD p(32,32,5,3);
//    p.lookup(1,1);
//    filter_wrapper
//    multi_dict<CPD, multi_hash_table<uint64_t>, uint64_t> md64(max_distinct_capacity, remainder_length, l1_counter_size,
//                                                               l2_counter_size, l1_LF, l2_LF);

    std::cout << "End!" << std::endl;
    return 0;
}

void call_all_filters() {
    size_t reps = 1u << 21u, max_distinct_capacity = 1u << 16u;
    size_t remainder_length = 8;
    size_t l1_counter_size = 3, l2_counter_size = 7;
    double l1_LF = 0.6, l2_LF = 0.25;

    string line_sep = string(32, '*') + "\n";
    string block_sep = line_sep + line_sep;
    block_sep += block_sep;
    block_sep += block_sep;

    cout << block_sep;
    cout << "multi_dict64\n" << endl;
    name_compare::b_filter_wrapper<multi_dict64>(max_distinct_capacity, reps, remainder_length, l1_counter_size,
                                                 l2_counter_size, l1_LF, l2_LF, cout);
    cout << block_sep;
    cout << "Filter_QF\n" << endl;
    name_compare::b_filter_wrapper<Filter_QF>(max_distinct_capacity, reps, remainder_length, l1_counter_size,
                                              l2_counter_size, l1_LF, l2_LF, cout);

    cout << block_sep;
    cout << "dict32\n" << endl;
    name_compare::b_filter_wrapper<dict32>(max_distinct_capacity, reps, remainder_length, l1_counter_size,
                                           l2_counter_size, l1_LF, l2_LF, cout);

    /*cout << block_sep;
    cout << "GeneralBF\n" << endl;
    name_compare::b_filter_wrapper<GeneralBF>(max_distinct_capacity, reps, remainder_length, l1_counter_size,
                                              l2_counter_size, l1_LF, l2_LF, cout);
    cout << block_sep;*/

}