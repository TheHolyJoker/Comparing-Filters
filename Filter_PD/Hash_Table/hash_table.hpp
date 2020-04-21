/*Todo: I assumed */

#ifndef CLION_CODE_HASH_TABLE_HPP
#define CLION_CODE_HASH_TABLE_HPP

#include <iostream>
#include <zconf.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <cmath>
#include "../Global_functions/macros.h"
#include "../Global_functions/cuckoo_and_counting_macros.hpp"
#include "../Hash/static_hashing.h"
#include "../bit_operations/my_bit_op.hpp"


//#define FREE_IND (0x80000000)
//#define IS_FREE(x) ((x & MASK(31u)) == 0)

template<typename T>
class hash_table {
    T *table;
    const size_t table_size{}, max_capacity{}, element_length{}, bucket_size{};
    size_t capacity{};
    const double max_load_factor{};
    const uint32_t seed1{}, seed2{};

    ////validation parameters.

    /** "max_cuckoo_insert" is used to store the maximal length of a cuckoo chain occurred during an insertion.*/
    size_t max_cuckoo_insert{};
    /** "cuckoo_insert_counter" is used to measure the total length of cuckoo chain occurred during all insertion.
     * The measure over single insertion "I_1", equal to the length of the cuckoo chain this insertion caused,
     * multiplied by the size of a bucket.
     */
    size_t cuckoo_insert_counter{};

    size_t max_capacity_reached{};


public:
    hash_table(size_t max_capacity, size_t element_length, size_t bucket_size, double max_load_factor);

    hash_table(size_t max_capacity, size_t element_length, double max_load_factor = DEFAULT_MAX_LOAD_FACTOR);

    virtual ~hash_table();

//    hash_table(size_t )
//    hash_table(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity, size_t remainder_length);

    auto find(T x) -> bool;

    void insert(T x);

    /**
 * Tries to insert "x" to bucket in "bucket_index".
 * If the bucket is not full, x will be inserted, and true will be returned.
 * Otherwise, x was not inserted, and false will be returned.
 * @param x
 * @param bucket_index
 * @return
 */
    auto insert_if_bucket_not_full(T x, size_t bucket_index) -> bool;

    void insert_by_table_index(T x, T table_index);

    /**
 *
 * @param x
 * @param bucket_index
 * @param location
 * table[bucket_index*bucket_size + location] = x;
 */
    void insert_by_bucket_index_and_location(T x, size_t bucket_index, size_t location);

    void insert_by_slot_pointer(T x, T *p_slot);

    void insert_without_counter(T x);

    void remove(T x);

    void full_buckets_handler(T x, size_t b1, size_t b2);

    void pop_attempt(T x);

    auto insert_single_cuckoo(T *hold, size_t *bucket_index, size_t cuckoo_counter) -> bool;


    /**
     * Picks random element in the bucket. denote by "temp_val".
     * Insert "hold" in "temp_val" position.
     * Set *hold = "temp_val".
     * Store in "bucket_index" the OTHER bucket "temp_val" can be stored in.
     * @param hold
     * @param bucket_index
     */
    void cuckoo_swap(T *hold, size_t *bucket_index);

    //void insert_with_pop_attempt(T x, vector<D> *pd_vec);


    ////Getters
    auto get_max_cuckoo_insert() const -> size_t;

    auto get_cuckoo_insert_counter() const -> size_t;

    auto get_max_capacity_reached() const -> size_t;

    void get_data();

    auto get_bucket_address(size_t bucket_index) -> T *;

    auto get_table_size() const -> const size_t;

    auto get_max_capacity() const -> const size_t;

    auto get_element_length() const -> const size_t;

    auto get_bucket_size() const -> const size_t;

    auto get_capacity() const -> size_t;

    auto get_max_load_factor() const -> const double;


    ////Setters

    void increase_capacity();

    void decrease_capacity();

    ////validation
    auto is_state_valid() -> bool;

    auto is_bucket_elements_unique(size_t bucket_index) -> bool;

    auto find_table_location(T x) -> size_t;


    /**
     * @param table_index the index of table, in which the element is stored.
     * @return the element without the counter.
     */
    auto get_element_without_counter_by_index(size_t table_index) -> T;

    auto get_element_without_counter_by_bucket_index_and_location(size_t bucket_index, size_t location) -> T;

    auto get_element_without_counter(T element) -> T;

    /**
     * currently define for clarity.
     * @param table_index
     * @return
     */
    auto get_element_with_counter(size_t table_index) -> T;

    void update_max_cuckoo_insert(size_t i);

    /**
     * Increase "cuckoo_insert_counter" by "cuckoo_chain_length" * "bucket_size".
     * @param cuckoo_chain_length
     */
    void update_cuckoo_insert_counter(size_t cuckoo_chain_length);


    auto is_empty_by_index(size_t table_index) -> bool;

    auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool;

    auto is_empty_by_slot(T slot) -> bool;

    auto is_bucket_full_by_index(size_t bucket_index) -> bool;

    inline void my_hash(T x, uint32_t *b1, uint32_t *b2) {
        size_t number_of_buckets_in_each_table = (table_size / bucket_size) / 2;
        *b1 = (hashint(x)) % number_of_buckets_in_each_table;
        *b2 = (hashint2(x) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
    }

private:

    auto insert_helper(T x, size_t bucket_index);

    auto find_helper(T x, size_t bucket_index) -> bool;

    auto find_helper_table_location(T x, size_t bucket_index) -> int;

    auto remove_helper(T x, size_t bucket_index) -> bool;


    /**
     *
     * @param with_counter
     * @param without_counter
     * @return compares x,y first "element length" bits.
     */
    auto is_equal(T with_counter, T without_counter) -> bool;

    auto is_deleted(size_t table_index) -> bool;

    auto get_bucket_capacity(size_t bucket_index) -> size_t;

    /**
     * This function is called whenever an element is trying to be inserted into full bucket.
     * It tries to move one of the elements in "bucket_index" to it's other possible location (cuckoo scheme).
     * If it succeed, a pointer to a free location will be returned.
     * Otherwise, a null pointer will be returned.
     * @param bucket_index The bucket from whom elements will try to be moved.
     * @return pointer to the free location.
     */
    auto move_el_from_bucket_to_other(size_t bucket_index) -> T *;
/*
//    inline void my_hash(T x, uint32_t *i1, uint32_t *i2) {
//        *i1 = hashint(x);
//        *i2 = hashint2(x);
//    }*/

};


//auto compute_size(const size_t max_capacity, const double max_load_factor) -> size_t;


auto
compute_max_capacity(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity,
                     size_t remainder_length) -> size_t;

auto
compute_element_length(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity,
                       size_t remainder_length) -> size_t;

auto
compute_element_length(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity, size_t remainder_length,
                       size_t counter_length) -> size_t;

#endif //CLION_CODE_HASH_TABLE_HPP
