#ifndef FILTERS_MINIMAL_TESTS_HPP
#define FILTERS_MINIMAL_TESTS_HPP

#include "new_tests.hpp"

template<typename Table, typename itemType>
auto default_validation_test_single() -> bool {
    std::stringstream ss;
    return w_validate_filter<Table, itemType>(100000ul, 200000ul, 8ul, 42.0, 42.0, &ss);
}

template<typename Table, typename itemType>
auto validation_test_single(size_t filter_max_capacity, size_t lookup_reps) -> bool {
    std::stringstream ss;
    return w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, 8ul, 42.0, 42.0, &ss);
}



template<typename Table, typename itemType>
auto single_fp_rates(size_t filter_max_capacity, size_t lookup_reps, size_t bits_per_item, bool validate_before_benchmarking, vector<unordered_set<itemType> *> *elements) -> std::stringstream {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    string filter_name = FilterAPI<Table>::get_name(&filter);

    bool valid = true;
    if (validate_before_benchmarking) {
        valid = default_validation_test_single<Table, itemType>();
    }
    if (valid) {
        std::stringstream end;
        auto tp = fp_rates_single_filter<Table, itemType>(&filter, elements);
        end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_item, std::get<1>(tp), std::get<0>(tp), true);
        std::cout << end.str();
        return end;
    }
    std::stringstream end;
    return end;
}


template<typename Table, typename itemType>
auto single_fp_rates(size_t filter_max_capacity, size_t lookup_reps, size_t bits_per_item, bool validate_before_benchmarking) -> std::stringstream {
    unordered_set<itemType> v_add, v_find;//, v_delete;
    size_t add_size = std::floor(filter_max_capacity * 1);
    set_init(add_size, &v_add);
    set_init(lookup_reps, &v_find);
    vector<unordered_set<itemType> *> elements{&v_add, &v_find};//, &v_delete};
    return single_fp_rates<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_item, validate_before_benchmarking, &elements);
}

template<typename Table, typename itemType>
auto single_fp_rates_probabilistic(size_t filter_max_capacity, size_t lookup_reps, size_t bits_per_item, vector<vector<itemType> *> *elements) -> std::stringstream {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    string filter_name = FilterAPI<Table>::get_name(&filter);

    auto tp = fp_rates_single_filter_probabilistic<Table, itemType>(&filter, elements);
    size_t false_counter = std::get<0>(tp);
    size_t true_counter = std::get<1>(tp);
    assert(false_counter >= true_counter);
    std::stringstream end = print_single_round_false_positive_rates_probabilistic(filter_name, lookup_reps, bits_per_item, true_counter, false_counter, true);
    std::cout << end.str();
    return end;
}


template<typename Table, typename itemType>
auto single_fp_rates_probabilistic(size_t filter_max_capacity, size_t lookup_reps, size_t bits_per_item) -> std::stringstream {
    vector<itemType> v_add, v_find;//, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find};//, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements, false);
    return single_fp_rates_probabilistic<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_item, &elements);
}


template<typename Table, typename itemType>
void single_bench(size_t filter_max_capacity, size_t bench_precision, bool validate_before_benchmarking, vector<vector<itemType> *> *elements) {
    bool valid = true;
    if (validate_before_benchmarking) {
        valid = default_validation_test_single<Table, itemType>();
        
    }
    if (valid) {
        std::stringstream ss;
        ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, elements);
    }
}


template<typename Table, typename itemType>
void single_bench(size_t filter_max_capacity, size_t lookup_reps, size_t bench_precision, bool validate_before_benchmarking, bool with_deletions) {
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements, with_deletions);
    return single_bench<Table, itemType>(filter_max_capacity, bench_precision, validate_before_benchmarking, &elements);
}


#endif// FILTERS_MINIMAL_TESTS_HPP