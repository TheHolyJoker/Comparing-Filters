//
// Created by root on 31/05/2020.
//

#ifndef FILTERS_NEW_TESTS_HPP
#define FILTERS_NEW_TESTS_HPP

#include "tests.hpp"
#include "wrappers.hpp"
#include <Validation/validate_hash_table.hpp>
#include <chrono>
#include <set>

typedef chrono::nanoseconds ns;


////////////// Declarations:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////Basic functions:
template<typename itemType>
auto init_elements(size_t max_filter_capacity, size_t lookup_reps, vector<vector<itemType> *> *elements, size_t bench_precision = 20);


// template<typename itemType>
// auto init_elements_att(size_t max_filter_capacity, size_t lookup_reps, vector<unordered_set<itemType> *> *elements);

template<class Table, typename itemType>
auto time_lookups(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong;

template<class Table, typename itemType>
auto time_insertions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong;

template<class Table, typename itemType>
auto time_deletions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong;


///////Benchmarking and wrappers:
template<class Table, typename itemType>
void benchmark_single_round(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t round_counter, size_t benchmark_precision, std::stringstream *ss);

template<class Table, typename itemType>
auto benchmark_generic_filter(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t bench_precision) -> std::stringstream;

template<class Table, typename itemType>
auto benchmark_single_filter_wrapper(size_t filter_max_capacity, size_t bench_precision, vector<vector<itemType> *> *elements) -> std::stringstream;

template<typename itemType, template<typename> class hashTable>
auto benchmark_dict(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision, vector<vector<itemType> *> *elements, ostream &os) -> std::stringstream;


template<typename itemType, size_t bits_per_element, size_t CF_ss_bits>
auto b_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision, bool validate_before_benchmarking,
                   bool BF = true, bool CF = true, bool CF_ss = true, bool MT = true, bool SIMD = true, bool call_PD = true, bool pd512 = true, bool pd512_ver2 = true, bool pd320 = true, bool pd320_v2 = true, bool pd512_CF = true, bool TC = true, ostream &os = cout) -> std::stringstream;

template<typename itemType, size_t bits_per_element, typename HashFamily>
auto b_hash_with_Dict512(size_t filter_max_capacity, size_t lookup_reps, bool validate_before_benchmarking) -> std::stringstream;

///// Compute false positive rate.
template<typename itemType, size_t bits_per_element, size_t CF_ss_bits>
auto fp_rates_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, bool validate_before_benchmarking, bool BF = true, bool CF = true, bool CF_ss = true, bool MT = true, bool SIMD = true, bool call_PD = true, bool pd512 = true, bool pd512_ver2 = true, bool pd320 = true, bool pd320_v2 = true, bool pd512_CF = true, bool TC = true, double load = 1, ostream &os = cout) -> std::stringstream;


// template<typename Table, typename itemType>
// auto single_fp_rates(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, bool validate_before_benchmarking) -> std::stringstream;


// template<typename Table, typename itemType>
// auto single_fp_rates(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, bool validate_before_benchmarking, vector<unordered_set<itemType> *> *elements) -> std::stringstream;

template<class Table, typename itemType>
auto fp_rates_single_filter(Table *wrap_filter, vector<unordered_set<itemType> *> *elements) -> std::tuple<size_t, size_t>;


template<typename itemType, size_t bits_per_element, typename HashFamily>
auto hash_fp_rates_with_Dict512(size_t filter_max_capacity, size_t lookup_reps, bool validate_before_benchmarking) -> std::stringstream;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename itemType, size_t bits_per_element, size_t CF_ss_bits>
auto b_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision, bool validate_before_benchmarking,
                   bool BF, bool CF, bool CF_ss, bool MT, bool SIMD, bool call_PD, bool pd512, bool pd512_ver2, bool pd320, bool pd320_v2, bool pd512_CF, bool TC, ostream &os) -> std::stringstream {

    std::stringstream debug_info;
    // assert(filter_max_capacity % bench_precision == 0);
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements);
    if (BF) {
        using Table = bloomfilter::bloom<itemType, bits_per_element, false, hashing::TwoIndependentMultiplyShift>;
        auto temp = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
        debug_info << temp.str();
        // benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements, os);
        // os << ss.str();
        // debug_info << ss.str();
    }
    if (CF) {

        // while (true) {
        std::stringstream v_info;
        using Table = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "CF is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            debug_info << ss.str();
        }
    }
    if (CF_ss) {
        std::stringstream v_info;
        using Table = cuckoofilter::CuckooFilter<uint64_t, CF_ss_bits, cuckoofilter::PackedTable>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, CF_ss_bits, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "CF-ss is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            debug_info << ss.str();
        }
    }
    if (SIMD) {
        std::stringstream v_info;
        using Table = SimdBlockFilter<>;
        std::stringstream ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
        debug_info << ss.str();
    }
    if (MT) {
        std::stringstream v_info;
        using Table = MortonFilter;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "MT is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            debug_info << ss.str();
        }
    }
    if (call_PD) {
        //        using Table = dict<PD, hash_table, itemType, uint32_t>;
        auto ss = benchmark_dict<uint64_t, hash_table>(filter_max_capacity, error_power_inv, bench_precision, &elements, os);
        debug_info << ss.str();
    }
    if (pd512) {

        // b_hash_with_Dict512<itemType, bits_per_element, hashing::TwoIndependentMultiplyShift>(filter_max_capacity, lookup_reps, validate_before_benchmarking);
        // b_hash_with_Dict512<itemType, bits_per_element, hashing::my_wyhash64>(filter_max_capacity, lookup_reps, validate_before_benchmarking);
        // b_hash_with_Dict512<itemType, bits_per_element, hashing::my_xxhash64>(filter_max_capacity, lookup_reps, validate_before_benchmarking);
        // b_hash_with_Dict512<itemType, bits_per_element, hashing::my_BobHash>(filter_max_capacity, lookup_reps, validate_before_benchmarking);

        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        std::stringstream v_info;

        using Table = Dict512<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        // using Table = Dict512<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift, 8, 51, 32>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "pd512 is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss;
            // while (true) {
            ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            // }
            debug_info << ss.str();
        }
    }

    if (pd512_ver2) {
        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        std::stringstream v_info;

        using Table = Dict512_Ver2<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "pd512_ver2 is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss;
            // while (true) {
            ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            // }
            debug_info << ss.str();
        }
    }
    if (pd320) {
        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        std::stringstream v_info;

        using Table = Dict320<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "pd320 is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss;
            // while (true) {
            ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            // }
            debug_info << ss.str();
        }
    }
    if (pd320_v2) {
        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        std::stringstream v_info;

        using Table = Dict320_v2<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "pd320_v2 is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss;
            // while (true) {
            ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            // }
            debug_info << ss.str();
        }
    }

    if (pd512_CF) {
        using spare_item = uint64_t;
        using temp_hash = hashTable_CuckooFilter;
        std::stringstream v_info;

        using Table = Dict512_With_CF<itemType>;
        // using Table = Dict512<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift, 8, 51, 32>;
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "pd512 is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss;
            // while (true) {
            ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            // }
            debug_info << ss.str();
        }
    }

    if (TC) {
        // while (true) {
        using Table = twoChoicer<itemType>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);
        std::stringstream v_info;


        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            debug_info << v_info.str();
            if (!valid)
                std::cout << "twoChoicer is not valid!" << std::endl;
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
            debug_info << ss.str();
        }
    }
    // return os;
    return debug_info;
}

template<typename itemType, size_t bits_per_element, typename HashFamily>
auto b_hash_with_Dict512(size_t filter_max_capacity, size_t lookup_reps, bool validate_before_benchmarking) -> std::stringstream {
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements);

    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;
    std::stringstream v_info;

    using Table = Dict512<temp_hash, spare_item, itemType, HashFamily>;
    bool valid = true;
    if (validate_before_benchmarking) {
        valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, 8, 1, .5, &v_info);
        // debug_info << v_info.str();
        if (!valid)
            std::cout << "pd512 is not valid!" << std::endl;
    }
    if ((!validate_before_benchmarking) or (valid)) {
        std::stringstream ss;
        ss = benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, 8, &elements);
        // debug_info << ss.str();
    }
    return v_info;
}
template<typename itemType, size_t bits_per_element, typename HashFamily>
auto hash_fp_rates_with_Dict512(size_t filter_max_capacity, size_t lookup_reps, bool validate_before_benchmarking) -> std::stringstream {
    unordered_set<itemType> v_add, v_find, v_delete;
    size_t add_size = std::floor(filter_max_capacity * 1);
    set_init(add_size, &v_add);
    set_init(lookup_reps, &v_find);
    vector<unordered_set<itemType> *> elements{&v_add, &v_find, &v_delete};

    std::stringstream ss;
    using spare_item = uint64_t;
    using temp_hash = att_hTable<spare_item, 4>;
    // using Table = Dict512<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift, 8, 51, 32>;
    using Table = Dict512<temp_hash, spare_item, itemType, HashFamily>;
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    string filter_name = FilterAPI<Table>::get_name(&filter);

    bool valid = true;
    if (validate_before_benchmarking) {
        std::stringstream v_info;
        valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
        if (!valid)
            std::cout << filter_name << " is not valid!" << std::endl;
        // debug_info << ss.str();
    }
    if ((!validate_before_benchmarking) or (valid)) {
        std::stringstream end;
        auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
        end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), false);
        std::cout << end.str();
    }
    return ss;
}


template<class Table, typename itemType>
auto benchmark_single_filter_wrapper(size_t filter_max_capacity, size_t bench_precision, vector<vector<itemType> *> *elements) -> std::stringstream {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();


    std::stringstream ss = print_name(FilterAPI<Table>::get_name(&filter), 134);
    std::cout << ss.str();
    auto ss2 = benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision);
    ss << ss2.str();
    return ss;
    // if (FilterAPI<Table>::get_ID(&filter) == CF) {
    //     FilterAPI<Table>::get_info(&filter);
    // }
    // return os;
}

template<class Table, typename itemType>
auto benchmark_generic_filter(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t bench_precision) -> std::stringstream {

    std::stringstream ss = print_round_header();
    std::cout << ss.str();

    // *ss << temp_ss.rdbuf();
    for (int round = 0; round < bench_precision; ++round) {
        std::stringstream flusher;
        benchmark_single_round<Table, itemType>(wrap_filter, elements, round, bench_precision, &flusher);
        std::cout << flusher.str();
        ss << flusher.str();
    }
    return ss;
    // *ss << flusher.str();
}

template<class Table, typename itemType>
void benchmark_single_round(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t round_counter,
                            size_t benchmark_precision, std::stringstream *ss) {

    auto add_vec = elements->at(0);
    auto find_vec = elements->at(1);
    auto delete_vec = elements->at(2);


    size_t add_step = add_vec->size() / benchmark_precision;
    size_t find_step = find_vec->size() / benchmark_precision;
    size_t true_find_step = add_step;
    // size_t delete_step = delete_vec->size() / benchmark_precision;

    size_t removal_time = 0;
    if (delete_vec->size()) {
        auto del_insertion_time = time_insertions(wrap_filter, delete_vec, 0, delete_vec->size());
        removal_time = time_deletions(wrap_filter, delete_vec, 0, delete_vec->size());
    }


    auto insertion_time = time_insertions(wrap_filter, add_vec, round_counter * add_step, (round_counter + 1) * add_step);
    auto uniform_lookup_time = time_lookups(wrap_filter, find_vec, round_counter * find_step, (round_counter + 1) * find_step);
    // size_t true_lookup_time = 0;
    auto true_lookup_time = time_lookups(wrap_filter, add_vec, 0, true_find_step);


    const size_t var_num = 6;
    //    string names[var_num] = {"Load", "insertion_time", "uniform_lookup_time", "true_lookup_time", "removal_time"};
    size_t values[var_num] = {round_counter + 1, benchmark_precision, insertion_time, uniform_lookup_time,
                              true_lookup_time, removal_time};

    size_t divisors[var_num - 2] = {add_step, find_step, true_find_step, delete_vec->size()};
    auto temp = print_single_round(var_num, values, divisors);
    *ss << temp.str();

    if (FilterAPI<Table>::get_ID(wrap_filter) == d512_ver2)
        FilterAPI<Table>::get_info(wrap_filter);
    // return os;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// template<typename itemType>
// auto weird_name2(size_t max_filter_capacity, size_t lookup_reps, vector<unordered_set<itemType> *> *elements) {
//     set_init(max_filter_capacity, elements->at(0));
//     set_init(lookup_reps, elements->at(1));
//     // set_init(max_filter_capacity, elements->at(2)); // for deletions.
// }


template<typename itemType, size_t bits_per_element, size_t CF_ss_bits>
auto fp_rates_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, bool validate_before_benchmarking,
                          bool BF, bool CF, bool CF_ss, bool MT, bool SIMD, bool call_PD, bool pd512, bool pd512_ver2, bool pd320, bool pd320_v2, bool pd512_CF, bool TC,
                          double load, ostream &os) -> std::stringstream {
    unordered_set<itemType> v_add, v_find, v_delete;
    size_t add_size = std::floor(filter_max_capacity * load);
    set_init(add_size, &v_add);
    set_init(lookup_reps, &v_find);
    vector<unordered_set<itemType> *> elements{&v_add, &v_find, &v_delete};

    std::stringstream debug_info;
    std::stringstream min_output = print_false_positive_rates_header();
    std::cout << min_output.str();
    if (BF) {
        using Table = bloomfilter::bloom<itemType, bits_per_element, false, hashing::TwoIndependentMultiplyShift>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        std::stringstream end;
        auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
        end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp));
        min_output << end.str();
        std::cout << end.str();
    }
    if (CF) {
        std::stringstream ss;
        using Table = cuckoofilter::CuckooFilter<uint64_t, bits_per_element, cuckoofilter::SingleTable>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5, &ss);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp));
            min_output << end.str();
            std::cout << end.str();
        }
        // os << ss.str();
    }
    if (CF_ss) {
        std::stringstream ss;

        using Table = cuckoofilter::CuckooFilter<uint64_t, CF_ss_bits, cuckoofilter::PackedTable>;

        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);
        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, CF_ss_bits, 1, .5, &ss);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, CF_ss_bits, std::get<1>(tp), std::get<0>(tp));
            min_output << end.str();
            std::cout << end.str();
        }
        // os << ss.str();
    }
    if (SIMD) {
        std::stringstream ss;
        using Table = SimdBlockFilter<>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);
        std::stringstream end;
        auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
        end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp));
        min_output << end.str();
        std::cout << end.str();

        // os << ss.str();
    }
    if (MT) {
        std::stringstream ss;
        using Table = MortonFilter;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);
        bool valid = true;
        if (validate_before_benchmarking) {
            Table validation_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity >> 2u);
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &ss);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp));
            min_output << end.str();
            std::cout << end.str();
        }
        // os << ss.str();
    }

    if (pd512) {
        // hash_fp_rates_with_Dict512<itemType, bits_per_element, hashing::TwoIndependentMultiplyShift>(filter_max_capacity, lookup_reps, validate_before_benchmarking);
        // hash_fp_rates_with_Dict512<itemType, bits_per_element, hashing::my_wyhash64>(filter_max_capacity, lookup_reps, validate_before_benchmarking);
        // hash_fp_rates_with_Dict512<itemType, bits_per_element, hashing::my_xxhash64>(filter_max_capacity, lookup_reps, validate_before_benchmarking);

        // while (true) {
        std::stringstream ss;
        using spare_item = uint64_t;
        using temp_hash = att_hTable<spare_item, 4>;
        using Table = Dict512<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        bool valid = true;
        if (validate_before_benchmarking) {
            std::stringstream v_info;
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), true ^ TC);
            min_output << end.str();
            std::cout << end.str();
        }
        // }
    }
    if (pd512_ver2) {
        std::stringstream ss;
        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        using Table = Dict512_Ver2<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        bool valid = true;
        if (validate_before_benchmarking) {
            std::stringstream v_info;
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), false);
            min_output << end.str();
            std::cout << end.str();
        }
    }
    if (pd320) {
        std::stringstream ss;
        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        using Table = Dict320<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        bool valid = true;
        if (validate_before_benchmarking) {
            std::stringstream v_info;
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), false);
            min_output << end.str();
            std::cout << end.str();
        }
    }
    if (pd320_v2) {
        std::stringstream ss;
        using spare_item = uint64_t;
        using temp_hash = hashTable_Aligned<spare_item, 4>;
        using Table = Dict320_v2<temp_hash, spare_item, itemType, hashing::TwoIndependentMultiplyShift>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        bool valid = true;
        if (validate_before_benchmarking) {
            std::stringstream v_info;
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), false);
            min_output << end.str();
            std::cout << end.str();
        }
    }
    if (pd512_CF) {
        std::stringstream ss;
        using spare_item = uint64_t;
        // using temp_hash = att_hTable<spare_item, 4>;
        using Table = Dict512_With_CF<itemType>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        bool valid = true;
        if (validate_before_benchmarking) {
            std::stringstream v_info;
            valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_element, 1, .5, &v_info);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), true ^ TC);
            min_output << end.str();
            std::cout << end.str();
        }
    }
    if (TC) {
        std::stringstream ss;
        using Table = twoChoicer<itemType>;
        Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
        string filter_name = FilterAPI<Table>::get_name(&filter);

        bool valid = true;
        if (validate_before_benchmarking) {
            valid = w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5, &ss);
            if (!valid)
                std::cout << filter_name << " is not valid!" << std::endl;
            debug_info << ss.str();
        }
        if ((!validate_before_benchmarking) or (valid)) {
            std::stringstream end;
            auto tp = fp_rates_single_filter<Table, itemType>(&filter, &elements);
            end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_element, std::get<1>(tp), std::get<0>(tp), true);
            min_output << end.str();
            std::cout << end.str();
        }
    }
    // os << min_output.str();
    return debug_info;
}

// template<typename Table, typename itemType>
// auto single_fp_rates(size_t filter_max_capacity, size_t lookup_reps, size_t bits_per_item, bool validate_before_benchmarking) -> std::stringstream {
//     unordered_set<itemType> v_add, v_find;//, v_delete;
//     size_t add_size = std::floor(filter_max_capacity * 1);
//     set_init(add_size, &v_add);
//     set_init(lookup_reps, &v_find);
//     vector<unordered_set<itemType> *> elements{&v_add, &v_find};//, &v_delete};
//     return single_fp_rates<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_item, validate_before_benchmarking, &elements);
// }

// template<typename Table, typename itemType>
// auto single_fp_rates(size_t filter_max_capacity, size_t lookup_reps, size_t bits_per_item, bool validate_before_benchmarking, vector<unordered_set<itemType> *> *elements) -> std::stringstream {
//     Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
//     string filter_name = FilterAPI<Table>::get_name(&filter);

//     bool valid = true;
//     if (validate_before_benchmarking) {
//         std::stringstream v_info;
//         valid = w_validate_filter<Table, itemType>(filter_max_capacity >> 2u, lookup_reps >> 2u, bits_per_item, 42.0, 42.0, &v_info);
//         if (!valid)
//             std::cout << filter_name << " is not valid!" << std::endl;
//         // debug_info << ss.str();
//     }
//     if (valid) {
//         std::stringstream end;
//         auto tp = fp_rates_single_filter<Table, itemType>(&filter, elements);
//         end = print_single_round_false_positive_rates(filter_name, lookup_reps, bits_per_item, std::get<1>(tp), std::get<0>(tp), true);
//         std::cout << end.str();
//         return end;
//     }
//     std::stringstream end;
//     return end;
// }


template<class Table, typename itemType>
auto fp_rates_single_filter(Table *wrap_filter, vector<unordered_set<itemType> *> *elements) -> std::tuple<size_t, size_t> {

    size_t counter = 0;
    /**Insertion*/
    for (auto el : *elements->at(0)) {
        FilterAPI<Table>::Add(el, wrap_filter);
        counter++;
    }

    size_t fp_counter = 0;
    size_t tp_counter = 0;
    for (auto el : *elements->at(1)) {
        if (FilterAPI<Table>::Contain(el, wrap_filter)) {
            bool is_tp = (elements->at(0))->find(el) != (elements->at(0))->end();
            (is_tp) ? tp_counter++ : fp_counter++;
        }
    }
    return std::make_tuple(fp_counter, tp_counter);

    /* 
    // att_print_single_round_false_positive_rates(lookup_set.size(), error_power_inv, fp_counter, tp_counter);
    // print_single_round_false_positive_rates(filter_max_capacity, lookup_set.size() >> error_power_inv, tp_counter,
    //                                         fp_counter);
    //     cout << "filter_max_capacity: " << filter_max_capacity << endl;
    //    cout << "\nnumber of false-positive is out of total number of lookups: " << fp_counter << "/ " << lookup_reps << endl;
    //    cout << "Expected FP count: " << (lookup_set.size() >> error_power_inv) << endl;
    counter = 0;
    cond &= v_deleting<Table, itemType>(wrap_filter, &to_be_deleted_set, &member_set);
    if (!cond)
        return false;

    assert(cond);
    ////Deletions////

    //Verifying no unwanted element was deleted (prone to error as deleting from filter is not well defined)//
    cond &= v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    // if (!cond)
    //     return false;

    assert(cond);
    if (FilterAPI<Table>::get_ID(wrap_filter) == CF) {
        FilterAPI<Table>::get_info(wrap_filter);
    }
    return cond; 
    */
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<typename itemType>
auto init_elements(size_t max_filter_capacity, size_t lookup_reps, vector<vector<itemType> *> *elements, size_t bench_precision) {
    fill_vec(elements->at(0), max_filter_capacity);
    // size_t del_size = max_filter_capacity / (double) bench_precision;
    // fill_vec(elements->at(2), del_size);
    fill_vec(elements->at(1), lookup_reps);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Table, typename itemType>
auto time_lookups(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong {
    static volatile bool dummy;
    bool x = 0;

    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start; i < end; ++i)
        x |= FilterAPI<Table>::Contain(element_set->at(i), wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();

    dummy = x;
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto time_insertions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    FilterAPI<Table>::AddAll(*element_set, start, end, wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto time_deletions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong {
    try {
        FilterAPI<Table>::Remove(element_set->at(start), wrap_filter);
    } catch (std::runtime_error &msg) {
        return 0;
    }
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start + 1; i < end; ++i)
        FilterAPI<Table>::Remove(element_set->at(i), wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename itemType, template<typename> class hashTable>
auto benchmark_dict(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision, vector<vector<itemType> *> *elements, ostream &os) -> std::stringstream {
    std::stringstream ss;
    using Table = dict<PD, hashTable, itemType, uint64_t>;

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity, error_power_inv);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    auto name = print_name(FilterAPI<Table>::get_name(&filter), 134);
    std::cout << name.str();
    ss << name.str();
    auto temp_ss = benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision);
    // std::cout << temp_ss.str();
    ss << temp_ss.str();
    return ss;
}

//template<template <typename slot_type, size_t bits_per_item, size_t max_capacity> class temp_PD ,typename itemType, template<typename> class hashTable>
template<typename itemType, template<typename> class hashTable>
auto att_benchmark_single_filter_wrapper(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision,
                                         vector<vector<itemType> *> *elements, ostream &os) -> ostream & {
    using temp_PD = TPD_name::TPD<uint32_t, 8, 64>;
    using Table = dict<temp_PD, hashTable, itemType, uint64_t>;

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity, error_power_inv);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(FilterAPI<Table>::get_name(), 134);
    benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision, os);
    return os;
}

template<typename itemType, size_t bits_per_element>
auto att_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                     uint indicator, ostream &os) -> ostream & {
    // assert(filter_max_capacity % bench_precision == 0);
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements);

    // uint b = 1u;
    // if (indicator & b) {
    //     using spare_item = uint64_t;
    //     using temp_hash = att_hTable<spare_item, 4>;
    //     using Table = dict512<temp_hash, spare_item, itemType>;
    //     w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5,os);
    //     benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
    // }

    using spare_item = uint64_t;
    using temp_hash = att_hTable<spare_item, 4>;
    using Table = Dict512<temp_hash, spare_item, itemType>;
    benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
    // // bool valid = (w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5,os));
    // // if (!valid) {
    // //     std::cout << "PD512 is not valid!" << std::endl;
    // // }
    // else


    /* if (indicator & b) {
        using spare_item = uint64_t;
        const size_t max_capacity = 48u;
        using temp_PD = TPD_name::TPD<uint32_t, 8, max_capacity>;
        using temp_hash = att_hTable<spare_item , 4>;
        using Table = T_dict<temp_PD, uint64_t, 8, max_capacity, temp_hash, spare_item , itemType>;
        benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
    }
       b <<= 1u;
       if (indicator & b) {
           using spare_item = uint64_t;
           using temp_PD = TPD_name::TPD<uint32_t, 8, 64>;
           using temp_hash = att_hTable<spare_item, 4>;
           using Table = T_dict<temp_PD, uint64_t, 8, 64, temp_hash, spare_item, itemType>;
        //    w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5,os);
           benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
       }
       b <<= 1u;

       b <<= 1u;
    */

    /*if (indicator & b) {
        w_validate_filter<itemType, hash_table>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5,os);
        benchmark_single_filter_wrapper<uint64_t, hash_table>(filter_max_capacity, error_power_inv, bench_precision,
            &elements);
    }*/
    return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//template<typename itemType, template<typename> class hashTable>
//template<template <typename slot_type, size_t bits_per_item, size_t max_capacity> class temp_PD ,typename itemType, template<typename> class hashTable>
template<template<typename, size_t, size_t> class temp_PD, typename slot_type, size_t bits_per_item, size_t max_capacity, typename itemType,
         template<typename> class hashTable>
auto benchmark_single_TPD_filter(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision,
                                 vector<vector<itemType> *> *elements, ostream &os = cout) -> ostream & {
    //    using temp_TPD =
    using Table = dict<temp_PD<slot_type, bits_per_item, max_capacity>, hashTable, itemType, uint64_t>;

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(FilterAPI<Table>::get_name(), 134);
    benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision, os);
    return os;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//template<typename itemType, size_t bits_per_element, size_t max_capacity, template<typename> class hashTable>
template<typename itemType, size_t bits_per_element, size_t max_capacity>
auto bench_all_PD(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                  ostream &os = cout) -> ostream & {
    assert(filter_max_capacity % bench_precision == 0);

    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements);

    using pd_32 = TPD_name::TPD<uint32_t, bits_per_element, max_capacity>;
    using pd_64 = TPD_name::TPD<uint64_t, bits_per_element, max_capacity>;

    using d_32 = dict<pd_32, hash_table, itemType, itemType>;
    using d_64 = dict<pd_64, hash_table, itemType, itemType>;

    benchmark_single_filter_wrapper<d_32, itemType>(filter_max_capacity, error_power_inv, bench_precision, &elements);
    benchmark_single_filter_wrapper<d_64, itemType>(filter_max_capacity, error_power_inv, bench_precision, &elements);
    benchmark_single_filter_wrapper<uint64_t, hash_table>(filter_max_capacity, error_power_inv, bench_precision,
                                                          &elements);

    return os;
}

auto example1();

template<size_t bits_per_item>
auto example2(ulong shift, ulong filters_indicator = -1, size_t bench_precision = 16) {
    size_t filter_max_capacity = 1u << shift;
    size_t lookup_reps = 1u << (shift + 2u);
    size_t error_power_inv = bits_per_item;
    b_all_wrapper<uint64_t, BITS_PER_ELEMENT_MACRO, BITS_PER_ELEMENT_MACRO>(filter_max_capacity, lookup_reps, error_power_inv, bench_precision,
                                                                            filters_indicator & 1, filters_indicator & 2, filters_indicator & 4,
                                                                            filters_indicator & 8, filters_indicator & 16);
}

/**High load benchmarking*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Table, typename itemType>
auto benchmark_single_filter_high_load(size_t filter_max_capacity, size_t bench_precision, size_t reps,
                                       ostream &os = cout) -> ostream & {

    assert(filter_max_capacity % bench_precision == 0);
    size_t step_size = filter_max_capacity / bench_precision;
    assert(step_size * bench_precision == filter_max_capacity);

    size_t vec_size = filter_max_capacity + step_size;
    vector<itemType> member_vec(vec_size);
    fill_vec(&member_vec, vec_size);

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    v_insertions(&filter, &member_vec, 0, filter_max_capacity);

    bool cond = true;
    for (int i = 0; i < reps; ++i) {
        size_t start = (i % (bench_precision + 1)) * step_size;
        cout << "delete (start, end): "
             << "( " << start << ", " << start + step_size << ")" << endl;
        cond = v_deleting(&filter, &member_vec, start, start + step_size);
        assert(cond);
        start = ((i + bench_precision) % (bench_precision + 1)) * step_size;
        cout << "add (start, end): "
             << "( " << start << ", " << start + step_size << ")" << endl;
        v_insertions(&filter, &member_vec, start, start + step_size);
    }
    return os;
}

template<typename itemType, template<typename> class hashTable>
auto benchmark_single_filter_high_load(size_t filter_max_capacity, size_t bits_per_element, size_t bench_precision,
                                       size_t reps, ostream &os = cout) -> ostream & {

    assert(filter_max_capacity % bench_precision == 0);
    size_t step_size = filter_max_capacity / bench_precision;
    assert(step_size * bench_precision == filter_max_capacity);

    size_t vec_size = filter_max_capacity + step_size;
    vector<itemType> member_vec(vec_size);
    fill_vec(&member_vec, vec_size);

    using Table = dict<PD, hashTable, itemType, uint64_t>;
    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity, bits_per_element);
    v_insertions(&filter, &member_vec, 0, filter_max_capacity);

    bool cond = true;
    for (int i = 0; i < reps; ++i) {
        cout << i << endl;
        size_t start = (i % (bench_precision + 1)) * step_size;
        cout << "delete (start, end): "
             << "( " << start << ", " << start + step_size << ")" << endl;
        //        if()
        cond = vec_deleting(&filter, &member_vec, start, start + step_size);
        assert(cond);
        start = ((i + bench_precision) % (bench_precision + 1)) * step_size;
        cout << "add (start, end): "
             << "( " << start << ", " << start + step_size << ")" << endl;
        v_insertions(&filter, &member_vec, start, start + step_size);

        //        cout << "Second level capacity :" << filter.get_second_level_capacity() << endl;
        //        cout << "max capacity to current capacity ratio :" << filter.get_second_level_load_ratio() << endl;
    }
    return os;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif//FILTERS_NEW_TESTS_HPP
