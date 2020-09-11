//
// Created by tomer on 25/05/2020.
//

#include "tests.hpp"


uint64_t xorshf96() {          //period 2^96-1
    uint64_t t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}


auto rand_item() -> string {
    int minLength = 8, charsNum = 32, numOfDiffLength = 8;
    int len = minLength bitor (rand() % numOfDiffLength);
    assert(4 < len);
    char s[len];
    for (int i = 0; i < len; ++i) s[i] = 64 bitor (rand() % charsNum);
    s[len] = 0;
    string res(s);
    return res;
}



auto fill_vec(std::vector<uint64_t> *vec, size_t number_of_elements) -> void {
    vec->resize(number_of_elements);
    for (int i = 0; i < number_of_elements; ++i)
        vec->at(i) = random();

}


void set_init(size_t size, set<string> *mySet) {
    int minLength = 8, charsNum = 32;
    for (int i = 0; i < size; ++i) mySet->insert(rand_item());
}


// void validate_example1() {
//     ulong shift = 18u;
//     size_t shift_add_to_lookups = 2u;

//     size_t reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;
//     double l1_LF = 0.95, l2_LF = 0.65;

//     using simple_bloom = bloomfilter::bloom<uint64_t, BITS_PER_ELEMENT_MACRO, false, hashing::TwoIndependentMultiplyShift>;
//     bool cond = w_validate_filter<simple_bloom, uint64_t>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
//         l2_LF, cout);
//     assert(cond);
//     cond = w_validate_filter<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
//         max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF, cout);
//     assert(cond);

//     cond = w_validate_filter<SimdBlockFilter<>, uint64_t>(
//         max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF, cout);
//     assert(cond);
//     cond = w_validate_filter<MortonFilter, uint64_t, false>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
//         l2_LF, cout);
//     assert(cond);
//     cond = w_validate_filter<uint64_t, hash_table>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF);
//     assert(cond);

// }

// void validate_example2(ulong shift, ulong filter_indicator) {
//     size_t shift_add_to_lookups = 2u;
// 
//     size_t reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;
//     double l1_LF = 0.95, l2_LF = 0.5;
// 
//     bool cond = true;
//     ulong power = 1;
// 
//     if (power & filter_indicator) {
//         using simple_bloom = bloomfilter::bloom<uint64_t, BITS_PER_ELEMENT_MACRO, false, hashing::TwoIndependentMultiplyShift>;
//         cond = w_validate_filter<simple_bloom, uint64_t>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
//             l2_LF, cout);
//         assert(cond);
//     }
//     power <<= 1u;
//     if (power & filter_indicator) {
//         cond = w_validate_filter<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
//             max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF, cout);
//         assert(cond);
//     }
//     power <<= 1u;
//     if (power & filter_indicator) {
//         cond = w_validate_filter<SimdBlockFilter<>, uint64_t>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO,
//             l1_LF, l2_LF, cout);
//         assert(cond);
//     }
//     power <<= 1u;
//     if (power & filter_indicator) {
//         cond = w_validate_filter<MortonFilter, uint64_t, false>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO,
//             l1_LF, l2_LF, cout);
//         assert(cond);
//     }
//     power <<= 1u;
//     if (power & filter_indicator) {
//         cond = w_validate_filter<uint64_t, hash_table>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
//             l2_LF,cout);
//         assert(cond);
//     }
// }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

template<>
auto CF_rates_wrapper<s_dict32>(size_t filter_max_capacity, size_t lookup_reps,
                                size_t error_power_inv, size_t l1_counter_size,
                                size_t l2_counter_size, double level1_load_factor,
                                double level2_load_factor, ostream &os) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    using Table = s_dict32;
    Table filter(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(std::string("s_dict32"));
    CF_rates_core<Table>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
    return os;
}*/


int very_old_main(int argc, char **argv) {
    //Default values
    size_t filter_indicator = 0;
    ulong shift = 18u;
    size_t shift_add_to_lookups = 2u;
    size_t bench_precision = 20;
    size_t remainder_length = BITS_PER_ELEMENT_MACRO;

    size_t reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;

    /**Validation of the filters */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//    size_t l1_counter_size = 3, l2_counter_size = 7;
    double l1_LF = 0.95, l2_LF = 0.65;

    // using simple_bloom = bloomfilter::bloom<uint64_t, BITS_PER_ELEMENT_MACRO, false, hashing::TwoIndependentMultiplyShift>;
    // bool cond = w_validate_filter<simple_bloom, uint64_t>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
    //     l2_LF, std::cout);
    // assert(cond);
    // cond = w_validate_filter<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
    //     max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF, std::cout);
    // assert(cond);
    // cond = w_validate_filter<MortonFilter, uint64_t, false>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
    //     l2_LF, std::cout);
    // assert(cond);
    // cond = w_validate_filter<uint64_t, hash_table>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF, std::cout);
    // assert(cond);

    /**Parsing*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char *end;
    size_t values[4]{ filter_indicator, shift, shift_add_to_lookups, bench_precision };
    for (int i = 1; i < argc; ++i) {
        values[i - 1] = strtol(argv[i], &end, 10);
    }

    filter_indicator = values[0];
    shift = values[1];
    shift_add_to_lookups = values[2];
    bench_precision = values[3];

    /**Old Benchmarking*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;
    // switch (filter_indicator) {
    // case 0:
    //     benchmark_wrapper<simple_bloom, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
    //     benchmark_wrapper<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
    //         max_distinct_capacity, reps, remainder_length, bench_precision);
    //     benchmark_wrapper<MortonFilter, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
    //     benchmark_wrapper<uint64_t, hash_table>(max_distinct_capacity, reps, remainder_length, bench_precision);
    //     break;
    // case 1:
    //     benchmark_wrapper<simple_bloom, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
    // case 2:
    //     benchmark_wrapper<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
    //         max_distinct_capacity, reps, remainder_length, bench_precision);
    // case 3:
    //     benchmark_wrapper<MortonFilter, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
    // case 4:
    //     benchmark_wrapper<uint64_t, hash_table>(max_distinct_capacity, reps, remainder_length, bench_precision);
    // default:
    //     break;
    // }

    return 0;
}

/*

//    v_true_positive<simple_pd>(1u << 12u);
//    bool cond =  v_true_positive<bloomfilter::bloom<uint64_t, 8, false, HashUtil>>(1u<<12u);
//    bool cond = v_true_positive<simple_bloom>(1u << 3u);
//    assert(cond);
//    cond = v_return_false_when_empty<simple_bloom>();
//    assert(cond);
//    std::cout << "Bloom passed" << std::endl;
//
//    cond = v_true_positive_att<uint64_t, 8, false, hashing::TwoIndependentMultiplyShift, bloomfilter::bloom>(8);
//    assert(cond);

//    cond = v_true_positive_att< uint64_t, 8, false, HashUtil, bloomfilter::bloom<uint64_t, 8, false,HashUtil>>(1000);
//    cond = v_true_positive_att<bloomfilter::bloom<uint64_t, 8, false,HashUtil>>(1000);

//    cond = w_validate_filter<uint64_t, 8, false, hashing::TwoIndependentMultiplyShift, bloomfilter::bloom>(
//            max_distinct_capacity, reps, 8, l1_LF, l2_LF);
//    assert(cond);

//    using Table_bloom = FilterAPI<simple_bloom>;
//    ulong shift = 15u;
//    size_t reps = 1u << (shift + 3u), max_distinct_capacity = 1u << shift;
//    size_t remainder_length = BITS_PER_ELEMENT_MACRO;
//    size_t l1_counter_size = 3, l2_counter_size = 7;
//    double l1_LF = 0.95, l2_LF = 0.65;
//    bool cond;
*/
