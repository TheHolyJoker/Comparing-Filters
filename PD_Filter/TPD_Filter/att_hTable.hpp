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
#include <zconf.h>
#include "macros.h"
#include "hashutil.hpp"

static bool found_something = false;
static bool el3002_was_inserted = false;
// static int el3002 = 0;
static size_t HT_insert_counter = 0;
static size_t HT_lookup_counter = 0;
static size_t p2_cond_counter = 0;
static size_t swap_counter = 0;

template<typename bucket_type,
        size_t bucket_size>
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
        assert(num_of_buckets <= MASK32);

        int ok = posix_memalign((void **) &Table, sizeof(Bucket) * CHAR_BIT, sizeof(Bucket) * num_of_buckets);

        if (ok != 0) {
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
        free(Table);
        // delete[] Table;
    }

    auto f(const bucket_type x) const -> bool {
        return (x == 3340244266);
    }

    auto h() -> bool {
        bucket_type x = 3340244266;

//        assert(my_hash(x) == 60481692);
//        auto x_hash_val = my_hash(x);
        int res = in_which_bucket_is_the_element(x);
//        if (res == -1) {
//            std::cout << "T2" << std::endl;
//            std::cout << "HT_insert counter is: " << HT_insert_counter << std::endl;
//        }
        return in_which_bucket_is_the_element(x) != -1;
    }


    auto find(bucket_type x) const -> bool {
        HT_lookup_counter++;
        bool db_cond = f(x);
        assert((x & MASK(element_length)) == x);

        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);
        assert(does_bucket_contain_valid_elements(b1));
        assert(does_bucket_contain_valid_elements(b2));

        if (db_cond) {
            std::cout << "b1: " << b1 << std::endl;
            std::cout << "b2: " << b2 << std::endl;
            std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
        }
        found_something |= ((find_helper(x, b1)) || find_helper(x, b2));
        return ((find_helper(x, b1)) || find_helper(x, b2));
    }

    auto print_bucket(uint32_t bucket_index, ostream &os = std::cout) -> ostream & {
        auto line = std::string(80, '-') + "\n";
        os << line;
        os << "bucket number " << bucket_index << " content:" << std::endl;
        auto *bp = Table[bucket_index].bits_;
        os << "[";
        bool is_slot_empty = is_empty_by_bucket_index_and_location(bucket_index, 0);
        if (!is_slot_empty)
            os << bp[0];
        else
            os << "Empty";

        for (int i = 1; i < bucket_size; ++i) {
            if (is_empty_by_bucket_index_and_location(bucket_index, i)) {
                os << ", Empty!";
            } else {
                os << ", " << bp[i];
            }
        }
        os << "]" << std::endl;
        os << line;
        return os;
    }

    void insert(bucket_type x) {
        bool was_flipped_now = false;
        HT_insert_counter++;
//        bool ic_cond = (HT_insert_counter == 1118697);
        bool ic_cond = false;
        /*if (ic_cond) {
            std::cout << std::string(128, '@') << std::endl;
        }*/
        if (el3002_was_inserted) {
            if (!h()) {
                std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
                std::cout << "x: " << x << std::endl;
                assert(false);
            }
        }
        bool printer = false;
        /*bool skip_assert = false;
        if (f(x) or ic_cond) {
            if (!el3002_was_inserted) {
                std::cout << std::string(80, '*') << std::endl;
                std::cout << "insert index: " << HT_insert_counter << std::endl;
                std::cout << "el3002 was flipped! " << std::endl;
                std::cout << std::string(80, '*') << std::endl;
                skip_assert = true;
                was_flipped_now = true;
            }
            printer = true;
//            el3002_was_inserted = true;
        }*/
        assert((x & MASK(element_length)) == x);

        if (capacity >= max_capacity) {
            std::cout << "Trying to insert into fully loaded hash table" << std::endl;
            assert(false);
        }
        //        capacity++;
        /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

        uint32_t b1 = -1, b2 = -1;

        my_hash(x, &b1, &b2);
        assert(does_bucket_contain_valid_elements(b1));
        assert(does_bucket_contain_valid_elements(b2));

        assert((was_flipped_now) or (!el3002_was_inserted) || h());
        /*if (was_flipped_now) {
            std::cout << "Trying to insert (" << x << ") into the second bucket, (" << b2 << ").\n";
            std::cout << "Bucket contents before insertion attempt is:\n";
            print_bucket(b2);
        }*/
        if (insert_if_bucket_not_full(x, b2)) {
            /*if (was_flipped_now) {
                std::cout << "Trying to insert (" << x << ") into the second bucket, (" << b2 << ").\n";
                std::cout << "Bucket contents After insertion attempt is:\n";
                print_bucket(b2);
            }
            if (f(x))
                assert(h());

            bool will_not_assert = (!el3002_was_inserted) || h();
            if (!will_not_assert) {
                std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
                std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
                std::cout << "swap counter: " << swap_counter << std::endl;
                std::cout << "x: " << x << std::endl; //2499577067
                assert(false);
            }

            if (printer)
                std::cout << "was inserted to b2. bucket number " << b2 << std::endl;
            assert(does_bucket_contain_valid_elements(b2));
            */
            return;
        }
//        if (printer)
//            std::cout << "was inserted to b1. bucket number " << b1 << std::endl;

        /*bucket_type y = 287449031;
        size_t troubled_bucket = 98497;
//        bool p2_cond = (x == y) and (b1 == troubled_bucket);
        bool p2_cond = false;
        if (p2_cond) {
            p2_cond_counter++;
            std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
            std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
        }*/
        auto hold = x;
        size_t bucket_index = b1;
        for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            /*if ((x == y) and (b1 == troubled_bucket)) {
                std::cout << "T_Before_T" << std::endl;
                std::cout << "h() := " << h() << std::endl;
                print_bucket(b1);
            }*/
            assert(does_bucket_contain_valid_elements(bucket_index));
            if (insert_if_bucket_not_full(hold, bucket_index)) {
                assert(does_bucket_contain_valid_elements(bucket_index));
                /*if (p2_cond) {
                    std::cout << "T_After_T" << std::endl;
                    std::cout << "h() := " << h() << std::endl;
                    print_bucket(b1);
                }
                bool going_to_assert = ((!el3002_was_inserted) || h());
                if (!going_to_assert) {
                    std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
                    std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
                    std::cout << "i: " << i << std::endl;
                    std::cout << "sleeping: ";
                    usleep(500000);
                    std::cout << ".";
                    usleep(300000);
                    std::cout << ".";
                    usleep(200000);
                    std::cout << ".";
                    usleep(100000);
                    std::cout << ".";
                    std::cout << std::endl;
                }
                assert(going_to_assert);
                */
                return;
            }
            bucket_type old_hold = hold;
            assert(does_bucket_contain_valid_elements(bucket_index));
            cuckoo_swap(&hold, &bucket_index);
            assert(does_bucket_contain_valid_elements(bucket_index));
            /*if (printer) {
                std::cout << "was swapped to " << hold << std::endl;
                std::cout << "i is: " << i << std::endl;
            }*/
            if (old_hold == hold) {
                std::cout << "swap counter" << swap_counter << std::endl;
                assert(false);
            }
        }
        // assert(!el3002_was_inserted || h());
        assert(false);
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
//        assert(false);
        // assert(!el3002_was_inserted || h());
        assert(does_bucket_contain_valid_elements(*bucket_index));
        swap_counter++;
        bucket_type old_val = *hold;
        bucket_type junk = swap_elements_from_bucket(*bucket_index, *hold);
        assert(junk != empty_slot);
        *hold = junk;

        uint32_t temp_b1 = -1, temp_b2 = -1;
        my_hash(*hold, &temp_b1, &temp_b2);
        assert(does_bucket_contain_valid_elements(*bucket_index));
        assert(temp_b2 != temp_b1);

        if (temp_b1 == *bucket_index)
            *bucket_index = temp_b2;
        else if (temp_b2 == *bucket_index)
            *bucket_index = temp_b1;
        else {
            std::cout << "h4" << std::endl;
            std::cout << "*hold: " << *hold;
            std::cout << "\tbuckets: (" << temp_b1 << ", " << temp_b2 << ")" << std::endl;
//            my_hash(old_val, &old_b2, &old_b1);
//            std::cout << "\tRevered: (" << old_b1<<", " << old_b2 << ")" << std::endl;


            std::cout << "*bucket_index: " << *bucket_index << std::endl;
            std::cout << "num_of_buckets: " << num_of_buckets << std::endl;

            uint32_t old_b1, old_b2;
            my_hash(old_val, &old_b1, &old_b2);

            std::cout << "old_val: " << old_val;
            std::cout << "\tbuckets: (" << old_b1 << ", " << old_b2 << ")";
            my_hash(old_val, &old_b2, &old_b1);
            std::cout << "\tRevered: (" << old_b1 << ", " << old_b2 << ")" << std::endl;


            std::cout << "*hold: " << *hold;
            my_hash(*hold, &old_b1, &old_b2);
            std::cout << "\tbuckets: (" << old_b1 << ", " << old_b2 << ")";
            my_hash(*hold, &old_b2, &old_b1);
            std::cout << "\tRevered: (" << old_b1 << ", " << old_b2 << ")" << std::endl;

            std::cout << "empty_slot: " << empty_slot << std::endl;
            assert(false);
        }
        assert(does_bucket_contain_valid_elements(*bucket_index));

        // assert(!el3002_was_inserted || h());
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

    auto swap_elements_from_bucket(size_t bucket_index, bucket_type x) -> bucket_type {
        assert(is_bucket_full_by_index(bucket_index));
        auto *bp = Table[bucket_index].bits_;

        auto rand_bucket_index = random() % bucket_size;
        auto correct_index = 1ULL << 63ul;
        bucket_type temp = bp[rand_bucket_index];
        if (temp == x) {
            bool did_fail = true;
            for (int i = 1; i < bucket_size; ++i) {
                correct_index = (rand_bucket_index + i) % bucket_size;
                temp = bp[correct_index];
                if (temp != x) {
                    did_fail = false;
                    break;
                }
            }
            if (did_fail) {
                std::cout << "insertion failed. The bucket contains is full with the same element. (" << x << ")"
                          << std::endl;
                print_bucket(bucket_index);
                assert(false);
            }
        }
        assert(temp != x);
        bp[correct_index] = x;
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

    auto get_cuckoo_HT_insert_counter() const -> size_t {
        return cuckoo_HT_insert_counter;
    }

    auto get_max_capacity_reached() const -> size_t {
        return max_capacity_reached;
    }

    void get_data() {
        std::cout << "max_cuckoo_insert " << get_max_cuckoo_insert() << std::endl;
        std::cout << "cuckoo_HT_insert_counter " << get_cuckoo_HT_insert_counter() << std::endl;
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

    auto get_load_factor() const -> double {
        return capacity / (double) max_capacity;
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
     * Increase "cuckoo_HT_insert_counter" by "cuckoo_chain_length" * "bucket_size".
     * @param cuckoo_chain_length
     *
    void update_cuckoo_HT_insert_counter(size_t cuckoo_chain_length) {
        cuckoo_HT_insert_counter += cuckoo_chain_length * bucket_size;
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

    auto is_bucket_full_by_index(uint32_t bucket_index) const -> bool {
        return get_bucket_capacity(bucket_index) == bucket_size;
    }

    auto does_bucket_contain_valid_elements(uint32_t bucket_index) const -> bool {
        auto *bp = Table[bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            auto temp = bp[i];
            if (temp == empty_slot)
                continue;

            uint32_t b1, b2;
            my_hash(temp, &b1, &b2);
            bool cond = (bucket_index == b1) or (bucket_index == b2);
            if (!cond) {
                std::cout << "h5" << std::endl;
                std::cout << "bucket_index is: " << bucket_index << std::endl;
                std::cout << "temp: " << temp;
                std::cout << "\tbuckets: (" << b1 << ", " << b2 << ")" << std::endl;
                assert(false);
            }
        }
        return true;
    }

    inline void my_hash(bucket_type x, uint32_t *b1, uint32_t *b2) const {
        assert(x == (x & MASK(element_length)));
        size_t number_of_buckets_in_each_table = num_of_buckets / 2;
        *b1 = (s_pd_filter::hashint64(x)) % number_of_buckets_in_each_table;
        *b2 = (s_pd_filter::hashint64_2(x) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
    }

    auto get_name() -> std::string {
        return "Bucket HT";
    }


    auto in_which_bucket_is_the_element(bucket_type x) -> int {
        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        // if (db_cond)
        // {
        //     std::cout << "b1: " << b1 << std::endl;
        //     std::cout << "b2: " << b2 << std::endl;
        //     std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
        // }
        if (find_helper(x, b1))
            return 1;
        if (find_helper(x, b2))
            return 2;
        return -1;
        // return ((find_helper(x, b1)) || find_helper(x, b2));
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

    auto get_bucket_capacity(size_t bucket_index) const -> size_t {
        size_t res = 0;
        auto *bp = Table[bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            if (bp[i] != empty_slot) {
                res++;
            }
        }
        return res;
    }
};

#endif //FILTERS_ATT_HTABLE_HPP
