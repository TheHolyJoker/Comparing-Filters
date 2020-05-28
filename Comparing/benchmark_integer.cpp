//
// Created by tomer on 4/21/20.
//

#include "benchmark_integer.hpp"


namespace name_compare {
    template<class D, typename T>
    auto
    b_filter_wrapper_int(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                         size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                         ostream &os) -> ostream & {
        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();

        auto filter = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
                        level2_load_factor);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

        b_filter_rates_core_int<D, T>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
        return os;
    }

    template<class D, typename T>
    auto
    b_filter_rates_core_int(D *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
                            size_t error_power_inv,
                            std::ostream &os) -> std::ostream & {
        auto start_run_time = std::chrono::high_resolution_clock::now();


        /**Sets initializing*/

        std::set<T> member_set, lookup_set, to_be_deleted_set;
        /**Member set init*/

        auto t0 = std::chrono::high_resolution_clock::now();
        set_init(filter_max_capacity / 2, &member_set);
        auto t1 = std::chrono::high_resolution_clock::now();
        ulong member_set_init_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

        set_init(filter_max_capacity / 2, &to_be_deleted_set);

        set_init(lookup_reps, &lookup_set);
        double set_ratio = lookup_set.size() / (double) lookup_reps;

        auto insertion_time = b_time_insertions_int(filter, &member_set);
        auto insertion_time_higher_load = b_time_insertions_int(filter, &to_be_deleted_set);
        auto lookup_time = b_time_lookups_int(filter, &member_set);

        vector<set<T> *> member_sets_vector;
        member_sets_vector.insert(member_sets_vector.end(), &member_set);
        member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
        auto FP_count_higher_load = count_false_positives_int(filter, &lookup_set, &member_sets_vector);

        auto removal_time = b_time_deletions_int(filter, &to_be_deleted_set);
//    member_sets_vector.erase(member_sets_vector.begin() + 1);
        vector<set<T> *> vector_of_single_set;
        vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
        auto FP_count_mid_load = count_false_positives_int(filter, &lookup_set, &vector_of_single_set);

        auto end_time = std::chrono::high_resolution_clock::now();
        ulong total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_run_time).count();
        total_time += init_time;


        if (set_ratio < 1) {
            cout << "set_ratio=" << set_ratio << endl;
        }

        size_t exp_FP_count = ceil(((double) lookup_reps / (1u << error_power_inv)));
        table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);
//        cout << "FP_count_higher_load=" << FP_count_higher_load << endl;
//        cout << "FP_count_mid_load=" << FP_count_mid_load << endl;


        const size_t var_num = 7;
        string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
                                 "lookup_time", "removal_time", "total_time"};
        size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load,
                                  lookup_time,
                                  removal_time, total_time};

        size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(),
                                    lookup_set.size(),
                                    to_be_deleted_set.size(), 1};
        table_print_rates(var_num, names, values, divisors);

        return os;
     }


    template<class D, typename T>
    auto b_time_lookups_int(D *filter, set<T> *element_set) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        for (T iter : *element_set) filter->lookup_int(iter);
        auto t1 = chrono::high_resolution_clock::now();
        return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    }

    template<class D, typename T>
    auto b_time_insertions_int(D *filter, set<T> *element_set) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        for (T iter : *element_set) filter->insert_int(iter);
        auto t1 = chrono::high_resolution_clock::now();
        return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    }

    template<class D, typename T>
    auto b_time_deletions_int(D *filter, set<T> *element_set) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        for (T iter : *element_set) filter->remove_int(iter);
        auto t1 = chrono::high_resolution_clock::now();
        return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    }

    template<class D, typename T>
    auto
    count_false_positives_int(D *filter, std::set<T> *lookup_set,
                              std::vector<std::set<T> *> *member_vec) -> std::size_t {
        size_t fp_counter = 0;
        for (T iter : *lookup_set) {
            if (filter->lookup_int(iter)) {
                bool iter_is_fp = true;
                for (auto temp_set : *member_vec) {
                    if (temp_set->find(iter) != temp_set->end()) {
                        iter_is_fp = false;
                        break;
                    }
                }
                if (iter_is_fp)
                    fp_counter++;
            }
        }
        return fp_counter;
    }

/*
    template<>
    auto b_time_deletions_int<GeneralBF>(GeneralBF *filter, set<string> *element_set) -> ulong {
        return 0;

    }
*/


    template<>
    auto b_time_deletions_int<Filter_QF, uint32_t>(Filter_QF *filter, set<uint32_t> *element_set) -> ulong {
        return 0;
    }


    template<>
    auto b_time_deletions_int<Filter_QF, uint64_t>(Filter_QF *filter, set<uint64_t> *element_set) -> ulong {
        return 0;
    }

/*
template<>
auto b_filter_wrapper<multi_dict64>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                    size_t l1_counter_size,
                                    size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                                    ostream &os) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    auto filter = multi_dict<CPD, multi_hash_table<uint64_t>, uint64_t>(filter_max_capacity, error_power_inv,
                                                                        l1_counter_size, l2_counter_size,
                                                                        level1_load_factor, level2_load_factor);

    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;

    b_filter_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, os);
    return os;
}
*/



    template<>
    auto
    b_filter_wrapper_int<Filter_QF, uint32_t>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                              size_t l1_counter_size,
                                              size_t l2_counter_size, double level1_load_factor,
                                              double level2_load_factor,
                                              ostream &os) -> ostream & {

        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();
        size_t q = ceil(log2(filter_max_capacity / level1_load_factor));
        auto Mb_size = 3;
        Filter_QF filter = Filter_QF(q, error_power_inv, Mb_size);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

        b_filter_rates_core_int<Filter_QF, uint32_t>(&filter, filter_max_capacity, lookup_reps, init_time,
                                                     error_power_inv, os);
        return os;
    }

    /*
    template<>
    auto b_filter_wrapper_int<GeneralBF>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                     size_t l1_counter_size,
                                     size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                                     ostream &os) -> ostream & {

        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();
        auto filter = GeneralBF(filter_max_capacity, error_power_inv, false);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();


        b_filter_rates_core_int(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
        return os;
    }*/

    template<>
    auto b_filter_wrapper_int<dict32, uint32_t>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                                size_t l1_counter_size,
                                                size_t l2_counter_size, double level1_load_factor,
                                                double level2_load_factor,
                                                ostream &os) -> ostream & {

        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();
//    size_t q = ceil(log2(filter_max_capacity));
//    auto Mb_size = 3;
        auto filter = dict32(filter_max_capacity, error_power_inv, level1_load_factor,
                             level2_load_factor);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

//        cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;

        b_filter_rates_core_int<dict32, uint32_t>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv,
                                                  os);
        return os;
    }


    template<>
    auto
    b_filter_wrapper_int <basic_cf,uint32_t>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                             size_t l1_counter_size,
                                             size_t l2_counter_size, double level1_load_factor,
                                             double level2_load_factor,
                                             ostream &os) -> ostream & {
        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();

        const size_t bits_per_el = error_power_inv;
        auto filter = basic_cf(filter_max_capacity);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

        b_filter_rates_core_int<basic_cf, uint32_t>(&filter, filter_max_capacity, lookup_reps, init_time,
                                                    error_power_inv, os);
        return os;
    }


    template<>
    auto b_time_lookups_int<basic_cf, uint32_t>(basic_cf *filter, set<uint32_t> *element_set) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        for (uint32_t iter : *element_set) filter->Contain(iter);
        auto t1 = chrono::high_resolution_clock::now();
        return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    }

    template<>
    auto b_time_insertions_int<basic_cf, uint32_t>(basic_cf *filter, set<uint32_t> *element_set) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        for (uint32_t iter : *element_set) filter->Add(iter);
        auto t1 = chrono::high_resolution_clock::now();
        return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    }

    template<>
    auto b_time_deletions_int<basic_cf, uint32_t>(basic_cf *filter, set<uint32_t> *element_set) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        for (uint32_t iter : *element_set) filter->Delete(iter);
        auto t1 = chrono::high_resolution_clock::now();
        return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

    }

    template<>
    auto
    count_false_positives_int<basic_cf, uint32_t>(basic_cf *filter, std::set<uint32_t> *lookup_set,
                                                  std::vector<std::set<uint32_t> *> *member_vec) -> std::size_t {
        size_t fp_counter = 0;
        for (uint32_t iter : *lookup_set) {
            if (filter->Contain(iter) != cuckoofilter::Ok) {
                bool iter_is_fp = true;
                for (auto temp_set : *member_vec) {
                    if (temp_set->find(iter) != temp_set->end()) {
                        iter_is_fp = false;
                        break;
                    }
                }
                if (iter_is_fp)
                    fp_counter++;
            }
        }
        return fp_counter;
    }


    /*

    template<class D, typename T>
    auto b_time_lookups_int(D *filter, set<T> *element_set) -> ulong;

    template<class D, typename T>
    auto b_time_insertions_int(D *filter, set<T> *element_set) -> ulong;

    template<class D, typename T>
    auto b_time_deletions_int(D *filter, set<T> *element_set) -> ulong;
*/


    template auto
    b_filter_wrapper_int<multi_dict64, uint64_t>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                                 size_t l1_counter_size, size_t l2_counter_size,
                                                 double level1_load_factor, double level2_load_factor,
                                                 ostream &os) -> ostream &;


/*
    template auto b_filter_wrapper<Filter_QF>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                              size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                              double level2_load_factor, ostream &os) -> ostream &;

    template auto b_filter_wrapper<GeneralBF>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                              size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                              double level2_load_factor, ostream &os) -> ostream &;


*/
    template auto
    b_filter_rates_core_int<multi_dict64, uint64_t>(multi_dict64 *filter, size_t filter_max_capacity,
                                                    size_t lookup_reps,
                                                    ulong init_time, size_t error_power_inv,
                                                    std::ostream &os) -> std::ostream &;

/*

    template auto
    b_filter_rates_core<GeneralBF>(GeneralBF *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time,
                                   size_t error_power_inv,
                                   std::ostream &os) -> std::ostream &;
*/


    template auto
    b_filter_rates_core_int<Filter_QF, uint32_t>(Filter_QF *filter, size_t filter_max_capacity, size_t lookup_reps,
                                                 ulong init_time,
                                                 size_t error_power_inv,
                                                 std::ostream &os) -> std::ostream &;

    template auto
    b_filter_rates_core_int<dict32, uint32_t>(dict32 *filter, size_t filter_max_capacity, size_t lookup_reps,
                                              ulong init_time,
                                              size_t error_power_inv,
                                              std::ostream &os) -> std::ostream &;

    template auto
    b_filter_rates_core_int<basic_cf, uint32_t>(basic_cf *filter, size_t filter_max_capacity, size_t lookup_reps,
                                                ulong init_time, size_t error_power_inv,
                                                std::ostream &os) -> std::ostream &;

//
}