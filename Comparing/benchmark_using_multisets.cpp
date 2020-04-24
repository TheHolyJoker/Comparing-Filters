//
// Created by tomer on 4/24/20.
//

#include "benchmark_using_multisets.hpp"

namespace name_compare {
    template<class filter_type, typename block_type>
    auto b_filter_wrapper_multi(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                double level2_load_factor, size_t universe_size,
                                ostream &os) -> ostream & {
        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();

        auto filter = filter_type(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size,
                                  level1_load_factor,
                                  level2_load_factor);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

        b_filter_rates_core_multi<filter_type, block_type>(&filter, filter_max_capacity, lookup_reps, init_time,
                                                           error_power_inv,
                                                           universe_size, os);
        return os;
    }


    template<class filter_type, typename block_type>
    auto b_filter_rates_core_multi(filter_type *filter, size_t filter_max_capacity, size_t lookup_reps,
                                   ulong init_time, size_t error_power_inv, size_t universe_size,
                                   ostream &os) -> std::ostream & {
        auto start_run_time = std::chrono::high_resolution_clock::now();


        /**Sets initializing*/

        std::multiset<block_type> member_set, lookup_set, to_be_deleted_set;
        /**Member set init*/

        auto t0 = std::chrono::high_resolution_clock::now();
        multiset_init(filter_max_capacity / 2, &member_set, universe_size);
        auto t1 = std::chrono::high_resolution_clock::now();
        ulong member_set_init_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

        multiset_init(filter_max_capacity / 2, &to_be_deleted_set, universe_size);

        multiset_init(lookup_reps, &lookup_set, universe_size);
        double set_ratio = lookup_set.size() / (double) lookup_reps;

        auto insertion_time = do_single_op<filter_type, block_type>(filter, &member_set, insert);
        auto insertion_time_higher_load = do_single_op<filter_type, block_type>(filter, &to_be_deleted_set, insert);
        auto lookup_time = do_single_op<filter_type, block_type>(filter, &member_set, lookup);

        vector<multiset<block_type> *> member_sets_vector;
        member_sets_vector.insert(member_sets_vector.end(), &member_set);
        member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
        auto FP_count_higher_load = do_single_op<filter_type, block_type>(filter, &lookup_set, &member_sets_vector,
                                                                          count_FP);

        auto removal_time = do_single_op<filter_type, block_type>(filter, &to_be_deleted_set, ops::remove);
//    member_sets_vector.erase(member_sets_vector.begin() + 1);
        vector<multiset<block_type> *> vector_of_single_set;
        vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
        auto FP_count_mid_load = do_single_op<filter_type, block_type>(filter, &lookup_set, &vector_of_single_set,
                                                                       count_FP);

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

    template<class filter_type, typename block_type, class set_type>
    auto do_single_op(filter_type *filter, set_type *element_set, ops op) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        auto t1 = chrono::high_resolution_clock::now();
        switch (op) {
            case insert:
                t0 = chrono::high_resolution_clock::now();
                for (block_type iter : *element_set) filter->insert_int(iter);
                t1 = chrono::high_resolution_clock::now();
                return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            case lookup:
                t0 = chrono::high_resolution_clock::now();
                for (block_type iter : *element_set) filter->lookup_int(iter);
                t1 = chrono::high_resolution_clock::now();
                return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            case ops::remove:
                t0 = chrono::high_resolution_clock::now();
                for (block_type iter : *element_set) filter->remove_int(iter);
                t1 = chrono::high_resolution_clock::now();
                return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            default:
                assert(false);
        }
    }

    template<class filter_type, typename block_type, class set_type>
    auto do_single_op(filter_type *filter, set_type *lookup_set, vector<set_type *> *member_vec,
                      ops op) -> std::size_t {
        assert(op == count_FP);
        size_t fp_counter = 0;
        for (block_type iter : *lookup_set) {
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

}

namespace name_compare {

    template<>
    auto
    b_filter_wrapper_multi<basic_cf, uint32_t>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                               size_t l1_counter_size, size_t l2_counter_size,
                                               double level1_load_factor, double level2_load_factor,
                                               size_t universe_size, ostream &os) -> ostream & {
        auto start_run_time = chrono::high_resolution_clock::now();
        auto t0 = chrono::high_resolution_clock::now();

        auto filter = basic_cf(filter_max_capacity);

        auto t1 = chrono::high_resolution_clock::now();
        auto init_time = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

        b_filter_rates_core_multi<basic_cf, uint32_t>(&filter, filter_max_capacity, lookup_reps, init_time,
                                                      error_power_inv, universe_size, os);
        return os;
    }

    template<>
    auto do_single_op<basic_cf, uint32_t, multiset<uint32_t>>(basic_cf *filter, multiset<uint32_t> *element_set,
                                                              ops op) -> ulong {
        auto t0 = chrono::high_resolution_clock::now();
        auto t1 = chrono::high_resolution_clock::now();
        switch (op) {
            case insert:
                t0 = chrono::high_resolution_clock::now();
                for (uint32_t iter : *element_set) filter->Add(iter);
                t1 = chrono::high_resolution_clock::now();
                return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            case lookup:
                t0 = chrono::high_resolution_clock::now();
                for (uint32_t iter : *element_set) filter->Contain(iter);
                t1 = chrono::high_resolution_clock::now();
                return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            case ops::remove:
                t0 = chrono::high_resolution_clock::now();
                for (uint32_t iter : *element_set) filter->Delete(iter);
                t1 = chrono::high_resolution_clock::now();
                return chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
            default:
                assert(false);
        }
    }

    template<>
    auto do_single_op<basic_cf, uint32_t, multiset<uint32_t>>(basic_cf *filter, multiset<uint32_t> *lookup_set,
                                                              vector<multiset<uint32_t> *> *member_vec,
                                                              ops op) -> std::size_t {
        assert(op == count_FP);
        size_t fp_counter = 0;
        for (uint32_t iter : *lookup_set) {
            if (filter->Contain(iter) == cuckoofilter::Ok) {
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


}


template auto
name_compare::b_filter_wrapper_multi<multi_dict64, uint64_t>(size_t filter_max_capacity, size_t lookup_reps,
                                                             size_t error_power_inv, size_t l1_counter_size,
                                                             size_t l2_counter_size, double level1_load_factor,
                                                             double level2_load_factor, size_t universe_size,
                                                             ostream &os) -> ostream &;

template auto
name_compare::b_filter_rates_core_multi<multi_dict64, uint64_t>(multi_dict64 *filter, size_t filter_max_capacity,
                                                                size_t lookup_reps, ulong init_time,
                                                                size_t error_power_inv, size_t universe_size,
                                                                ostream &os) -> std::ostream &;


template
auto name_compare::do_single_op<multi_dict64, uint64_t, multiset<uint64_t>>(multi_dict64 *filter,
                                                                            multiset<uint64_t> *element_set,
                                                                            ops op) -> ulong;

template
auto name_compare::do_single_op<multi_dict64, uint64_t, multiset<uint64_t>>(multi_dict64 *filter,
                                                                            multiset<uint64_t> *lookup_set,
                                                                            vector<multiset<uint64_t> *> *member_vec,
                                                                            ops op) -> std::size_t;