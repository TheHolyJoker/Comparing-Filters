//
// Created by tomer on 4/7/20.
//

/* Empty slot counter should be zero.
 * No slot should have counter equal to zero, with "element" != 0.
 * Every element counter is larger than 0.
 *****************************************************************
 * Some notation.
 * @table_index
 * @bucket_index
 * @location: the relative index of element inside some bucket.
 * For example, an element x with table_index i, has bucket_index table_index / bucket_size,
 * and location table_index % bucket_size.
 * @x element without counter.
 * @y element with counter.
 *****************************************************************
 */

#ifndef CLION_CODE_MULTI_HASH_TABLE_HPP
#define CLION_CODE_MULTI_HASH_TABLE_HPP

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


/**Higher is less critical or not efficient*/
#define MHT_DB_MODE0 (false)
#define MHT_DB_MODE1 (true & MHT_DB_MODE0)
#define MHT_DB_MODE2 (true & MHT_DB_MODE1)

/*
#ifndef ENUM_COUNTER_STATUS
#define ENUM_COUNTER_STATUS

enum counter_status {
    inc_overflow, dec_underflow, OK, not_a_member
};

#endif //
*/


template<typename T>
class multi_hash_table {
    T *table;
    const size_t table_size, max_distinct_capacity, bucket_size, element_size, counter_size;
    size_t distinct_capacity, capacity;
    const double max_load_factor;

    ////validation parameters.
    /**
     * "max_cuckoo_insert" is used to store the maximal length of a cuckoo chain occurred during an insertion.
     */
    size_t max_cuckoo_insert;
    /** "cuckoo_insert_counter" is used to measure the total length of cuckoo chain occurred during all insertion.
     * The measure over single insertion "I_1", equal to the length of the cuckoo chain this insertion caused,
     * multiplied by the size of a bucket.
     */
    size_t cuckoo_insert_counter;

    size_t max_capacity_reached;


public:

    multi_hash_table(size_t max_distinct_capacity, size_t element_size, size_t counter_size, double max_load_factor,
                     size_t bucket_size = DEFAULT_BUCKET_SIZE_MULT);

    /*

//    multi_hash_table(size_t max_distinct_capacity, size_t element_size, size_t counter_size) : multi_hash_table(
//            max_distinct_capacity, element_size, counter_size, DEFAULT_MAX_LOAD_FACTOR_MULT, DEFAULT_BUCKET_SIZE_MULT) {
//        cout << "MHT second constructor." << endl;
//    }


//    hash_table(size_t max_capacity, size_t element_length, double max_load_factor = DEFAULT_MAX_LOAD_FACTOR);
*/

    virtual ~multi_hash_table();


    auto find(T x) -> bool;

    /**
     *
     * @param x
     * @return The bucket in which the element is stored, if the element is in the HT.
     * Returns "table_size" otherwise.
     */
    auto find_element_table_index(T x) -> size_t;

    /**
     *
     * @param x
     * @param bucket_index
     * @return The location {0,1,.."bucket_size" - 1} in which the element is stored inside the bucket.
     * If the element is not in the bucket, returns "bucket_size".
     */
    auto find_element_location_in_bucket(T x, size_t bucket_index) -> size_t;

    /**
     *
     * @param x
     * @return zero if x is not a member.
     */
    auto find_multiplicities(T x) -> uint32_t;

    /**
     * x \in "this" ==> increase "x" counter.
     * x \not\in "this" ==> insert_new(x).
     * @param x element.
     * @return Did this insertion caused an "x"'s counter to overflow.
     */
    auto insert(T x) -> counter_status;

    auto insert_inc_att(T x) -> counter_status;

    /**
     * insert new element, i.e element currently not \in "this".
     * @param y
     */
    void insert_new(T y);

    /**
     * this should function should be used, when an insertion from a lower level caused overflow (counter should be inc by 1 ?).
     * Therefore
     * 1) the element inserted is not in the table.
     * 2) to pop?
     * @param y
     */
    void insert_after_lower_memory_hierarchy_counter_overflow_with_counter(T y);

    /**
     * If x \in table, increase it counter, and returns counter status.
     * If x \not\in table, return -1;
     * Todo should also look for empty slot, in case x is not in the table.
     * @param x
     * @return
     */
//    auto insert_inc_attempt(T x) ->int;

    /**
     * Tries to insert "x" to bucket in "bucket_index".
     * If the bucket is not full, x will be inserted, and true will be returned.
     * Otherwise, x was not inserted, and false will be returned.
     * @param x
     * @param bucket_index
     * @return
     */
    auto insert_new_element_if_bucket_not_full(T x, size_t bucket_index) -> bool;

    auto insert_element_with_counter_if_bucket_not_full(T y, size_t bucket_index) -> bool;

    /**
     *
     * @param x
     * @param bucket_index
     * @param location
     * table[bucket_index*bucket_size + location] = x;
     */
    void insert_by_bucket_index_and_location(T x, size_t bucket_index, size_t location);

    void insert_new_element_without_counter_by_index(T x, size_t table_index);

    void insert_new_element_with_counter_by_index(T y, size_t table_index);


    /**
     * Assuming the element is in table.
     * @param x
     * @return
     */
    auto remove(T x) -> counter_status;

    /**
     *
     * @param x
     * @param bucket_index
     * @return
     * "x" \in table ==> return non negative integer from the set {dec_underflow,OK}.
     * "x" \not\in table ==> return not_a_member.
     */
    auto remove_helper(T x, size_t bucket_index) -> counter_status;

    /**
     * Picks random element in the bucket. denote by "temp_val".
     * Insert "hold" in "temp_val" position.
     * Set *hold = "temp_val".
     * Store in "bucket_index" the OTHER bucket "temp_val" can be stored in.
     * @param hold
     * @param bucket_index
     */
    void cuckoo_swap(T *hold, size_t *bucket_index);

    auto get_element_without_counter_by_bucket_index_and_location(size_t bucket_index, size_t location) -> T;

    auto get_element_with_counter_by_bucket_index_and_location(size_t bucket_index, size_t location) -> T;

    /**
     * Assuming the element is not empty.
     * @param table_index
     * @return
     */
    auto get_counter_by_table_index(size_t table_index) -> size_t;

    auto is_occupied(size_t table_index) -> bool;

    auto is_equal(T with_counter, T without_counter) -> bool;

    auto is_equal_by_index(size_t table_index, T without_counter) -> bool;

    auto is_equal_by_bucket_and_location(size_t bucket_index, size_t location, T without_counter) -> bool;

    auto if_equal_return_multiplicity_by_table_index(T without_counter, size_t table_index) -> bool;

    auto is_empty_by_index(size_t table_index) -> bool;

    auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool;

    auto is_bucket_full_by_index(size_t bucket_index) -> bool;

    inline void my_hash(T x, uint32_t *b1, uint32_t *b2) {
        x &= MASK(element_size);
        size_t number_of_buckets_in_each_table = (table_size / bucket_size) / 2;
        *b1 = (hashint(x)) % number_of_buckets_in_each_table;
        *b2 = (hashint2(x) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
    }

    auto get_hash_buckets(T x) -> std::tuple<size_t, size_t>;


    //Getters

    auto get_element(size_t table_index) -> T;

    auto get_split_element(size_t table_index) -> std::tuple<T, T>;

    void set_element(T x, size_t table_index);

    void set_element(T x, size_t counter, size_t table_index);


    auto get_bucket_capacity(size_t bucket_index) -> size_t;

    auto get_bucket_size() -> size_t;

    auto get_table_size() -> size_t;
    ////Setters

    void increase_capacity();

    void decrease_capacity();

    void store_new_element(T x, size_t table_index);

    void update_max_cuckoo_insert(size_t i);

    /**
     * Increase "cuckoo_insert_counter" by "cuckoo_chain_length" * "bucket_size".
     * @param cuckoo_chain_length
     */
    void update_cuckoo_insert_counter(size_t cuckoo_chain_length);

    auto add_counter_to_new_element(T x) -> T;

    auto add_counter_to_element(T x, T counter) -> T;

    auto increase_counter(size_t table_index) -> counter_status;

private:
    auto find_in_bucket(T x, size_t bucket_index) -> bool;

    auto find_multiplicity_in_bucket(T x, size_t bucket_index) -> size_t;

    auto decrease_counter(size_t table_index) -> counter_status;

    auto safe_get_distinct_capacity() -> size_t;

    auto safe_get_total_capacity() -> size_t;
};


#endif //CLION_CODE_MULTI_HASH_TABLE_HPP
