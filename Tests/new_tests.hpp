//
// Created by root on 31/05/2020.
//

#ifndef FILTERS_NEW_TESTS_HPP
#define FILTERS_NEW_TESTS_HPP

#include "tests.hpp"
#include "wrappers.hpp"
#include <chrono>
#include <set>

typedef chrono::nanoseconds ns;

template <typename itemType>
auto init_elements(size_t max_filter_capacity, size_t lookup_reps, vector<vector<itemType> *> *elements)
{
    fill_vec(elements->at(0), max_filter_capacity);
    fill_vec(elements->at(1), lookup_reps);
    //    fill_vec(&elements->at(2),0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class Table, typename itemType>
auto time_lookups(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong
{
    static volatile bool dummy;
    bool x = 0;

    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start; i < end; ++i)
        x |= FilterAPI<Table>::Contain(element_set->at(i), wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();

    dummy = x;
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template <class Table, typename itemType>
auto time_insertions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong
{
    auto t0 = chrono::high_resolution_clock::now();
    FilterAPI<Table>::AddAll(*element_set, start, end, wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template <class Table, typename itemType>
auto time_deletions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong
{
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start; i < end; ++i)
        FilterAPI<Table>::Remove(element_set->at(i), wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class Table, typename itemType>
auto benchmark_single_round(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t round_counter,
                            size_t benchmark_precision, ostream &os) -> ostream &
{

    auto add_vec = elements->at(0);
    auto find_vec = elements->at(1);
    auto delete_vec = elements->at(2);

    size_t add_step = add_vec->size() / benchmark_precision;
    size_t find_step = find_vec->size() / benchmark_precision;
    size_t delete_step = delete_vec->size() / benchmark_precision;
    auto insertion_time = time_insertions(wrap_filter, add_vec, round_counter * add_step,
                                          (round_counter + 1) * add_step);
    auto uniform_lookup_time = time_lookups(wrap_filter, find_vec, round_counter * find_step,
                                            (round_counter + 1) * find_step);
    auto true_lookup_time = time_lookups(wrap_filter, add_vec, 0, add_step);

    size_t removal_time = 0;
    if (delete_vec->size())
    {
        removal_time = time_deletions(wrap_filter, delete_vec, round_counter * delete_step,
                                      (round_counter + 1) * delete_step);
    }

    /*     if (FilterAPI<Table>::get_ID(wrap_filter) == d512) {
        FilterAPI<Table>::get_dynamic_info(wrap_filter);
    }
 */
    const size_t var_num = 6;
    //    string names[var_num] = {"Load", "insertion_time", "uniform_lookup_time", "true_lookup_time", "removal_time"};
    size_t values[var_num] = {round_counter + 1, benchmark_precision, insertion_time, uniform_lookup_time,
                              true_lookup_time, removal_time};

    size_t divisors[var_num - 2] = {add_step, find_step, delete_step};
    print_single_round(var_num, values, divisors);

    //    if (FilterAPI<Table>::get_ID(wrap_filter) == tpd_id)
    //        FilterAPI<Table>::get_dynamic_info(wrap_filter);
    return os;
}

template <class Table, typename itemType>
auto benchmark_generic_filter(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t bench_precision,
                              ostream &os) -> ostream &
{

    print_round_header();
    for (int round = 0; round < bench_precision; ++round)
    {
        benchmark_single_round<Table, itemType>(wrap_filter, elements, round, bench_precision, os);
        /* if (FilterAPI<Table>::get_ID(wrap_filter) == d512){

        } */
    }

    return os;
}

template <class Table, typename itemType>
auto benchmark_single_filter_wrapper(size_t filter_max_capacity,
                                     size_t bench_precision, vector<vector<itemType> *> *elements,
                                     ostream &os = cout) -> ostream &
{

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(FilterAPI<Table>::get_name(&filter), 134);
    benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision, os);
    return os;
}

template <typename itemType, template <typename> class hashTable>
auto benchmark_single_filter_wrapper(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision,
                                     vector<vector<itemType> *> *elements, ostream &os = cout) -> ostream &
{
    using Table = dict<PD, hashTable, itemType, uint64_t>;

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity, error_power_inv);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(FilterAPI<Table>::get_name(&filter), 134);
    benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision, os);
    return os;
}

//template<template <typename slot_type, size_t bits_per_item, size_t max_capacity> class temp_PD ,typename itemType, template<typename> class hashTable>
template <typename itemType, template <typename> class hashTable>
auto att_benchmark_single_filter_wrapper(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision,
                                         vector<vector<itemType> *> *elements, ostream &os = cout) -> ostream &
{
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

template <typename itemType, size_t bits_per_element>
auto b_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                   bool BF = true, bool CF = true, bool MT = true, bool SIMD = true, bool call_PD = true,
                   ostream &os = cout) -> ostream &
{

    assert(filter_max_capacity % bench_precision == 0);
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements);

    if (BF)
    {
        using Table = bloomfilter::bloom<itemType, bits_per_element, false, hashing::TwoIndependentMultiplyShift>;
        benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision,
                                                         &elements, os);
    }
    if (CF)
    {
        using Table = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>;
        assert((w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5)));
        benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision,
                                                         &elements);
    }
    if (SIMD)
    {
        using Table = SimdBlockFilter<>;
        benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision,
                                                         &elements);
    }
    if (MT)
    {
        using Table = MortonFilter;
        benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision,
                                                         &elements);
    }
    //    using temp_PD2 = TPD_name::TPD<uint64_t, 8, 64>;
    //    using Table2 = dict<temp_PD2 , hash_table, itemType, uint64_t>;
    //    benchmark_single_filter_wrapper<Table2, itemType>(filter_max_capacity, bench_precision, &elements);

    //    using temp_PD = TPD_name::TPD<uint32_t, 8, 64>;
    //    using Table = dict<temp_PD, hash_table, itemType, uint64_t>;
    //    benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
    //
    //    using temp_PD2 = TPD_name::TPD<uint32_t, 8, 32>;
    //    using Table2 = dict<temp_PD2, hash_table, itemType, uint64_t>;
    //    benchmark_single_filter_wrapper<Table2, itemType>(filter_max_capacity, bench_precision, &elements);
    //    att_benchmark_single_filter_wrapper<uint64_t, hash_table>(filter_max_capacity, error_power_inv, bench_precision,
    //                                                              &elements);

    if (call_PD)
    {
        //        using Table = dict<PD, hash_table, itemType, uint32_t>;
        benchmark_single_filter_wrapper<uint64_t, hash_table>(filter_max_capacity, error_power_inv, bench_precision,
                                                              &elements);
    }

    return os;
}

template <typename itemType, size_t bits_per_element>
auto att_all_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                     uint indicator = -1, ostream &os = cout) -> ostream &
{
    assert(filter_max_capacity % bench_precision == 0);
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    init_elements(filter_max_capacity, lookup_reps, &elements);

    // uint b = 1u;
    // if (indicator & b) {
    //     using spare_item = uint64_t;
    //     using temp_hash = att_hTable<spare_item, 4>;
    //     using Table = dict512<temp_hash, spare_item, itemType>;
    //     w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5);
    //     benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
    // }

    using spare_item = uint64_t;
    using temp_hash = att_hTable<spare_item, 4>;
    using Table = att_d512<temp_hash, spare_item, itemType>;
    assert((w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5)));
    benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);

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
        //    w_validate_filter<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5);
           benchmark_single_filter_wrapper<Table, itemType>(filter_max_capacity, bench_precision, &elements);
       }
       b <<= 1u;
    
       b <<= 1u;
    */

    /*if (indicator & b) {
        w_validate_filter<itemType, hash_table>(filter_max_capacity, lookup_reps, bits_per_element, 1, .5);
        benchmark_single_filter_wrapper<uint64_t, hash_table>(filter_max_capacity, error_power_inv, bench_precision,
            &elements);
    }*/
    return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//template<typename itemType, template<typename> class hashTable>
//template<template <typename slot_type, size_t bits_per_item, size_t max_capacity> class temp_PD ,typename itemType, template<typename> class hashTable>
template <template <typename, size_t, size_t> class temp_PD, typename slot_type, size_t bits_per_item, size_t max_capacity, typename itemType,
          template <typename> class hashTable>
auto benchmark_single_TPD_filter(size_t filter_max_capacity, size_t error_power_inv, size_t bench_precision,
                                 vector<vector<itemType> *> *elements, ostream &os = cout) -> ostream &
{
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
template <typename itemType, size_t bits_per_element, size_t max_capacity>
auto bench_all_PD(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                  ostream &os = cout) -> ostream &
{
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

template <size_t bits_per_item>
auto example2(ulong shift, ulong filters_indicator = -1, size_t bench_precision = 16)
{
    size_t filter_max_capacity = 1u << shift;
    size_t lookup_reps = 1u << (shift + 2u);
    size_t error_power_inv = bits_per_item;
    b_all_wrapper<uint64_t, BITS_PER_ELEMENT_MACRO>(filter_max_capacity, lookup_reps, error_power_inv, bench_precision,
                                                    filters_indicator & 1, filters_indicator & 2, filters_indicator & 4,
                                                    filters_indicator & 8, filters_indicator & 16);
}

/**High load benchmarking*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Table, typename itemType>
auto benchmark_single_filter_high_load(size_t filter_max_capacity, size_t bench_precision, size_t reps,
                                       ostream &os = cout) -> ostream &
{

    assert(filter_max_capacity % bench_precision == 0);
    size_t step_size = filter_max_capacity / bench_precision;
    assert(step_size * bench_precision == filter_max_capacity);

    size_t vec_size = filter_max_capacity + step_size;
    vector<itemType> member_vec(vec_size);
    fill_vec(&member_vec, vec_size);

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    v_insertions(&filter, &member_vec, 0, filter_max_capacity);

    bool cond = true;
    for (int i = 0; i < reps; ++i)
    {
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

template <typename itemType, template <typename> class hashTable>
auto benchmark_single_filter_high_load(size_t filter_max_capacity, size_t bits_per_element, size_t bench_precision,
                                       size_t reps, ostream &os = cout) -> ostream &
{

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
    for (int i = 0; i < reps; ++i)
    {
        cout << i << endl;
        size_t start = (i % (bench_precision + 1)) * step_size;
        cout << "delete (start, end): "
             << "( " << start << ", " << start + step_size << ")" << endl;
        //        if()
        cond = v_deleting(&filter, &member_vec, start, start + step_size);
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

#endif //FILTERS_NEW_TESTS_HPP
