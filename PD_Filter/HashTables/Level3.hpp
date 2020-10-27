//
// Created by tomer on 8/18/20.
//
/* This implementation rely on
 * 1) Every element in the hashTable consists of three parts (msb to lsb) pd_index, quot, rem.
 * 2) quot is an integer in range [0,51). More specificity, quot != 63.
 * 3) number of bits for rem is 8. (not a must).
 * 4) The empty slot can be seen is equal to (63 << 8). 
 empty slot in a way tha */
#ifndef FILTERS_LEVEL3_HPP
#define FILTERS_LEVEL3_HPP


//#include "../../hashutil.h"
//#include "../basic_function_util.h"
//#include "../macros.h"
// #include "../L2_pd/twoDimPD.hpp"
//#include "Spare_Validator.hpp"
#include "HistoryLog.hpp"
#include <algorithm>
#include <vector>
//#include <unordered_set>

class Level3 {
public:
    size_t capacity{0};
    // const size_t number_of_buckets;
    // const size_t max_spare_capacity;

    // MainBucket<bucket_capacity, batch_size, bits_per_item> *main_buckets;
    // Quotients<bucket_capacity, batch_size, quot_length> *q_buckets;

    // unordered_set<uint64_t> big_quots_set;
    vector<item_key_t> elements;

    explicit Level3() {
        // elements.resize(32);
    }

    // virtual ~Level3() {}


    auto find(item_key_t itemKey) const -> bool {
        return std::find(elements.begin(), elements.end(), itemKey) != elements.end();
    }

    auto find(uint64_t pd_index, uint8_t quot, uint8_t rem) const -> bool {
        item_key_t itemKey = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        return find(itemKey);
    }


    void insert(item_key_t itemKey) {
        capacity++;
        elements.insert(std::upper_bound(elements.begin(), elements.end(), itemKey), itemKey);
        assert(find(itemKey));
    }


    void insert(uint64_t pd_index, uint8_t quot, uint8_t rem) {
        item_key_t itemKey = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        insert(itemKey);
    }


    void remove(item_key_t itemKey) {
        auto it_res = std::find(elements.begin(), elements.end(), itemKey);
        assert(it_res != elements.end());
        capacity--;
        elements.erase(it_res, it_res + 1);
    }

    void remove(uint64_t pd_index, uint8_t quot, uint8_t rem) {
        item_key_t itemKey = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        remove(itemKey);
    }

    item_key_t get_pop_element(uint64_t pd_index) const {
        item_key_t itemKey = {pd_index, 15, 256};
        item_key_t itemKey2 = {pd_index, 0, 0};
        auto it_res = std::upper_bound(elements.begin(), elements.end(), itemKey);
        auto it_res2 = std::upper_bound(elements.begin(), elements.end(), itemKey2);

        item_key_t pop_item = {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        item_key_t pop_item2 = {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        if (it_res != elements.end()) {
            pop_item = it_res[0];
        }

        if (it_res2 != elements.end()) {
            pop_item2 = it_res2[0];
        }

        assert(pop_item == pop_item2);
        return pop_item;
    }

    item_key_t pop(uint64_t pd_index) {
        item_key_t pop_item = get_pop_element(pd_index);
        if (pop_item.pd_index != static_cast<uint64_t>(-1))
            remove(pop_item);

        return pop_item;
    }
};
#endif//FILTERS_LEVEL3_HPP
