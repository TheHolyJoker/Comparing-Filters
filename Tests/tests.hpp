//
// Created by tomer on 25/05/2020.
//

#ifndef FILTERS_TESTS_HPP
#define FILTERS_TESTS_HPP

#include "printutil.hpp"
#include "wrappers.hpp"
#include <chrono>
#include <set>

#define BITS_PER_ELEMENT_MACRO (8)

typedef chrono::nanoseconds ns;

/*Basic functions*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint64_t x = 123456789, y = 362436069, z = 521288629;

/**
 * https://stackoverflow.com/a/1227137/5381404
 * @return
 */
unsigned long xorshf96();

template<typename itemType>
auto rand_item() -> itemType {
    return (itemType) random();
    //    return (itemType) xorshf96();
}

auto rand_item() -> string;

template<typename itemType>
void set_init(size_t size, unordered_set<itemType> *mySet) {
    for (int i = 0; i < size; ++i)
        mySet->insert(rand_item<uint64_t>());
}

template<typename itemType>
auto fill_vec(std::vector<itemType> *vec, size_t number_of_elements, ulong universe_mask = UNIVERSE_SIZE) -> void {
    vec->resize(number_of_elements);
    for (int i = 0; i < number_of_elements; ++i)
        vec->at(i) = rand_item<uint64_t>();
    /*unordered_set<itemType> temp_set(vec->begin(), vec->end());
    if (temp_set.size() < 0.95 * vec->size()) {
        std::cout << "unique size is: " << temp_set.size() << "( " << temp_set.size() / ((double) vec->size()) << ")"
                  << std::endl;
    }*/
}

/*
auto fill_vec(std::vector<uint64_t> *vec, size_t number_of_elements, ulong universe_mask) -> void {
    for (int i = 0; i < number_of_elements; ++i) {
        vec->push_back(xorshf96() & universe_mask);
    }
}*/

/*Validation functions*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename itemType, size_t bits_per_item, bool brancless, typename HashFamily, template<typename, std::size_t, bool, typename> class Filter>
auto v_insertion_plus_imm_lookups(Filter<itemType, bits_per_item, brancless, HashFamily> *filter,
                                  unordered_set<itemType> *el_set) -> bool {
    std::cout << "h1" << std::endl;
    size_t counter = 0;
    for (auto el : *el_set) {
        bool already_in_filter = filter->Contain(el); //For debugging
        filter->Add(el);
        if (!filter->Contain(el)) {
            cout << "lookup failed." << endl;
            cout << "counter: " << counter << endl;
            cout << "element: " << el << endl;

            filter->Add(el);
            filter->Contain(el);
            return false;
        }
        counter++;
    }
    return true;
}

template<class Table, typename itemType, bool block_insertion = false>
auto v_insertion_plus_imm_lookups(Table *wrap_filter, unordered_set<itemType> *el_set) -> bool {
    // std::cout << "h2" << std::endl;
    size_t counter = 0;
    if (block_insertion) {
        vector<itemType> vec(el_set->begin(), el_set->end());
        FilterAPI<Table>::AddAll(vec, wrap_filter);
        for (auto el : vec) {
            if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
                cout << "lookup failed." << endl;
                cout << "counter: " << counter << endl;
                cout << "element: " << el << endl;

                //            wrap_filter->Add(el);
                //            wrap_filter->Contain(el);
                return false;
            }
        }
        return true;
    }

    for (auto el : *el_set) {
        FilterAPI<Table>::Add(el, wrap_filter);
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            cout << "lookup failed." << endl;
            cout << "counter: " << counter << endl;
            cout << "element: " << el << endl;

            //            wrap_filter->Add(el);
            //            wrap_filter->Contain(el);
            return false;
        }
        counter++;
    }
    return true;
}

template<class Table, typename itemType>
auto v_true_positive_elements(Table *wrap_filter, unordered_set<itemType> *el_set) -> bool {
    size_t counter = 0;
    for (auto el : *el_set) {
        counter++;
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            cout << "False negative:" << endl;

            cout << "lookup failed." << endl;
            cout << "counter: " << counter << endl;
            cout << "element: " << el << endl;

            assert(FilterAPI<Table>::Contain(el, wrap_filter));
        }
    }
    return true;
}

template<class Table, typename itemType>
auto v_deleting(Table *wrap_filter, unordered_set<itemType> *to_be_deleted_set,
                unordered_set<itemType> *to_keep_elements_set) -> bool {
    size_t counter = 0;
    for (auto el : *to_be_deleted_set) {
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            return false;
        }
        bool c = to_be_deleted_set->find(el) != to_be_deleted_set->end();
        if (c)
            continue;
        counter++;
        try {
            FilterAPI<Table>::Remove(el, wrap_filter);
        }
        catch (std::runtime_error &msg) {
            break;
        }
    }
    return true;
}

template<class Table, typename itemType>
auto v_deleting(Table *wrap_filter, vector<itemType> *to_be_deleted_vec, size_t start, size_t end) -> bool {
    try {
        FilterAPI<Table>::Remove(to_be_deleted_vec->at(start), wrap_filter);
    }
    catch (std::runtime_error &msg) {
        std::cout << FilterAPI<Table>::get_name(wrap_filter) << "Does not support deletions" << std::endl;
        return true;
    }
    for (int i = start + 1; i < end; ++i) {
        auto el = to_be_deleted_vec->at(i);
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            auto db_res = FilterAPI<Table>::Contain(el, wrap_filter);
            return false;
        }
        FilterAPI<Table>::Remove(el, wrap_filter);
    }
    return true;
}

template<class Table, typename itemType>
auto v_insertions(Table *wrap_filter, vector<itemType> *to_add_vec, size_t start, size_t end) -> void {
    for (int i = start; i < end; ++i)
        FilterAPI<Table>::Add(to_add_vec->at(i), wrap_filter);
}

template<class Table, typename itemType, bool block_insertion = false>
auto v_filter_core(Table *wrap_filter, size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                   double level1_load_factor, double level2_load_factor) -> bool {
    //    Table wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    //    auto number_of_elements_in_the_filter = filter_max_capacity;

    unordered_set<itemType> member_set, lookup_set, to_be_deleted_set;
    set_init(filter_max_capacity / 2, &member_set);
    set_init(filter_max_capacity / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);

    size_t counter = 0;
    /**Insertion*/
    bool cond = v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(wrap_filter, &member_set);
    assert(cond);
    cond = v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(wrap_filter, &to_be_deleted_set);
    assert(cond);

    /**Lookup*/
    /*TODO SUPER IMPORTANT. UN COMMENT THE NEXT LINE*/
//    cond = v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    std::cout << "should uncomment here, line 237" << std::endl;
    assert(cond);
    cond = v_true_positive_elements<Table, itemType>(wrap_filter, &to_be_deleted_set);
    assert(cond);

    /**Count False positive*/
    size_t fp_counter = 0;
    size_t tp_counter = 0;
    for (auto iter : lookup_set) {
        /*For debugging:
         bool iter_not_in_filter = !FilterAPI<Table>::Contain(iter, &wrap_filter);
        if (iter_not_in_filter){
            cout << "here" << endl;
        }*/
        bool c1, c2;
        //Exact answer to: is "iter" in filter.
        c1 = member_set.find(iter) != member_set.end();
        //Exact answer to: is "iter" in filter.
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2) {
            //Validating there is no false negative.
            tp_counter++;
            assert(FilterAPI<Table>::Contain(iter, wrap_filter));
            continue;
        } else if (FilterAPI<Table>::Contain(iter, wrap_filter)) {
            fp_counter++;
        }
    }

    att_print_single_round_false_positive_rates(lookup_set.size(), error_power_inv, fp_counter, tp_counter);

    print_single_round_false_positive_rates(filter_max_capacity, lookup_set.size() >> error_power_inv, tp_counter,
                                            fp_counter);
    //    cout << "filter_max_capacity: " << filter_max_capacity << endl;
    //    cout << "\nnumber of false-positive is out of total number of lookups: " << fp_counter << "/ " << lookup_reps << endl;
    //    cout << "Expected FP count: " << (lookup_set.size() >> error_power_inv) << endl;

    counter = 0;
    cond = v_deleting<Table, itemType>(wrap_filter, &to_be_deleted_set, &member_set);
    assert(cond);
    /**Deletions*/

    /**Verifying no unwanted element was deleted (prone to error as deleting from filter is not well defined)*/
    cond = v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    assert(cond);
    return true;
}

template<class Table, typename itemType, bool block_insertion = false>
auto w_validate_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                       double level1_load_factor, double level2_load_factor) -> bool {

    Table wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    size_t line_width = 160; // number of columns (6) * column's width (24)
    print_name(FilterAPI<Table>::get_name(&wrap_filter));
    bool res = v_filter_core<Table, itemType, block_insertion>(&wrap_filter, filter_max_capacity, lookup_reps,
                                                               error_power_inv, level1_load_factor, level2_load_factor);
    assert(res);
    return res;
}

template<typename itemType, template<typename> class hashTable>
auto w_validate_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                       double level1_load_factor, double level2_load_factor) -> bool {
    //    using HT = hashTable<hashTableType>;
    //    using Table = dict<PD, HT, itemType>;
    using Table = dict<PD, hashTable, itemType, uint32_t>;

    Table wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity, error_power_inv);
    print_name(FilterAPI<Table>::get_name(&wrap_filter));
    bool res = v_filter_core<Table, itemType, false>(&wrap_filter, filter_max_capacity, lookup_reps,
                                                     error_power_inv, level1_load_factor, level2_load_factor);
    assert(res);
    return res;
}

void validate_example1();

void validate_example2(ulong shift, ulong filter_indicator = -1);

/*Old function for sanity checks*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Table>
auto v_true_positive(size_t number_of_elements) -> bool {
    std::vector<uint64_t> vec;
    fill_vec(&vec, number_of_elements);
    Table filter = FilterAPI<Table>::ConstructFromAddCount(number_of_elements);

    for (auto item : vec) {
        FilterAPI<Table>::Add(item, &filter);
    }
    size_t counter = 0;
    for (auto item : vec) {
        if (!FilterAPI<Table>::Contain(item, &filter)) {
            std::cout << counter << std::endl;
            return false;
        }
    }
    return true;
}

template<typename itemType, size_t bits_per_item, bool brancless, typename HashFamily, template<typename, std::size_t, bool, typename> class Filter>
auto v_true_positive_att(size_t number_of_elements) -> bool {
    //    std::vector<itemType> vec;
    //    fill_vec(&vec, number_of_elements);
    std::set<itemType> member_set;
    set_init(number_of_elements, &member_set);
    using Table = Filter<itemType, bits_per_item, brancless, HashFamily>;
    auto filter = FilterAPI<Table>::ConstructFromAddCount(number_of_elements);

    for (auto item : member_set) {
        FilterAPI<Table>::Add(item, &filter);
    }

    size_t counter = 0;
    for (auto item : member_set) {
        if (!FilterAPI<Table>::Contain(item, &filter)) {
            std::cout << counter << std::endl;
            return false;
        }
    }
    return true;

    /*
    for (auto item : vec) {
        FilterAPI<Table>::Add(item, &filter);
    }

    size_t counter = 0;
    for (auto item : vec) {
        if (!FilterAPI<Table>::Contain(item, &filter)) {
            std::cout << counter << std::endl;
            return false;
        }
    }
    return true;
*/
}

template<typename Table>
auto v_return_false_when_empty() -> bool {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(1u << 10u);
    return filter.Contain(42);
}

/*Benchmark functions*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Table, typename itemType>
auto time_lookups(Table *wrap_filter, unordered_set<itemType> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set)
        FilterAPI<Table>::Contain(iter, wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto time_insertions(Table *wrap_filter, unordered_set<itemType> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set)
        FilterAPI<Table>::Add(iter, wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto time_deletions(Table *wrap_filter, unordered_set<itemType> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set)
        FilterAPI<Table>::Remove(iter, wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto
benchmark_single_round(Table *wrap_filter, unordered_set<itemType> *to_add_set, unordered_set<itemType> *to_lookup_set,
                       unordered_set<itemType> *to_delete_set, size_t round_counter, size_t total_rounds_num,
                       ostream &os) -> ostream & {

    auto insertion_time = time_insertions(wrap_filter, to_add_set);
    auto lookup_time = time_lookups(wrap_filter, to_lookup_set);
    std::size_t removal_time = 0;
    if (to_delete_set->size()) {
        removal_time = time_deletions(wrap_filter, to_delete_set);
    }
    const size_t var_num = 4;
    string names[var_num] = {"Load", "insertion_time", "lookup_time", "removal_time"};
    size_t values[var_num + 1] = {round_counter, total_rounds_num, insertion_time, lookup_time, removal_time};

    size_t divisors[var_num - 1] = {to_add_set->size(), to_lookup_set->size(), to_delete_set->size()};
    print_single_round(var_num, values, divisors);
    return os;
    /*
    vector<set<string> *> member_sets_vector;
    member_sets_vector.insert(member_sets_vector.end(), &member_set);
    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);

//    member_sets_vector.erase(member_sets_vector.begin() + 1);
    vector<set<string> *> vector_of_single_set;
    vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &vector_of_single_set);
*/
    /*

    auto end_time = chrono::high_resolution_clock::now();
    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
    total_time += init_time;


    if (set_ratio < 1) {
        cout << "set_ratio=" << set_ratio << endl;
    }
*/
    /*
    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);
    */
    //    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << error_power_inv)));
    //    table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);
}

template<class Table, typename itemType>
auto benchmark_core(Table *wrap_filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
                    size_t error_power_inv, size_t bench_precision, ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();

    /**Sets initializing*/
    vector<unordered_set<itemType>> member_vec(bench_precision);
    vector<unordered_set<itemType>> lookup_vec(bench_precision);
    vector<unordered_set<itemType>> to_be_deleted(bench_precision);

    /**Member set init*/
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < bench_precision; ++i) {
        set_init(filter_max_capacity, &member_vec[i]);
        //        set_init(filter_max_capacity / (bench_precision << 1u), &to_be_deleted[i]);
        set_init(lookup_reps / bench_precision, &lookup_vec[i]);
        set_init(lookup_reps / bench_precision, &lookup_vec[i]);
    }
    auto t1 = chrono::high_resolution_clock::now();
    ulong member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();

    unordered_set<itemType> empty_set;
    print_round_header();
    for (int round = 0; round < bench_precision; ++round) {
        //        benchmark_single_round<Table, itemType>(wrap_filter, member_vec[round], lookup_vec[round], to_be_deleted[round])
        benchmark_single_round<Table, itemType>(wrap_filter, &member_vec[round], &lookup_vec[round], &empty_set,
                                                round + 1,
                                                bench_precision, os);
    }
    //    print_seperating_line();
    return os;

    /*

    auto insertion_time = time_insertions(wrap_filter, member_set);
    auto insertion_time_higher_load = time_insertions(filter, &to_be_deleted_set);
    auto lookup_time = time_lookups(filter, &member_set);

    vector<set<string> *> member_sets_vector;
    member_sets_vector.insert(member_sets_vector.end(), &member_set);
    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);

    auto removal_time = time_deletions(filter, &to_be_deleted_set);
//    member_sets_vector.erase(member_sets_vector.begin() + 1);
    vector<set<string> *> vector_of_single_set;
    vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &vector_of_single_set);

    auto end_time = chrono::high_resolution_clock::now();
    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
    total_time += init_time;


    if (set_ratio < 1) {
        cout << "set_ratio=" << set_ratio << endl;
    }


*/
    /*
    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);
    */
    /*
    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << error_power_inv)));
    table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);

    const size_t var_num = 6;
    string names[var_num] = {"init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    table_print_rates(var_num, names, values, divisors);
    return os;*/
}

template<class Table, typename itemType>
auto benchmark_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                       ostream &os = cout) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(FilterAPI<Table>::get_name(&filter));
    benchmark_core<Table, itemType>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv,
                                    bench_precision, os);
    return os;
}

template<typename itemType, template<typename> class hashTable>
auto benchmark_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t bench_precision,
                       ostream &os = cout) -> ostream & {
    using Table = dict<PD, hashTable, itemType, uint32_t>;

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity, error_power_inv);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(FilterAPI<Table>::get_name(&filter));
    benchmark_core<Table, itemType>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv,
                                    bench_precision, os);
    return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class D>
auto
CF_rates_wrapper_old(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                     size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                     ostream &os) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    /*if (std::is_same<D, dict32>::value) {
        auto filter = dict32(filter_max_capacity, error_power_inv, level1_load_factor,
                             level2_load_factor);
    }*/

    auto filter = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
                    level2_load_factor);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
    //    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;

    print_name(filter.get_name(&filter));
    CF_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
    return os;
}

/*

template<class Table>
auto
benchmark_core(FilterAPI<Table> *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
               size_t error_power_inv,
               ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();


    */
/**Sets initializing*/ /*

    unordered_set<uint64_t> member_set, lookup_set, to_be_deleted_set;
    */
/**Member set init*/   /*

    auto t0 = chrono::high_resolution_clock::now();
    set_init(filter_max_capacity / 2, &member_set);
    auto t1 = chrono::high_resolution_clock::now();
    ulong member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();

    set_init(filter_max_capacity / 2, &to_be_deleted_set);

    set_init(lookup_reps, &lookup_set);
    double set_ratio = lookup_set.size() / (double) lookup_reps;

    auto insertion_time = time_insertions(filter, &member_set);
    auto insertion_time_higher_load = time_insertions(filter, &to_be_deleted_set);
    auto lookup_time = time_lookups(filter, &member_set);

    vector<set<string> *> member_sets_vector;
    member_sets_vector.insert(member_sets_vector.end(), &member_set);
    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);

    auto removal_time = time_deletions(filter, &to_be_deleted_set);
//    member_sets_vector.erase(member_sets_vector.begin() + 1);
    vector<set<string> *> vector_of_single_set;
    vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &vector_of_single_set);

    auto end_time = chrono::high_resolution_clock::now();
    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
    total_time += init_time;


    if (set_ratio < 1) {
        cout << "set_ratio=" << set_ratio << endl;
    }



    */
/*
    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);
    */
/*


    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << error_power_inv)));
    table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);

    const size_t var_num = 6;
    string names[var_num] = {"init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    table_print_rates(var_num, names, values, divisors);
    return os;
}
*/

auto benchmark_wrapper(size_t filter_max_capacity, size_t lookup_reps,
                       size_t error_power_inv, size_t l1_counter_size,
                       size_t l2_counter_size, double level1_load_factor,
                       double level2_load_factor, ostream &os) -> ostream &;

template<class D>
auto CF_rates_core_old(D *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
                       size_t error_power_inv,
                       ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();

    /**Sets initializing*/
    unordered_set<string> member_set, lookup_set, to_be_deleted_set;
    /**Member set init*/
    auto t0 = chrono::high_resolution_clock::now();
    set_init(filter_max_capacity / 2, &member_set);
    auto t1 = chrono::high_resolution_clock::now();
    ulong member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();

    set_init(filter_max_capacity / 2, &to_be_deleted_set);

    set_init(lookup_reps, &lookup_set);
    double set_ratio = lookup_set.size() / (double) lookup_reps;

    auto insertion_time = time_insertions(filter, &member_set);
    auto insertion_time_higher_load = time_insertions(filter, &to_be_deleted_set);
    auto lookup_time = time_lookups(filter, &member_set);

    vector<unordered_set<string> *> member_sets_vector;
    member_sets_vector.insert(member_sets_vector.end(), &member_set);
    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);

    auto removal_time = time_deletions(filter, &to_be_deleted_set);
    //    member_sets_vector.erase(member_sets_vector.begin() + 1);
    vector<unordered_set<string> *> vector_of_single_set;
    vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &vector_of_single_set);

    auto end_time = chrono::high_resolution_clock::now();
    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
    total_time += init_time;

    if (set_ratio < 1) {
        cout << "set_ratio=" << set_ratio << endl;
    }

    /*
    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);
    */

    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << error_power_inv)));
    table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);

    const size_t var_num = 6;
    string names[var_num] = {"init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    table_print_rates(var_num, names, values, divisors);
    return os;
}

template<class D>
auto time_lookups(D *filter, unordered_set<string> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set)
        filter->lookup(&iter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class D>
auto time_insertions(D *filter, unordered_set<string> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set)
        filter->insert(&iter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class D>
auto time_deletions(D *filter, unordered_set<string> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set)
        filter->remove(&iter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //FILTERS_TESTS_HPP

/*
template<>
auto
CF_rates_wrapper<basic_cf>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                           size_t l1_counter_size,
                           size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                           ostream &os) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    auto filter = basic_cf(filter_max_capacity);
    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    name_compare::print_name(std::string("Cuckoo filter"));
    //Todo: Cuckoo filter only work with integers.
//    Cuckoo_rates_core<basic_cf>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
//    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;

//    name_compare::print_name(std::string("dict32"));
//    CF_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
    return os;
}*/

//
//template<typename itemType, size_t bits_per_item>
//auto Cuckoo_rates_core(CuckooFilter<itemType, bits_per_item> *filter, size_t filter_max_capacity, size_t lookup_reps,
//                       ulong init_time, ostream &os) -> ostream & {
//    auto start_run_time = chrono::high_resolution_clock::now();
//
//
//    /**Sets initializing*/
//    unordered_set<string> member_set, lookup_set, to_be_deleted_set;
//    /**Member set init*/
//    auto t0 = chrono::high_resolution_clock::now();
//    set_init(filter_max_capacity / 2, &member_set);
//    auto t1 = chrono::high_resolution_clock::now();
//    ulong member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//
//    set_init(filter_max_capacity / 2, &to_be_deleted_set);
//
//    set_init(lookup_reps, &lookup_set);
//    double set_ratio = lookup_set.size() / (double) lookup_reps;
//
//    auto insertion_time = time_insertions(filter, &member_set);
//    auto insertion_time_higher_load = time_insertions(filter, &to_be_deleted_set);
//    auto lookup_time = time_lookups(filter, &member_set);
//
//    vector<set<string> *> member_sets_vector;
//    member_sets_vector.insert(member_sets_vector.end(), &member_set);
//    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
//    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);
//
//    auto removal_time = time_deletions(filter, &to_be_deleted_set);
//    member_sets_vector.erase(member_sets_vector.begin() + 1);
//    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);
//
//    auto end_time = chrono::high_resolution_clock::now();
//    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
//    total_time += init_time;
////    vector<ulong> times(init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
////                        removal_time, total_time);
//
//
//    if (set_ratio < 1) {
//        cout << "set_ratio=" << set_ratio << endl;
//    }
//    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << bits_per_item)));
//    name_compare::table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);
//
//    const size_t var_num = 6;
//    string names[var_num] = {"init_time", "insertion_time", "insertion_time_higher_load",
//                             "lookup_time", "removal_time", "total_time"};
//    size_t values[var_num] = {init_time, insertion_time, insertion_time_higher_load, lookup_time,
//                              removal_time, total_time};
//
//    size_t divisors[var_num] = {1, member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
//                                to_be_deleted_set.size(), 1};
//    name_compare::table_print_rates(var_num, names, values, divisors);
//    return os;
//}

/*
template<class Table>
auto
validate_filter_core(Table *wrap_filter, size_t filter_max_capacity,
                     size_t lookup_reps) -> bool {
    auto number_of_elements_in_the_filter = filter_max_capacity;

    unordered_set<itemType> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);


    size_t counter = 0;

    */
/**Insertion*/            /*

//    bool cond = v_insertion_plus_imm_lookups<itemType, bits_per_item, brancless, HashFamily, Filter<itemType, bits_per_item, brancless, HashFamily>>(filter, &member_set);
    bool cond = v_insertion_plus_imm_lookups<itemType, bits_per_item, brancless, HashFamily, Filter>(filter,
                                                                                                     &member_set);
    assert(cond);
//    cond = v_insertion_plus_imm_lookups<itemType, bits_per_item, brancless, HashFamily, Filter<itemType, bits_per_item, brancless, HashFamily>>(
//            filter, &to_be_deleted_set);
//    assert(cond);


    */
/**Lookup*/               /*

    counter = 0;
    for (auto iter : member_set) {
//        assert(filter->lookup(&bad_str));
        counter++;
        if (!filter->Contain(iter)) {
            cout << "False negative:" << endl;
//            filter->lookup(&iter);
            assert(filter->Contain(iter));
        }
    }
    counter = 0;
    for (auto iter : to_be_deleted_set) {
        if (!filter->Contain(iter)) {
            cout << "False negative:" << endl;
            cout << iter << endl;
            assert(filter->Contain(iter));
        }
        counter++;
    }

    */
/**Count False positive*/ /*

    size_t fp_counter = 0;
    for (auto iter : lookup_set) {
        bool c1, c2;
        //Exact answer to: is "iter" in filter.
        c1 = member_set.find(iter) != member_set.end();
        //Exact answer to: is "iter" in filter.
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2)
            assert(filter->Contain(iter));
//            continue;
        if (filter->Contain(iter)) {
            fp_counter++;
//            cout << "False Positive:" << endl;
        }
    }
    cout << "\nnumber of false-positive is: " << fp_counter << endl;


    counter = 0;

    */
/**Deletions*/            /*
*/
/*
    for (auto iter : to_be_deleted_set) {
        if (!filter->Contain(iter)) {
            cout << "False negative:" << endl;
            assert(filter->Contain(iter));
        }
        filter->Remove(iter);
        counter++;

    };

    counter = 0;
    for (auto iter : member_set) {
        bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c)
            continue;
        counter++;
        if (!filter->Contain(iter)) {
            cout << "False negative:" << endl;
            assert(filter->Contain(iter));
        }
    }
*/
/*

    return true;

}

*/

/*

template<typename itemType, size_t bits_per_item, bool brancless, typename HashFamily, template<typename, std::size_t, bool, typename> class Filter>
auto w_validate_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                       double level1_load_factor, double level2_load_factor) -> bool {
    using Table = Filter<itemType, bits_per_item, brancless, HashFamily>;
    auto wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    return w_validate_filter_core<Table, itemType>(&wrap_filter, filter_max_capacity, lookup_reps, error_power_inv,
                                                   level1_load_factor, level2_load_factor);
}


template<class Table, typename itemType>
auto w_validate_filter_core(Table *wrap_filter, size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                            double level1_load_factor, double level2_load_factor) -> bool {
//    using Table = Filter<itemType, bits_per_item, brancless, HashFamily>;
//    auto wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
//    return validate_filter_core(&wrap_filter, filter_max_capacity, lookup_reps);

    auto number_of_elements_in_the_filter = filter_max_capacity;

    unordered_set<itemType> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);


    size_t counter = 0;
    */
/**Insertion*/            /*

    bool cond = v_insertion_plus_imm_lookups<Table, itemType>(wrap_filter, &member_set);
    assert(cond);
    cond = v_insertion_plus_imm_lookups<Table, itemType>(wrap_filter, &to_be_deleted_set);
    assert(cond);


    */
/**Lookup*/               /*

    cond = v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    assert(cond);
    cond = v_true_positive_elements<Table, itemType>(wrap_filter, &to_be_deleted_set);
    assert(cond);

    */
/**Count False positive*/ /*

    size_t fp_counter = 0;
    for (auto iter : lookup_set) {
        */
/*For debugging:
         bool iter_not_in_filter = !FilterAPI<Table>::Contain(iter, &wrap_filter);
        if (iter_not_in_filter){
            cout << "here" << endl;
        }*/
/*

        bool c1, c2;
        //Exact answer to: is "iter" in filter.
        c1 = member_set.find(iter) != member_set.end();
        //Exact answer to: is "iter" in filter.
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2) {
            assert(FilterAPI<Table>::Contain(iter, wrap_filter));
            continue;
        } else if (FilterAPI<Table>::Contain(iter, wrap_filter)) {
            fp_counter++;
        }
    }

    cout << "\nnumber of false-positive is: " << fp_counter << endl;
    cout << "filter_max_capacity: " << filter_max_capacity << endl;
    cout << "Expected FP count: " << (lookup_set.size() >> error_power_inv) << endl;


    counter = 0;
    cond = v_deleting<Table, itemType>(wrap_filter, &to_be_deleted_set, &member_set);
    assert(cond);
    */
/**Deletions*/                                                                                              /*


    */
/**Verifying no unwanted element was deleted (prone to error as deleting from filter is not well defined)*/ /*

    cond = v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    assert(cond);
    return true;

}


template<typename itemType, size_t bits_per_item, bool brancless, typename HashFamily, template<typename, std::size_t, bool, typename> class Filter>
auto w_validate_filter_att(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                           double level1_load_factor, double level2_load_factor) -> bool {
    using Table = Filter<itemType, bits_per_item, brancless, HashFamily>;
    auto wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    return w_validate_filter_core<Table, itemType>(&wrap_filter, filter_max_capacity, lookup_reps, error_power_inv,
                                                   level1_load_factor, level2_load_factor);
}

*/

/*
template<class Table, typename itemType, bool block_insertion = false>
auto w_validate_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                       double level1_load_factor, double level2_load_factor) -> bool {
    Table wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    return v_filter_core<Table, itemType, block_insertion>(&wrap_filter, filter_max_capacity, lookup_reps,
                                                           error_power_inv, level1_load_factor, level2_load_factor);



    unordered_set<itemType> member_set, lookup_set, to_be_deleted_set;
    set_init(filter_max_capacity / 2, &member_set);
    set_init(filter_max_capacity / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);


    size_t counter = 0;
    */
/**Insertion*/            /*

    bool cond = v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(&wrap_filter, &member_set);
    assert(cond);
    cond = v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(&wrap_filter, &to_be_deleted_set);
    assert(cond);


    */
/**Lookup*/               /*

    cond = v_true_positive_elements<Table, itemType>(&wrap_filter, &member_set);
    assert(cond);
    cond = v_true_positive_elements<Table, itemType>(&wrap_filter, &to_be_deleted_set);
    assert(cond);

    */
/**Count False positive*/ /*

    size_t fp_counter = 0;
    for (auto iter : lookup_set) {
        */
/*For debugging:
         bool iter_not_in_filter = !FilterAPI<Table>::Contain(iter, &wrap_filter);
        if (iter_not_in_filter){
            cout << "here" << endl;
        }*/
/*

        bool c1, c2;
        //Exact answer to: is "iter" in filter.
        c1 = member_set.find(iter) != member_set.end();
        //Exact answer to: is "iter" in filter.
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2) {
            assert(FilterAPI<Table>::Contain(iter, &wrap_filter));
            continue;
        } else if (FilterAPI<Table>::Contain(iter, &wrap_filter)) {
            fp_counter++;
        }
    }

    cout << "\nnumber of false-positive is: " << fp_counter << endl;
    cout << "filter_max_capacity: " << filter_max_capacity << endl;
    cout << "Expected FP count: " << (lookup_set.size() >> error_power_inv) << endl;


    counter = 0;
    cond = v_deleting<Table, itemType>(&wrap_filter, &to_be_deleted_set, &member_set);
    assert(cond);
    */
/**Deletions*/                                                                                              /*


    */
/**Verifying no unwanted element was deleted (prone to error as deleting from filter is not well defined)*/ /*

    cond = v_true_positive_elements<Table, itemType>(&wrap_filter, &member_set);
    assert(cond);
    return true;

}
*/
