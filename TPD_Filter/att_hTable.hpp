//
// Created by tomer on 7/11/20.
//

#ifndef FILTERS_ATT_HTABLE_HPP
#define FILTERS_ATT_HTABLE_HPP

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <iostream>
#include <cmath>
#include <climits>
#include "macros.h"
#include "hashutil.hpp"

template<typename bucket_type,
        size_t bucket_size
>
class att_hTable {

    struct Bucket {
        bucket_type bits_[bucket_size];
    };

    Bucket *Table;
    const size_t num_of_buckets, max_capacity, element_length;
    size_t capacity{0};
    const double max_load_factor;
    const bucket_type empty_slot{(bucket_type) -1};


public:
    att_hTable(size_t max_capacity, size_t element_length, double max_load_factor)
            : max_capacity(max_capacity), element_length(element_length), max_load_factor(max_load_factor),
              num_of_buckets(std::ceil(max_capacity / (max_load_factor * bucket_size))) {

        int ok = posix_memalign((void**)&Table, sizeof(Bucket) * CHAR_BIT, sizeof(Bucket) * num_of_buckets);

        if (ok != 0){
            cout << "Failed!!!" << endl;
            return;
        }
//        Table = new Bucket[num_of_buckets];

        assert(element_length < sizeof(bucket_type) * CHAR_BIT);
        for (int i = 0; i < num_of_buckets; ++i) {
            auto bp = Table[i].bits_;
            for (int j = 0; j < bucket_size; ++j) {
                bp[j] = empty_slot;
            }
        }
    }


    virtual ~att_hTable() {
        delete[] Table;
    }


    auto find(bucket_type x) const -> bool {
        assert((x & MASK(element_length)) == x);

        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        return ((find_helper(x, b1)) || find_helper(x, b2));

    }

    void insert(bucket_type x) {
        assert((x & MASK(element_length)) == x);

        if (capacity >= max_capacity) {
            std::cout << "Trying to insert into fully loaded hash table" << std::endl;
            assert(false);
        }
//        capacity++;
        /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

        uint32_t b1 = -1, b2 = -1;

        my_hash(x, &b1, &b2);

        if (insert_if_bucket_not_full(x, b2)) return;
        auto hold = x;
        size_t bucket_index = b1;
        for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            if (insert_if_bucket_not_full(hold, bucket_index))
                return;

            cuckoo_swap(&hold, bucket_index);
        }

        assert(false);
    }

    void remove(bucket_type x) {
//    if (HT_DB_MODE2)
//        assert(find(x));

        if (capacity == 0) {
            std::cout << "Trying to delete from empty hash table" << std::endl;
//        assert(false);
        }


        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        // remove_helper will return "true" if x in the hash_table.
        if (remove_helper(x, b1))
            return;
        remove_helper(x, b2);

    }

    /**
     * Picks random element in the bucket. denote by "temp_val".
     * Insert "hold" in "temp_val" position.
     * Set *hold = "temp_val".
     * Store in "bucket_index" the OTHER bucket "temp_val" can be stored in.
     * @param hold
     * @param bucket_index
     */
    void cuckoo_swap(bucket_type *hold, size_t *bucket_index) {
        auto junk = swap_elements_from_bucket(*bucket_index, *hold);
        hold = &junk;

        uint32_t temp_b1 = -1, temp_b2 = -1;
        my_hash(*hold, &temp_b1, &temp_b2);

        assert(temp_b2 != temp_b1);

        if (temp_b1 == *bucket_index)
            *bucket_index = temp_b2;
        else if (temp_b2 == *bucket_index)
            *bucket_index = temp_b1;
        else {
            assert(false);
        }
    }

    auto swap_elements_from_bucket(size_t bucket_index, bucket_type x) -> bucket_type {
        auto rand_bucket_index = random() % bucket_size;
        auto *bp = Table[bucket_index].bits_;
        bucket_type temp = bp[rand_bucket_index];
        bp[rand_bucket_index] = x;
        return temp;
    }


    /**
     * Tries to insert "x" to bucket in "bucket_index".
     * If the bucket is not full, x will be inserted, and true will be returned.
     * Otherwise, x was not inserted, and false will be returned.
     * @param x
     * @param bucket_index
     * @return
     */
    auto insert_if_bucket_not_full(bucket_type x, size_t bucket_index) -> bool {
        assert((x & MASK(element_length)) == x);
        auto *bp = Table[bucket_index].bits_;

        for (int i = 0; i < bucket_size; ++i) {
            if (is_empty_by_bucket_index_and_location(bucket_index, i)) {
                bp[i] = x;
                capacity++;
                return true;
            }
            /*Add pop attempt*/
        }
        return false;
    }

    void insert_by_table_index(bucket_type x, bucket_type table_index) {
        auto bucket_index = table_index / bucket_size;
        auto in_bucket_index = table_index % bucket_size;
        auto *bp = Table[bucket_index];
        bp[in_bucket_index] = x;
        capacity++;
    }

    /**
     *
     * @param x
     * @param bucket_index
     * @param location
     * table[bucket_index*bucket_size + location] = x;
     */
    void insert_by_bucket_index_and_location(bucket_type x, size_t bucket_index, size_t location) {
        auto *bp = Table[bucket_index].bits_;
        bp[location] = x;
        capacity++;

//        insert_by_table_index(x, bucket_index * bucket_size + location);
    }


    ////Getters
/*
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
        std::cout << "max_cuckoo_insert " << get_max_cuckoo_insert() << std::endl;
        std::cout << "cuckoo_insert_counter " << get_cuckoo_insert_counter() << std::endl;
        std::cout << "get_max_capacity_reached " << get_max_capacity_reached() << std::endl;
    }
*/

    auto get_table_size() const -> const size_t {
        return num_of_buckets;
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

/*
    void clear_slot_bucket_index_and_location(size_t bucket_index, size_t location) {
        table[bucket_index * bucket_size + location] = EMPTY;
    }
*/

    ////validation
    auto is_state_valid() -> bool;

    auto is_bucket_elements_unique(size_t bucket_index) -> bool {
        auto *bp = Table[bucket_index].bits_;
        for (int j = 0; j < bucket_size; ++j) {
            if (is_empty_by_bucket_index_and_location(bucket_index, j))
                continue;
            for (int i = j + 1; i < bucket_size; ++i) {
                if (bp[j] == bp[i])
                    return false;
            }
        }
        return true;
    }

    /*auto find_table_location(slot_type x) -> size_t {
        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        auto res = find_helper_table_location(x, b1);
        if (res != -1)
            return res;
        return find_helper_table_location(x, b2);


    }*/


    /**
     * @param table_index the index of table, in which the element is stored.
     * @return the element without the counter.
     */
    auto get_element_by_index(size_t table_index) -> bucket_type {
        size_t bucket_index = table_index / bucket_size;
        size_t in_bucket_index = table_index % bucket_size;
        return get_element_by_bucket_index_and_location(bucket_index, in_bucket_index);
//        assert(false);
//        return table[table_index];
    }

    auto get_element_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bucket_type {
        auto *bp = Table[bucket_index].bits_;
        return bp[location];
    }

/*
    void update_max_cuckoo_insert(size_t i) {
        max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
    }

    /**
     * Increase "cuckoo_insert_counter" by "cuckoo_chain_length" * "bucket_size".
     * @param cuckoo_chain_length
     *
    void update_cuckoo_insert_counter(size_t cuckoo_chain_length) {
        cuckoo_insert_counter += cuckoo_chain_length * bucket_size;
    }
    */

    auto is_empty_by_index(size_t table_index) -> bool {
        size_t bucket_index = table_index / bucket_size;
        size_t in_bucket_index = table_index % bucket_size;
        return is_empty_by_bucket_index_and_location(bucket_index, in_bucket_index);
//        assert(false);
//        return (table[table_index] == EMPTY);
    }

    auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool {
        auto *bp = Table[bucket_index].bits_;
        return bp[location] == empty_slot;
    }

    auto clear_slot_bucket_index_and_location(size_t bucket_index, size_t location) -> void {
        auto *bp = Table[bucket_index].bits_;
        bp[location] = empty_slot;
    }


    auto is_bucket_full_by_index(size_t bucket_index) -> bool {
        return get_bucket_capacity(bucket_index) == bucket_size;
    }

    inline void my_hash(bucket_type x, uint32_t *b1, uint32_t *b2) const {
        size_t number_of_buckets_in_each_table = num_of_buckets / 2;
        *b1 = (s_pd_filter::hashint(x)) % number_of_buckets_in_each_table;
        *b2 = (s_pd_filter::hashint2(x) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
    }

    auto get_name() -> std::string {
        return "Bucket HT";
    }

private:

    inline auto find_helper(bucket_type x, size_t bucket_index) const -> bool {
//        auto table_index = bucket_index * bucket_size;
        auto *bp = Table[bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            if (bp[i] == x)
//            if (is_equal(table[table_index + i], x))
                return true;
        }
        return false;
    }

/*
    auto find_helper_table_location(bucket_type x, size_t bucket_index) -> int {
        auto table_index = bucket_index * bucket_size;
        for (int i = 0; i < bucket_size; ++i) {
            if (is_equal(table[table_index + i], x))
                return table_index + i;
        }
        return -1;
    }*/

    auto remove_helper(bucket_type x, size_t bucket_index) -> bool {
//        auto table_index = bucket_index * bucket_size;
        auto *bp = Table[bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            if (bp[i] == x) {
                bp[i] = empty_slot;
                capacity--;
                return true;
            }
            /*if (is_equal(table[table_index + i], x)) {
            table[table_index + i] = EMPTY;
//            auto prev_val = table[table_index + i];
                        if (find(x)) {
    //                auto res = find_table_location(x);
    //                find(x);
    //                assert(false);
    //            }
                capacity--;
                return true;*/
        }
        return false;
    }


    /**
     *
     * @param with_counter
     * @param without_counter
     * @return compares x,y first "element length" bits.
     */
    auto is_equal(bucket_type with_counter, bucket_type without_counter) const -> bool {
        bucket_type after_mask = without_counter & MASK(element_length);
//        assert((without_counter & MASK(element_length)) == without_counter);
        return (with_counter & MASK(element_length)) == without_counter;
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

};


#endif //FILTERS_ATT_HTABLE_HPP
