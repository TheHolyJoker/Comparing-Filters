//
// Created by tomer on 4/8/20.
//

//Todo: move element from pd with overflowing counter, to spare. (and underflow counter from spare to pd)
// Also, all this moves should be done using some lazy scheme.

#ifndef CLION_CODE_MULTI_DICT_HPP
#define CLION_CODE_MULTI_DICT_HPP

#include <ostream>
#include "../../CPD/CPD.hpp"
#include "../../Hash_Table/multi_hash_table.hpp"
#include "../../CPD/CPD_validator.hpp"
#include "../../PD/Contiguous_PD/cg_PD.h"


/**Higher is less critical or not efficient*/
#define MD_DB_MODE0 (false)
#define MD_DB_MODE1 (true & MHT_DB_MODE0)
#define MD_DB_MODE2 (true & MHT_DB_MODE1)


template<class D, class S, typename S_T>
class multi_dict {
    vector<D> pd_vec;
//    vector<CPD> pd_vec;
    vector<uint> pd_capacity_vec;
    S *spare;
//    multi_hash_table<S_T> *spare;

    size_t capacity;
    const size_t number_of_pd, remainder_length, quotient_range, single_pd_capacity;
    const size_t quotient_length, pd_index_length;
    const size_t level1_counter_size, level2_counter_size;

    const size_t spare_element_length, sparse_counter_length;
    /*const string bad_str = "GLXDVIQ\\AIYBN";
    const string prev_str = "AJFCEPF\\XHQZOD^";
    const string bad_str = "@BO@JWLZWGC";*/
public:
    multi_dict(size_t max_number_of_elements, size_t error_power_inv, size_t level1_counter_size,
               size_t level2_counter_size, double level1_load_factor, double level2_load_factor);


    auto lookup(const string *s) -> bool;

    auto lookup_multi(const string *s) -> size_t;

    void insert(const string *s);


    void remove(const string *s);

    void get_info();

//    friend ostream &operator<<(ostream &os, const multi_dict &dict);

    auto sum_pd_capacity() -> size_t;

    auto get_PDs_hash_val(const string *s) -> std::tuple<S_T, S_T, S_T>;

    auto get_spare_hash_val(const string *s) -> std::tuple<std::tuple<size_t, size_t>, std::tuple<S_T, S_T, S_T>>;

    auto do_elements_collide(const string *s1, const string *s2) -> bool;

private:
    void insert_full_PD_helper(S_T hash_val, size_t pd_index, uint32_t quot, uint32_t r);

    void insert_to_spare(S_T y);

    void insert_to_spare_with_pop(S_T hash_val);

    void insert_to_spare_with_known_counter(S_T hash_val, size_t counter);

    void insert_level1_inc_overflow_handler(S_T hash_val);

    auto insert_to_bucket_attempt(S_T y, size_t bucket_index) -> counter_status;

    auto insert_to_bucket_attempt(S_T y, size_t bucket_index, bool pop_attempt) -> counter_status;

    auto insert_inc_to_bucket_attempt(S_T y, size_t bucket_index) -> std::tuple<counter_status, size_t>;

    auto pop_attempt(string *s) -> S_T *;

    /**
     * Try to drop an element in the bucket to lower level.
     * Stops after the first time the popping succeed, and return the poped element position.
     * @param bucket_index
     * @param y element with counter.
     * @return
     */
    auto pop_attempt_by_bucket(S_T y, size_t bucket_index) -> size_t;

    auto pop_attempt_with_insertion_by_bucket(S_T hash_val, size_t bucket_index) -> bool;

    /**
     * reads the element if
     * @param element
     * @return
     */
    auto single_pop_attempt(S_T element) -> bool;

    auto single_pop_attempt(S_T temp_el, S_T counter) -> bool;


    inline auto wrap_hash(const string *s) -> S_T {
        return my_hash(s, HASH_SEED) & MASK(spare_element_length);
    }

    auto wrap_hash_split(const string *s) -> std::tuple<S_T, S_T, S_T> {
        S_T h = my_hash(s, HASH_SEED) & MASK(spare_element_length);
        S_T r, q, pd_index;
        r = h & MASK(remainder_length);
        h >>= remainder_length;
        q = h % (quotient_range);
        h >>= quotient_length;
        pd_index = h % pd_vec.size();
        return std::make_tuple(pd_index, q, r);
    }

    inline void wrap_split(const string *s, size_t *pd_index, D_TYPE *q, D_TYPE *r) {
        auto h = my_hash(s, HASH_SEED);
        split(h, pd_index, q, r);
    }

    inline void split(ulong h, size_t *pd_index, D_TYPE *q, D_TYPE *r) {
        *r = h & MASK(remainder_length);
        h >>= remainder_length;
        *q = h % (quotient_range);
        h >>= quotient_length;
        *pd_index = h % pd_vec.size();
    }

};

static auto get_multi_spare_max_capacity(size_t dict_max_capacity, double level1_load_factor) -> size_t;

template<typename S_T>
using multi_dict_ST = multi_dict<CPD, multi_hash_table<S_T>, S_T>;

template<typename S_T>
using multi_dict_validator_ST = multi_dict<CPD_validator, multi_hash_table<S_T>, S_T>;

typedef multi_dict_ST<uint32_t> basic_multi_dict;
typedef multi_dict_ST<uint64_t> multi_dict64;
typedef multi_dict_validator_ST<uint32_t> safe_multi_dict;
typedef multi_dict_validator_ST<uint64_t> safe_multi_dict64;

/*
typedef multi_dict<CPD, multi_hash_table<uint32_t>> basic_multi_dict;
typedef multi_dict<CPD_validator, multi_hash_table<uint32_t>> safe_multi_dict;
typedef multi_dict<CPD_validator, multi_hash_table<size_t>> multi_dict64;
*/

#endif //CLION_CODE_MULTI_DICT_HPP
