//
// Created by root on 31/05/2020.
//

#include "new_tests.hpp"

auto example1() {
    ulong shift = 16u;
    size_t filter_max_capacity = 1u << shift;
    size_t lookup_reps = 1u << (shift + 2u);
    size_t error_power_inv = BITS_PER_ELEMENT_MACRO;
    size_t bench_precision = 16;
    b_all_wrapper<uint64_t, BITS_PER_ELEMENT_MACRO>(filter_max_capacity, lookup_reps, error_power_inv, bench_precision,
                                                    1, 1, 1, 1, 1);
}

auto example3() {
    //    ulong shift = 1e6;
    //    size_t shift_add_to_lookups = 2u;
    size_t bench_precision = 100;
    size_t remainder_length = BITS_PER_ELEMENT_MACRO;
    size_t max_distinct_capacity = 8e6;

    using Table = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>;
    for (int i = 0; i < 4; ++i) {
        cout << i << endl;
        //        benchmark_single_filter_high_load<Table, uint64_t>(max_distinct_capacity, bench_precision, 200);
        //        cout  << std::string(48,'$') << endl;
        //        cout  << std::string(48,'$') << endl;
        //        cout  << std::string(48,'$') << endl;
        benchmark_single_filter_high_load<uint64_t, hash_table>(max_distinct_capacity, remainder_length,
                                                                bench_precision, 200);
    }
}

void validate_HT_example() {
    using simple_ht = hash_table<uint32_t>;
    bool res = v_hash_table_rand<simple_ht, uint32_t>(1u << 15u, 1u << 14u, 18, 4, .5);
    assert(res);
    res = v_hash_table_rand_gen_load<simple_ht, uint32_t>(1u << 15u, 1u << 14u, 18, 4, .5, .5);
    assert(res);
    res = v_wrap_test<simple_ht, uint32_t>(1u << 15u, 1u << 14u, 18, .5, .5, 4);
    assert(res);
    //// other example using the wrapper: (different number of args.)
    res = v_wrap_test<uint32_t>(1u << 15u, 1u << 14u, 18, .5, .5);
    assert(res);

    res = v_wrap_test<uint32_t>(1u << 15u, 1u << 14u, 18, .5, .5);
    assert(res);
    for (int i = 33; i < 64; ++i) {
        std::cout << i << std::endl;
        res = v_wrap_test<uint64_t>(1u << 15u, 1u << 14u, i, .5, .8);
        assert(res);
    }
}


int main(int argc, char **argv) {
    srand(45);
    /* uint32_t x = -1;
    std::string s = "tomer";
    auto y = s_pd_filter::cuckoofilter::HashUtil::BobHash(&s, 64, 0);

    std::cout << "y is: " << y << std::endl;
    return 0; */
    /*validate_HT_example();*/
    //    bool res;
    //    res = v_wrap_test<uint32_t>(1u << 15u, 1u << 14u, 18, .5,.8);
    //    assert(res);
    //    for (int i = 33; i < 64; ++i) {
    //        std::cout << i << std::endl;
    //        res = v_wrap_test<uint64_t>(1u << 15u, 1u << 14u, i, .5, .85);
    //        assert(res);
    //    }

    //    return 0;


    //Default values
    size_t filter_indicator = 127;
    ulong shift = 25u;
    size_t shift_add_to_lookups = 1u;
    size_t bench_precision = 8;
    size_t remainder_length = BITS_PER_ELEMENT_MACRO;

    size_t reps = 1u << (shift), max_distinct_capacity = 1u << shift;
    /*bench_all_PD<uint64_t, 8, 64>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO,
                                  bench_precision);
    return 0;*/
    /**Parsing*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char *end;
    size_t values[4]{filter_indicator, shift, shift_add_to_lookups, bench_precision};
    for (int i = 1; i < argc; ++i) {
        values[i - 1] = strtol(argv[i], &end, 10);
    }

    filter_indicator = values[0];
    shift = values[1];
    shift_add_to_lookups = values[2];
    bench_precision = values[3];

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    reps = 1u << (shift), max_distinct_capacity = 1u << shift;
    // max_distinct_capacity = std::ceil(max_distinct_capacity * .51);
    using itemType = uint64_t;

    size_t temp_cap = (1 << 21ul) * 0.88;

    std::stringstream ss;
    size_t c = std::ceil((1 << 22u) * 0.88);
    size_t r = 1 << 23u;
    fp_rates_all_wrapper<itemType, 8>(
            c,
            r,
            BITS_PER_ELEMENT_MACRO,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true);


    b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(
            5000000, 10000000,
            BITS_PER_ELEMENT_MACRO,
            bench_precision,
            false,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true);

    fp_rates_all_wrapper<itemType, 12>(
            c,
            r,
            BITS_PER_ELEMENT_MACRO_12,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true,
            true);


    b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO_12>(
            5000000, 10000000,
            BITS_PER_ELEMENT_MACRO_12,
            bench_precision,
            false,
            true,
            true,
            false,
            false,
            true,
            false,
            true,
            true);

    // b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(
    //         max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, bench_precision,
    //         false,
    //         false,
    //         true,
    //         true,
    //         true,
    //         true,
    //         false,
    //         true);
    // att_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(1 << 18, 1 << 18, BITS_PER_ELEMENT_MACRO,
    //   bench_precision);
    // for (size_t i = 1; i < 16; i++)
    // {
    // att_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(10000 * i, 20000 * i, BITS_PER_ELEMENT_MACRO,
    // bench_precision);
    // }
    //
    // att_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(max_distinct_capacity, max_distinct_capacity, BITS_PER_ELEMENT_MACRO,
    //                                                   bench_precision);
    /* std::string line = std::string(128, '#');
    size_t shift_start = 20;
    for (size_t my_shift = shift_start; my_shift < 27; my_shift++) {
        std::cout << line << std::endl;
        std::cout << (my_shift - shift_start) << ")" << std::endl;
        size_t reps = 1u << (my_shift), max_distinct_capacity = std::ceil((1u << my_shift) * 0.94);
        std::cout << "spare_element_size: " << compute_spare_element_size(max_distinct_capacity, .96) << std::endl;
        att_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO,
                                                          bench_precision);
    } */
    return 0;
}
