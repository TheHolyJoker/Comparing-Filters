//
// Created by root on 31/05/2020.
//

#ifndef FILTERS_NEW_TESTS_HPP
#define FILTERS_NEW_TESTS_HPP

#include "tests.hpp"
// #include "wrappers.hpp"
#include <unistd.h>// For perf.
//#include <unordered_set>
//#include <chrono>


// #include <Validation/validate_hash_table.hpp>
typedef chrono::nanoseconds ns;


////////////// Declarations:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////Basic functions:
template<typename itemType>
auto init_elements(size_t max_filter_capacity, size_t lookup_reps, vector<vector<itemType> *> *elements, size_t bench_precision = 20, bool with_deletions = false);


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


template<class Table, typename itemType>
void profile_benchmark(Table *wrap_filter, vector<vector<itemType> *> *elements);

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

template<class Table, typename itemType>
auto benchmark_single_filter_wrapper(size_t filter_max_capacity, size_t bench_precision, vector<vector<itemType> *> *elements) -> std::stringstream {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();

    Table filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();


    std::stringstream ss = print_name(FilterAPI<Table>::get_name(&filter), 134);
    std::cout << ss.str();

#ifdef PROF
    profile_benchmark(&filter, elements);
#endif

    auto ss2 = benchmark_generic_filter<Table, itemType>(&filter, elements, bench_precision);
    ss << ss2.str();
    return ss;
}

template<class Table, typename itemType>
auto benchmark_generic_filter(Table *wrap_filter, vector<vector<itemType> *> *elements, size_t bench_precision) -> std::stringstream {

    std::stringstream ss = print_round_header();
    std::cout << ss.str();

#ifdef COUNT
    static volatile int res = FilterAPI<Table>::get_functionality(wrap_filter);
#endif// COUNT


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
    constexpr bool to_delete = true;

    size_t add_step = add_vec->size() / benchmark_precision;
    size_t find_step = find_vec->size() / benchmark_precision;
    size_t true_find_step = add_step;

    size_t removal_time = 0;
    if (delete_vec->size() and to_delete) {
        // size_t delete_step = delete_vec->size() / benchmark_precision;
        auto del_insertion_time = time_insertions(wrap_filter, delete_vec, 0, delete_vec->size());
        removal_time = time_deletions(wrap_filter, delete_vec, 0, delete_vec->size());
    }


    auto insertion_time = time_insertions(wrap_filter, add_vec, round_counter * add_step, (round_counter + 1) * add_step);
// size_t true_lookup_time = 0;


//Zeroing out stuff.
#ifdef COUNT
    FilterAPI<Table>::get_functionality(wrap_filter);
#endif// COUNT

    // ulong uniform_lookup_time = 0;
    auto uniform_lookup_time = time_lookups(wrap_filter, find_vec, round_counter * find_step, (round_counter + 1) * find_step);

#ifdef COUNT
    // if (FilterAPI<Table>::get_ID(wrap_filter) == d512_ver3) {
    std::cout << "\nUniform" << std::endl;
    FilterAPI<Table>::get_functionality(wrap_filter);
    // } else if (FilterAPI<Table>::get_ID(wrap_filter) == d256_ver4) {
    // std::cout << "\nUniform" << std::endl;
    // FilterAPI<Table>::get_functionality(wrap_filter);
    // }
#endif// COUNT

    // ulong true_lookup_time = 0;
    auto true_lookup_time = time_lookups(wrap_filter, add_vec, 0, true_find_step);
#ifdef COUNT
    // if (FilterAPI<Table>::get_ID(wrap_filter) == d512_ver3) {
    //     std::cout << "\nTrue" << std::endl;
    //     FilterAPI<Table>::get_functionality(wrap_filter);
    // } else if (FilterAPI<Table>::get_ID(wrap_filter) == d256_ver4) {
    std::cout << "\nTrue" << std::endl;
    FilterAPI<Table>::get_functionality(wrap_filter);
    // }
#endif// COUNT


    const size_t var_num = 6;
    //    string names[var_num] = {"Load", "insertion_time", "uniform_lookup_time", "true_lookup_time", "removal_time"};
    size_t values[var_num] = {round_counter + 1, benchmark_precision, insertion_time, uniform_lookup_time,
                              true_lookup_time, removal_time};

    size_t divisors[var_num - 2] = {add_step, find_step, true_find_step, delete_vec->size()};
    auto temp = print_single_round(var_num, values, divisors);
    *ss << temp.str();

#ifdef COUNT
    std::cout << std::string(88, '=') << std::endl;

#endif// COUNT
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


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
}

template<class Table, typename itemType>
auto fp_rates_single_filter_probabilistic(Table *wrap_filter, vector<vector<itemType> *> *elements) -> std::tuple<size_t, size_t> {
    size_t counter = 0;
    /**Insertion*/
    for (auto el : *elements->at(0)) {
        FilterAPI<Table>::Add(el, wrap_filter);
        counter++;
    }
    // size_t false_counter = 0;
    // size_t true_counter = 0;
    size_t counters[2] = {0, 0};
    for (auto el : *elements->at(1)) {
        counters[FilterAPI<Table>::Contain(el, wrap_filter)]++;
    }
    return std::make_tuple(counters[0], counters[1]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<typename itemType>
auto init_elements(size_t max_filter_capacity, size_t lookup_reps, vector<vector<itemType> *> *elements, size_t bench_precision, bool with_deletions) {
    fill_vec(elements->at(0), max_filter_capacity);
    fill_vec(elements->at(1), lookup_reps);
    if (with_deletions) {
        size_t del_size = 1.0 * max_filter_capacity / bench_precision;
        fill_vec(elements->at(2), del_size);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Table, typename itemType>
auto time_lookups(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong {
    static volatile bool dummy;
    bool x = 0;

    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start; i < end; ++i) {
        x |= FilterAPI<Table>::Contain(element_set->at(i), wrap_filter);
        asm volatile(
                "rdtscp\n\t"
                "lfence"
                :
                :
                : "rax", "rcx", "rdx", "memory");
    }
    auto t1 = chrono::high_resolution_clock::now();

    dummy = x;
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto time_insertions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start; i < end; ++i) {
        FilterAPI<Table>::Add(element_set->at(i), wrap_filter);
        asm volatile(
                "rdtscp\n\t"
                "lfence"
                :
                :
                : "rax", "rcx", "rdx", "memory");
    }
    // FilterAPI<Table>::AddAll(*element_set, start, end, wrap_filter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

template<class Table, typename itemType>
auto time_deletions(Table *wrap_filter, vector<itemType> *element_set, size_t start, size_t end) -> ulong {
    if (!(FilterAPI<Table>::get_functionality(wrap_filter) & 4)) {
        //FIXME: UNCOMMENT!
        std::cout << FilterAPI<Table>::get_name(wrap_filter) << " does not support deletions." << std::endl;
        return 0;
    }

    auto t0 = chrono::high_resolution_clock::now();
    for (int i = start; i < end; ++i) {
        FilterAPI<Table>::Remove(element_set->at(i), wrap_filter);
        asm volatile(
                "rdtscp\n\t"
                "lfence"
                :
                :
                : "rax", "rcx", "rdx", "memory");
    }
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**High load benchmarking*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Table, typename itemType>
auto benchmark_single_filter_high_load(size_t filter_max_capacity, size_t bench_precision, size_t reps,
                                       ostream &os = cout)
        -> ostream & {

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**Profile benchmarking*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Table, typename itemType>
void profile_benchmark(Table *wrap_filter, vector<vector<itemType> *> *elements) {
    auto add_vec = elements->at(0);
    auto find_vec = elements->at(1);
    auto delete_vec = elements->at(2);

    auto insertion_time = time_insertions(wrap_filter, add_vec, 0, add_vec->size());

    printf("insertions done\n");
    fflush(stdout);
    ulong uniform_lookup_time = 0;
    ulong true_lookup_time = 0;
    // size_t true_lookup_time = 0;
    char buf[1024];
    // sprintf(buf, "perf record -p %d &", getpid());
    // sprintf(buf, "perf stat -p %d -e cycles -e instructions -e cache-misses -e cache-references -e L1-dcache-load-misses -e L1-dcache-loads -e LLC-load-misses -e LLC-loads -e dTLB-load-misses -e dTLB-loads -e node-load-misses -e node-loads -e branches -e branch-misses &", getpid());
    sprintf(buf, "perf stat -p %d \
    -e cycles                   \
    -e instructions             \
    -e cache-misses             \
    -e cache-references         \
    -e L1-dcache-load-misses    \
    -e L1-dcache-loads          \
    -e LLC-load-misses          \
    -e LLC-loads                \
    -e dTLB-load-misses         \
    -e dTLB-loads               \
    -e node-load-misses         \
    -e node-loads               \
    -e alignment-faults         \
    -e branches                 \
    -e branch-misses            \
    -e branch-loads             \
    -e branch-loads-misses      \
    &",
            getpid());
    // sprintf(buf, "perf stat -p %d -e cycles -e instructions -e cache-misses -e cache-references -e L1-dcache-load-misses -e L1-dcache-loads -e LLC-load-misses -e LLC-loads -e dTLB-load-misses -e dTLB-loads -e node-load-misses -e node-loads -e branches -e branch-misses -e uops_executed.stall_cycles &", getpid());
    auto junk = system(buf);
    for (int i = 0; i < 16; i++) {
        // true_lookup_time = time_lookups(wrap_filter, add_vec, 0, add_step);
        uniform_lookup_time += time_lookups(wrap_filter, find_vec, 0, find_vec->size());
        // true_lookup_time += time_lookups(wrap_filter, add_vec, 0, true_find_step);
    }
    // printf("%zd\n", 500 * add_step);
    printf("%zd\n", 16 * find_vec->size());
    // printf("%zd\n", 500 * true_find_step);
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**Counting benchmarking*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**2021 Phase*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Table, typename itemType>
auto prob_compute_yes_queries(Table *wrap_filter, vector<itemType> *query_vec, size_t start, size_t end) {
    auto vec_size = query_vec->size();
    assert(end <= vec_size);
    size_t yes_query_counter = 0;
    for (size_t i = start; i < end; ++i) {
        bool temp_res = FilterAPI<Table>::Contain(query_vec->at(i), wrap_filter);
        if (temp_res)
            yes_query_counter++;
    }
    return yes_query_counter;
}

template<class Table, typename itemType>
auto prob_compute_yes_queries(Table *wrap_filter, vector<itemType> *query_vec) {
    size_t start = 0;
    size_t end = query_vec->size();
    return prob_compute_yes_queries(wrap_filter, query_vec, start, end);
}

template<class Table, typename itemType>
auto strong_random_prob_compute_yes_queries(Table *wrap_filter, size_t reps) {
    vector<itemType> vec;
    fill_vec_better_but_slower_randomness(&vec, reps);
    return prob_compute_yes_queries(wrap_filter, &vec);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif//FILTERS_NEW_TESTS_HPP
