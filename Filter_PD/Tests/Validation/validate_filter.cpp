//
// Created by tomer on 4/6/20.
//

#include "validate_filter.hpp"

template<class T>
auto
validate_filter(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l, size_t lookup_reps) -> bool {
    auto d = T(number_of_pds, m, f, l);
    if (d.is_const_size()) {
        cout << "here" << endl;
        f = 32, m = 32, l = 8;
    }
    /*
    //    vector<string> member_set, non_member_set, to_be_deleted_set;
    //    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &member_set);
    //    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    //    vector_lexicographic_init(lookup_reps, &non_member_set);
    */

    auto TEMP_VAR_FOR_DEPENDENCY_CHECK = 1000u;
//    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);
    auto number_of_elements_in_the_filter = TEMP_VAR_FOR_DEPENDENCY_CHECK;

    set<string> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);

    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);

    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    size_t fp_counter = 0;
    for (auto iter : lookup_set) {
        bool c1, c2;
        c1 = member_set.find(iter) != member_set.end();
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2)
            assert(d.lookup(&iter));
//            continue;
        if (d.lookup(&iter)) {
            fp_counter++;
//            cout << "False Positive:" << endl;
        }
    }
    printf("False positive ratio is:%f, for l:%zu. expected ratio:%f.\n", fp_counter / (double) (lookup_set.size()), l,
           pow(.5, l));

    counter = 0;
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
        d.remove(&iter);
        counter++;
    };

    counter = 0;
    for (auto iter : member_set) {
        bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c)
            continue;
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    return true;

}
//
//template<class D>
//auto v_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, double level1_load_factor,
//              double level2_load_factor) -> bool {
//    auto filter = D(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
////    cout << "\n\nfilter's info:" << endl;
////    filter.get_info();
//    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
//    return validate_filter_core(&filter, filter_max_capacity, lookup_reps);
//
//
//}

template<class D>
auto validate_filter_core_mid(D *filter, size_t filter_max_capacity, size_t lookup_reps) -> bool {
    auto number_of_elements_in_the_filter = filter_max_capacity;
//    D* d = filter;
//    cout << "\n\nd's info:" << endl;
//    filter->get_info();

    set<string> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);

    auto it = member_set.begin();
    auto it_backup = member_set.begin();

    size_t counter = 0;
    for (; it != member_set.end(); ++it) {
        const string val = *it;
        auto temp = &(*it);
        bool before_insertion = filter->lookup(temp);
//        bool BPC = (val == "SRLSTIKAL[^A_");
        filter->insert(temp);
        if (!filter->lookup(temp)) {
            filter->lookup(temp);
            assert(false);
        }
        counter++;
    }
    /*for (auto iter : member_set) {
    filter->insert(&iter);
    if (!filter->lookup(&iter)) {
        filter->lookup(&iter);
        assert(false);
    }
    counter++;
}*/

    for (auto iter : to_be_deleted_set)filter->insert(&iter);

    counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!filter->lookup(&iter)) {
            cout << "False negative:" << endl;
            filter->lookup(&iter);
            assert(filter->lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!filter->lookup(&iter)) {
            cout << "False negative:" << endl;
            filter->lookup(&iter);
            assert(filter->lookup(&iter));
        }
    }
    size_t fp_counter = 0;
    for (auto iter : lookup_set) {
        bool c1, c2;
        /*Exact answer to: is "iter" in filter.*/
        c1 = member_set.find(iter) != member_set.end();
        /*Exact answer to: is "iter" in filter.*/
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2)assert(filter->lookup(&iter));
        /*            continue;*/
        if (filter->lookup(&iter)) {
            fp_counter++;
            /*            cout << "False Positive:" << endl;*/
        }
    } //    printf("False positive ratio is:%f, for l:%zu. expected ratio:%f.\n", fp_counter / (double) (lookup_set.size()), l,
    // /*           pow(.5, l));*/ counter = 0; for ( auto iter : to_be_deleted_set) { if (!filter-> lookup(&iter) )
    // { cout << "False negative:" << endl; filter-> lookup(&iter); assert(filter->lookup(&iter)); } filter-> remove(&iter); counter++; };
    // counter = 0; for ( auto iter : member_set) { bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end(); if (c) continue; counter++; if (!filter-> lookup(&iter) ) { cout << "False negative:" << endl; filter-> lookup(&iter); assert(filter->lookup(&iter)); } }

    return true;

}

template<class D>
auto v_TP_lookups(D *filter, set<string> *el_set) -> bool {
    size_t counter = 0;
    for (const string &el: *el_set) {
        if (!filter->lookup(&el)) {
            cout << "lookup failed." << endl;
            cout << "counter: " << counter << endl;
            cout << "element: " << el << endl;
            filter->lookup(&el);
            return false;
        }
        counter++;
    }
    return true;

}

template<class D>
auto v_FP_counter(D *filter, set<string> *lookup_set, vector<set<string> *> *member_vec) -> size_t {
    size_t fp_counter = 0;
    for (auto iter : *lookup_set) {
        if (filter->lookup(&iter)) {
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

template<class D>
auto validate_filter_core(D *filter, size_t filter_max_capacity, size_t lookup_reps) -> bool {
    auto number_of_elements_in_the_filter = filter_max_capacity;

    set<string> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);


    size_t counter = 0;

    /**Insertion*/
    for (auto iter : member_set) {
        filter->insert(&iter);
        counter++;
    }
    for (auto iter : to_be_deleted_set) filter->insert(&iter);


    /**Lookup*/
    counter = 0;
    for (auto iter : member_set) {
//        assert(filter->lookup(&bad_str));
        counter++;
        if (!filter->lookup(&iter)) {
            cout << "False negative:" << endl;
            filter->lookup(&iter);
            assert(filter->lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!filter->lookup(&iter)) {
            cout << "False negative:" << endl;
            filter->lookup(&iter);
            assert(filter->lookup(&iter));
        }
    }

    /**Count False positive*/
    size_t fp_counter = 0;
    for (auto iter : lookup_set) {
        bool c1, c2;
        //Exact answer to: is "iter" in filter.
        c1 = member_set.find(iter) != member_set.end();
        //Exact answer to: is "iter" in filter.
        c2 = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c1 || c2)
            assert(filter->lookup(&iter));
//            continue;
        if (filter->lookup(&iter)) {
            fp_counter++;
//            cout << "False Positive:" << endl;
        }
    }
    cout << "\nnumber of false-positive is: " << fp_counter << endl;

    //    printf("False positive ratio is:%f, for l:%zu. expected ratio:%f.\n", fp_counter / (double) (lookup_set.size()), l,
//           pow(.5, l));

/*//    string bad_str = "GLXDVIQ\\AIYBN";
//    string prev_str = "AJFCEPF\\XHQZOD^";
//    cout << "filter->lookup_multi(&bad_str) " << filter->lookup_multi(&bad_str) << endl;
//    cout << "filter->lookup_multi(&prev_str) " << filter->lookup_multi(&prev_str) << endl;
//    cout << endl;*/
    counter = 0;
//    size_t bad_iter = 30315;
    string bad_el = "Z]\\P]SZF";
    assert(filter->lookup(&bad_el));

    /**Deletions*/
    for (auto iter : to_be_deleted_set) {
        /*
    //        bool BPC = (counter == 5431);
    //        if ((counter < bad_iter) and (!filter->lookup(&bad_str))) {
    ////            cout << "counter is " << counter << endl;
    //
    //        }

    //        if (iter == bad_str) {
    //            cout << "removing bad_str. counter is " << counter << endl;
    //            cout << "filter->lookup_multi(&bad_str) " << filter->lookup_multi(&bad_str) << endl;
    //            cout << endl;
    //        }
    */
        if (!filter->lookup(&iter)) {
            cout << "False negative:" << endl;
            filter->lookup(&iter);
            assert(filter->lookup(&iter));
        }
        filter->remove(&iter);
        counter++;
        assert(filter->lookup(&bad_el));

    };

    counter = 0;
    assert(filter->lookup(&bad_el));
    for (auto iter : member_set) {
        bool c = to_be_deleted_set.find(iter) != to_be_deleted_set.end();
        if (c)
            continue;
        counter++;
        if (!filter->lookup(&iter)) {
            cout << "False negative:" << endl;
            filter->lookup(&iter);
            assert(filter->lookup(&iter));
        }
    }

    return true;

}


template bool validate_filter<pow2c_filter>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
                                            size_t lookup_reps);

template bool
validate_filter<pow2c_naive_filter>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
                                    size_t lookup_reps);

template bool
validate_filter<const_filter>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
                              size_t lookup_reps);

template bool
validate_filter<gen_2Power<cg_PD>>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
                                   size_t lookup_reps);

//
//template
//auto v_filter<dict<cg_PD, hash_table<uint32_t>>>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
//                                                 double level1_load_factor, double level2_load_factor) -> bool;


template
auto validate_filter_core_mid<basic_multi_dict>(basic_multi_dict *filter, size_t filter_max_capacity,
                                                size_t lookup_reps) -> bool;

template
auto validate_filter_core<basic_multi_dict>(basic_multi_dict *filter, size_t filter_max_capacity,
                                            size_t lookup_reps) -> bool;

template
auto validate_filter_core_mid<safe_multi_dict>(safe_multi_dict *filter, size_t filter_max_capacity,
                                               size_t lookup_reps) -> bool;

template
auto validate_filter_core<safe_multi_dict>(safe_multi_dict *filter, size_t filter_max_capacity,
                                           size_t lookup_reps) -> bool;


template
auto validate_filter_core_mid<multi_dict64>(multi_dict64 *filter, size_t filter_max_capacity,
                                            size_t lookup_reps) -> bool;

template
auto validate_filter_core<multi_dict64>(multi_dict64 *filter, size_t filter_max_capacity,
                                        size_t lookup_reps) -> bool;

template
auto validate_filter_core<safe_multi_dict64>(safe_multi_dict64 *filter, size_t filter_max_capacity,
                                             size_t lookup_reps) -> bool;


template
auto validate_filter_core_mid<safe_multi_dict64>(safe_multi_dict64 *filter, size_t filter_max_capacity,
                                                 size_t lookup_reps) -> bool;


template auto v_TP_lookups<dict32>(dict32 *filter, set<string> *el_set) -> bool;

template auto v_TP_lookups<multi_dict64>(multi_dict64 *filter, set<string> *el_set) -> bool;

template auto v_TP_lookups<safe_multi_dict64>(safe_multi_dict64 *filter, set<string> *el_set) -> bool;


template auto
v_FP_counter<multi_dict64>(multi_dict64 *filter, set<string> *lookup_set, vector<set<string> *> *member_vec) -> size_t;


template auto v_FP_counter<safe_multi_dict64>(safe_multi_dict64 *filter, set<string> *lookup_set,
                                              vector<set<string> *> *member_vec) -> size_t;


template auto
v_FP_counter<dict32>(dict32 *filter, set<string> *lookup_set, vector<set<string> *> *member_vec) -> size_t;





//
//template
//auto v_filter<multi_dict<CPD, multi_hash_table<uint32_t>>>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
//double level1_load_factor, double level2_load_factor) -> bool;

