//
// Created by tomer on 25/05/2020.
//

#ifndef FILTERS_TESTS_HPP
#define FILTERS_TESTS_HPP

#include "printutil.hpp"
#include "wrappers.hpp"
#include <chrono>
#include <set>
#include <unordered_set>

#define BITS_PER_ELEMENT_MACRO (8)
#define BITS_PER_ELEMENT_MACRO_12 (12)
#define CUCKOO_BITS_PER_ITEM_MACRO (12)

typedef chrono::nanoseconds ns;

/*Basic functions*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint64_t x = 123456789, y = 362436069, z = 521288629;

/**
 * https://stackoverflow.com/a/1227137/5381404
 * @return
 */
auto xorshf96() -> uint64_t;

template<typename itemType>
auto rand_item() -> itemType {
    return (itemType) rand();
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
    // std::cout << "G3" << std::endl;
    vec->resize(number_of_elements);
    for (int i = 0; i < number_of_elements; ++i)
        vec->at(i) = ((uint64_t)rand());
        // vec->at(i) = rand_item<uint64_t>();
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
        bool already_in_filter = filter->Contain(el);//For debugging
        filter->Add(el);
        if (!filter->Contain(el)) {
            cout << "lookup failed." << endl;
            cout << "counter: " << counter << endl;
            cout << "element: " << el << endl;

            // filter->Add(el);
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
                FilterAPI<Table>::Contain(el, wrap_filter);
                return false;
            }
        }
        return true;
    }

    for (auto el : *el_set) {
        FilterAPI<Table>::Add(el, wrap_filter);
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            double ratio = 1.0 * counter /el_set->size();
            cout << "lookup failed." << endl;
            cout << "counter: " << counter << "/" << el_set->size() << " = " << ratio << endl;
            cout << "element: " << el << endl;

            //            wrap_filter->Add(el);
            FilterAPI<Table>::Contain(el, wrap_filter);
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
            cout << "counter: " << counter << "/" << el_set->size() << endl;
            cout << "element: " << el << endl;

            assert(FilterAPI<Table>::Contain(el, wrap_filter));
            return false;
        }
    }
    return true;
}

template<class Table, typename itemType>
auto uset_deleting(Table *wrap_filter, unordered_set<itemType> *to_be_deleted_set,
                unordered_set<itemType> *to_keep_elements_set) -> bool {
    size_t counter = 0;
    for (auto el : *to_be_deleted_set) {
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            string line = std::string(80,'&');
            std::cout << line << std::endl;
            std::cout << "In deletions (uset_deleting with unordered set type): " << std::endl;
            std::cout << "Lookup failed.\t counter =  " << counter << "/" << to_be_deleted_set->size() << std::endl;
            std::cout << line << std::endl;

            return false;
        }
        bool sanity = FilterAPI<Table>::Contain(el, wrap_filter);
        assert(sanity);
        bool c = to_keep_elements_set->find(el) != to_keep_elements_set->end();
        if (c)
            continue;
        counter++;
        try {
            FilterAPI<Table>::Remove(el, wrap_filter);
        } catch (std::runtime_error &msg) {
            break;
        }
    }
    return true;
}

template<class Table, typename itemType>
auto vec_deleting(Table *wrap_filter, vector<itemType> *to_be_deleted_vec, size_t start, size_t end) -> bool {
    try {
        FilterAPI<Table>::Remove(to_be_deleted_vec->at(start), wrap_filter);
    } catch (std::runtime_error &msg) {
        std::cout << FilterAPI<Table>::get_name(wrap_filter) << "Does not support deletions" << std::endl;
        return true;
    }
    for (int i = start + 1; i < end; ++i) {
        auto el = to_be_deleted_vec->at(i);
        if (!FilterAPI<Table>::Contain(el, wrap_filter)) {
            auto db_res = FilterAPI<Table>::Contain(el, wrap_filter);
            std::cout << "In deletions (vec_deleting with vector type): " << std::endl;
            std::cout << "Lookup failed.\t counter =  " << i << "/" << (to_be_deleted_vec->size()) << std::endl;

            return false;
        }
        FilterAPI<Table>::Remove(el, wrap_filter);
    }
    return true;
}

/* template<class Table, typename itemType, bool block_insertion = false>
auto v_deletions_wrapper(Table *wrap_filter, size_t del_reps) -> bool {
    size_t del_reps = 1 << 10;
    unordered_set<itemType> to_be_deleted_set;
    // set_init(filter_max_capacity / 2, &member_set);
    set_init(del_reps, &to_be_deleted_set);
    // set_init(lookup_reps, &lookup_set);

    cond &= v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(wrap_filter, &to_be_deleted_set);
    assert(cond);
    cond &= v_true_positive_elements<Table, itemType>(wrap_filter, &to_be_deleted_set);
    assert(cond);
    cond &= v_deleting<Table, itemType>(wrap_filter, &to_be_deleted_set, &member_set);
    assert(cond);
    return true;
}
 */

template<class Table, typename itemType>
auto v_insertions(Table *wrap_filter, vector<itemType> *to_add_vec, size_t start, size_t end) -> void {
    for (int i = start; i < end; ++i)
        FilterAPI<Table>::Add(to_add_vec->at(i), wrap_filter);
}

template<class Table, typename itemType, bool block_insertion = false>
auto v_filter_core(Table *wrap_filter, size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                   double level1_load_factor, double level2_load_factor, std::stringstream *ss) -> int {
    //    Table wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    //    auto number_of_elements_in_the_filter = filter_max_capacity;

    unordered_set<itemType> member_set, lookup_set, to_be_deleted_set;
    size_t del_size = 0;
    // size_t del_size = filter_max_capacity / 2;
    set_init(filter_max_capacity - del_size, &member_set);
    set_init(lookup_reps, &lookup_set);
    // set_init(del_size, &to_be_deleted_set);

    size_t counter = 0;
    /**Insertion*/
    bool valid = v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(wrap_filter, &member_set);
    if (!valid)
        return -1;

    valid &= v_insertion_plus_imm_lookups<Table, itemType, block_insertion>(wrap_filter, &to_be_deleted_set);
    if (!valid)
        return -2;

    /**Lookup*/
    valid &= v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    if (!valid)
        return -3;
    valid &= v_true_positive_elements<Table, itemType>(wrap_filter, &to_be_deleted_set);
    if (!valid)
        return -4;


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
            valid &= FilterAPI<Table>::Contain(iter, wrap_filter);
            if (!valid)
                return -5;
        } else if (FilterAPI<Table>::Contain(iter, wrap_filter)) {
            fp_counter++;
        }
    }

    auto temp = att_print_single_round_false_positive_rates(lookup_set.size(), error_power_inv, fp_counter, tp_counter);
    *ss << temp.str();

    auto temp2 = print_single_round_false_positive_rates(filter_max_capacity, lookup_set.size() >> error_power_inv, tp_counter, fp_counter);
    *ss << temp2.str();
    //    cout << "filter_max_capacity: " << filter_max_capacity << endl;
    //    cout << "\nnumber of false-positive is out of total number of lookups: " << fp_counter << "/ " << lookup_reps << endl;
    //    cout << "Expected FP count: " << (lookup_set.size() >> error_power_inv) << endl;

    // std::cout << "deletion validation" << std::endl;
    counter = 0;
    valid &= uset_deleting<Table, itemType>(wrap_filter, &to_be_deleted_set, &member_set);
    if (!valid)
        return -6;

    /**Deletions*/

    /**Verifying no unwanted element was deleted (prone to error as deleting from filter is not well defined)*/
    valid &= v_true_positive_elements<Table, itemType>(wrap_filter, &member_set);
    if (!valid)
        return -7;

    return 1;
    // if (!valid)
    //     return false;
    // if (FilterAPI<Table>::get_ID(wrap_filter) == CF) {
    //     FilterAPI<Table>::get_dynamic_info(wrap_filter);
    // }
}


template<class Table, typename itemType, bool block_insertion = false>
auto w_validate_filter(size_t filter_max_capacity, size_t lookup_reps, size_t error_power_inv,
                       double level1_load_factor, double level2_load_factor, std::stringstream *ss) -> bool {
    Table wrap_filter = FilterAPI<Table>::ConstructFromAddCount(filter_max_capacity);
    size_t line_width = 160;// number of columns (6) * column's width (24)
    print_name(FilterAPI<Table>::get_name(&wrap_filter));
    auto res = v_filter_core<Table, itemType, block_insertion>(&wrap_filter, filter_max_capacity, lookup_reps,
                                                               error_power_inv, level1_load_factor, level2_load_factor, ss);
    if (res != 1){
        std::cout << FilterAPI<Table>::get_name(&wrap_filter) << " Failed in validation." << std::endl;
        std::cout << "v_filter_core failed. Error value is: " << res << std::endl;
        assert(false);
    }
    assert(res == 1);
    return res;
}
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

    }

template<typename Table>
auto v_return_false_when_empty() -> bool {
    Table filter = FilterAPI<Table>::ConstructFromAddCount(1u << 10u);
    return filter.Contain(42);
}


#endif//FILTERS_TESTS_HPP
