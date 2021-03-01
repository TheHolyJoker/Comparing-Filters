#include "dict_approximation_tests.hpp"


void error_rates_test() {
    // std::cout << "In pre_main:" << std::endl;
    using itemType = uint64_t;

    using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_DictApx512 = DictApx512<itemType>;

    constexpr size_t max_filter_capacity = (1 << 18);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    constexpr size_t lookup_reps = (1 << 19);
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    auto line = std::string(40, '*');
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
}

void error_rates_test_param(size_t shift, float load) {
    using itemType = uint64_t;

    using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_DictApx512 = DictApx512<itemType>;

    const size_t max_filter_capacity = (1 << shift);// load is .94
    const float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    const size_t lookup_reps = (2 << shift);
    const size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 1.05, &elements);

    auto line = std::string(40, '-');
    // std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.5, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
}

void pre_main() {
    std::cout << "In pre_main:" << std::endl;
    using itemType = uint64_t;

    using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_DictApx512 = DictApx512<itemType>;

    constexpr size_t max_filter_capacity = (1 << 18);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    constexpr size_t lookup_reps = (1 << 19);
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    auto line = std::string(40, '*');
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;


    std::cout << line << std::endl;
    std::cout << line << std::endl;
    std::cout << line << std::endl;
    std::cout << line << std::endl;
    std::cout << line << std::endl;
}


void helper_for_printing_fp_rates() {
    using itemType = uint64_t;

    using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_DictApx512 = DictApx512<itemType>;
    using Table_Dict256_Ver7 = Dict256_Ver7<itemType>;

    constexpr size_t max_filter_capacity = (1 << 22);
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    constexpr size_t lookup_reps = (1 << 23);
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};
    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 3, &elements);

    auto line = std::string(40, '*');

    // std::cout << line << std::endl;
    // std::cout << line << std::endl;
    // bench_symmetric_difference_wrapper<Table_Dict256_Ver7, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    // std::cout << line << std::endl;
    auto big_sep = "\n\n" + std::string(40, '$') + "\n\n";
    for (size_t i = 0; i < 4; i++) {
        compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.94, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        std::cout << big_sep << std::endl;
    }
    return;

    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    return;
    // bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);


    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, work_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // return 0;
}

int main(int argc, char **argv) {
    // #ifdef VALIDATE
    //     pre_main();
    // #endif

    // auto big_sep = "\n\n" + std::string(40, '$') + "\n\n";
    // std::cout << big_sep << std::endl;
    // helper_for_printing_fp_rates();
    // std::cout << big_sep << std::endl;

    // error_rates_test();
    // error_rates_test_param(16, 0.05);
    // std::cout << std::string(80, '$') << std::endl;
    // error_rates_test_param(17, 0.05);
    // std::cout << std::string(80, '$') << std::endl;
    // error_rates_test_param(18, 0.05);
    // std::cout << std::string(80, '$') << std::endl;

    // error_rates_test_param(20, 0.25);
    // error_rates_test_param(20, 0.5);
    // error_rates_test_param(20, 0.75);
    
    error_rates_test();
    return 0;

    using itemType = uint64_t;

    using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_DictApx512 = DictApx512<itemType>;
    // constexpr size_t max_filter_capacity = 15435038UL; // load is .92
    // constexpr size_t max_filter_capacity = 15770583UL;// load is .94
    constexpr size_t max_filter_capacity = (1 << 24);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    // constexpr size_t max_filter_capacity = 15435038UL >> 3;// load is .92
    // const size_t max_filter_capacity =  62411242;
    // constexpr size_t lookup_reps = 124822484;
    constexpr size_t lookup_reps = (1 << 26);
    // constexpr size_t bench_precision = 16;
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 32;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 3, &elements);

    auto line = std::string(40, '*');
    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    // compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, work_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    return 0;
}
