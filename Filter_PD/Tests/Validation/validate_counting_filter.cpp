//
// Created by tomer on 4/11/20.
//

#include "validate_counting_filter.hpp"

template<class D>
auto
v_CF_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
             size_t l2_counter_size, double level1_load_factor, double level2_load_factor) -> bool {
    auto filter = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
                    level2_load_factor);
    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
    return v_filter_no_deletions(&filter, filter_max_capacity, lookup_reps, MAX_PART);
}

template<class D>
auto v_CF_wrapper(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                  size_t l2_counter_size, double level1_load_factor, double level2_load_factor,
                  size_t seq_reps) -> bool {
    auto filter = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
                    level2_load_factor);
    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
    auto res = v_filter_no_deletions(&filter, filter_max_capacity, lookup_reps, MAX_PART);
    if (!res) return false;
    for (int i = 0; i < seq_reps; ++i) {
        auto percentage = 0.05;
        assert(percentage + max(level1_load_factor, level2_load_factor) <= 1);
        res = v_CF_seq_op(&filter, ceil(filter_max_capacity * percentage), lookup_reps >> 2u);
        if (!res) {
            cout << "Failed in " << i << endl;
            return false;
        }
    }
    return true;
}

template<class D>
auto v_counting_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv, size_t l1_counter_size,
                       size_t l2_counter_size, double level1_load_factor, double level2_load_factor) -> bool {
    auto filter = D(filter_max_capacity, error_power_inv, l1_counter_size, l2_counter_size, level1_load_factor,
                    level2_load_factor);
    cout << "\nexpected #fp is: " << ((double) lookup_reps / (1u << error_power_inv)) << endl;
    auto res = validate_filter_core_mid(&filter, filter_max_capacity, lookup_reps);
    if (!res) {
        std::cout << "mid failed" << endl;
        return false;
    }
//
    return validate_filter_core<D>(&filter, filter_max_capacity, lookup_reps);
    return true;
}

template<class D>
auto v_filter_no_deletions(D *filter, size_t filter_max_capacity, size_t lookup_reps, size_t part) -> bool {

    set<string> member_set, lookup_set, to_be_deleted_set;
    set_init(filter_max_capacity / 2, &member_set);
    set_init(filter_max_capacity / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);


    size_t counter = 0;

    /**Insertion*/
    for (auto iter : member_set) {
        assert(v_CF_insert_single(filter, &iter));
        counter++;
    }
    cout << "part 0 passed" << endl;
    if (part == 0)
        return true;

    for (auto iter : to_be_deleted_set) {
        assert(v_CF_insert_single(filter, &iter));
        counter++;
    }
    cout << "part 1 passed" << endl;
    if (part == 1)
        return true;



    /**Lookup*/
    assert(v_TP_lookups(filter, &member_set));
    assert(v_TP_lookups(filter, &to_be_deleted_set));
    cout << "part 2 passed" << endl;
    if (part == 2)
        return true;



    /**Count False positive*/
    vector<set<string> *> member_set_vector;
    member_set_vector.insert(member_set_vector.end(), &member_set);
    member_set_vector.insert(member_set_vector.end(), &to_be_deleted_set);

    size_t fp_count = v_FP_counter(filter, &lookup_set, &member_set_vector);
    cout << "Number of FP " << fp_count << endl;
    cout << "part 3 passed" << endl;
    if (part == 3)
        return true;

    /*
 *
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
    cout << "\nnumber of false-positive is: " << fp_counter << endl;*/

    size_t del_counter = 0;
    for (auto iter : to_be_deleted_set) {
        assert(v_single_deletion(filter, &iter));
        del_counter++;
    }
    cout << "part 4 passed" << endl;
    if (part == 4)
        return true;


    /**Validate non removed members are still in the filter (no False Negative)*/
    assert(v_TP_lookups(filter, &member_set));
    cout << "part 5 passed" << endl;
    if (part == 5)
        return true;

    if (part == MAX_PART)
        return true;
    assert(false);

}

template<class D>
auto v_CF_insert_single(D *filter, const string *s) -> bool {
/*
//    if (!filter->lookup(s)){
//        cout << "no need to call \"v_CF_insert_single\" function." << endl;
//        return true;
//    }
*/

    size_t counter_before_insertion = filter->lookup_multi(s);
    filter->insert(s);
    size_t counter_after_insertion = filter->lookup_multi(s);
    if (counter_before_insertion + 1 != counter_after_insertion) {
        cout << "failed" << endl;
        cout << "counter_before_insertion " << counter_before_insertion << endl;
        cout << "counter_after_insertion " << counter_after_insertion << endl;
        return false;
    }
    return true;

}

template<class D>
auto v_single_deletion(D *filter, const string *s) -> bool {
    if (!filter->lookup(s)) {
        cout << "no need to call \"v_single_deletion\" function." << endl;
        return false;
    }

    size_t counter_before = filter->lookup_multi(s);
    assert(counter_before > 0);
    filter->remove(s);
    size_t counter_after = filter->lookup_multi(s);
    if (counter_before != counter_after + 1) {
        cout << "failed" << endl;
        cout << "counter_before " << counter_before << endl;
        cout << "counter_after " << counter_after << endl;
        return false;
    }
    return true;

}


template<class D>
auto v_CF_seq_op(D *filter, size_t new_elements_to_add, size_t lookup_reps) -> bool {
    auto number_of_elements_in_the_filter = new_elements_to_add;

    set<string> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);


    size_t counter = 0;

    /**Insertion*/
    for (auto iter : member_set) {
        assert(v_CF_insert_single(filter, &iter));
        counter++;
    }

    for (auto iter : to_be_deleted_set) {
        assert(v_CF_insert_single(filter, &iter));
        counter++;
    }



    /**Lookup*/
    assert(v_TP_lookups(filter, &member_set));
    assert(v_TP_lookups(filter, &to_be_deleted_set));



    /**Count False positive*/
    vector<set<string> *> member_set_vector;
    member_set_vector.insert(member_set_vector.end(), &member_set);
    member_set_vector.insert(member_set_vector.end(), &to_be_deleted_set);

    size_t fp_count = v_FP_counter(filter, &lookup_set, &member_set_vector);
    cout << "Number of FP " << fp_count << endl;


    /**Deletions*/
    size_t del_counter = 0;
    for (auto iter : to_be_deleted_set) {
        assert(v_single_deletion(filter, &iter));
        del_counter++;
    }


    /**Validate non removed members are still in the filter (no False Negative)*/
    assert(v_TP_lookups(filter, &member_set));

    /** Restore filter original state*/
    del_counter = 0;
    for (auto iter : member_set) {
        assert(v_single_deletion(filter, &iter));
        del_counter++;
    }
    return true;
}


template<class D>
auto v_CF_insert_overflow_single(D *filter) -> bool {
    return 0;
}

//template auto
//v_counting_filter<basic_multi_dict>(size_t filter_max_capacity, size_t lookup_reps,
//                                    size_t error_power_inv, size_t l1_counter_size,
//                                    size_t l2_counter_size, double level1_load_factor,
//                                    double level2_load_factor) -> bool;
//
//
//template auto
//v_counting_filter<safe_multi_dict>(size_t filter_max_capacity, size_t lookup_reps,
//                                   size_t error_power_inv, size_t l1_counter_size,
//                                   size_t l2_counter_size, double level1_load_factor,
//                                   double level2_load_factor) -> bool;

template auto v_CF_wrapper<multi_dict64>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                         size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                         double level2_load_factor) -> bool;

template auto v_CF_wrapper<multi_dict64>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                         size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                         double level2_load_factor, size_t seq_reps) -> bool;

//template auto
//v_counting_filter<safe_multi_dict64>(size_t filter_max_capacity, size_t lookup_reps,
//                                     size_t error_power_inv, size_t l1_counter_size,
//                                     size_t l2_counter_size, double level1_load_factor,
//                                     double level2_load_factor) -> bool;

template auto v_CF_wrapper<safe_multi_dict64>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                              size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                              double level2_load_factor) -> bool;

template auto v_CF_wrapper<safe_multi_dict64>(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                                              size_t l1_counter_size, size_t l2_counter_size, double level1_load_factor,
                                              double level2_load_factor, size_t seq_reps) -> bool;


template auto
v_filter_no_deletions<multi_dict64>(multi_dict64 *filter, size_t filter_max_capacity, size_t lookup_reps,
                                    size_t part) -> bool;


template auto
v_filter_no_deletions<safe_multi_dict64>(safe_multi_dict64 *filter, size_t filter_max_capacity,
                                         size_t lookup_reps, size_t part) -> bool;