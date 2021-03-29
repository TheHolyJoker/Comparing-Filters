#include "minimal_tests.hpp"

bool print_pass() {
    std::cout << "Passed tests!" << std::endl;
    return true;
}


auto get_fp_ratios() -> bool {

    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    // using Table_TC320 = twoChoicer320<itemType>;
    using Table_Dict320 = Dict320<temp_hash, spare_item, itemType>;
    // using Table_Dict512 = Dict512<temp_hash, spare_item, itemType>;
    // using Table_Dict512_Ver2 = Dict512_Ver2<temp_hash, spare_item, itemType>;
    using Table_CF = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    // using Table_Dict_CF = Dict512_With_CF<itemType>;
    // using Table_Dict512_SS = Dict512_SparseSpare<itemType>;
    using Table_Dict512_Ver3 = Dict512_Ver3<temp_hash, spare_item, itemType>;
    using Table_Dict256_Ver4 = Dict256_Ver4<spare_item, itemType>;
    using Table_Dict256_Ver5 = Dict256_Ver5<spare_item, itemType>;
    using Table_Dict256_Ver6 = Dict256_Ver6<itemType>;

    using Table_Fixed_Dict = Fixed_Dict<spare_item, itemType>;
    using Table_Fixed_Dict_Ver2 = Fixed_Dict_Ver2<spare_item, itemType>;
    using Table_CF_ss_13 = cuckoofilter::CuckooFilter<uint64_t, 13, cuckoofilter::PackedTable>;
    // assert((default_validation_test_single<Table_Dict256_Ver4, itemType>()));
    // assert((default_validation_test_single<Table_Dict512, itemType>()));
    // assert((default_validation_test_single<Table_Dict512_SS, itemType>()));
    // assert((default_validation_test_single<Table_Dict_CF, itemType>()));
    // assert(print_pass());

    single_fp_rates<Table_CF_ss_13, itemType>(std::ceil((1 << 20) * 0.94), 1 << 21, 11, 1);
    single_fp_rates<Table_Dict256_Ver5, itemType>(std::ceil((1 << 20) * 0.94), 1 << 21, 11, 1);
    // single_fp_rates<Table_CF_ss_13, itemType>(std::ceil((1 << 20)* 0.88), 1 << 21, 11, 1);
    const size_t fp_capacity = std::ceil((1 << 23u) * 0.88);
    const size_t fp_lookups = (1 << 25u);
    const size_t bits_per_element = 8;

    vector<itemType> fp_v_add, fp_v_find;                         //, v_delete;
    vector<vector<itemType> *> fp_elements{&fp_v_add, &fp_v_find};//, &v_delete};
    init_elements(fp_capacity, fp_lookups, &fp_elements, false);
    // return single_fp_rates_probabilistic<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_item, &fp_elements);
    // for (size_t i = 0; i < 8; i++) {
    //     single_fp_rates_probabilistic<Table_Fixed_Dict, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // }

    // single_fp_rates_probabilistic<Table_Fixed_Dict, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Fixed_Dict, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<SimdBlockFilter<>, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<SimdBlockFilter<>, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // // single_fp_rates_probabilistic<Table_Fixed_Dict_Ver2, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // // single_fp_rates_probabilistic<Table_Fixed_Dict_Ver2, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver4, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver4, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver4, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver4, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_CF_ss_13, itemType>(fp_capacity, fp_lookups, 11, &fp_elements);
    single_fp_rates_probabilistic<Table_CF_ss_13, itemType>(fp_capacity, fp_lookups, 11, &fp_elements);
    single_fp_rates_probabilistic<Table_CF_ss_13, itemType>(fp_capacity, fp_lookups, 11, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver6, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver6, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver6, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict256_Ver6, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict512_Ver3, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_CF, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict512, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    // single_fp_rates_probabilistic<Table_Dict512_SS, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    return true;
}

void heavy_validation() {
    using itemType = uint64_t;
    using Table_Dict256_Ver6 = Dict256_Ver6<itemType>;
    bool temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x1111, 0x1111);
    assert(temp);
    std::cout << "passed:  \t" << 0x1111 << std::endl;

    temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x1'1111, 0x1'1111);
    assert(temp);
    std::cout << "passed   \t" << 0x11111 << std::endl;

    // temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x2'1111,0x2'1111);
    // assert(temp);
    // std::cout << "passed   \t" << 0x21111 << std::endl;

    // temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x4'1111,0x4'1111);
    // assert(temp);
    // std::cout << "passed   \t" << 0x41111 << std::endl;

    // temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x8'1111,0x8'1111);
    // assert(temp);
    // std::cout << "passed   \t" << 0x81111 << std::endl;

    temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x11'1111, 0x11'1111);
    assert(temp);
    std::cout << "passed   \t" << 0x11'1111 << std::endl;

    temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x21'1111, 0x21'1111);
    assert(temp);
    std::cout << "passed   \t" << 0x21'1111 << std::endl;

    temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x41'1111, 0x41'1111);
    assert(temp);
    std::cout << "passed   \t" << 0x41'1111 << std::endl;

    temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x81'1111, 0x81'1111);
    assert(temp);
    std::cout << "passed   \t" << 0x81'1111 << std::endl;

    for (size_t i = 0; i < 16; i++) {
        temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x111'1111, 0x111'1111);
        assert(temp);
        std::cout << "passed   \t" << 0x81'1111 << std::endl;
    }
}

// void attempt_to_use_simd_bf(){

// }

void rel_code() {

    constexpr int x = 25542240;
    constexpr int y = 25165824;
    constexpr auto r = 1.0 * x / y;
    constexpr auto r2 = (1.0 * x / (y * (1.0 * 11 / 12)));

    using itemType = uint64_t;

    using Table_CF = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_CF12 = cuckoofilter::CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_Dict256_Ver7 = Dict256_Ver7<itemType>;
    using Table_Dict512_Ver4 = Dict512_Ver4<hashTable_Aligned<itemType, 4>, itemType, itemType>;

    assert((default_validation_test_single<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single_with_deletions<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single_with_deletions<Table_Dict256_Ver7, itemType>()));

    // constexpr size_t max_filter_capacity = 15435038UL; // load is .92
    constexpr size_t max_filter_capacity = 15770583UL;// load is .94
    // constexpr size_t max_filter_capacity = 15435038UL >> 3;// load is .92
    // const size_t max_filter_capacity =  62411242;
    constexpr size_t lookup_reps = 124822484;
    constexpr size_t bench_precision = 16;

    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};

    fill_vec(elements.at(0), max_filter_capacity);
    fill_vec(elements.at(1), lookup_reps);
    size_t del_size = 1.0 * max_filter_capacity / (double) bench_precision;
    // del_size = 0;
    fill_vec(elements.at(2), del_size);

    // for (size_t i = 0; i < 100; i++) {
    //     single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // }
    // single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    while (true) {

        // single_bench<SimdBlockFilter<>, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
        single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // std::cout << std::string(80, '=') << std::endl;
    }
    int counter = 4;
    while (counter-- > 0) {
        single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    }


    // single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
}
int main(int argc, char **argv) {
    // std::cout << "Tomer1" << std::endl;
    rel_code();
    return 0;
    // std::cout << "Here!" << std::endl;
    // assert(0);
    int x = 24562848;//24921392;
    int y = 25165831;
    auto ratio = 1.0 * x / y;
    std::cout << "ratio: " << ratio << std::endl;
    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    // using Table_TC = twoChoicer<itemType>;
    // using Table_TC320 = twoChoicer320<itemType>;
    using Table_TC256 = twoChoicer256<itemType>;
    using Table_Dict320 = Dict320<temp_hash, spare_item, itemType>;
    // using Table_Dict512 = Dict512<temp_hash, spare_item, itemType>;
    // using Table_Dict512_Ver2 = Dict512_Ver2<temp_hash, spare_item, itemType>;
    using Table_CF = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_CF12 = cuckoofilter::CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;

    using Table_CF_ss_13 = cuckoofilter::CuckooFilter<uint64_t, 13, cuckoofilter::PackedTable>;

    // using Table_Dict_CF = Dict512_With_CF<itemType>;
    // using Table_Dict512_SS = Dict512_SparseSpare<itemType>;
    using Table_Dict512_Ver3 = Dict512_Ver3<temp_hash, spare_item, itemType>;
    using Table_Dict512_Ver4 = Dict512_Ver4<temp_hash, spare_item, itemType>;
    using Table_Dict256_Ver4 = Dict256_Ver4<spare_item, itemType>;
    using Table_Dict256_Ver5 = Dict256_Ver5<spare_item, itemType>;
    using Table_Dict256_Ver6 = Dict256_Ver6<itemType>;
    using Table_Dict256_Ver7 = Dict256_Ver7<itemType>;
    using Table_Dict256_Ver6_db = Dict256_Ver6_DB<itemType>;
    using Table_Fixed_Dict = Fixed_Dict<spare_item, itemType>;
    using Table_Fixed_Dict_Ver2 = Fixed_Dict_Ver2<spare_item, itemType>;


    // assert((default_validation_test_single<Table_TC256, itemType>()));
    // assert((default_validation_test_single<Table_CF_ss_5, itemType>()));
    // std::cout << "5: " << 5 << std::endl;
    // assert((default_validation_test_single<Table_CF_ss_6, itemType>()));
    // std::cout << "6: " << 6 << std::endl;
    // assert((default_validation_test_single<Table_CF_ss_7, itemType>()));
    // std::cout << "7: " << 7 << std::endl;
    // assert((default_validation_test_single<Table_CF_ss_8, itemType>()));
    // assert((default_validation_test_single<Table_CF_ss_9, itemType>()));
    // assert((default_validation_test_single<Table_CF_ss_13, itemType>()));
    // assert((default_validation_test_single<Table_CF_ss_17, itemType>()));

    // return 0;

    // temp = validation_test_single<Table_Dict256_Ver6, itemType>(0x111'1111,0x111'1111);
    // assert(temp);
    // std::cout << "passed   \t" << 0x1111111 << std::endl;

    //    for (int i = 0; i < 64; ++i) {
    // assert((default_validation_test_single<Table_Dict256_Ver6_db, itemType>()));
    //    }
    assert((default_validation_test_single<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single<Table_CF, itemType>()));
    assert((default_validation_test_single<Table_CF, itemType>()));
    // assert((default_validation_test_single<Table_Dict256_Ver6_db, itemType>()));
    // assert((default_validation_test_single<Table_Dict256_Ver6_db, itemType>()));
    // assert((default_validation_test_single<Table_Dict256_Ver6_db, itemType>()));
    // std::cout << "Passed no deletions" << std::endl;
    assert((default_validation_test_single_with_deletions<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single_with_deletions<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single_with_deletions<Table_CF, itemType>()));
    assert((default_validation_test_single_with_deletions<Table_CF, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6_db, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6_db, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6_db, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6_db, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6_db, itemType>()));
    // return 0;


    //    assert((default_validation_test_single<Table_Dict256_Ver6, itemType>()));
    //    assert((default_validation_test_single<Table_Dict256_Ver6, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6, itemType>()));
    // assert((default_validation_test_single_with_deletions<Table_Dict256_Ver6, itemType>()));
    // assert(print_pass());
    // return 0;
    // assert((default_validation_test_single<SimdBlockFilter<>, itemType>()));
    // assert((default_validation_test_single<Table_Dict256_Ver4, itemType>()));
    // assert((default_validation_test_single<Table_Dict256_Ver5, itemType>()));

    // bool temp = validation_test_single<Table_Dict256_Ver7, itemType>(0x11'1111, 0x11'1111);
    // assert(temp);
    // temp = validation_test_single<Table_Dict256_Ver7, itemType>(0xf1'1111, 0xf11'1111);
    // assert(temp);
    // assert((default_validation_test_single<Table_Fixed_Dict, itemType>()));
    // assert((default_validation_test_single<Table_Fixed_Dict_Ver2, itemType>()));
    //    assert((default_validation_test_single<Table_Dict512_Ver4, itemType>()));
    // assert((default_validation_test_single<Table_Dict256_Ver4, itemType>()));
    // // assert((default_validation_test_single<Table_Dict512, itemType>()));
    // assert((default_validation_test_single<Table_Dict512_Ver3, itemType>()));
    // assert((default_validation_test_single<Table_Dict512_Ver4, itemType>()));
    // assert((validation_test_single<Table_Dict512_Ver3, itemType>(1<<22,1<<22)));
    // assert((default_validation_test_single<Table_Dict512_SS, itemType>()));
    // assert((default_validation_test_single<Table_Dict_CF, itemType>()));
    // assert((default_validation_test_single<Table_TC, itemType>()));
    // assert(print_pass());
    // return 0;


    // get_fp_ratios();
    constexpr size_t max_filter_capacity = 15435038UL;// load is .92
    // const size_t max_filter_capacity = 62411242;
    const size_t lookup_reps = 124822484;
    const size_t bits_per_element = 8;
    const size_t bench_precision = 16;
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    fill_vec(elements.at(0), max_filter_capacity);
    fill_vec(elements.at(1), lookup_reps);
    //    size_t del_size = 0;
    size_t del_size = 1.0 * max_filter_capacity / (double) bench_precision;
    fill_vec(elements.at(2), del_size);

    // single_bench<Table_Fixed_Dict_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    std::cout << "Start7!" << std::endl;
    single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    std::cout << "End13!" << std::endl;
    return 0;

    // single_bench<Table_CF_ss_13, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF_ss_13, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF_ss_13, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // return 0;
    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);

    // single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //    single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //    single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //    single_bench<Table_Dict256_Ver6, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver6, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver6, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_TC256, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_TC256, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_TC256, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<SimdBlockFilter<>, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);

    while (true) {
        // single_bench<SimdBlockFilter<>, itemType>(max_filter_capacity, bench_precision, false, &elements);
        single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
        single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
        single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
        //     single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
        //     single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
        //     single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    }

    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF12, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Fixed_Dict_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Fixed_Dict_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Fixed_Dict_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Fixed_Dict_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);

    /** Single benching  */
    //    single_bench<Table_Fixed_Dict, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // return 0;
    // int c = 3;
    // while (c--) {
    //        single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //        single_bench<Table_Fixed_Dict, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //        single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //        single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //        single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //        single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //        single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);

    // single_bench<Table_Dict512_Ver3, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // }
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict512_Ver3, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //    single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);

    return 0;
    // while (true) {
    //     single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     single_bench<Table_Dict256_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     single_bench<Table_Dict512_Ver4, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     // single_bench<Table_Dict512_Ver3, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);

    //     // single_bench<Table_Dict512_SS, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     // single_bench<Table_Dict512_Ver3, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     // single_bench<Table_Dict512, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // }

    // // single_bench<Table_TC, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict512_SS, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_Dict_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_Dict512_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_Dict320, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // }


    // return 0;
}
