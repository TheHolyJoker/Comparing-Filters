#include "minimal_tests.hpp"

bool print_pass() {
    std::cout << "Passed tests!" << std::endl;
    return true;
}

int main(int argc, char **argv) {
    std::cout << "Here!" << std::endl;
    // assert(0);
    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    using Table_TC320 = twoChoicer320<itemType>;
    using Table_Dict320 = Dict320<temp_hash, spare_item, itemType>;
    using Table_Dict512 = Dict512<temp_hash, spare_item, itemType>;
    using Table_Dict512_Ver2 = Dict512_Ver2<temp_hash, spare_item, itemType>;
    using Table_CF = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_Dict_CF = Dict512_With_CF<itemType>;

    // assert((default_validation_test_single<Table_TC320, itemType>()));
    assert((default_validation_test_single<Table_Dict512_Ver2, itemType>()));
    assert((default_validation_test_single<Table_Dict320, itemType>()));
    assert((default_validation_test_single<Table_Dict512, itemType>()));
    assert((default_validation_test_single<Table_Dict_CF, itemType>()));
    assert(print_pass());


    // single_fp_rates<Table_TC320, itemType>(std::ceil((1 << 20u) * 0.88), 1 << 22, 8, 0);
    // single_fp_rates<Table_Dict320, itemType>(std::ceil((1 << 20u) * 0.88), 1 << 22, 8, 0);
        // single_fp_rates<Table_Dict512, itemType>(std::ceil((1 << 21u) * 0.88), 1 << 24, 8, 0);
        // single_fp_rates<Table_Dict512, itemType>(std::ceil((1 << 21u) * 0.88), 1 << 24, 8, 0); 
    // single_fp_rates<Table_Dict512_Ver2, itemType>(std::ceil((1 << 21u) * 0.88), 1 << 24, 8, 0);
    // single_fp_rates<Table_Dict512, itemType>(std::ceil((1 << 21u) * 0.88), 1 << 24, 8, 0);
    // single_fp_rates<Table_Dict512, itemType>(std::ceil((1 << 21u) * 0.88), 1 << 24, 8, 0);
    // single_fp_rates<Table_CF, itemType>(std::ceil((1 << 22u) * 0.88), 1 << 24, 8, 0);
    // single_fp_rates<Table_CF, itemType>(std::ceil((1 << 22u) * 0.88), 1 << 24, 8, 0);

    const size_t max_filter_capacity = 62411242;
    const size_t lookup_reps = 124822484;
    const size_t bits_per_element = 8;
    const size_t bench_precision = 16;
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    fill_vec(elements.at(0), max_filter_capacity);
    fill_vec(elements.at(1), lookup_reps);
    // size_t del_size = max_filter_capacity / (double) bench_precision;
    // fill_vec(elements->at(2), del_size);

    /** Single benching  */
    // single_bench<Table_TC320, itemType>(std::ceil((1 << 24u) * 0.88), 1<<25, 8, 0, 0);

    // single_bench<Table_TC320, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict320, itemType>(max_filter_capacity, bench_precision, false, &elements);
    while (true) {
        // single_bench<Table_Dict_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_Dict512_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
        single_bench<Table_Dict512, itemType>(max_filter_capacity, bench_precision, false, &elements);
        // single_bench<Table_Dict320, itemType>(max_filter_capacity, bench_precision, false, &elements);
        single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    }


    return 0;
}
