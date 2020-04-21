//
// Created by tomer on 4/12/20.
//

#include "benchmark_counting_filter.hpp"

template<class D>
auto CF_rates_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
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
    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;

    CF_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, os);
    return os;
}

template<>
auto
CF_rates_wrapper<dict32>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                         size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                         ostream &os) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    auto filter = dict32(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;

    CF_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, os);
    return os;
}


template<class D>
auto
CF_rates_core(D *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time, ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();


    /**Sets initializing*/
    set<string> member_set, lookup_set, to_be_deleted_set;
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
    cout << "FP_count_higher_load=" << FP_count_higher_load << endl;
    cout << "FP_count_mid_load=" << FP_count_mid_load << endl;


    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);

    return os;
}


template<typename ItemType, size_t bits_per_item>
auto Cuckoo_rates_core(CuckooFilter<ItemType, bits_per_item> *filter, size_t filter_max_capacity, size_t lookup_reps,
                       ulong init_time, ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();


    /**Sets initializing*/
    set<string> member_set, lookup_set, to_be_deleted_set;
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

    vector<set<string> *> member_sets_vector;
    member_sets_vector.insert(member_sets_vector.end(), &member_set);
    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);

    auto removal_time = time_deletions(filter, &to_be_deleted_set);
    member_sets_vector.erase(member_sets_vector.begin() + 1);
    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);

    auto end_time = chrono::high_resolution_clock::now();
    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
    total_time += init_time;
//    vector<ulong> times(init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
//                        removal_time, total_time);


    if (set_ratio < 1) {
        cout << "set_ratio=" << set_ratio << endl;
    }
    cout << "FP_count_higher_load=" << FP_count_higher_load << endl;
    cout << "FP_count_mid_load=" << FP_count_mid_load << endl;


    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);

    return os;
}

template<class D>
auto time_lookups(D *filter, set<string> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set) filter->lookup(&iter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();

}

template<class D>
auto time_insertions(D *filter, set<string> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set) filter->insert(&iter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();

}

template<class D>
auto time_deletions(D *filter, set<string> *element_set) -> ulong {
    auto t0 = chrono::high_resolution_clock::now();
    for (auto iter : *element_set) filter->remove(&iter);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<ns>(t1 - t0).count();

}




template<class D, typename T>
auto
multiset_rates_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                       size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                       multiset_types type, ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();
    ulong init_time;
    multiset<T> ms;
    switch (type) {
        case std_multiset: {
            auto t0 = chrono::high_resolution_clock::now();
            multiset<T> s;
            auto t1 = chrono::high_resolution_clock::now();
            init_time = chrono::duration_cast<ns>(t1 - t0).count();
            break;
        }
        default: {
            cout << "multiset type is not handled." << endl;
            assert(false);
        }
    }
/*
    auto t0 = chrono::high_resolution_clock::now();

    auto ms = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
                    level2_load_factor);
    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
*/

    multiset_rates_core(&ms, filter_max_capacity, lookup_reps, init_time, os);
    return os;

}


template<class D>
auto
multiset_rates_core(D *ms, size_t filter_max_capacity, size_t lookup_reps, ulong init_time, ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();


    /**Sets initializing*/
    set<string> member_set, lookup_set, to_be_deleted_set;
    /**Member set init*/
    auto t0 = chrono::high_resolution_clock::now();
    set_init(filter_max_capacity / 2, &member_set);
    auto t1 = chrono::high_resolution_clock::now();
    ulong member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();

    set_init(filter_max_capacity / 2, &to_be_deleted_set);

    set_init(lookup_reps, &lookup_set);
    double set_ratio = lookup_set.size() / (double) lookup_reps;

    auto insertion_time = time_insertions(ms, &member_set);
    auto insertion_time_higher_load = time_insertions(ms, &to_be_deleted_set);
    auto lookup_time = time_lookups(ms, &member_set);

    vector<set<string> *> member_sets_vector;
    member_sets_vector.insert(member_sets_vector.end(), &member_set);
    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);

    auto removal_time = time_deletions(ms, &to_be_deleted_set);

    vector<set<string> *> vector_of_single_set;
    vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);

    auto end_time = chrono::high_resolution_clock::now();
    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
    total_time += init_time;


    if (set_ratio < 1) {
        cout << "set_ratio=" << set_ratio << endl;
    }


    const size_t var_num = 7;
    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                             "lookup_time", "removal_time", "total_time"};
    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
                              removal_time, total_time};

    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
                                to_be_deleted_set.size(), 1};
    name_compare::table_print_rates(var_num, names, values, divisors);

    return os;
}


template auto CF_rates_wrapper<multi_dict64>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                             size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                             double level2_load_factor, ostream &os) -> ostream &;

template auto CF_rates_wrapper<dict32>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                       size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                       double level2_load_factor, ostream &os) -> ostream &;