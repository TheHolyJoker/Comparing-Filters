#ifndef FILTERS_DICT_APPROXIMATION_TESTS_HPP
#define FILTERS_DICT_APPROXIMATION_TESTS_HPP

#include "minimal_tests.hpp"


template<class Table, typename itemType>
size_t
compute_false_negative_exact_with_limits(Table *wrap_filter, vector<itemType> *add_vec, size_t start, size_t end);

template<typename Table, typename itemType>
void compute_prob_symmetric_difference(vector<vector<itemType> *> *elements, size_t filter_max_capacity, size_t max_elements_in_filter, size_t num_of_blocks_it_takes_to_fill_the_filter, size_t add_block_size, size_t find_block_size);


template<class Table, typename itemType>
size_t prob_compute_false_negative(Table *wrap_filter, vector<itemType> *add_vec, size_t start, size_t end, size_t reps);

template<typename Table, typename itemType>
void fpc_sanity_check(Table *wrap_filter, vector<itemType> *query_vec);

template<typename Table, typename itemType>
void fpc_sanity_check(Table *wrap_filter, vector<itemType> *query_vec, size_t start, size_t end);

template<typename Table, typename itemType>
void fpc_strong_sanity_check(Table *wrap_filter, size_t reps);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Table, typename itemType>
void init_vectors(size_t filter_max_capacity, float work_load, size_t lookup_reps, size_t insertions_factor,
                  vector<vector<itemType> *> *elements) {

    const size_t max_elements_in_filter = (size_t) ceil(filter_max_capacity * work_load);
    size_t total_insertions_num = (std::size_t) ceil(max_elements_in_filter * insertions_factor);
    init_elements(total_insertions_num, lookup_reps, elements, 0, 0);
}


/**
 * @brief 
 * 
 * @tparam Table 
 * @tparam itemType 
 * @param elements Pointer to a vector of pointers to vectors.
 * @param filter_max_capacity 
 * @param max_elements_in_filter 
 * @param num_of_blocks_it_takes_to_fill_the_filter 
 * @param add_block_size 
 * @param find_block_size 
 */
template<typename Table, typename itemType>
void compute_prob_symmetric_difference(vector<vector<itemType> *> *elements, size_t filter_max_capacity,
                                       size_t max_elements_in_filter, size_t num_of_blocks_it_takes_to_fill_the_filter,
                                       size_t add_block_size, size_t find_block_size) {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    Table *wrap_filter = &filter;
    string filter_name = FilterAPI<Table>::get_name(&filter);
    std::cout << filter_name << std::endl;

    auto add_vec = elements->at(0);
    auto find_vec = elements->at(1);

    size_t total_insertions_num = add_vec->size();

    // Reaching the desired load.
    const size_t rounded_max_elements = (max_elements_in_filter / num_of_blocks_it_takes_to_fill_the_filter) *
                                        num_of_blocks_it_takes_to_fill_the_filter;
    auto insertion_time = time_insertions(wrap_filter, add_vec, 0, rounded_max_elements);

    // Number of rounds left. Each round contain the same number of insertions and deletions.
    size_t number_of_rounds_left = (total_insertions_num / add_block_size) - num_of_blocks_it_takes_to_fill_the_filter;


    // size_t fpc_sanity_check = 0;
    // fpc_sanity_check = prob_compute_yes_queries(wrap_filter, find_vec);
    // std::cout << "fpc_sanity_check: " << fpc_sanity_check << std::endl;
    // std::cout << "fpc_sanity_check_normalized: " << (1.0 * fpc_sanity_check / find_vec->size()) << std::endl;
    fpc_sanity_check(wrap_filter, find_vec);
    fpc_strong_sanity_check<Table, itemType>(wrap_filter, find_vec->size() / 4);

    size_t fnc_arr_exact[number_of_rounds_left];
    size_t fnc_arr_prob[number_of_rounds_left];
    size_t fpc_arr[number_of_rounds_left];
    for (int round_index = 0; round_index < number_of_rounds_left; ++round_index) {
        size_t del_start = add_block_size * round_index;
        size_t del_end = del_start + add_block_size;

        size_t add_start = rounded_max_elements + del_start;
        size_t add_end = add_start + add_block_size;

        auto removal_time = time_deletions(wrap_filter, add_vec, del_start, del_start + add_block_size);
        auto insertions_time = time_insertions(wrap_filter, add_vec, add_start, add_start + add_block_size);

        //Queries
        fnc_arr_exact[round_index] = compute_false_negative_exact_with_limits(wrap_filter, add_vec, del_end, add_end);
        fnc_arr_prob[round_index] = prob_compute_false_negative(wrap_filter, add_vec, del_end, add_end, add_block_size);

        size_t find_start = find_block_size * round_index;
        size_t find_end = find_start + find_block_size;
        fpc_arr[round_index] = prob_compute_yes_queries(wrap_filter, find_vec, find_start, find_end);
    }
    std::string fp_header = "False Positive Probabilities";
    std::string fn_header_exact = "exact False Negative Probabilities";
    std::string fn_header_prob = "prob False Negative Probabilities";

    // const size_t var_num = 3;
    // std::string names[var_num] = {"FP", "FN-Exact", "FN-prob"};
    // size_t values[var_num] = {};
    // table_print_normalized<size_t>(3, names, )
    print_array_normalized_vertical(fpc_arr, number_of_rounds_left, find_block_size, fp_header);
    print_array_normalized_vertical(fnc_arr_exact, number_of_rounds_left, rounded_max_elements, fn_header_exact);
    print_array_normalized_vertical(fnc_arr_prob, number_of_rounds_left, add_block_size, fn_header_prob);
}

template<typename Table, typename itemType>
void compute_prob_symmetric_difference_without_deletions(vector<vector<itemType> *> *elements, size_t filter_max_capacity,
                                                         size_t max_elements_in_filter, size_t num_of_blocks_it_takes_to_fill_the_filter,
                                                         size_t add_block_size, size_t find_block_size) {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    Table *wrap_filter = &filter;
    string filter_name = FilterAPI<Table>::get_name(&filter);
    std::cout << filter_name << std::endl;

    auto add_vec = elements->at(0);
    auto find_vec = elements->at(1);

    size_t total_insertions_num = add_vec->size();

    // Reaching the desired load.
    const size_t rounded_max_elements = (max_elements_in_filter / num_of_blocks_it_takes_to_fill_the_filter) *
                                        num_of_blocks_it_takes_to_fill_the_filter;
    auto insertion_time = time_insertions(wrap_filter, add_vec, 0, rounded_max_elements);

    // Number of rounds left. Each round contain the same number of insertions and deletions.
    size_t number_of_rounds_left = (total_insertions_num / add_block_size) - num_of_blocks_it_takes_to_fill_the_filter;


    fpc_sanity_check(wrap_filter, find_vec);
    fpc_strong_sanity_check<Table, itemType>(wrap_filter, find_vec->size() / 4);

    size_t fnc_arr_exact[number_of_rounds_left];
    size_t fnc_arr_prob[number_of_rounds_left];
    size_t fpc_arr[number_of_rounds_left];
    
    //Queries
    fnc_arr_exact[0] = compute_false_negative_exact_with_limits(wrap_filter, add_vec, 0, rounded_max_elements);
    fnc_arr_prob[0] = prob_compute_false_negative(wrap_filter, add_vec, 0, rounded_max_elements, add_block_size);

    size_t find_start = find_block_size * 0;
    size_t find_end = find_start + find_block_size;
    fpc_arr[0] = prob_compute_yes_queries(wrap_filter, find_vec, find_start, find_end);
    
    std::string fp_header = "False Positive Probabilities";
    std::string fn_header_exact = "exact False Negative Probabilities";
    std::string fn_header_prob = "prob False Negative Probabilities";

    print_array_normalized_vertical(fpc_arr, 1, find_block_size, fp_header);
    print_array_normalized_vertical(fnc_arr_exact, 1, rounded_max_elements, fn_header_exact);
    print_array_normalized_vertical(fnc_arr_prob, 1, add_block_size, fn_header_prob);
}

template<typename Table, typename itemType>
void compute_prob_symmetric_difference_wrapper(vector<vector<itemType> *> *elements, size_t filter_max_capacity,
                                               float work_load, size_t lookup_reps, size_t num_of_blocks_it_takes_to_fill_the_filter) {
    size_t max_elements_in_filter = (size_t) ceil(filter_max_capacity * work_load);
    size_t total_insertions_num = elements->at(0)->size();
    size_t insertion_blocks_num = num_of_blocks_it_takes_to_fill_the_filter;
    size_t add_block_size = (size_t) ceil(max_elements_in_filter / num_of_blocks_it_takes_to_fill_the_filter);
    float lookup_factor = (1.0 * lookup_reps) / filter_max_capacity;

    // If this assertion fails, then `compute_prob_symmetric_difference` will also fail due to an "std::out_of_range" error of the find_vec.
    size_t total_number_of_blocks = total_insertions_num / add_block_size;
    assert(total_insertions_num < lookup_factor * lookup_reps);

    size_t find_block_size = (std::size_t) floor(lookup_reps / total_number_of_blocks);

    // compute_prob_symmetric_difference_without_deletions<Table, itemType>(elements, filter_max_capacity, max_elements_in_filter, num_of_blocks_it_takes_to_fill_the_filter, add_block_size, find_block_size);
    compute_prob_symmetric_difference<Table, itemType>(elements, filter_max_capacity, max_elements_in_filter, num_of_blocks_it_takes_to_fill_the_filter, add_block_size, find_block_size);
}

/**
 * @brief Get the rand position in vec object
 * 
 * @tparam itemType 
 * @param vec 
 * @param start 
 * @param end 
 * @param reps 
 * @return size_t Uniformly random index between [start, end - reps]. 
 */
template<typename itemType>
size_t get_rand_position_in_vec(vector<itemType> *vec, size_t start, size_t end, size_t reps) {
    assert(end < vec->size());
    assert(start + reps < end);

    size_t fn_counter = 0;
    const size_t mod = end - reps - start;
    size_t rand_start = start + (rand() % mod);
    assert(rand_start + reps <= end);
    return rand_start;
}

template<class Table, typename itemType>
size_t prob_compute_false_negative(Table *wrap_filter, vector<itemType> *add_vec, size_t start, size_t end, size_t reps) {
    assert(end < add_vec->size());
    assert(start + reps < end);

    size_t fn_counter = 0;
    const size_t mod = end - reps - start;
    size_t rand_start = start + (rand() % mod);
    assert(rand_start + reps <= end);
    for (size_t i = 0; i < reps; ++i) {
        bool temp_res = FilterAPI<Table>::Contain(add_vec->at(rand_start + i), wrap_filter);
        if (!temp_res)
            fn_counter++;
    }
    return fn_counter;
}


template<class Table, typename itemType>
size_t
compute_false_negative_exact(Table *wrap_filter, vector<itemType> *add_vec, size_t round_index, size_t op_block_size,
                             size_t insertion_blocks_num) {
    size_t fn_counter = 0;
    const size_t start = round_index * op_block_size;
    const size_t end = start + (op_block_size * round_index);
    compute_false_negative_exact_with_limits(wrap_filter, add_vec, start, end);
    for (size_t i = start; i < end; ++i) {
        bool temp_res = FilterAPI<Table>::Contain(add_vec->at(i), wrap_filter);
        if (!temp_res)
            fn_counter++;
    }
    return fn_counter;
}

template<class Table, typename itemType>
size_t
compute_false_negative_exact_with_limits(Table *wrap_filter, vector<itemType> *add_vec, size_t start, size_t end) {
    size_t fn_counter = 0;
    for (size_t i = start; i < end; ++i) {
        bool temp_res = FilterAPI<Table>::Contain(add_vec->at(i), wrap_filter);
        if (!temp_res)
            fn_counter++;
    }
    return fn_counter;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Table, typename itemType>
void bench_symmetric_difference(vector<vector<itemType> *> *elements, size_t filter_max_capacity,
                                size_t max_elements_in_filter, size_t num_of_blocks_it_takes_to_fill_the_filter,
                                size_t add_block_size, size_t find_block_size) {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    Table *wrap_filter = &filter;
    string filter_name = FilterAPI<Table>::get_name(&filter);
    std::cout << filter_name << std::endl;

    auto add_vec = elements->at(0);
    auto find_vec = elements->at(1);


    /*
    size_t empty_filter_fp_sanity_check = 0;
    empty_filter_fp_sanity_check = prob_compute_yes_queries(wrap_filter, find_vec);
    std::cout << " | empty_filter_fp_sanity_check: \t\t" << empty_filter_fp_sanity_check << std::endl;
    double empty_filter_fp_sanity_check_normalized = (1.0 * empty_filter_fp_sanity_check / find_vec->size());
    std::cout << " | empty_filter_fp_sanity_check normalized: \t\t" << empty_filter_fp_sanity_check_normalized << std::endl;
    */
    fpc_sanity_check(wrap_filter, find_vec);


    size_t total_insertions_num = add_vec->size();

    // Reaching the desired load.
    const size_t rounded_max_elements = (max_elements_in_filter / num_of_blocks_it_takes_to_fill_the_filter) *
                                        num_of_blocks_it_takes_to_fill_the_filter;
    auto junk_insertion_time = time_insertions(wrap_filter, add_vec, 0, rounded_max_elements);

    // Number of rounds left. Each round contain the same number of insertions and deletions.
    size_t number_of_rounds_left = (total_insertions_num / add_block_size) - num_of_blocks_it_takes_to_fill_the_filter;


    std::stringstream ss = print_name(FilterAPI<Table>::get_name(&filter), 134);
    std::cout << ss.str();


    /*
    size_t fpc_sanity_check = 0;
    fpc_sanity_check = prob_compute_yes_queries(wrap_filter, find_vec);
    std::cout << " | fpc_sanity_check:            \t" << fpc_sanity_check << "\t\tvec_size:\t" << find_vec->size() << std::endl;
    std::cout << " | fpc_sanity_check normalized: \t" << (1.0 * fpc_sanity_check / find_vec->size()) << "\t\tExpecting:\t" << 1.0 / 32 << std::endl;
    */

    fpc_sanity_check(wrap_filter, find_vec);
    fpc_strong_sanity_check<Table, itemType>(wrap_filter, find_vec->size() / 4);

    std::stringstream ss1 = print_round_header();
    std::cout << ss1.str();

    // size_t fnc_arr_exact[number_of_rounds_left];
    // size_t fnc_arr_prob[number_of_rounds_left];
    // size_t fpc_arr[number_of_rounds_left];
    for (size_t round_index = 0; round_index < number_of_rounds_left; ++round_index) {
        size_t del_start = add_block_size * round_index;
        size_t del_end = del_start + add_block_size;

        size_t add_start = rounded_max_elements + del_start;
        size_t add_end = add_start + add_block_size;

        auto removal_time = time_deletions(wrap_filter, add_vec, del_start, del_start + add_block_size);
        auto insertions_time = time_insertions(wrap_filter, add_vec, add_start, add_start + add_block_size);

        size_t uni_start = round_index * find_block_size;
        size_t uni_end = uni_start + find_block_size;
        auto uniform_lookup_time = time_lookups(wrap_filter, find_vec, uni_start, uni_end);

        size_t rand_start = get_rand_position_in_vec(add_vec, del_end, add_end, find_block_size);
        size_t yes_q_start = rand_start;
        size_t yes_q_end = yes_q_start + find_block_size;
        auto yes_queries_lookup_time = time_lookups(wrap_filter, add_vec, yes_q_start, yes_q_end);


        // size_t last_round_indicator = (round_index + 1 == number_of_rounds_left) ? round_index : round_index + 1;
        const size_t var_num = 6;
        size_t values[var_num] = {round_index, number_of_rounds_left - 1, insertions_time, uniform_lookup_time,
                                  yes_queries_lookup_time, removal_time};


        size_t divisors[var_num - 2] = {add_block_size, find_block_size, find_block_size, add_block_size};
        auto temp = print_single_round(var_num, values, divisors);
        std::cout << temp.str();
    }

    size_t del_end = add_block_size * number_of_rounds_left;
    size_t add_end = del_end + rounded_max_elements;

    size_t prob_fnc = prob_compute_false_negative(wrap_filter, add_vec, del_end, add_end, add_block_size << 1);
    size_t fpc = prob_compute_yes_queries(wrap_filter, find_vec, 0, find_block_size << 1);

    double fnc_ratio = 1.0 * prob_fnc / (add_block_size << 1);
    double fpc_ratio = 1.0 * fpc / (find_block_size << 1);
    std::cout << "fp ratio:  " << fpc_ratio << std::endl;
    std::cout << "fn ratio:  " << fnc_ratio << std::endl;
}

template<typename Table, typename itemType>
void bench_symmetric_difference_wrapper(vector<vector<itemType> *> *elements, size_t filter_max_capacity,
                                        float work_load, size_t lookup_reps, size_t num_of_blocks_it_takes_to_fill_the_filter) {
    size_t max_elements_in_filter = (size_t) ceil(filter_max_capacity * work_load);
    size_t total_insertions_num = elements->at(0)->size();
    size_t insertion_blocks_num = num_of_blocks_it_takes_to_fill_the_filter;
    size_t add_block_size = (size_t) ceil(max_elements_in_filter / num_of_blocks_it_takes_to_fill_the_filter);
    float lookup_factor = (1.0 * lookup_reps) / filter_max_capacity;

    // If this assertion fails, then `compute_prob_symmetric_difference` will also fail due to an "std::out_of_range" error of the find_vec.
    size_t total_number_of_blocks = total_insertions_num / add_block_size;
    assert(total_insertions_num < lookup_factor * lookup_reps);

    size_t find_block_size = (std::size_t) floor(lookup_reps / total_number_of_blocks);

    bench_symmetric_difference<Table, itemType>(elements, filter_max_capacity, max_elements_in_filter, num_of_blocks_it_takes_to_fill_the_filter, add_block_size, find_block_size);
}

template<typename Table, typename itemType>
void fpc_sanity_check(Table *wrap_filter, vector<itemType> *query_vec, size_t start, size_t end) {
    assert(start < end);
    size_t size = end - start;
    size_t filter_fp_count = 0;
    filter_fp_count = prob_compute_yes_queries(wrap_filter, query_vec, start, end);
    // std::cout << " | filter_fp_count: \t\t" << filter_fp_count << std::endl;
    double filter_fp_count_normalized = (1.0 * filter_fp_count / size);
    std::cout << " | filter_fp_count normalized: \t\t" << filter_fp_count_normalized << std::endl;
}

template<typename Table, typename itemType>
void fpc_sanity_check(Table *wrap_filter, vector<itemType> *query_vec) {
    fpc_sanity_check(wrap_filter, query_vec, 0, query_vec->size());

    // strong_random_prob_compute_yes_queries
}

template<typename Table, typename itemType>
void fpc_strong_sanity_check(Table *wrap_filter, size_t reps) {
    size_t filter_fp_count = strong_random_prob_compute_yes_queries<Table, itemType>(wrap_filter, reps);
    double filter_fp_counter_normed = (1.0 * filter_fp_count / reps);
    std::cout << " | Strong fpc test: " << std::endl;
    std::cout << " | filter_fp_count normalized: \t\t" << filter_fp_counter_normed << std::endl;
    // fpc_sanity_check(wrap_filter, query_vec, 0, query_vec->size());
}


#endif// FILTERS_DICT_APPROXIMATION_TESTS_HPP