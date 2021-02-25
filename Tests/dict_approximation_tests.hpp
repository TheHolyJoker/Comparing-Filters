#ifndef FILTERS_DICT_APPROXIMATION_TESTS_HPP
#define FILTERS_DICT_APPROXIMATION_TESTS_HPP

#include "minimal_tests.hpp"


template<class Table, typename itemType>
size_t
compute_false_negative_exact_with_limits(Table *wrap_filter, vector<itemType> *add_vec, size_t start, size_t end);

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


    size_t fpc_sanity_check = 0;
    fpc_sanity_check = prob_compute_yes_queries(wrap_filter, find_vec);
    std::cout << "fpc_sanity_check: " << fpc_sanity_check << std::endl;
    std::cout << "fpc_sanity_check_normalized: " << (1.0 * fpc_sanity_check / find_vec->size()) << std::endl;
    size_t fnc_arr[number_of_rounds_left];
    size_t fpc_arr[number_of_rounds_left];
    for (int round_index = 0; round_index < number_of_rounds_left; ++round_index) {
        size_t del_start = add_block_size * round_index;
        size_t del_end = del_start + add_block_size;

        size_t add_start = rounded_max_elements + del_start;
        size_t add_end = add_start + add_block_size;

        auto removal_time = time_deletions(wrap_filter, add_vec, del_start, del_start + add_block_size);
        auto insertions_time = time_insertions(wrap_filter, add_vec, add_start, add_start + add_block_size);

        //Queries
        fnc_arr[round_index] = compute_false_negative_exact_with_limits(wrap_filter, add_vec, del_end, add_end);

        size_t find_start = find_block_size * round_index;
        size_t find_end = find_start + find_block_size;
        fpc_arr[round_index] = prob_compute_yes_queries(wrap_filter, find_vec, find_start, find_end);
    }
    std::string fp_header = "False Positive Probabilities";
    std::string fn_header = "False Negative Probabilities";
    print_array_normalized_vertical(fpc_arr, number_of_rounds_left, find_block_size, fp_header);
    print_array_normalized_vertical(fnc_arr, number_of_rounds_left, rounded_max_elements, fn_header);
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

    compute_prob_symmetric_difference<Table, itemType>(elements, filter_max_capacity, max_elements_in_filter, num_of_blocks_it_takes_to_fill_the_filter, add_block_size, find_block_size);
}

template<class Table, typename itemType>
size_t compute_false_negative(Table *wrap_filter, vector<itemType> *add_vec, size_t round) {
    assert(0);
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


//void v_get_vector_intersection


#endif// FILTERS_DICT_APPROXIMATION_TESTS_HPP