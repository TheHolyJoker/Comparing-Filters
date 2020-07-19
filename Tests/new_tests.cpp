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


int main(int argc, char **argv) {

/*
//    auto tpd = TPD_name::TPD<uint32_t, 8, 64>(0,0,0);
//    tpd.insert(0, 1);
//    assert(tpd.lookup(0, 1));

//    auto d = dict<TPD_name::TPD<uint32_t, 8, 48>, />
//    cout << << endl;
//    dict<

//    return 0;
*/
/*
//    validate_example2( 18,16);
//    validate_example2( 20,16);
//    validate_example2( 22,16);
//    example3();
//    return 0;

    //    validate_example2(16, 16);
//    example2<8>(26, -1);
//    return 0;
//    example2<4>(22, 16);
//    return 0;

//    example3();
//    return 0;
//    validate_example1();

*/

    //Default values
    size_t filter_indicator = 127;
    ulong shift = 20u;
    size_t shift_add_to_lookups = 1u;
    size_t bench_precision = 16;
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

    reps = 1u << (shift ), max_distinct_capacity = 1u << shift;
    using itemType = uint64_t;

//    b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO,
//                                                    bench_precision, filter_indicator & 1, filter_indicator & 2,
//                                                    false, filter_indicator & 8, filter_indicator & 16);
    att_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO,
                                                      bench_precision);
    return 0;

}
