#include "minimal_tests.hpp"

bool print_pass() {
    std::cout << "Passed tests!" << std::endl;
    return true;
}


auto get_fp_ratios() -> bool {

    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    using Table_Dict256_Ver5 = Dict256_Ver5<spare_item, itemType>;
    using Table_Dict256_Ver6 = Dict256_Ver6<itemType>;


    single_fp_rates<Table_Dict256_Ver5, itemType>(std::ceil((1 << 20) * 0.94), 1 << 21, 11, 1);

    const size_t fp_capacity = std::ceil((1 << 23u) * 0.88);
    const size_t fp_lookups = (1 << 25u);
    const size_t bits_per_element = 8;

    vector<itemType> fp_v_add, fp_v_find;                         //, v_delete;
    vector<vector<itemType> *> fp_elements{&fp_v_add, &fp_v_find};//, &v_delete};
    init_elements(fp_capacity, fp_lookups, &fp_elements, false);


    single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict256_Ver5, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
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

int main(int argc, char **argv) {
    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    using Table_Dict256_Ver5 = Dict256_Ver5<spare_item, itemType>;
    // using Table_Dict256_Ver6 = Dict256_Ver6<itemType>;
    using Table_Dict256_Ver7 = Dict256_Ver7<itemType>;
    using Table_Dict256_Ver6_db = Dict256_Ver6_DB<itemType>;


    assert((default_validation_test_single<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single<Table_Dict256_Ver7, itemType>()));

    assert((default_validation_test_single_with_deletions<Table_Dict256_Ver7, itemType>()));
    assert((default_validation_test_single_with_deletions<Table_Dict256_Ver7, itemType>()));

    // get_fp_ratios();

    const size_t max_filter_capacity = 62411242;
    const size_t lookup_reps = 124822484;
    const size_t bits_per_element = 8;
    const size_t bench_precision = 16;
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    fill_vec(elements.at(0), max_filter_capacity);
    fill_vec(elements.at(1), lookup_reps);
    size_t del_size = 0;
    // size_t del_size = 1.0 * max_filter_capacity / (double) bench_precision;
    fill_vec(elements.at(2), del_size);


    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_Dict256_Ver7, itemType>(max_filter_capacity, bench_precision, false, &elements);
    single_bench<Table_Dict256_Ver5, itemType>(max_filter_capacity, bench_precision, false, &elements);
    return 0;
}
