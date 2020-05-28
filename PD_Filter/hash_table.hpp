/*Todo: I assumed */

#ifndef CLION_CODE_HASH_TABLE_HPP
#define CLION_CODE_HASH_TABLE_HPP

#include <iostream>
//#include <zconf.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <iostream>
#include <cmath>
#include <climits>
#include "macros.h"
#include "hashutil.hpp"

//#define FREE_IND (0x80000000)
//#define IS_FREE(x) ((x & MASK(31u)) == 0)

#define HT_DB_MODE0 (true)
#define HT_DB_MODE1 (HT_DB_MODE0 & true)
#define HT_DB_MODE2 (HT_DB_MODE1 & true)

using namespace std;


template<typename T>
class hash_table {
    T *table;
    const size_t table_size, max_capacity, element_length, bucket_size;
    size_t capacity;
    const double max_load_factor;
    const uint32_t seed1, seed2;

    ////validation parameters.

    /** "max_cuckoo_insert" is used to store the maximal length of a cuckoo chain occurred during an insertion.*/
    size_t max_cuckoo_insert;
    /** "cuckoo_insert_counter" is used to measure the total length of cuckoo chain occurred during all insertion.
     * The measure over single insertion "I_1", equal to the length of the cuckoo chain this insertion caused,
     * multiplied by the size of a bucket.
     */
    size_t cuckoo_insert_counter;

    size_t max_capacity_reached;


public:
    hash_table(size_t max_capacity, size_t element_length, double max_load_factor,
               size_t bucket_size = DEFAULT_BUCKET_SIZE) :
            max_capacity(max_capacity), element_length(element_length), bucket_size(bucket_size),
            max_load_factor(max_load_factor), capacity(0),
            table_size(std::ceil(((double) max_capacity) / max_load_factor)), seed1(42), seed2(43),
            max_cuckoo_insert(0), cuckoo_insert_counter(0), max_capacity_reached(0) {
        table = new T[table_size];
        // The msb is indicator to whether the cell is free or not. (0 might be valid fingerprint)
        assert(element_length < sizeof(T) * CHAR_BIT);
//    cout << "table_size is: " << table_size << endl;
        for (int i = 0; i < table_size; ++i) {
            table[i] = EMPTY;
        }
    }


//    hash_table(size_t max_capacity, size_t element_length, double max_load_factor = DEFAULT_MAX_LOAD_FACTOR) {
////    cout << "Constructor 2" << endl;
//
//    }


    virtual ~hash_table() {
//    cout << "max_cuckoo_insert " << this->max_cuckoo_insert << endl;
//    cout << "cuckoo_insert_counter " << this->cuckoo_insert_counter << endl;
        delete[] table;
    }

//    hash_table(size_t )
//    hash_table(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity, size_t remainder_length);

    auto find(T x) -> bool const {
        if (HT_DB_MODE1)
            assert((x & MASK(element_length)) == x);

        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        return ((find_helper(x, b1)) || find_helper(x, b2));

    }

    void insert(T x) {
        if (HT_DB_MODE1)
            assert((x & MASK(element_length)) == x);
//    assert(!find(x));

        if (HT_DB_MODE0) {
            if (capacity >= max_capacity) {
                std::cout << "Trying to insert into fully loaded hash table" << std::endl;
                assert(false);
            }
        }
        capacity++;
        /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

        max_capacity_reached = (max_capacity_reached >= capacity) ? max_capacity_reached : capacity;
        uint32_t b1 = -1, b2 = -1;

        my_hash(x, &b1, &b2);

        if (insert_if_bucket_not_full(x, b2)) return;
        T hold = x;
        size_t bucket_index = b1;
        cuckoo_insert_counter -= bucket_size;
        for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            this->cuckoo_insert_counter += bucket_size;
            if (insert_if_bucket_not_full(hold, bucket_index)) {
                max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
                return;
            }

            auto rand_table_index = (bucket_index * bucket_size) + (random() % bucket_size);
            auto temp = table[rand_table_index];

            table[rand_table_index] = hold;
            hold = temp;

            uint32_t temp_b1 = -1, temp_b2 = -1;
            my_hash(hold, &temp_b1, &temp_b2);

            if (HT_DB_MODE1)
                assert(temp_b2 != temp_b1);

            if (temp_b1 == bucket_index)
                bucket_index = temp_b2;
            else if (temp_b2 == bucket_index)
                bucket_index = temp_b1;
            else {
                assert(false);
            }
        }

        assert(false);
    }

    /**
 * Tries to insert "x" to bucket in "bucket_index".
 * If the bucket is not full, x will be inserted, and true will be returned.
 * Otherwise, x was not inserted, and false will be returned.
 * @param x
 * @param bucket_index
 * @return
 */
    auto insert_if_bucket_not_full(T x, size_t bucket_index) -> bool {
        if (HT_DB_MODE1)
            assert((x & MASK(element_length)) == x);
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {
            if (is_empty_by_index(table_index + i)) {
                insert_by_table_index(x, table_index + i);
                return true;
            }
            /*Add pop attempt*/
        }
        return false;
    }

    void insert_by_table_index(T x, T table_index) {
        table[table_index] = x;
        capacity++;
    }

    /**
 *
 * @param x
 * @param bucket_index
 * @param location
 * table[bucket_index*bucket_size + location] = x;
 */
    void insert_by_bucket_index_and_location(T x, size_t bucket_index, size_t location) {
        insert_by_table_index(x, bucket_index * bucket_size + location);
    }

    void insert_by_slot_pointer(T x, T *p_slot) {
        assert(false);
        *p_slot = x;
    }

    void insert_without_counter(T x);

    void remove(T x) {
//    if (HT_DB_MODE2)
//        assert(find(x));
        if (HT_DB_MODE0) {
            if (capacity == 0) {
                std::cout << "Trying to delete from empty hash table" << std::endl;
//        assert(false);
            }
        }

        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        // remove_helper will return "true" if x in the hash_table.
        if (remove_helper(x, b1))
            return;
        remove_helper(x, b2);

    }

    void full_buckets_handler(T x, size_t b1, size_t b2) {
        assert(false);

//    T *possible_free_slot = move_el_from_bucket_to_other(b1);

    }

    void pop_attempt(T x);

    auto insert_single_cuckoo(T *hold, size_t *bucket_index, size_t cuckoo_counter) -> bool {
        cuckoo_insert_counter += bucket_size;
        if (insert_if_bucket_not_full(*hold, *bucket_index)) {
            update_max_cuckoo_insert(cuckoo_counter);
            return true;
        }

        auto rand_table_index = (*bucket_index * bucket_size) + (random() % bucket_size);
        auto temp = table[rand_table_index];

        table[rand_table_index] = *hold;
        *hold = temp;

        uint32_t temp_b1 = -1, temp_b2 = -1;
        my_hash(*hold, &temp_b1, &temp_b2);

        if (HT_DB_MODE1)
            assert(temp_b2 != temp_b1);

        if (temp_b1 == *bucket_index)
            *bucket_index = temp_b2;
        else if (temp_b2 == *bucket_index)
            *bucket_index = temp_b1;
        else {
            assert(false);
        }
        return false;
    }


    /**
     * Picks random element in the bucket. denote by "temp_val".
     * Insert "hold" in "temp_val" position.
     * Set *hold = "temp_val".
     * Store in "bucket_index" the OTHER bucket "temp_val" can be stored in.
     * @param hold
     * @param bucket_index
     */
    void cuckoo_swap(T *hold, size_t *bucket_index) {
        auto rand_table_index = (*bucket_index * bucket_size) + (random() % bucket_size);
        auto temp = table[rand_table_index];

        table[rand_table_index] = *hold;
        *hold = temp;

        uint32_t temp_b1 = -1, temp_b2 = -1;
        my_hash(*hold, &temp_b1, &temp_b2);

        if (HT_DB_MODE1)
            assert(temp_b2 != temp_b1);

        if (temp_b1 == *bucket_index)
            *bucket_index = temp_b2;
        else if (temp_b2 == *bucket_index)
            *bucket_index = temp_b1;
        else {
            assert(false);
        }
    }

    //void insert_with_pop_attempt(T x, vector<PDType> *pd_vec);


    ////Getters
    auto get_max_cuckoo_insert() const -> size_t {
        return max_cuckoo_insert;
    }

    auto get_cuckoo_insert_counter() const -> size_t {
        return cuckoo_insert_counter;
    }

    auto get_max_capacity_reached() const -> size_t {
        return max_capacity_reached;
    }

    void get_data() {
        cout << "max_cuckoo_insert " << get_max_cuckoo_insert() << endl;
        cout << "cuckoo_insert_counter " << get_cuckoo_insert_counter() << endl;
        cout << "get_max_capacity_reached " << get_max_capacity_reached() << endl;
    }

//    auto get_bucket_address(size_t bucket_index) -> T *;

    auto get_table_size() const -> const size_t {
        return table_size;
    }

    auto get_max_capacity() const -> const size_t {
        return max_capacity;
    }

    auto get_element_length() const -> const size_t {
        return element_length;
    }

    auto get_bucket_size() const -> const size_t {
        return bucket_size;
    }

    auto get_capacity() const -> size_t {
        return capacity;
    }

    auto get_max_load_factor() const -> const double {
        return max_load_factor;
    }


    ////Setters

    void increase_capacity() {
        capacity++;
    }

    void decrease_capacity() {
        capacity--;
    }


    ////validation
    auto is_state_valid() -> bool;

    auto is_bucket_elements_unique(size_t bucket_index) -> bool {
        auto table_index = bucket_index * bucket_size;
        for (int j = 0; j < bucket_size; ++j) {
            if (is_empty_by_index(table_index + j))
                continue;
            T x = table[table_index + j];
            for (int i = j + 1; i < bucket_size; ++i) {
                if (is_empty_by_index(table_index + i))
                    continue;
                if (is_equal(table[table_index + i], x))
                    return false;
            }
        }
        return true;
    }

    auto find_table_location(T x) -> size_t {
        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        auto res = find_helper_table_location(x, b1);
        if (res != -1)
            return res;
        return find_helper_table_location(x, b2);


    }


    /**
     * @param table_index the index of table, in which the element is stored.
     * @return the element without the counter.
     */
    auto get_element_by_index(size_t table_index) -> T {
        return table[table_index];
    }

    auto get_element_by_bucket_index_and_location(size_t bucket_index, size_t location) -> T {
        return get_element_by_index(bucket_index * bucket_size + location);
    }

    auto get_element(T element) -> T;

    /**
     * currently define for clarity.
     * @param table_index
     * @return
     */
    auto get_element_with_counter(size_t table_index) -> T;

    void update_max_cuckoo_insert(size_t i) {
        max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
    }

    /**
     * Increase "cuckoo_insert_counter" by "cuckoo_chain_length" * "bucket_size".
     * @param cuckoo_chain_length
     */
    void update_cuckoo_insert_counter(size_t cuckoo_chain_length) {
        cuckoo_insert_counter += cuckoo_chain_length * bucket_size;
    }


    auto is_empty_by_index(size_t table_index) -> bool {
        return (table[table_index] == EMPTY);
    }

    auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool {
        assert (location < bucket_size);
        return is_empty_by_index(bucket_index * bucket_size + location);
    }

    auto is_empty_by_slot(T slot) -> bool {
        return (slot == EMPTY);
    }

    auto is_bucket_full_by_index(size_t bucket_index) -> bool {
        return get_bucket_capacity(bucket_index) == bucket_size;
    }

    inline void my_hash(T x, uint32_t *b1, uint32_t *b2) {
        size_t number_of_buckets_in_each_table = (table_size / bucket_size) / 2;
        *b1 = (s_pd_filter::hashint(x)) % number_of_buckets_in_each_table;
        *b2 = (s_pd_filter::hashint2(x) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
    }

private:

    auto insert_helper(T x, size_t bucket_index) {
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {
            if (is_empty_by_index(table_index + i)) {
                table[table_index + i] = x;
                return;
            }
            /*Add pop attempt*/
        }
        /*Here need to use cuckoo hash scheme*/
        cout << "Trying to insert to full bucket" << endl;
        assert(false);
    }

    auto find_helper(T x, size_t bucket_index) -> bool const {
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {
            if (is_equal(table[table_index + i], x))
                return true;
        }
        return false;
    }

    auto find_helper_table_location(T x, size_t bucket_index) -> int {
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {
            if (is_equal(table[table_index + i], x))
                return table_index + i;
        }
        return -1;
    }

    auto remove_helper(T x, size_t bucket_index) -> bool {
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {
            if (is_equal(table[table_index + i], x)) {
                table[table_index + i] = EMPTY;
//            auto prev_val = table[table_index + i];
                /*//            if (find(x)) {
    //                auto res = find_table_location(x);
    //                find(x);
    //                assert(false);
    //            }*/
                capacity--;
                return true;
            }
        }
        return false;
    }


    /**
     *
     * @param with_counter
     * @param without_counter
     * @return compares x,y first "element length" bits.
     */
    auto is_equal(T with_counter, T without_counter) -> bool const {
        T after_mask = without_counter & MASK(element_length);
        assert((without_counter & MASK(element_length)) == without_counter);
        return (with_counter & MASK(element_length)) == without_counter;
    }

    auto is_deleted(size_t table_index) -> bool {
        return table[table_index] == EMPTY;
    }

    auto get_bucket_capacity(size_t bucket_index) -> size_t {
        auto table_index = bucket_index * bucket_size;
        size_t counter = 0;
        for (int i = 0; i < bucket_size; ++i) {
            if (!is_empty_by_index(table_index + i))
                counter++;
        }
        return counter;

    }

    /**
     * This function is called whenever an element is trying to be inserted into full bucket.
     * It tries to move one of the elements in "bucket_index" to it's other possible location (cuckoo scheme).
     * If it succeed, a pointer to a free location will be returned.
     * Otherwise, a null pointer will be returned.
     * @param bucket_index The bucket from whom elements will try to be moved.
     * @return pointer to the free location.
     */
    auto move_el_from_bucket_to_other(size_t bucket_index) -> T * {
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {

            auto x = table[table_index + i];
            uint32_t b1 = -1, b2 = -1;
            my_hash(x, &b1, &b2);

            size_t other_bucket = (b1 == bucket_index) ? b1 : b2;
            if (HT_DB_MODE1)
                assert((b1 == bucket_index) or (b2 == bucket_index));
            if (insert_if_bucket_not_full(x, other_bucket)) {
                return &(table[table_index + i]);
            }
        }
        return nullptr;
    }
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
