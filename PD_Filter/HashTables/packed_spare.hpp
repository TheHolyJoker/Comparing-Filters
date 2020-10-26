//
// Created by tomer on 8/18/20.
//
/* This implementation rely on
 * 1) Every element in the hashTable consists of three parts (msb to lsb) pd_index, spare_quot, rem.
 * 2) spare_quot is an integer in range [0,51). More specificity, spare_quot != 63.
 * 3) number of bits for rem is 8. (not a must).
 * 4) The empty slot can be seen is equal to (63 << 8). 
 empty slot in a way tha */
#ifndef FILTERS_PACKED_SPARE_HPP
#define FILTERS_PACKED_SPARE_HPP


//#include "../../hashutil.h"
//#include "../basic_function_util.h"
//#include "../macros.h"
#include "../L2_pd/twoDimPD.hpp"
//#include "Spare_Validator.hpp"
#include "HistoryLog.hpp"
//#include <unordered_set>
//#include <vector>

#define PS_QUOT_SIZE (25)
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
public:
    size_t capacity{0};
    const size_t number_of_buckets;
    const size_t max_spare_capacity;

    MainBucket<bucket_capacity, batch_size, bits_per_item> *main_buckets;
    Quotients<bucket_capacity, batch_size, quot_length> *q_buckets;

    unordered_set<uint64_t> big_quots_set;
    vector<size_t> capacity_vec;
    vector<size_t> pd_index_input_to_add_counter;


    //    Spare_Validator *validate_spare;
    HistoryLog *psLog;

    explicit packed_spare(size_t number_of_buckets_in_l1)
        : number_of_buckets((number_of_buckets_in_l1 + batch_size - 1) / batch_size),
          max_spare_capacity(((number_of_buckets_in_l1 + batch_size - 1) / batch_size) * bucket_capacity),
          big_quots_set() {
        assert(number_of_buckets <= MASK32);
        assert(64 < number_of_buckets);


        //        std::cout << "sizeof(MainBucket()): " << sizeof(MainBucket<bucket_capacity, batch_size, bits_per_item>()) << std::endl;
        main_buckets = new MainBucket<bucket_capacity, batch_size, bits_per_item>[number_of_buckets];
        q_buckets = new Quotients<bucket_capacity, batch_size, quot_length>[number_of_buckets];
        capacity_vec.resize(number_of_buckets);
        pd_index_input_to_add_counter.resize(number_of_buckets * 32, 0);
        //        validate_spare = new Spare_Validator(number_of_buckets_in_l1);
        //        assert(number_of_buckets_in_l1 + 1 >= number_of_buckets * batch_size);
        psLog = new HistoryLog(number_of_buckets * batch_size + 1, true);
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
            capacity_vec[i] = 0;
            //            pd_index_input_to_add_counter[i] = 0;
        }
    }

    virtual ~packed_spare() {
        delete[] main_buckets;
        delete[] q_buckets;
        //        delete validate_spare;
        delete psLog;
        // free(main_buckets);
        // free(q_buckets);
    }

    template<typename T>
    static uint8_t flip_quot(T spare_quot) {
        assert(spare_quot < PS_QUOT_SIZE);
        return (PS_QUOT_SIZE - 1) - spare_quot;
    }

    auto find(item_key_t itemKey) const -> bool {
        return find(itemKey.pd_index, itemKey.quot, itemKey.rem);
    }

    auto find(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) const -> bool {
        assert(pd_index / batch_size < number_of_buckets);
        static int find_counter = 0;
        find_counter++;
        if (spare_quot > MASK(quot_length)) {
            uint64_t set_key =
                    (pd_index << 16ul) | (static_cast<uint64_t>(spare_quot) << 8ul) | static_cast<uint64_t>(rem);
            return big_quots_set.count(set_key) != 0;
        }
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);
        auto res = find_helper(rel_pd_index, spare_quot, rem, 1, b1) ||
                   find_helper(rel_pd_index, spare_quot, rem, 0, b2);
        //        auto v_res = validate_spare->Find(pd_index, flip_quot(spare_quot), rem);
        uint64_t set_key = (pd_index << 16ul) | (static_cast<uint64_t>(spare_quot) << 8ul) | rem;
        auto high_quot_find = big_quots_set.count(set_key);
        bool old_res = res;
        res = res || high_quot_find;

        auto v2_res = psLog->Find(pd_index, spare_quot, rem);
        assert(psLog->using_flipped_quot);

        if ((res == false) && (v2_res == 0))
            return false;
        if ((res == true) && (v2_res))
            return true;
        //        assert(!v2_res == !v_res);
        if (!res) {
            psLog->print_element_history(pd_index, spare_quot, rem);
            psLog->print_bucket_log(pd_index);
            psLog->Find(pd_index, spare_quot, rem);
            assert(psLog->using_flipped_quot);

            assert(!v2_res);
        }

        if (res && !v2_res) {
            std::cout << "spare_fp" << std::endl;
        }
        return res;
    }


    auto find_db_aid(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) const -> bool {
        assert(false);
        //Todo: finish this function.
        // It is called after false negative.
        // It should print the element insertion & deletion & pop history.

        // 0) Check where the element we are looking for should be using a map or something.
        // I think there are multiple errors.
        // A) In pop operation in the pd.
        // B) In getting the pop element remainder in read remainder by index.
        // 1) Test that L1 works with this pd.
        // 2) ???
        // 3) ~~~


        //        if (spare_quot > MASK(quot_length)) {
        //            uint64_t set_key = (pd_index << 16ul) | (spare_quot << 8ul) | rem;
        //            return big_quots_set.reps(set_key) != 0;
        //        }
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);
        //        validate_spare->pop();

        auto res = find_helper(rel_pd_index, spare_quot, rem, 1, b1) ||
                   find_helper(rel_pd_index, spare_quot, rem, 0, b2);
        //        auto v_res = validate_spare->Find(pd_index, flip_quot(spare_quot), rem);
        auto v_res2 = psLog->Find(pd_index, spare_quot, rem);
        assert(psLog->using_flipped_quot);

        if (!res) { assert(!v_res2); }

        if (res && !v_res2) {
            std::cout << "spare_fp" << std::endl;
        }
        return res;
    }

    auto find_helper(uint64_t pd_index, uint8_t spare_quot, uint8_t rem, bool is_primary_bucket,
                     size_t bucket_index) const -> bool {

        uint64_t mask = main_buckets[bucket_index].find(pd_index, rem, is_primary_bucket);
        return mask && q_buckets[bucket_index].find(mask, spare_quot);
    }

    void insert(item_key_t itemKey) {
        insert(itemKey.pd_index, itemKey.quot, itemKey.rem);
    }


    void insert(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) {
        assert(pd_index / batch_size < number_of_buckets);
        auto before = pd_index_input_to_add_counter[pd_index];
        pd_index_input_to_add_counter[pd_index] += 1;
        auto after = pd_index_input_to_add_counter[pd_index];
        assert(before + 1 == after);

        if (pd_index_input_to_add_counter[pd_index] > (bucket_capacity * 2)) {
            assert(false);
        }
        capacity++;
        //        validate_spare->Add(pd_index, flip_quot(spare_quot), rem);
        psLog->Add(pd_index, spare_quot, rem);
        assert(psLog->using_flipped_quot);
        if (spare_quot > MASK(quot_length)) {
            uint64_t set_key = (pd_index << 16ul) | (static_cast<uint64_t>(spare_quot) << 8ul) | rem;
            big_quots_set.insert(set_key);
            return;
        }
        if (capacity >= max_spare_capacity) {
            std::cout << "Trying to insert into fully loaded hash table" << std::endl;
            assert(false);
        }
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);
        assert(validate_capacity_of_bucket(b1));
        assert(validate_capacity_of_bucket(b2));

        auto b1_cap = capacity_vec[b1];
        auto b2_cap = capacity_vec[b2];
        bool both_full = main_buckets[b1].is_full() && main_buckets[b2].is_full();
        if (both_full) {
            std::cout << "b1_cap: \t" << b1_cap << std::endl;
            std::cout << "b2_cap: \t" << b2_cap << std::endl;
            size_t index = getNumberOfBuckets() - 1;
            while (capacity_vec[index] == 0) {
                index--;
            }
            std::cout << "number_of_buckets: \t" << number_of_buckets << std::endl;
            std::cout << "index: \t" << index << std::endl;
            std::cout << "capacity: \t" << capacity << std::endl;
            std::cout << "max_spare_capacity: \t" << max_spare_capacity << std::endl;
        }


        assert(!(main_buckets[b1].is_full() && main_buckets[b2].is_full()));

        bool toWhichBucketToInsert = (main_buckets[b1].get_capacity() < main_buckets[b2].get_capacity());
        toWhichBucketToInsert ? insert_helper(rel_pd_index, spare_quot, rem, 1, b1) : insert_helper(rel_pd_index, spare_quot, rem, 0, b2);

        if (toWhichBucketToInsert)
            capacity_vec.at(b1) += 1;
        else {
            capacity_vec.at(b2) += 1;
        }

        assert(find(pd_index, spare_quot, rem));
    }


    auto insert_helper(uint64_t pd_index, uint8_t spare_quot, uint8_t rem, bool is_primary_bucket,
                       size_t bucket_index) -> bool {
        size_t index = main_buckets[bucket_index].insert(pd_index, rem, is_primary_bucket);
        if (index == bucket_capacity) {
            return false;
        }
        q_buckets[bucket_index].add(index, spare_quot);
        assert(find_helper(pd_index, spare_quot, rem, is_primary_bucket, bucket_index));
        return true;
    }

    void remove(item_key_t itemKey) {
        remove(itemKey.pd_index, itemKey.quot, itemKey.rem);
    }

    void remove(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) {
        //        assert(validate_spare->Find(pd_index, flip_quot(spare_quot), rem));
        static int remove_counter = 0;
        remove_counter++;
        if (remove_counter % 10000 == 0) {
            std::cout << (remove_counter - 1) << ":" << std::endl;
            item_key_t temp_key = {pd_index, static_cast<uint64_t>(spare_quot), static_cast<uint64_t>(rem)};
            std::cout << temp_key << std::endl;
        }
        assert(psLog->Find(pd_index, spare_quot, rem));
        assert(find(pd_index, spare_quot, rem));
        //        validate_spare->Remove(pd_index, flip_quot(spare_quot), rem);
        psLog->Remove(pd_index, spare_quot, rem);

        capacity--;
        if (spare_quot > MASK(quot_length)) {
            uint64_t set_key = (pd_index << 16ul) | (static_cast<uint64_t>(spare_quot) << 8ul) | rem;
            auto erase_res = big_quots_set.erase(set_key);
            assert(erase_res);
            return;
        }
        //        if (capacity == 0) {
        //            std::cout << "Trying to delete from empty hash table" << std::endl;
        //        }
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);

        bool f1 = find_helper(rel_pd_index, spare_quot, rem, 1, b1);
        bool f2 = find_helper(rel_pd_index, spare_quot, rem, 0, b2);
        bool both = f1 && f2;
        if (both)
            return remove_from_fuller(rel_pd_index, spare_quot, rem, b1, b2);
        const bool res = remove_helper(rel_pd_index, spare_quot, rem, 1, b1) ||
                         remove_helper(rel_pd_index, spare_quot, rem, 0, b2);
        assert(res);
    }

    auto remove_helper(uint64_t pd_index, uint8_t spare_quot, uint8_t rem, bool is_primary_bucket,
                       size_t bucket_index) -> bool {
        static int c = 0;
        c++;
        //        std::cout << std::string(80, '=') << std::endl;
        //        std::cout << std::string(80, '~') << std::endl;
        //        main_buckets[bucket_index].print_pd_header();
        //        print_memory::print_word_LE(main_buckets[bucket_index].get_pd_header_mask(pd_index & 31), 4);
        //        print_memory::print_word_LE(main_buckets[bucket_index].find(pd_index, rem, is_primary_bucket), 4);
        //        std::cout << std::string(80, '~') << std::endl;
        //        std::cout << std::string(80, '=') << std::endl;
        uint64_t mask = main_buckets[bucket_index].find(pd_index, rem,
                                                        is_primary_bucket);//todo: this mask was shifted right by pd_index.
        //        mask <<= pd_index;
        if (!mask) {
            return false;
        }
        size_t index = q_buckets[bucket_index].conditional_remove(mask, spare_quot);
        if (index == static_cast<size_t>(-1))
            return false;
        main_buckets[bucket_index].pop_remove_by_index(pd_index, index);
        capacity_vec[bucket_index] -= 1;
        return true;
    }

    void remove_from_fuller(size_t rel_pd_index, uint8_t spare_quot, uint8_t rem, size_t b1, size_t b2) {
        bool fromWhichBucketToRemove = (main_buckets[b1].get_capacity() >= main_buckets[b2].get_capacity());
        fromWhichBucketToRemove ? remove_helper(rel_pd_index, spare_quot, rem, 1, b1) : remove_helper(rel_pd_index, spare_quot, rem, 0, b2);
    }


    /**
     * @brief Get an element from bucket index which is the pop nominee of this bucket, according to rel_pd_index.
     *
     * @param rel_pd_index
     * @param is_primary_bucket
     * @param bucket_index
     * @return uint64_t
     */
    uint64_t get_pop_next_item_helper(uint64_t rel_pd_index, bool is_primary_bucket, size_t bucket_index) const {
        uint64_t pd_pop_mask = main_buckets[bucket_index].pop_get_mask(rel_pd_index, is_primary_bucket);
        if (!pd_pop_mask) {
            return -1;
        }
        return q_buckets[bucket_index].get_maximal_quot(pd_pop_mask);
    }


    bool should_take_nominee_from_primary_location(uint64_t pd_index) const {
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b1 = pd_index / batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);

        uint64_t pop_nom1 = get_pop_next_item_helper(rel_pd_index, 1, b1);
        if (pop_nom1 == -1)
            return false;

        uint64_t pop_nom2 = get_pop_next_item_helper(rel_pd_index, 0, b2);
        if (pop_nom2 == -1)
            return true;

        auto pop_nom1_spare_quot = pop_nom1 & MASK(quot_length);
        auto pop_nom2_spare_quot = pop_nom2 & MASK(quot_length);

        auto pop_nom1_quot = (PS_QUOT_SIZE - 1) - pop_nom1_spare_quot;
        auto pop_nom2_quot = (PS_QUOT_SIZE - 1) - pop_nom2_spare_quot;

        if (pop_nom1_quot != pop_nom2_quot)
            return (pop_nom1_quot < pop_nom2_quot);

        auto index1 = pop_nom1 >> 8ul;
        auto index2 = pop_nom2 >> 8ul;

        uint8_t rem1 = main_buckets[b1].pop_read_body_by_index(index1);
        uint8_t rem2 = main_buckets[b2].pop_read_body_by_index(index2);

        return (rem1 <= rem2);
    }

    uint64_t screen_pop_nominees_bad(uint64_t pop_nom1, uint64_t pop_nom2, size_t b1, size_t b2) const {
        bool BPC = (b1 == 263) && (b2 == 278);
        if (pop_nom1 == -1)
            return pop_nom2;
        if (pop_nom2 == -1)
            return pop_nom1;

        auto pop_nom1_spare_quot = pop_nom1 & MASK(quot_length);
        auto pop_nom2_spare_quot = pop_nom2 & MASK(quot_length);

        auto pop_nom1_quot = (PS_QUOT_SIZE - 1) - pop_nom1_spare_quot;
        auto pop_nom2_quot = (PS_QUOT_SIZE - 1) - pop_nom2_spare_quot;

        if (pop_nom1_quot != pop_nom2_quot) {
            return (pop_nom1_quot < pop_nom2_quot) ? pop_nom1 : pop_nom2;
        }


        auto index1 = pop_nom1 >> 8ul;
        auto index2 = pop_nom2 >> 8ul;

        uint8_t rem1 = main_buckets[b1].pop_read_body_by_index(index1);
        uint8_t rem2 = main_buckets[b2].pop_read_body_by_index(index2);

        return (rem1 <= rem2) ? pop_nom1 : pop_nom2;
    }

    bool screen_pop_nominees(uint64_t pop_nom1, uint64_t pop_nom2, size_t b1, size_t b2) const {
        bool BPC = (b1 == 263) && (b2 == 278);
        if (pop_nom1 == -1)
            return false;
        if (pop_nom2 == -1)
            return true;

        auto pop_nom1_spare_quot = pop_nom1 & MASK(quot_length);
        auto pop_nom2_spare_quot = pop_nom2 & MASK(quot_length);

        auto pop_nom1_quot = (PS_QUOT_SIZE - 1) - pop_nom1_spare_quot;
        auto pop_nom2_quot = (PS_QUOT_SIZE - 1) - pop_nom2_spare_quot;

        if (pop_nom1_quot != pop_nom2_quot) {
            return (pop_nom1_quot < pop_nom2_quot);
        }


        auto index1 = pop_nom1 >> 8ul;
        auto index2 = pop_nom2 >> 8ul;

        uint8_t rem1 = main_buckets[b1].pop_read_body_by_index(index1);
        uint8_t rem2 = main_buckets[b2].pop_read_body_by_index(index2);

        return (rem1 <= rem2);
    }

    /*item_key_t pop_test_for_the_validators(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) const{
        return pop_test_for_the_validators(pd_index);
    }
    item_key_t pop_test_for_the_validators(uint64_t pd_index) const {
        static int counter = 0;
        counter++;
//        auto valid_pop_tuple = validate_spare->get_min_element(pd_index);
//        uint8_t tp_q1 = get<0>(valid_pop_tuple);
//        uint8_t tp_rem1 = get<1>(valid_pop_tuple);
//        item_key_t item1 = {pd_index, static_cast<uint64_t>(flip_quot(tp_q1)), static_cast<uint64_t>(tp_rem1)};
//        auto item2 = psLog->get_pop_element(pd_index);

//        assert(item1.pd_index == item2.pd_index);
//        if (item1 == item2) {
//            return item2;
//        }

//        bool does_B_has_A_item = psLog->Find(item1);
//        bool does_A_has_B_item = validate_spare->Find(item2.pd_index, item2.quot, item2.rem);
//        bool does_A_has_B_item2 = validate_spare->Find(item2.pd_index, flip_quot(item2.quot), item2.rem);
//        bool both_contain_both = does_B_has_A_item && does_A_has_B_item;
//        bool is_item1_smaller = item1 < item2;
//
//        std::cout << "item1:\n " << item1 << std::endl;
//        std::cout << "item2:\n " << item2 << std::endl;
        assert(0);


    }
*/

    item_key_t pop_nom_to_item(uint64_t pop_nom, size_t bucket_index) const {
        if (pop_nom == static_cast<uint64_t>(-1))
            return {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        auto pop_nom1_spare_quot = pop_nom & MASK(quot_length);
        auto index = pop_nom >> 8ul;
        uint8_t rem1 = main_buckets[bucket_index].pop_read_body_by_index(index);

        return {0, static_cast<uint64_t>(pop_nom1_spare_quot), static_cast<uint64_t>(rem1)};
    }

    item_key_t get_pop_element(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) const {
        // static int counter = 0;
        // counter++;
        // auto psLog_pop_item = psLog->get_pop_element(pd_index);
        return get_pop_element(pd_index);
    }

    item_key_t pop(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) {
        auto pop_item = get_pop_element(pd_index, spare_quot, rem);
        if (pop_item.pd_index != -1)
            remove(pop_item);
        return pop_item;
    }

    item_key_t pop(uint64_t pd_index) {
        item_key_t pop_item = get_pop_element(pd_index);
        if (pop_item.pd_index != -1)
            remove(pop_item);
            
        return pop_item;
    }

    item_key_t get_pop_element(uint64_t pd_index) const {
        auto psLog_pop_item = psLog->get_pop_element(pd_index);

        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b1 = pd_index / batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);

        uint64_t pop_nom1 = get_pop_next_item_helper(rel_pd_index, 1, b1);
        uint64_t pop_nom2 = get_pop_next_item_helper(rel_pd_index, 0, b2);
        item_key_t nom1_item = pop_nom_to_item(pop_nom1, b1);
        item_key_t nom2_item = pop_nom_to_item(pop_nom2, b2);


        if ((pop_nom1 == static_cast<uint64_t>(-1)) && (pop_nom2 == static_cast<uint64_t>(-1))) {
            return pop_from_big_quots_set(pd_index);
        }

        bool should_use_primary = screen_pop_nominees(pop_nom1, pop_nom2, b1, b2);
        uint64_t pop_item = should_use_primary ? pop_nom1 : pop_nom2;
        auto b0 = should_use_primary ? b1 : b2;
        auto pop_item_spare_quot = pop_item & MASK(quot_length);
        assert(pop_item_spare_quot == psLog_pop_item.quot);
        auto pop_index = pop_item >> bits_per_item;
        auto pop_item_rem = main_buckets[b0].pop_read_body_by_index(pop_index);
        assert(pop_item_rem == psLog_pop_item.rem);

        return {pd_index, pop_item_spare_quot, pop_item_rem};
    }


    /**
     *
     * @param pd_index
     * @param spare_quot
     * @param rem
     * @return (spare_quot << 8ul) | (rem). [values of the popped element]
     */
    item_key_t get_pop_element_db(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) const {
        static int counter = 0;
        counter++;
        //        pop_test_for_the_validators(pd_index, spare_quot, rem);
        /*std::cout << "pop: " << counter << std::endl;
        std::cout << "\033[1;31mpd_index: \033[1;31m" << pd_index;
        std::cout << "\033[0m";

        auto valid_pop_tuple = validate_spare->get_min_element(pd_index);
        uint8_t v_pop_quot = get<0>(valid_pop_tuple);
        uint8_t v_pop_rem = get<1>(valid_pop_tuple);
*/

        auto psLog_pop_item = psLog->get_pop_element(pd_index);

        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b1 = pd_index / batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;
        assert(b1 < number_of_buckets);
        assert(b2 < number_of_buckets);
        /*std::cout << "\t\033[1;31mb1, b2:\t" << b1 << ", " << b2 << std::endl;
        std::cout << "\033[0m";*/

        uint64_t pop_nom1 = get_pop_next_item_helper(rel_pd_index, 1, b1);
        uint64_t pop_nom2 = get_pop_next_item_helper(rel_pd_index, 0, b2);

        if ((pop_nom1 == static_cast<uint64_t>(-1)) && (pop_nom2 == static_cast<uint64_t>(-1))) {
            return pop_from_big_quots_set(pd_index, spare_quot, rem);
        }

        /*std::cout << std::string(80, '=') << std::endl;
        std::cout << "pop_nom1: " << std::endl;
        std::cout << "index, spare_quot: \t" << (pop_nom1 >> 8ul) << "," << (pop_nom1 & 255) << std::endl;
        std::cout << "pop_nom2: " << std::endl;
        std::cout << "index, spare_quot: \t" << (pop_nom2 >> 8ul) << "," << (pop_nom2 & 255) << std::endl;
        std::cout << std::string(80, '=') << std::endl;
*/
        bool should_use_primary = screen_pop_nominees(pop_nom1, pop_nom2, b1, b2);
        uint64_t pop_item = should_use_primary ? pop_nom1 : pop_nom2;
        auto b0 = should_use_primary ? b1 : b2;


        // assert((pop_item == pop_nom1) || (pop_item == pop_nom2));

        if (pop_item == static_cast<uint64_t>(-1)) {
            return pop_from_big_quots_set(pd_index, spare_quot, rem);
        }

        auto pop_item_spare_quot = pop_item & MASK(quot_length);
        //        auto pop_item_quot = (PS_QUOT_SIZE - 1) - (pop_item_spare_quot);
        /*if (pop_item_spare_quot != psLog_pop_item.quot) {

            std::cout << std::string(80, '~') << std::endl;
            std::cout << "b1:          \t" << b1 << std::endl;
            std::cout << "b2:          \t" << b2 << std::endl;
            std::cout << "b1_capacity: \t" << capacity_vec.at(b1) << std::endl;
            std::cout << "b2_capacity: \t" << capacity_vec.at(b2) << std::endl;
            std::cout << "total_cap: \t" << capacity << std::endl;

            std::cout << std::string(80, '.') << std::endl;

            auto msg1 = (pop_nom1 == -1) ? "-1" : print_memory::str_word_LE(pop_nom1, GAP);
            std::cout << "pop_nom1:    \t" << msg1 << std::endl;

            auto msg2 = (pop_nom2 == -1) ? "-1" : print_memory::str_word_LE(pop_nom2, GAP);
            std::cout << "pop_nom2:    \t" << msg2 << std::endl;

            auto msg0 = (pop_item == -1) ? "-1" : print_memory::str_word_LE(pop_item, GAP);
            std::cout << "pop_item:    \t" << msg0 << std::endl;

            std::cout << std::string(80, '.') << std::endl;

            std::cout << "v_quot:      \t" << ((uint64_t) psLog_pop_item.quot) << std::endl;
            std::cout << "pd_index:    \t" << pd_index << std::endl;
            std::cout << "pd_rel:      \t" << rel_pd_index << std::endl;

            std::cout << std::endl;
            std::cout << "quot_att:    \t" << pop_item_quot << std::endl;

            // auto msg4 = (valid_pop_tuple == -1) ? "-1" : print_memory::str_word_LE(valid_pop_tuple, GAP);
            // std::cout << "vld_pop_tp:  \t" << msg4 << std::endl;

            bool pop_from_primary = (pop_item == pop_nom1);
            auto b0 = (pop_from_primary) ? b1 : b2;

            // std::cout << std::string(80, '=') << std::endl;
            main_buckets[b0].print_pd();
            q_buckets[b0].print_Q_list();
            // std::cout << std::string(80, '=') << std::endl;


            uint64_t junk1 = get_pop_next_item_helper(rel_pd_index, 1, b1);
            uint64_t junk2 = get_pop_next_item_helper(rel_pd_index, 0, b2);


            std::cout << std::string(80, '~') << std::endl;
            assert(0);
        }
*/
        assert(pop_item_spare_quot == psLog_pop_item.quot);

        // bool pop_from_primary = (pop_item == pop_nom1);
        auto pop_index = pop_item >> bits_per_item;
        auto pop_item_rem = main_buckets[b0].pop_read_body_by_index(pop_index);
        auto weird_att = main_buckets[b0].pop_read_body_by_index(capacity_vec.at(b0) - pop_index);


        if (pop_item_rem != psLog_pop_item.rem) {


            std::cout << std::string(80, '~') << std::endl;
            std::cout << std::string(80, '~') << std::endl;
            std::cout << std::string(80, '~') << std::endl;

            std::cout << "b1:          \t" << b1 << std::endl;
            std::cout << "b2:          \t" << b2 << std::endl;
            std::cout << "b1_capacity: \t" << capacity_vec.at(b1) << std::endl;
            std::cout << "b2_capacity: \t" << capacity_vec.at(b2) << std::endl;
            std::cout << "total_cap: \t" << capacity << std::endl;

            std::cout << std::string(80, '.') << std::endl;

            auto msg1 = (pop_nom1 == -1) ? "-1" : print_memory::str_word_LE(pop_nom1, GAP);
            std::cout << "pop_nom1:    \t" << msg1 << std::endl;

            auto msg2 = (pop_nom2 == -1) ? "-1" : print_memory::str_word_LE(pop_nom2, GAP);
            std::cout << "pop_nom2:    \t" << msg2 << std::endl;

            auto msg0 = (pop_item == -1) ? "-1" : print_memory::str_word_LE(pop_item, GAP);
            std::cout << "pop_item:    \t" << msg0 << std::endl;

            std::cout << std::string(80, '.') << std::endl;

            std::cout << "v_quot:      \t" << ((uint64_t) psLog_pop_item.quot) << std::endl;
            std::cout << "v_rem:       \t" << ((uint64_t) psLog_pop_item.rem) << std::endl;
            std::cout << "pd_index:    \t" << pd_index << std::endl;
            std::cout << "pd_rel:      \t" << rel_pd_index << std::endl;

            std::cout << std::endl;
            std::cout << "quot_att:    \t" << psLog_pop_item.quot << std::endl;
            std::cout << "rem_att:     \t" << ((uint64_t) pop_item_rem) << std::endl;

            // auto msg4 = (valid_pop_tuple == -1) ? "-1" : print_memory::str_word_LE(valid_pop_tuple, GAP);
            // std::cout << "vld_pop_tp:  \t" << msg4 << std::endl;

            // bool pop_from_primary = (pop_item == pop_nom1);
            // auto b0 = (pop_from_primary) ? b1 : b2;


            std::cout << std::string(80, '-') << std::endl;
            std::cout << std::string(80, '-') << std::endl;
            std::cout << "pop_item:    \t" << print_memory::str_word_LE(pop_item, GAP) << std::endl;
            std::cout << "pd_rel:      \t" << rel_pd_index << std::endl;
            std::cout << "quot:        \t" << ((uint64_t) psLog_pop_item.quot) << std::endl;
            std::cout << "sp_quot:     \t" << ((uint64_t) pop_item_spare_quot) << std::endl;
            std::cout << "is_primary:  \t" << should_use_primary << std::endl;
            std::cout << std::endl;
            std::cout << "v_rem:       \t" << ((uint64_t) psLog_pop_item.rem) << std::endl;
            std::cout << "rem_att:     \t" << ((uint64_t) pop_item_rem) << std::endl;
            std::cout << "index_att:   \t" << ((uint64_t) pop_index) << std::endl;


            uint64_t pd_pop_mask = main_buckets[b0].pop_get_mask(rel_pd_index, should_use_primary);
            std::cout << "pd_pop_mask: \t" << print_memory::str_word_LE(pd_pop_mask, GAP) << std::endl;

            std::cout << std::string(80, '*') << std::endl;
            std::cout << "Primary PD: " << std::endl;
            main_buckets[b0].print_pd();
            std::cout << "Q list screened: " << std::endl;
            q_buckets[b0].print_Q_list_screened(capacity_vec.at(b0), psLog_pop_item.quot);
            std::cout << "Q list: " << std::endl;
            q_buckets[b0].print_Q_list(capacity_vec.at(b0));
            std::cout << "All remainders: " << std::endl;
            main_buckets[b0].print_all_remainders();
            std::cout << "screened by pd_index remainders: " << std::endl;
            main_buckets[b0].print_remainders_by_pd_index(rel_pd_index);
            std::cout << std::string(80, '*') << std::endl;
            std::cout << std::string(80, '*') << std::endl;
            std::cout << std::string(80, '*') << std::endl;
            std::cout << "secondary PD: " << std::endl;
            size_t alt_b = b0 ^ b1 ^ b2;
            assert((alt_b == b1) || (alt_b == b2));
            main_buckets[alt_b].print_pd();
            std::cout << "Q list screened: " << std::endl;
            q_buckets[alt_b].print_Q_list_screened(capacity_vec.at(alt_b), psLog_pop_item.quot);
            std::cout << "Q list: " << std::endl;
            q_buckets[alt_b].print_Q_list(capacity_vec.at(alt_b));
            std::cout << "All remainders: " << std::endl;
            main_buckets[alt_b].print_all_remainders();
            std::cout << "screened by pd_index remainders: " << std::endl;
            main_buckets[alt_b].print_remainders_by_pd_index(rel_pd_index);
            std::cout << std::string(80, '*') << std::endl;


            uint64_t junk1 = get_pop_next_item_helper(rel_pd_index, 1, b1);
            uint64_t junk2 = get_pop_next_item_helper(rel_pd_index, 0, b2);

            std::cout << std::string(80, '~') << std::endl;
            assert(0);
        }
        assert(pop_item_rem == psLog_pop_item.rem);

        //        remove(pd_index, pop_item_spare_quot, pop_item_rem);
        return psLog_pop_item;
    }


    item_key_t get_pop_element_db_old(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) {
        //        auto v_pop_tuple = validate_spare->pop(pd_index);
        //        assert(v_pop_tuple == valid_pop_tuple);
        //        return pop_from_bucket_with_smaller_pop_nom(rel_pd_index, pop_item, b0, pop_from_primary);


        /* auto pop_nom1_quot = (pop_nom1 & MASK(quot_length));
        auto pop_nom2_quot = pop_nom2 & MASK(quot_length);

        if (pop_nom1 == -1) {
            pop_nom1_quot = 0;
        }
        if (pop_nom2 == -1) {
            pop_nom2_quot = 0;
        }
        //        bool c1 = (spare_quot >= pop_nom1_quot);
        //        bool c2 = spare_quot >= pop_nom2_quot;

        if (pop_nom1_quot > pop_nom2_quot) {
            assert(pop_nom1 != -1);
            assert(pop_nom1_quot <= spare_quot);
            // deleted element quot [flip(spare_quot)] is bigger then pop element quot (pop_nom1_quot).
            // auto res = pop_from_bucket_with_smaller_pop_nom(rel_pd_index, pop_nom1, b1, 1);
            return pop_from_bucket_with_smaller_pop_nom(rel_pd_index, pop_nom1, b1, 1);
        } else if (pop_nom2_quot > pop_nom1_quot) {
            assert(pop_nom2 != -1);
            return pop_from_bucket_with_smaller_pop_nom(rel_pd_index, pop_nom2, b2, 0);
        }
        assert(pop_nom1 != -1);
        assert(pop_nom2 != -1);

        // read remainder and then decide from which bucket to pop.
        auto index1 = pop_nom1 >> 8ul;
        auto index2 = pop_nom2 >> 8ul;

        uint8_t rem1 = main_buckets[b1].pop_read_body_by_index(index1);
        uint8_t rem2 = main_buckets[b2].pop_read_body_by_index(index2);

        if (rem1 < rem2) {
            return pop_from_bucket_with_smaller_pop_nom(rel_pd_index, pop_nom1, b1, 1);
        }
        return pop_from_bucket_with_smaller_pop_nom(rel_pd_index, pop_nom2, b2, 0); */
        return {-1, 42, 1024};
    }

    /**
     * @brief The actual pop operation on the correct bucket. returns qr denoting the pop element spare_quot and rem.
     *
     * @param rel_pd_index
     * @param popnom
     * @param bucket_index
     * @param is_primary for validation.
     * @return uint64_t
     *
     */

    auto pop_from_bucket_with_smaller_pop_nom(uint8_t rel_pd_index, uint64_t popnom, size_t bucket_index,
                                              bool is_primary) -> uint64_t {
        auto spare_quot = popnom & MASK(quot_length);
        auto index = popnom >> 8ul;
        uint8_t removed_rem = main_buckets[bucket_index].pop_remove_by_index(rel_pd_index, index);
        q_buckets[bucket_index].conditional_remove_simple_shift_helper(index, 42);// remove_by_index. NOT conditional.
        return (spare_quot << bits_per_item) | removed_rem;
    }

    item_key_t pop_from_big_quots_set(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) const {
        if (this->big_quots_set.size() == 0) {
            return {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        }

        assert(0);
        return {42, 42, 42};
    }

    item_key_t pop_from_big_quots_set(uint64_t pd_index) const {
        if (this->big_quots_set.size() == 0) {
            return {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        }

        assert(0);
        return {42, 42, 42};
    }

    bool validate_capacity_of_bucket(size_t bucket_index) {
        auto temp_capacity = main_buckets[bucket_index].get_capacity();
        auto v_capacity = capacity_vec.at(bucket_index);
        if (temp_capacity != v_capacity) {
            std::cout << "temp_capacity: " << temp_capacity << std::endl;
            std::cout << "v_capacity: " << v_capacity << std::endl;
            //            assert(0);
        }
        assert(temp_capacity == v_capacity);
        return true;
    }

    size_t getNumberOfBuckets() const {
        return number_of_buckets;
    }


    //    void print_both_joint_bucket_matching_pd_index(item_key_t itemKey) {
    //
    //    }
    void print_both_joint_bucket_matching_pd_index(size_t pd_index) {
        const uint32_t b1 = pd_index / batch_size;
        uint64_t rel_pd_index = pd_index % batch_size;
        const uint32_t b2 = (b1 + 1 + rel_pd_index) % number_of_buckets;

        std::cout << std::string(80, '=') << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        print_joint_bucket(b1);
        std::cout << std::string(80, '~') << std::endl;
        std::cout << std::string(80, '~') << std::endl;

        print_joint_bucket(b2);
        std::cout << std::string(80, '=') << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }

    void print_joint_bucket(size_t bucket_index) {
        Joint_bucket::print_JBucket(&main_buckets[bucket_index], &q_buckets[bucket_index]);
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
