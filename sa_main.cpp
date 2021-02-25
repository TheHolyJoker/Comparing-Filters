#include "dict_approximation_tests.hpp"


int main(int argc, char **argv) {
    using itemType = uint64_t;

    using Table_ts_CF = ts_cuckoofilter::ts_CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_ts_CF12 = ts_cuckoofilter::ts_CuckooFilter<uint64_t, 12, cuckoofilter::SingleTable>;
    using Table_DictApx512 = DictApx512<itemType>;

    // constexpr size_t max_filter_capacity = 15435038UL; // load is .92
    // constexpr size_t max_filter_capacity = 15770583UL;// load is .94
    constexpr size_t max_filter_capacity = (1 << 21);// load is .94
    constexpr float work_load = 0.95;
    const size_t max_number_of_elements_in_the_filter = (size_t) ceil(max_filter_capacity * work_load);
    // constexpr size_t max_filter_capacity = 15435038UL >> 3;// load is .92
    // const size_t max_filter_capacity =  62411242;
    // constexpr size_t lookup_reps = 124822484;
    constexpr size_t lookup_reps = (1 << 22);
    // constexpr size_t bench_precision = 16;
    constexpr size_t num_of_blocks_it_takes_to_fill_the_filter = 32;

    vector<itemType> vec_add, vec_find;
    vector<vector<itemType> *> elements{&vec_add, &vec_find};

    init_vectors<itemType>(max_filter_capacity, work_load, lookup_reps, 4, &elements);

    auto line = std::string(40 , '*');
    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_ts_CF12, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);

    std::cout << line << std::endl;
    compute_prob_symmetric_difference_wrapper<Table_DictApx512, itemType>(&elements, max_filter_capacity, 0.8, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    // std::cout << line << std::endl;
    
    // compute_prob_symmetric_difference_wrapper<Table_ts_CF, itemType>(&elements, max_filter_capacity, work_load, lookup_reps, num_of_blocks_it_takes_to_fill_the_filter);
    return 0;
}
