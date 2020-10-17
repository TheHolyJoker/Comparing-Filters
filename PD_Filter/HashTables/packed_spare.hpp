//
// Created by tomer on 8/18/20.
//
/* This implementation rely on
 * 1) Every element in the hashTable consists of three parts (msb to lsb) pd_index, quot, rem.
 * 2) quot is an integer in range [0,51). More specificity, quot != 63.
 * 3) number of bits for rem is 8. (not a must).
 * 4) The empty slot can be seen is equal to (63 << 8). 
 empty slot in a way tha */
#ifndef FILTERS_PACKED_SPARE_HPP
#define FILTERS_PACKED_SPARE_HPP


#include "../../hashutil.h"
#include "../L2_pd/L2Bucket_mq.hpp"
#include "../basic_function_util.h"
#include "../macros.h"
#include <vector>


// static bool found_something = false;
// static bool el3002_was_inserted = false;
// // static int el3002 = 0;
// static size_t HT_insert_counter = 0;
// static size_t HT_lookup_counter = 0;
// static size_t p2_cond_counter = 0;
// static size_t swap_counter = 0;
// static size_t line_counter = 0;


template<size_t bucket_capacity, size_t batch_size, size_t bits_per_item, size_t quot_length>
class packed_spare {
    size_t capacity{0};
    const size_t number_of_buckets;
    const size_t max_spare_capacity;
    hashing::TwoIndependentMultiplyShift Hasher;

    MainBucket<bucket_capacity, batch_size, bits_per_item> *main_buckets;
    Quotients<bucket_capacity, batch_size, quot_length> *q_buckets;


public:
    explicit packed_spare(size_t number_of_buckets_in_l1)
        : number_of_buckets((number_of_buckets_in_l1 + batch_size - 1) / batch_size),
          max_spare_capacity(((number_of_buckets_in_l1 + batch_size - 1) / batch_size) * bucket_capacity),
          Hasher() {
        assert(number_of_buckets <= MASK32);

        //        std::cout << "sizeof(MainBucket()): " << sizeof(MainBucket<bucket_capacity, batch_size, bits_per_item>()) << std::endl;
        main_buckets = new MainBucket<bucket_capacity, batch_size, bits_per_item>[number_of_buckets];
        q_buckets = new Quotients<bucket_capacity, batch_size, quot_length>[number_of_buckets];
        /* int ok1 = posix_memalign((void **) &main_buckets, 64, 64 * number_of_buckets);
               int ok2 = posix_memalign((void **) &q_buckets, 8, 8 * number_of_buckets);
               if (ok1 == 0) {
               std::cout << "ok1: " << ok1 << std::endl;
               std::cout << "ok2: " << ok2 << std::endl;
               std::cout << "ok1 Failed!!!" << std::endl;
        std::cout << "ok1 Failed!!!" << std::endl;
                   assert(false);
               return;
               }
               if (ok2 == 0) {
                   std::cout << "ok2 Failed!!!" << std::endl;
                   assert(false);
                   return;
               }
 */
        
        for (size_t i = 0; i < number_of_buckets; ++i) {
            main_buckets[i].init();
            q_buckets[i].init();
        }
        
    }

    virtual ~packed_spare() {
        delete[] main_buckets; 
        delete[] q_buckets; 
        // free(main_buckets);
        // free(q_buckets);
    }

    auto find(uint64_t pd_index, uint8_t quot, uint8_t rem) const -> bool {
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = reduce32(Hasher(pd_index), (uint32_t) number_of_buckets);
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);
        return find_helper(rel_pd_index, quot, rem, 1, b1) ||
               find_helper(rel_pd_index, quot, rem, 0, b2);
    }

    auto find_helper(uint64_t pd_index, uint8_t quot, uint8_t rem, bool is_primary_bucket,
                     size_t bucket_index) const -> bool {

        uint64_t mask = main_buckets[bucket_index].find(pd_index, rem, is_primary_bucket);
        return mask && q_buckets[bucket_index].find(mask, quot);
    }


    void insert(uint64_t pd_index, uint8_t quot, uint8_t rem) {
        if (capacity >= max_spare_capacity) {
            std::cout << "Trying to insert into fully loaded hash table" << std::endl;
            assert(false);
        }
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = reduce32(Hasher(pd_index), (uint32_t) number_of_buckets);
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);


        assert(!(main_buckets[b1].is_full() && main_buckets[b2].is_full()));

        (main_buckets[b1].get_capacity() < main_buckets[b2].get_capacity()) ? insert_helper(rel_pd_index, quot, rem, 1,
                                                                                            b1)
                                                                            : insert_helper(rel_pd_index, quot, rem, 0,
                                                                                            b2);
    }

    auto insert_helper(uint64_t pd_index, uint8_t quot, uint8_t rem, bool is_primary_bucket,
                       size_t bucket_index) -> bool {

        size_t index = main_buckets[bucket_index].insert(pd_index, rem, is_primary_bucket);
        q_buckets[bucket_index].add(index, quot);
        return index != bucket_capacity;
    }


    void remove(uint64_t pd_index, uint8_t quot, uint8_t rem) {
        assert(find(pd_index, quot, rem));
        if (capacity == 0) {
            std::cout << "Trying to delete from empty hash table" << std::endl;
        }
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = reduce32(Hasher(pd_index), (uint32_t) number_of_buckets);
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);

        const bool res = remove_helper(rel_pd_index, quot, rem, 1, b1) || remove_helper(rel_pd_index, quot, rem, 0, b2);
        assert(res);
    }

    auto remove_helper(uint64_t pd_index, uint8_t quot, uint8_t rem, bool is_primary_bucket,
                       size_t bucket_index) -> bool {

        uint64_t mask = main_buckets[bucket_index].find(pd_index, rem, is_primary_bucket);
        if (!mask) {
            return false;
        }
        size_t index = q_buckets[bucket_index].conditional_remove(mask, quot);
        main_buckets[bucket_index].remove_by_index(pd_index, rem, is_primary_bucket, index);
        return true;
    }

    auto get_name() -> std::string {
        return "packed spare";
    }

    auto get_byte_size() {
        auto main_bucket_size = get_l2_MainBucket_bit_size<bucket_capacity, batch_size, bits_per_item>();
        auto q_bucket_size = quot_length * bucket_capacity;
        auto bucket_bit_size = main_bucket_size + q_bucket_size;
        return number_of_buckets * (bucket_bit_size);
    }

    auto get_table_size() const -> size_t {
        return number_of_buckets;
    }

    auto get_max_capacity() const -> size_t {
        return max_spare_capacity;
    }

    //    auto get_element_length() const -> const size_t {
    //        return element_length;
    //    }

    auto get_bucket_size() const -> size_t {
        auto main_bucket_size = get_l2_MainBucket_bit_size<bucket_capacity, batch_size, bits_per_item>();
        auto q_bucket_size = quot_length * bucket_capacity;
        auto bucket_bit_size = main_bucket_size + q_bucket_size;
        return bucket_bit_size;
    }

    auto get_capacity() const -> size_t {
        return capacity;
    }

    //    auto get_max_load_factor() const -> const double {
    //        return max_load_factor;
    //    }

    auto get_load_factor() const -> double {
        return capacity / (double) max_spare_capacity;
    }

    void get_info(std::stringstream *os) {

        auto line = std::string(64, '-');
        *os << line << std::endl;
        *os << "Spare max_capacity is: " << str_format(get_max_capacity()) << std::endl;
        *os << "Spare capacity is: " << str_format(get_capacity()) << std::endl;
        *os << "Byte size is: " << str_format(get_byte_size()) << std::endl;
        *os << "Spare load factor is: " << get_load_factor() << std::endl;

        /*if (insert_existing_counter) {
            *os << "insert_existing_counter: " << insert_existing_counter << std::endl;
            double ratio = insert_existing_counter / (double) max_capacity;
            *os << "ratio to max capacity: " << ratio << std::endl;
        }*/
        //        double waste_ratio = (sizeof(bucket_size) * CHAR_BIT) / ((double) element_length);
        //        *os << "Waste ratio (by not packing): " << waste_ratio << std::endl;
        //        *os << "element bit size: " << element_length << std::endl;

        size_t empty_buckets = count_empty_buckets();
        *os << "Number of empty buckets " << empty_buckets << "/" << number_of_buckets << "\t";
        *os << "ratio is: " << (empty_buckets / (double) number_of_buckets) << std::endl;

        auto tp = find_empty_bucket_interval();
        size_t start = std::get<0>(tp), end = std::get<1>(tp);
        *os << "longest_empty_interval length is: " << end - start << std::endl;
        *os << "start: " << start << std::endl;
        *os << "end: " << end << std::endl;
        if (start + 10 < end) {
            *os << "longest_empty_interval length is: " << end - start << std::endl;
            *os << "start: " << start << std::endl;
            *os << "end: " << end << std::endl;
        }
        *os << line << std::endl;
        // return *os;
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

    /*void get_all_elements(vector<bucket_type> *vec) {
        auto resize_val = get_capacity();
        vec->resize(resize_val);
        for (size_t i = 0; i < num_of_buckets; i++) {
            auto *bp = Table[i].bits_;

            for (size_t j = 0; j < bucket_size; j++) {
                if (bp[j] == empty_slot)
                    continue;
                vec->push_back(bp[j]);
            }
        }
    }
*/
    auto count_empty_buckets() -> size_t {
        size_t res = 0;
        for (size_t i = 0; i < number_of_buckets; i++) {
            if (is_bucket_empty(i))
                res++;
        }
        return res;
    }

    auto is_state_valid() -> bool;

    /*auto is_bucket_elements_unique(size_t bucket_index) -> bool {
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
*/
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
    /*auto get_element_by_index(size_t table_index) -> bucket_type {
        size_t bucket_index = table_index / bucket_size;
        size_t in_bucket_index = table_index % bucket_size;
        return get_element_by_bucket_index_and_location(bucket_index, in_bucket_index);
        //        assert(false);
        //        return table[table_index];
    }*/

    //    auto is_empty_by_index(size_t table_index) -> bool {
    //        size_t bucket_index = table_index / bucket_size;
    //        size_t in_bucket_index = table_index % bucket_size;
    //        return is_empty_by_bucket_index_and_location(bucket_index, in_bucket_index);
    //        //        assert(false);
    //        //        return (table[table_index] == EMPTY);
    //    }
    //
    //    auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool {
    //        auto *bp = Table[bucket_index].bits_;
    //        return bp[location] == empty_slot;
    //    }
    //
    //    auto clear_slot_bucket_index_and_location(size_t bucket_index, size_t location) -> void {
    //        auto *bp = Table[bucket_index].bits_;
    //        bp[location] = empty_slot;
    //    }
    //
    //    inline auto is_bucket_full_by_index(uint32_t bucket_index) const -> bool {
    //        return get_bucket_capacity(bucket_index) == bucket_size;
    //    }
    //
    //    auto does_bucket_contain_valid_elements(uint32_t bucket_index) const -> bool {
    //        auto *bp = Table[bucket_index].bits_;
    //        for (int i = 0; i < bucket_size; ++i) {
    //            auto temp = bp[i];
    //            if (temp == empty_slot)
    //                continue;
    //
    //            uint32_t b1, b2;
    //            my_hash(temp, &b1, &b2);
    //            bool cond = (bucket_index == b1) or (bucket_index == b2);
    //            if (!cond) {
    //                std::cout << "h5" << std::endl;
    //                std::cout << "bucket_index is: " << bucket_index << std::endl;
    //                std::cout << "temp: " << temp;
    //                std::cout << "\tbuckets: (" << b1 << ", " << b2 << ")" << std::endl;
    //                assert(false);
    //            }
    //        }
    //        return true;

    /*auto get_element_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bucket_type {
        auto *bp = Table[bucket_index].bits_;
        return bp[location];
    }*/

    /*
        void update_max_cuckoo_insert(size_t i) {
            max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
        }

        **
         * Increase "cuckoo_HT_insert_counter" by "cuckoo_chain_length" * "bucket_size".
         * @param cuckoo_chain_length
         *
        void update_cuckoo_HT_insert_counter(size_t cuckoo_chain_length) {
            cuckoo_HT_insert_counter += cuckoo_chain_length * bucket_size;
        }
        */


    //    }

    //    auto in_which_bucket_is_the_element(bucket_type x) -> int {
    //        uint32_t b1 = -1, b2 = -1;
    //        my_hash(x, &b1, &b2);
    //
    //        // if (db_cond)
    //        // {
    //        //     std::cout << "b1: " << b1 << std::endl;
    //        //     std::cout << "b2: " << b2 << std::endl;
    //        //     std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
    //        // }
    //        if (find_helper(x, b1))
    //            return 1;
    //        if (find_helper(x, b2))
    //            return 2;
    //        return -1;
    //        // return ((find_helper(x, b1)) || find_helper(x, b2));
    //    }

private:
    //    auto get_bucket_capacity(size_t bucket_index) const -> size_t {
    //        size_t res = 0;
    //        auto *bp = Table[bucket_index].bits_;
    //        for (int i = 0; i < bucket_size; ++i) {
    //            if (bp[i] != empty_slot) {
    //                res++;
    //            }
    //        }
    //        return res;
    //    }

    auto is_bucket_empty(size_t bucket_index) const -> bool {
        return main_buckets[bucket_index].get_capacity() == 0;
    }

    auto find_empty_bucket_interval() -> std::tuple<size_t, size_t> {
        size_t max_length = 0;
        size_t start = 0, end = 0;
        size_t temp_start = 0, temp_end = 0;
        size_t index = 0;
        // __m512i *ppd = &pd_array[0];
        while (index < number_of_buckets) {
            //            size_t temp_length = 0;
            if (is_bucket_empty(index)) {
                size_t temp_length = 1;
                size_t temp_index = index + 1;
                while ((temp_index < number_of_buckets) and (is_bucket_empty(temp_index))) {
                    temp_index++;
                    temp_length++;
                }
                if (temp_index == number_of_buckets) {
                    std::cout << "h8!" << std::endl;
                }
                temp_length = temp_index - index;
                if (temp_length > max_length) {
                    start = index;
                    end = temp_index;
                    max_length = temp_length;
                }
                index = temp_index + 1;
            } else
                index++;
        }
        return {start, end};
    }
};

#endif//FILTERS_PACKED_SPARE_HPP
