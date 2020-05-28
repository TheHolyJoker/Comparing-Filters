////
//// Created by tomer on 25/05/2020.
////
//
//#ifndef MINIMAL_S_PD_FILTER_TESTS_HPP
//#define MINIMAL_S_PD_FILTER_TESTS_HPP
//
//#include <set>
//#include <chrono>
//#include "dict.hpp"
////typedef dict<PD, hash_table<uint32_t>> s_dict32;
//typedef chrono::nanoseconds ns;
//
//using namespace std;
//
///*Basic functions*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//static const size_t default_line_width = 116;
//
//auto rand_string(int minLength, int charsNum, int numOfDiffLength = 8) -> string;
//
//void set_init(size_t size, set<string> *mySet, int minLength = 8, int charsNum = 32);
//
//
//void print_name(std::string filter_name);
//
//void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors);
//
//void
//table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///*Validation functions*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//template<class D>
//auto w_validate_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
//                       double level1_load_factor, double level2_load_factor) -> bool {
//    auto filter = D(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
//    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
//    return validate_filter_core(&filter, filter_max_capacity, lookup_reps);
//
//}
//
//
//template<class T>
//auto
//validate_filter(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l, size_t lookup_reps) -> bool {
//    auto d = T(number_of_pds, m, f, l);
//    if (d.is_const_size()) {
//        cout << "here" << endl;
//        f = 32, m = 32, l = 8;
//    }
//
//    auto TEMP_VAR_FOR_DEPENDENCY_CHECK = 1000u;
//    auto number_of_elements_in_the_filter = TEMP_VAR_FOR_DEPENDENCY_CHECK;
//
//    std::set<string> member_set, lookup_set, to_be_deleted_set;
//    set_init(number_of_elements_in_the_filter / 2, &member_set);
//    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
//    set_init(lookup_reps, &lookup_set);
//
//    for (auto iter : member_set) d.insert(&iter);
//    for (auto iter : to_be_deleted_set) d.insert(&iter);
//
//    size_t counter = 0;
//    for (auto iter : member_set) {
//        counter++;
//        if (!d.lookup(&iter)) {
//            cout << "False negative:" << endl;
//            d.lookup(&iter);
//            assert(d.lookup(&iter));
//        }
//    }
//    for (auto iter : to_be_deleted_set) {
//        if (!d.lookup(&iter)) {
//            cout << "False negative:" << endl;
//            d.lookup(&iter);
//            assert(d.lookup(&iter));
//        }
//    }
//
//    size_t fp_counter = 0;
//    for (auto iter : lookup_set) {
//        bool c1, c2;
//        c1 = member_set.find(iter) != member_set.end();
//        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
//        if (c1 || c2)
//            assert(d.lookup(&iter));
////            continue;
//        if (d.lookup(&iter)) {
//            fp_counter++;
////            cout << "False Positive:" << endl;
//        }
//    }
//    printf("False positive ratio is:%f, for l:%zu. expected ratio:%f.\n", fp_counter / (double) (lookup_set.size()), l,
//           pow(.5, l));
//
//    counter = 0;
//    for (auto iter : to_be_deleted_set) {
//        if (!d.lookup(&iter)) {
//            cout << "False negative:" << endl;
//            d.lookup(&iter);
//            assert(d.lookup(&iter));
//        }
//        d.remove(&iter);
//        counter++;
//    };
//
//    counter = 0;
//    for (auto iter : member_set) {
//        bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
//        if (c)
//            continue;
//        counter++;
//        if (!d.lookup(&iter)) {
//            cout << "False negative:" << endl;
//            d.lookup(&iter);
//            assert(d.lookup(&iter));
//        }
//    }
//
//    return true;
//
//}
////
////template<class D>
////auto v_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, double level1_load_factor,
////              double level2_load_factor) -> bool {
////    auto filter = D(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
//////    cout << "\n\nfilter's info:" << endl;
//////    filter.get_info();
////    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
////    return validate_filter_core(&filter, filter_max_capacity, lookup_reps);
////
////
////}
//
//
//
//template<class D>
//auto validate_filter_core_mid(D *filter, size_t filter_max_capacity, size_t lookup_reps) -> bool {
//    auto number_of_elements_in_the_filter = filter_max_capacity;
////    D* d = filter;
////    cout << "\n\nd's info:" << endl;
////    filter->get_info();
//
//    set<string> member_set, lookup_set, to_be_deleted_set;
//    set_init(number_of_elements_in_the_filter / 2, &member_set);
//    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
//    set_init(lookup_reps, &lookup_set);
//
//    auto it = member_set.begin();
//    auto it_backup = member_set.begin();
//
////    assert(v_insertion_plus_imm_lookups(filter, &member_set));
////    assert(v_insertion_plus_imm_lookups(filter, &to_be_deleted_set));
//    size_t counter = 0;
//    for (; it != member_set.end(); ++it) {
//        const string val = *it;
//        auto temp = &(*it);
//        bool before_insertion = filter->lookup(temp);
////        bool BPC = (val == "SRLSTIKAL[^A_");
//        filter->insert(temp);
//        if (!filter->lookup(temp)) {
//            filter->lookup(temp);
//            assert(false);
//        }
//        counter++;
//    }
//    for (auto iter : member_set) {
//        filter->insert(&iter);
//        if (!filter->lookup(&iter)) {
//            filter->lookup(&iter);
//            assert(false);
//        }
//        counter++;
//    }
//
//    for (auto iter : to_be_deleted_set)filter->insert(&iter);
//
//    counter = 0;
//    for (auto iter : member_set) {
//        counter++;
//        if (!filter->lookup(&iter)) {
//            cout << "False negative:" << endl;
//            filter->lookup(&iter);
//            assert(filter->lookup(&iter));
//        }
//    }
//    for (auto iter : to_be_deleted_set) {
//        if (!filter->lookup(&iter)) {
//            cout << "False negative:" << endl;
//            filter->lookup(&iter);
//            assert(filter->lookup(&iter));
//        }
//    }
//    size_t fp_counter = 0;
//    for (auto iter : lookup_set) {
//        bool c1, c2;
//        /*Exact answer to: is "iter" in filter.*/
//        c1 = member_set.find(iter) != member_set.end();
//        /*Exact answer to: is "iter" in filter.*/
//        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
//        if (c1 || c2)assert(filter->lookup(&iter));
//        /*            continue;*/
//        if (filter->lookup(&iter)) {
//            fp_counter++;
//            /*            cout << "False Positive:" << endl;*/
//        }
//    } //    printf("False positive ratio is:%f, for l:%zu. expected ratio:%f.\n", fp_counter / (double) (lookup_set.size()), l,
//    // /*           pow(.5, l));*/ counter = 0; for ( auto iter : to_be_deleted_set) { if (!filter-> lookup(&iter) )
//    // { cout << "False negative:" << endl; filter-> lookup(&iter); assert(filter->lookup(&iter)); } filter-> remove(&iter); counter++; };
//    // counter = 0; for ( auto iter : member_set) { bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end(); if (c) continue; counter++; if (!filter-> lookup(&iter) ) { cout << "False negative:" << endl; filter-> lookup(&iter); assert(filter->lookup(&iter)); } }
//
//    return true;
//
//}
//
//
//template<class D>
//auto v_TP_lookups(D *filter, set<string> *el_set) -> bool {
//    size_t counter = 0;
//    for (const string &el: *el_set) {
//        if (!filter->lookup(&el)) {
//            cout << "lookup failed." << endl;
//            cout << "counter: " << counter << endl;
//            cout << "element: " << el << endl;
//            filter->lookup(&el);
//            return false;
//        }
//        counter++;
//    }
//    return true;
//
//}
//
//template<class D>
//auto v_insertion_plus_imm_lookups(D *filter, set<string> *el_set) -> bool {
//    size_t counter = 0;
//    for (const string &el: *el_set) {
//        bool already_in_filter = filter->lookup(&el);
//        filter->insert(&el);
//        if (!filter->lookup(&el)) {
//            cout << "lookup failed." << endl;
//            cout << "counter: " << counter << endl;
//            cout << "element: " << el << endl;
//
//            filter->insert(&el);
//            filter->lookup(&el);
//            return false;
//        }
//        counter++;
//    }
//    return true;
//}
//
//template<class D>
//auto v_FP_counter(D *filter, set<string> *lookup_set, vector<set<string> *> *member_vec) -> size_t {
//    size_t fp_counter = 0;
//    for (auto iter : *lookup_set) {
//        if (filter->lookup(&iter)) {
//            bool iter_is_fp = true;
//            for (auto temp_set : *member_vec) {
//                if (temp_set->find(iter) != temp_set->end()) {
//                    iter_is_fp = false;
//                    break;
//                }
//            }
//            if (iter_is_fp)
//                fp_counter++;
//        }
//    }
//    return fp_counter;
//}
//
//template<class D>
//auto validate_filter_core(D *filter, size_t filter_max_capacity, size_t lookup_reps) -> bool {
//    auto number_of_elements_in_the_filter = filter_max_capacity;
//
//    set<string> member_set, lookup_set, to_be_deleted_set;
//    set_init(number_of_elements_in_the_filter / 2, &member_set);
//    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
//    set_init(lookup_reps, &lookup_set);
//
//
//    size_t counter = 0;
//
//    /**Insertion*/
//    assert(v_insertion_plus_imm_lookups(filter, &member_set));
//    assert(v_insertion_plus_imm_lookups(filter, &to_be_deleted_set));
//
////    for (auto iter : member_set) {
////        filter->insert(&iter);
////        counter++;
////    }
////    for (auto iter : to_be_deleted_set) filter->insert(&iter);
//
//
//    /**Lookup*/
//    counter = 0;
//    for (auto iter : member_set) {
////        assert(filter->lookup(&bad_str));
//        counter++;
//        if (!filter->lookup(&iter)) {
//            cout << "False negative:" << endl;
//            filter->lookup(&iter);
//            assert(filter->lookup(&iter));
//        }
//    }
//    counter = 0;
//    for (auto iter : to_be_deleted_set) {
//        if (!filter->lookup(&iter)) {
//            cout << "False negative:" << endl;
//            cout << iter << endl;
//            filter->lookup(&iter);
//            assert(filter->lookup(&iter));
//        }
//        counter++;
//    }
//
//    /**Count False positive*/
//    size_t fp_counter = 0;
//    for (auto iter : lookup_set) {
//        bool c1, c2;
//        //Exact answer to: is "iter" in filter.
//        c1 = member_set.find(iter) != member_set.end();
//        //Exact answer to: is "iter" in filter.
//        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
//        if (c1 || c2)
//            assert(filter->lookup(&iter));
////            continue;
//        if (filter->lookup(&iter)) {
//            fp_counter++;
////            cout << "False Positive:" << endl;
//        }
//    }
//    cout << "\nnumber of false-positive is: " << fp_counter << endl;
//
//    //    printf("False positive ratio is:%f, for l:%zu. expected ratio:%f.\n", fp_counter / (double) (lookup_set.size()), l,
////           pow(.5, l));
//
///*//    string bad_str = "GLXDVIQ\\AIYBN";
////    string prev_str = "AJFCEPF\\XHQZOD^";
////    cout << "filter->lookup_multi(&bad_str) " << filter->lookup_multi(&bad_str) << endl;
////    cout << "filter->lookup_multi(&prev_str) " << filter->lookup_multi(&prev_str) << endl;
////    cout << endl;*/
//    counter = 0;
////    size_t bad_iter = 30315;
////    string bad_el = "Z]\\P]SZF";
////    assert(filter->lookup(&bad_el));
//
//    /**Deletions*/
//    for (auto iter : to_be_deleted_set) {
//        /*
//    //        bool BPC = (counter == 5431);
//    //        if ((counter < bad_iter) and (!filter->lookup(&bad_str))) {
//    ////            cout << "counter is " << counter << endl;
//    //
//    //        }
//
//    //        if (iter == bad_str) {
//    //            cout << "removing bad_str. counter is " << counter << endl;
//    //            cout << "filter->lookup_multi(&bad_str) " << filter->lookup_multi(&bad_str) << endl;
//    //            cout << endl;
//    //        }
//    */
//        if (!filter->lookup(&iter)) {
//            cout << "False negative:" << endl;
//            filter->lookup(&iter);
//            assert(filter->lookup(&iter));
//        }
//        filter->remove(&iter);
//        counter++;
////        assert(filter->lookup(&bad_el));
//
//    };
//
//    counter = 0;
////    assert(filter->lookup(&bad_el));
//    for (auto iter : member_set) {
//        bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
//        if (c)
//            continue;
//        counter++;
//        if (!filter->lookup(&iter)) {
//            cout << "False negative:" << endl;
//            filter->lookup(&iter);
//            assert(filter->lookup(&iter));
//        }
//    }
//
//    return true;
//
//}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///*Benchmark functions*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//template<class D>
//auto CF_rates_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
//                      size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
//                      ostream &os) -> ostream & {
//
//    auto start_run_time = chrono::high_resolution_clock::now();
//    auto t0 = chrono::high_resolution_clock::now();
//    /*if (std::is_same<D, dict32>::value) {
//        auto filter = dict32(filter_max_capacity, error_power_inv, level1_load_factor,
//                             level2_load_factor);
//    }*/
//
//    auto filter = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
//                    level2_load_factor);
//
//    auto t1 = chrono::high_resolution_clock::now();
//    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
////    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
//
//    print_name(filter.get_name());
//    CF_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
//    return os;
//}
//
//template<>
//auto CF_rates_wrapper<s_dict32>(size_t filter_max_capacity, size_t lookup_reps,
//                                size_t error_power_inv, size_t l1_counter_size,
//                                size_t l2_counter_size, double level1_load_factor,
//                                double level2_load_factor, ostream &os) -> ostream &;
//
///*
//template<>
//auto CF_rates_wrapper<multi_dict<cg_PD, multi_hash_table<uint64_t>,uint64_t>>(size_t filter_max_capacity, size_t lookup_reps,
//                                                         size_t error_power_inv, size_t l1_counter_size,
//                                                         size_t l2_counter_size, double level1_load_factor,
//                                                         double level2_load_factor, ostream &os) -> ostream &;
//*/
//
//
///*
//template<>
//auto
//CF_rates_wrapper<basic_cf>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
//                           size_t l1_counter_size,
//                           size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
//                           ostream &os) -> ostream & {
//
//    auto start_run_time = chrono::high_resolution_clock::now();
//    auto t0 = chrono::high_resolution_clock::now();
//    auto filter = basic_cf(filter_max_capacity);
//    auto t1 = chrono::high_resolution_clock::now();
//    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
//
//    name_compare::print_name(std::string("Cuckoo filter"));
//    //Todo: Cuckoo filter only work with integers.
////    Cuckoo_rates_core<basic_cf>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
////    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
//
////    name_compare::print_name(std::string("dict32"));
////    CF_rates_core(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
//    return os;
//}*/
//
//
//template<class D>
//auto
//CF_rates_core(D *filter, size_t filter_max_capacity, size_t lookup_reps, ulong init_time, size_t error_power_inv,
//              ostream &os) -> ostream & {
//    auto start_run_time = chrono::high_resolution_clock::now();
//
//
//    /**Sets initializing*/
//    set<string> member_set, lookup_set, to_be_deleted_set;
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
////    member_sets_vector.erase(member_sets_vector.begin() + 1);
//    vector<set<string> *> vector_of_single_set;
//    vector_of_single_set.insert(vector_of_single_set.begin(), &member_set);
//    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &vector_of_single_set);
//
//    auto end_time = chrono::high_resolution_clock::now();
//    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
//    total_time += init_time;
//
//
//    if (set_ratio < 1) {
//        cout << "set_ratio=" << set_ratio << endl;
//    }
//
//
//
//    /*
//    const size_t var_num = 7;
//    string names[var_num] = {"init_time", "member_set_init_time", "insertion_time", "insertion_time_higher_load",
//                             "lookup_time", "removal_time", "total_time"};
//    size_t values[var_num] = {init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
//                              removal_time, total_time};
//
//    size_t divisors[var_num] = {1, member_set.size(), member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
//                                to_be_deleted_set.size(), 1};
//    name_compare::table_print_rates(var_num, names, values, divisors);
//    */
//
//    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << error_power_inv)));
//    table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);
//
//    const size_t var_num = 6;
//    string names[var_num] = {"init_time", "insertion_time", "insertion_time_higher_load",
//                             "lookup_time", "removal_time", "total_time"};
//    size_t values[var_num] = {init_time, insertion_time, insertion_time_higher_load, lookup_time,
//                              removal_time, total_time};
//
//    size_t divisors[var_num] = {1, member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
//                                to_be_deleted_set.size(), 1};
//    table_print_rates(var_num, names, values, divisors);
//    return os;
//}
//
////
////template<typename ItemType, size_t bits_per_item>
////auto Cuckoo_rates_core(CuckooFilter<ItemType, bits_per_item> *filter, size_t filter_max_capacity, size_t lookup_reps,
////                       ulong init_time, ostream &os) -> ostream & {
////    auto start_run_time = chrono::high_resolution_clock::now();
////
////
////    /**Sets initializing*/
////    set<string> member_set, lookup_set, to_be_deleted_set;
////    /**Member set init*/
////    auto t0 = chrono::high_resolution_clock::now();
////    set_init(filter_max_capacity / 2, &member_set);
////    auto t1 = chrono::high_resolution_clock::now();
////    ulong member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
////
////    set_init(filter_max_capacity / 2, &to_be_deleted_set);
////
////    set_init(lookup_reps, &lookup_set);
////    double set_ratio = lookup_set.size() / (double) lookup_reps;
////
////    auto insertion_time = time_insertions(filter, &member_set);
////    auto insertion_time_higher_load = time_insertions(filter, &to_be_deleted_set);
////    auto lookup_time = time_lookups(filter, &member_set);
////
////    vector<set<string> *> member_sets_vector;
////    member_sets_vector.insert(member_sets_vector.end(), &member_set);
////    member_sets_vector.insert(member_sets_vector.end(), &to_be_deleted_set);
////    auto FP_count_higher_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);
////
////    auto removal_time = time_deletions(filter, &to_be_deleted_set);
////    member_sets_vector.erase(member_sets_vector.begin() + 1);
////    auto FP_count_mid_load = v_FP_counter(filter, &lookup_set, &member_sets_vector);
////
////    auto end_time = chrono::high_resolution_clock::now();
////    ulong total_time = chrono::duration_cast<ns>(end_time - start_run_time).count();
////    total_time += init_time;
//////    vector<ulong> times(init_time, member_set_init_time, insertion_time, insertion_time_higher_load, lookup_time,
//////                        removal_time, total_time);
////
////
////    if (set_ratio < 1) {
////        cout << "set_ratio=" << set_ratio << endl;
////    }
////    size_t exp_FP_count = ceil(((double) lookup_reps / (1u << bits_per_item)));
////    name_compare::table_print_false_positive_rates(exp_FP_count, FP_count_higher_load, FP_count_mid_load);
////
////    const size_t var_num = 6;
////    string names[var_num] = {"init_time", "insertion_time", "insertion_time_higher_load",
////                             "lookup_time", "removal_time", "total_time"};
////    size_t values[var_num] = {init_time, insertion_time, insertion_time_higher_load, lookup_time,
////                              removal_time, total_time};
////
////    size_t divisors[var_num] = {1, member_set.size(), to_be_deleted_set.size(), lookup_set.size(),
////                                to_be_deleted_set.size(), 1};
////    name_compare::table_print_rates(var_num, names, values, divisors);
////    return os;
////}
//
//template<class D>
//auto time_lookups(D *filter, set<string> *element_set) -> ulong {
//    auto t0 = chrono::high_resolution_clock::now();
//    for (auto iter : *element_set) filter->lookup(&iter);
//    auto t1 = chrono::high_resolution_clock::now();
//    return chrono::duration_cast<ns>(t1 - t0).count();
//
//}
//
//template<class D>
//auto time_insertions(D *filter, set<string> *element_set) -> ulong {
//    auto t0 = chrono::high_resolution_clock::now();
//    for (auto iter : *element_set) filter->insert(&iter);
//    auto t1 = chrono::high_resolution_clock::now();
//    return chrono::duration_cast<ns>(t1 - t0).count();
//
//}
//
//template<class D>
//auto time_deletions(D *filter, set<string> *element_set) -> ulong {
//    auto t0 = chrono::high_resolution_clock::now();
//    for (auto iter : *element_set) filter->remove(&iter);
//    auto t1 = chrono::high_resolution_clock::now();
//    return chrono::duration_cast<ns>(t1 - t0).count();
//
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#endif //MINIMAL_S_PD_FILTER_TESTS_HPP
