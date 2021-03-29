#include "dict_approximation_tests.hpp"


void symmetric_difference_for_multiple_filters(size_t n, float PD_load, float CF_load) {
    using itemType = uint64_t;

    // using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12_32 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 32>;
    using Table_ts_CF12_16 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 16>;
    using Table_ts_CF12_8 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 8>;
    using Table_ts_CF12_4 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 4>;
    using Table_DictApx512 = DictApx512<itemType>;
    using Table_TS_SIMD = TS_SimdBlockFilter<>;

    const size_t max_filter_capacity = (n);// load is .94
    const float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    const size_t lookup_reps = (n << 1);
    const size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);


    constexpr size_t reps = 4;
    vector<size_t> vec_arr[reps * 2];
    size_t vec_index = 0;

    vector<vector<size_t> *> temp_vec = {&vec_arr[vec_index++], &vec_arr[vec_index++]};
    constexpr float starting_load = 0.65;
    constexpr float inc = 0.05;
    for (size_t i = 0; i < reps; i++) {
        float temp_load = starting_load + i * inc;
        compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, temp_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        temp_vec.at(0) = &vec_arr[vec_index++];
        temp_vec.at(1) = &vec_arr[vec_index++];
    }


    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, PD_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    temp_vec.at(0) = &vec_arr[vec_index++];
    temp_vec.at(1) = &vec_arr[vec_index++];


    // auto line = std::string(40, '-');
    // std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, PD_load,
    //                                                                       lookup_reps,
    //                                                                       num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
}

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
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.95,
                                                                       lookup_reps,
                                                                       num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
}

void testing_error_rate_of_bff(size_t n) {
    using itemType = uint64_t;
    using Table_DictApx512 = DictApx512<itemType>;
    using Table_TS_SIMD = TS_SimdBlockFilter<>;

    const size_t max_filter_capacity = (n);// load is .94
    const float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    const size_t lookup_reps = (n << 1);
    const size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    auto line = std::string(120, '=');

    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    // sleep(300);
    compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.75, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.5, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.25, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.1, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    return;
}

void error_rates_test_param(size_t n, float PD_load, float CF_load) {
    using itemType = uint64_t;

    // using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12_32 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 32>;
    using Table_ts_CF12_16 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 16>;
    using Table_ts_CF12_8 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 8>;
    using Table_ts_CF12_4 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 4>;
    using Table_DictApx512 = DictApx512<itemType>;
    using Table_TS_SIMD = TS_SimdBlockFilter<>;


    const size_t max_filter_capacity = (n);// load is .94
    const float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    const size_t lookup_reps = (n << 1);
    const size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    auto line = std::string(120, '=');
    // std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, PD_load,
    //                                                                       lookup_reps,
    //                                                                       num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, PD_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, PD_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.75, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.5, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.25, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_TS_SIMD, itemType>(&elements, max_filter_capacity, 0.1, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    return;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, PD_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF12_32, itemType>(&elements, max_filter_capacity, CF_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF12_16, itemType>(&elements, max_filter_capacity, CF_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF12_8, itemType>(&elements, max_filter_capacity, CF_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, CF_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
}

void compute_how_many_evictions(size_t n, float load) {
    using itemType = uint64_t;

    // using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12_32 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 32>;
    using Table_ts_CF12_16 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 16>;
    using Table_ts_CF12_8 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 8>;
    using Table_ts_CF12_4 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 4>;
    using Table_DictApx512 = DictApx512<itemType>;

    const size_t max_filter_capacity = (n);// load is .94
    const float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    const size_t lookup_reps = (n << 1);
    const size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};
    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    auto line = std::string(40, '-');
    auto big_sep = "\n\n" + std::string(40, '$') + "\n\n";

    for (size_t i = 80; i < 95; i++) {
        double load = 1.0 * i / 100;
        std::cout << "load: " << load << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_32, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_16, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_8, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        std::cout << big_sep << std::endl;
    }
}

void compute_how_many_evictions_only_pd(size_t n) {
    using itemType = uint64_t;
    using Table_DictApx512 = DictApx512<itemType>;

    const size_t max_filter_capacity = (n);// load is .94
    const float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    const size_t lookup_reps = (n << 1);
    const size_t num_of_blocks_it_takes_to_fill_the_filter = 8;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};
    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    auto line = std::string(40, '-');

    for (size_t i = 68; i < 82; i++) {
        double load = 1.0 * i / 100;
        std::cout << "load: " << load << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
    }
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
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps,
                                                                   num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps,
                                                                   num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps,
                                                                   num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps,
                                                                       num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps,
                                                                num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps,
                                                                     num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps,
                                                              num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps,
                                                                     num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps,
                                                              num_of_blocks_it_takes_to_fill_the_filter);
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
        compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8,
                                                                              lookup_reps,
                                                                              num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.94,
                                                                         lookup_reps,
                                                                         num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << line << std::endl;
        std::cout << big_sep << std::endl;
    }
    return;

    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps,
                                                              num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps,
                                                              num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps,
                                                              num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    return;
    // bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps,
                                                                   num_of_blocks_it_takes_to_fill_the_filter);
    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps,
                                                              num_of_blocks_it_takes_to_fill_the_filter);


    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.7,
                                                                          lookup_reps,
                                                                          num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, work_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // return 0;
}


void plot_error_graphs() {
    using itemType = uint64_t;
    auto sep_line = "|" + std::string(129, '-') + "|";

    using Table_ts_CF12_32 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 32>;
    using Table_ts_CF12_16 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 16>;
    using Table_ts_CF12_8 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 8>;
    using Table_ts_CF12_4 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 4>;
    using Table_DictApx512 = DictApx512<itemType>;

    constexpr size_t max_filter_capacity = (1 << 24);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    constexpr size_t lookup_reps = (max_filter_capacity << 1);
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 32;


    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    std::cout << std::string(80, '~') << std::endl;
    std::cout << "max_filter_capacity:                     " << max_filter_capacity << std::endl;
    std::cout << "work_load:                               " << work_load << std::endl;
    std::cout << "max_number_of_elements_in_the_filter:    " << max_number_of_elements_in_the_filter << std::endl;
    std::cout << "lookup_reps:                             " << lookup_reps << std::endl;
    std::cout << "num_of_blocks_it_takes_to_fill_a_filter: " << num_of_blocks_it_takes_to_fill_the_filter << std::endl;
    std::cout << std::string(80, '~') << std::endl;
    std::cout << std::endl;

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 2, &elements);

    for (size_t i = 70; i < 91; i += 10) {
        float load = 1.0 * i / 100;
        // std::cout << "load: " << load << std::endl;
        compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << std::endl;
    }
    for (size_t i = 70; i < 91; i += 10) {
        float load = 1.0 * i / 100;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << std::endl;
    }

    for (size_t i = 70; i < 91; i += 10) {
        float load = 1.0 * i / 100;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_8, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << std::endl;
    }

    for (size_t i = 70; i < 91; i += 10) {
        float load = 1.0 * i / 100;
        compute_prob_symmetric_difference_wrapper<Table_ts_CF12_16, itemType>(&elements, max_filter_capacity, load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
        std::cout << std::endl;
    }
}

void plot_thorughput_graphs() {
    using itemType = uint64_t;
    auto sep_line = "|" + std::string(129, '-') + "|";

    using Table_ts_CF12_32 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 32>;
    using Table_ts_CF12_16 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 16>;
    using Table_ts_CF12_8 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 8>;
    using Table_ts_CF12_4 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 4>;
    using Table_DictApx512 = DictApx512<itemType>;

    constexpr size_t max_filter_capacity = (1 << 24);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    constexpr size_t lookup_reps = (max_filter_capacity << 2);
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 32;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    std::cout << std::string(80, '~') << std::endl;
    std::cout << "max_filter_capacity:                     " << max_filter_capacity << std::endl;
    std::cout << "work_load:                               " << work_load << std::endl;
    std::cout << "max_number_of_elements_in_the_filter:    " << max_number_of_elements_in_the_filter << std::endl;
    std::cout << "lookup_reps:                             " << lookup_reps << std::endl;
    std::cout << "num_of_blocks_it_takes_to_fill_a_filter: " << num_of_blocks_it_takes_to_fill_the_filter << std::endl;
    std::cout << std::string(80, '~') << std::endl;
    std::cout << std::endl;

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 3, &elements);

    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << std::endl;

    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .85, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << std::endl;

    bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.85, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << std::endl;

    std::cout << sep_line << std::endl;
    std::cout << sep_line << std::endl;
    return;
}


int old_main(int argc, char **argv) {

    using itemType = uint64_t;
    auto sep_line = "|" + std::string(129, '-') + "|";
    // #ifndef NDEBUG
    // assert(pd512::select64(4, 0) == 2);
    // error_rates_test();
    // return 0;

    //    auto temp = (DEBUG);
    // #ifndef NDEBUG
    //     std::cout << "here!" << std::endl;
    //     error_rates_test();
    // #else
    //     assert(0);
    // #endif
    // #ifdef VALIDATE
    //     pre_main();
    // #endif

    // auto big_sep = "\n\n" + std::string(40, '$') + "\n\n";
    // std::cout << big_sep << std::endl;
    // helper_for_printing_fp_rates();
    // std::cout << big_sep << std::endl;

    // error_rates_test();
    // error_rates_test_param(1ull << 16, 0.7, 0.7);
    // std::cout << std::string(80, '$') << std::endl;
    // error_rates_test_param(1ull << 17, 0.7, 0.7);
    // std::cout << std::string(80, '$') << std::endl;
    // error_rates_test_param(1ull << 18, 0.7, 0.7);
    // std::cout << std::string(80, '$') << std::endl;
    // error_rates_test_param(1ull << 21, 0.7, 0.7);
    // std::cout << std::string(80, '$') << std::endl;
    // return 0;
    // error_rates_test_param(20, 0.25);
    // error_rates_test_param(20, 0.5);
    // error_rates_test_param(20, 0.75);
    //    return 0;

    using Table_ts_CF12_32 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 32>;
    using Table_ts_CF12_16 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 16>;
    using Table_ts_CF12_8 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 8>;
    using Table_ts_CF12_4 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable, cuckoofilter::TwoIndependentMultiplyShift, 4>;


    using Table_DictApx512 = DictApx512<itemType>;
    using Table_TS_SIMD = TS_SimdBlockFilter<>;
    // constexpr size_t max_filter_capacity = 15435038UL; // load is .92
    // constexpr size_t max_filter_capacity = 15770583UL;// load is .94
    constexpr size_t max_filter_capacity = (1 << 26);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    // constexpr size_t max_filter_capacity = 15435038UL >> 3;// load is .92
    // const size_t max_filter_capacity =  62411242;
    // constexpr size_t lookup_reps = 124822484;
    constexpr size_t lookup_reps = (max_filter_capacity << 1);
    // constexpr size_t lookup_reps = (max_filter_capacity << 1);
    // constexpr size_t bench_precision = 16;
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 32;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    std::cout << std::string(80, '~') << std::endl;
    std::cout << "max_filter_capacity:                     " << max_filter_capacity << std::endl;
    std::cout << "work_load:                               " << work_load << std::endl;
    std::cout << "max_number_of_elements_in_the_filter:    " << max_number_of_elements_in_the_filter << std::endl;
    std::cout << "lookup_reps:                             " << lookup_reps << std::endl;
    std::cout << "num_of_blocks_it_takes_to_fill_a_filter: " << num_of_blocks_it_takes_to_fill_the_filter << std::endl;
    std::cout << std::string(80, '~') << std::endl;
    std::cout << std::endl;

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 3, &elements);
    auto line = std::string(40, '-');

    // bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;


    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .75, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .85, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, .95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    // bench_symmetric_difference_wrapper<Table_ts_CF12_32, itemType>(&elements, max_filter_capacity, 0.80, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_ts_CF12_16, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // // bench_symmetric_difference_wrapper<Table_ts_CF12_16, itemType>(&elements, max_filter_capacity, 0.90, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // // std::cout << line << std::endl;
    // bench_symmetric_difference_wrapper<Table_ts_CF12_8, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    // std::cout << sep_line << std::endl;
    // return 0;

    bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.7, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;
    // bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.75, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    bench_symmetric_difference_wrapper<Table_ts_CF12_4, itemType>(&elements, max_filter_capacity, 0.85, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.85, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.9, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;

    // bench_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, 0.95, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    std::cout << sep_line << std::endl;
    std::cout << sep_line << std::endl;
    return 0;
}

int main(int argc, char **argv) {
    old_main(argc, argv);
    // plot_error_graphs();
    // plot_thorughput_graphs();
    // testing_error_rate_of_bff(1<<22);
    return 0;
}
