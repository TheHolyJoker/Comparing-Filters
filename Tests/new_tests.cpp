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
    // b_all_wrapper<uint64_t, BITS_PER_ELEMENT_MACRO>(filter_max_capacity, lookup_reps, error_power_inv, bench_precision,
    //                                                 1, 1, 1, 1, 1);
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


void testing_cf_ss() {

    using Table5 = cuckoofilter::CuckooFilter<uint64_t, 5, cuckoofilter::PackedTable>;
    using Table6 = cuckoofilter::CuckooFilter<uint64_t, 6, cuckoofilter::PackedTable>;
    using Table7 = cuckoofilter::CuckooFilter<uint64_t, 7, cuckoofilter::PackedTable>;
    using Table8 = cuckoofilter::CuckooFilter<uint64_t, 8, cuckoofilter::PackedTable>;
    using Table9 = cuckoofilter::CuckooFilter<uint64_t, 9, cuckoofilter::PackedTable>;
    using Table13 = cuckoofilter::CuckooFilter<uint64_t, 13, cuckoofilter::PackedTable>;
    using Table17 = cuckoofilter::CuckooFilter<uint64_t, 17, cuckoofilter::PackedTable>;
    Table5 filter5 = FilterAPI<Table5>::ConstructFromAddCount(5000);
    Table6 filter6 = FilterAPI<Table6>::ConstructFromAddCount(5000);
    Table7 filter7 = FilterAPI<Table7>::ConstructFromAddCount(5000);
    Table8 filter8 = FilterAPI<Table8>::ConstructFromAddCount(5000);
    Table9 filter9 = FilterAPI<Table9>::ConstructFromAddCount(5000);
    Table13 filter13 = FilterAPI<Table13>::ConstructFromAddCount(5000);
    Table17 filter17 = FilterAPI<Table17>::ConstructFromAddCount(5000);
    uint64_t key = 42;
    FilterAPI<Table5>::Add(x, &filter5);
    FilterAPI<Table6>::Add(x, &filter6);
    FilterAPI<Table7>::Add(x, &filter7);
    FilterAPI<Table8>::Add(x, &filter8);
    FilterAPI<Table9>::Add(x, &filter9);
    FilterAPI<Table13>::Add(x, &filter13);
    FilterAPI<Table17>::Add(x, &filter17);

    bool arr[7] = {
            FilterAPI<Table5>::Contain(x, &filter5),
            FilterAPI<Table6>::Contain(x, &filter6),
            FilterAPI<Table7>::Contain(x, &filter7),
            FilterAPI<Table8>::Contain(x, &filter8),
            FilterAPI<Table9>::Contain(x, &filter9),
            FilterAPI<Table13>::Contain(x, &filter13),
            FilterAPI<Table17>::Contain(x, &filter17)};
    for (size_t i = 0; i < 7; i++) {
        std::cout << "arr[i]: " << arr[i] << std::endl;
    }

    // assert(FilterAPI<Table>::Contain(x, &filter));
}


void simple_deletion() {
    using spare_item = uint64_t;
    using temp_hash = att_hTable<spare_item, 4>;
    using Table = Dict512<temp_hash, spare_item, uint64_t>;
    Table filter = FilterAPI<Table>::ConstructFromAddCount(1000);

    std::cout << "Contain(4242): " << FilterAPI<Table>::Contain(4242, &filter) << std::endl;
    FilterAPI<Table>::Add(4242, &filter);
    std::cout << "Contain(4242): " << FilterAPI<Table>::Contain(4242, &filter) << std::endl;
    assert(FilterAPI<Table>::Contain(4242, &filter));
    FilterAPI<Table>::Remove(4242, &filter);
    std::cout << "Contain(4242): " << FilterAPI<Table>::Contain(4242, &filter) << std::endl;
    assert(!FilterAPI<Table>::Contain(4242, &filter));
}
int main(int argc, char **argv) {


    // return 0;


    // std::cout << "Add(4242): " << (FilterAPI<Table>::Add(4242, &filter)) << std::endl;


    // uint64_t arr[8] __attribute__((aligned(64))) = {(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    // for (size_t i = 0; i < 8; i++)
    // {
    //     std::cout << "arr[i]: " << arr[i] << std::endl;
    // }

    // auto pd __attribute__((aligned(64))) = _mm512_load_epi64(&arr);
    //  __m512i x;
    //  __epi
    //   = __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    //  pd512::pd_add_50((uint64_t)i,(uint8_t) i, &pd);
    // auto pd = __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    // pd512::print512(&pd);
    // pd512::pd_add_50((uint64_t)0,(char) 0, &pd);
    // pd512::print512(&pd);
    // pd512::pd_add_50((uint64_t)1,(char) 1, &pd);
    // pd512::print512(&pd);
    // pd512::pd_add_50(2,2, &pd);
    // pd512::print512(&pd);
    // pd512::pd_add_50(3,3, &pd);
    // pd512::print512(&pd);
    // for (size_t i = 0; i < 50; i++) {
    //     pd512::pd_add_50(i, i, &pd);
    // assert (pd512::pd_find_50(i, i, &pd));
    // pd512::print512(&pd);
    // }
    // pd512::print512(&pd);
    // return 0;

    // testing_cf_ss();
    // return 0;
    // srand(45);
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
    ulong shift = 21u;
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


    // size_t temp_cap = (1 << 21ul) * 0.88;

    // std::stringstream ss;
    size_t c = std::ceil((1 << 22u) * 0.88);
    size_t r = 1 << 23u;
    // fp_rates_all_wrapper<itemType, 8, 13>(
    //         c,
    //         r,
    //         BITS_PER_ELEMENT_MACRO,
    //         true,
    //         true,
    //         true,
    //         true,
    //         true,
    //         true,
    //         false,
    //         true,
    //         true);

    // for (size_t i = 0; i < 1024ul; i++)
    // {
    //     size_t size = (rand() & MASK(23)) + 500000;
    //     b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO, 8>(
    //         size, size<<1,
    //         BITS_PER_ELEMENT_MACRO,
    //         bench_precision,
    //         true,
    //         false,
    //         false,
    //         false,
    //         false,
    //         false,
    //         false,
    //         true,
    //         false);

    // }

    // b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO, 8>(
    //         500000, 500000 * 2,
    //         BITS_PER_ELEMENT_MACRO,
    //         bench_precision,
    //         true,
    //         false,
    //         false,
    //         false,
    //         false,
    //         false,
    //         false,
    //         1, 0);
    // return 0;
    b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO, 8>(
            31205621, 31205621 * 2,
            BITS_PER_ELEMENT_MACRO,
            bench_precision,
            false,
            false,
            1,
            0,
            false,
            false,
            false,
            true,
            false);


    return 0;
    fp_rates_all_wrapper<itemType, 12, 13>(
            c,
            r,
            BITS_PER_ELEMENT_MACRO_12,
            true,
            true,
            true,
            true,
            true,
            true,
            false,
            true,
            true);


    b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO_12, 13>(
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
