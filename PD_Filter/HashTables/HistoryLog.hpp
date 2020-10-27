

#ifndef FILTERS_HISTORYLOG_HPP
#define FILTERS_HISTORYLOG_HPP

//#include "../../hashutil.h"
// #include "../L2_pd/twoDimPD.hpp"
//#include "../basic_function_util.h"
//#include "../macros.h"
//#include "Spare_Validator.hpp"
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>


enum operations {
    Add_op = 1,
    Remove_op = -1,
    Pop_op = -2
};


//typedef std::tuple<int64_t, uint8_t> qr_key_t;
//typedef std::tuple<size_t, operations> log_vec_key_t;
//typedef unordered_map<qr_key_t, int> my_map_t;
//typedef vector<log_vec_key_t> log_vec_t;
//typedef std::tuple<uint64_t, uint64_t, uint64_t> item_key_t;
//typedef std::tuple<size_t, uint64_t, uint64_t> item_val_t;

struct operation_t {
    size_t time_stamp;
    operations kind;
};

struct item_key_t {
    bool operator<(const item_key_t &rhs) const;

    bool operator>(const item_key_t &rhs) const;

    bool operator<=(const item_key_t &rhs) const;

    bool operator>=(const item_key_t &rhs) const;

    bool operator==(const item_key_t &rhs) const;

    bool operator!=(const item_key_t &rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const item_key_t &key);


    uint64_t pd_index;
    uint64_t quot;
    uint64_t rem;
};

constexpr item_key_t emptyItem = {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};

void flip_quot(item_key_t *item);
//bool test_for_equality(const item_key_t &item1, const item_key_t &item2);


struct item_val_t {
    int reps;
    std::vector<operation_t> history_log_vec;
};


namespace all_map {

    struct key_hash : public std::unary_function<item_key_t, std::size_t> {
        std::size_t operator()(const item_key_t &k) const {
            auto res = (k.pd_index << 16ul) | (k.quot << 8ul) | (k.rem);
            return res;
        }
    };

    struct key_equal : public std::binary_function<item_key_t, item_key_t, bool> {
        bool operator()(const item_key_t &v0, const item_key_t &v1) const {
            return v0 == v1;
        }
    };

    typedef std::unordered_map<const item_key_t, item_val_t, key_hash, key_equal> umap_t;
    typedef std::unordered_map<const item_key_t, bool, key_hash, key_equal> umapset_t;
    typedef std::unordered_map<const item_key_t, int, key_hash, key_equal> umap_multiset_t;
    typedef std::vector<const item_key_t> vector_t;

    typedef std::unordered_set<const item_key_t, key_hash, key_equal> uset_t;
};// namespace all_map

namespace bucket_map {

    struct key_hash : public std::unary_function<item_key_t, std::size_t> {
        std::size_t operator()(const item_key_t &k) const {
            auto res = (k.pd_index << 16ul) | (k.quot << 8ul) | (k.rem);
            return res;
        }
    };

    struct key_equal : public std::binary_function<item_key_t, item_key_t, bool> {
        bool operator()(const item_key_t &v0, const item_key_t &v1) const {
            return v0 == v1;
        }
    };

    typedef std::unordered_map<const item_key_t, item_val_t, key_hash, key_equal> bucket_umap_t;

};// namespace bucket_map


class HistoryLog {
public:
    const size_t number_of_buckets;
    size_t capacity{0};
    size_t time_stamp = 0;
    const bool using_flipped_quot;
    all_map::umap_t all_history;
    bucket_map::bucket_umap_t *bucket_umap_array;
    size_t *bucket_array_time_stamp;

    HistoryLog(size_t number_of_buckets_in_l1, bool should_use_flipped_quot) : number_of_buckets(
                                                                                       number_of_buckets_in_l1),
                                                                               using_flipped_quot(should_use_flipped_quot) {
        bucket_umap_array = new bucket_map::bucket_umap_t[number_of_buckets_in_l1];
        bucket_array_time_stamp = new size_t[number_of_buckets_in_l1];
    }

    ~HistoryLog() {
        delete[] bucket_umap_array;
        delete[] bucket_array_time_stamp;
    }

    //    validate_quot

    void add_to_all_history(size_t pd_index, int64_t quot, uint8_t rem) {
        item_key_t temp_key = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        bool is_new_item = (all_history.count(temp_key) == 0);
        if (is_new_item) {
            operation_t vec_val = {time_stamp, Add_op};
            std::vector<operation_t> new_vec = {vec_val};
            item_val_t new_val = {1, new_vec};
            all_history[temp_key] = new_val;
        } else {
            auto *count_pointer = &all_history[temp_key].reps;
            *count_pointer += 1;

            operation_t vec_val = {time_stamp, Add_op};
            std::vector<operation_t> *vec = &all_history[temp_key].history_log_vec;
            vec->emplace_back(vec_val);
        }
    }

    void add_to_bucket_history(size_t pd_index, int64_t quot, uint8_t rem) {
        auto relative_time_stamp = bucket_array_time_stamp[pd_index];
        bucket_array_time_stamp[pd_index]++;

        item_key_t temp_key = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        bucket_map::bucket_umap_t *temp_bmap = &bucket_umap_array[pd_index];
        bool is_new_item = (temp_bmap->count(temp_key) == 0);
        if (is_new_item) {
            operation_t vec_val = {relative_time_stamp, Add_op};
            std::vector<operation_t> new_vec = {vec_val};
            item_val_t new_val = {1, new_vec};
            auto temp_pair = std::make_pair(temp_key, new_val);
            temp_bmap->insert(temp_pair);
            //            temp_bmap->at(temp_key) = new_val;
        } else {
            auto *count_pointer = &temp_bmap->at(temp_key).reps;
            *count_pointer += 1;

            operation_t vec_val = {relative_time_stamp, Add_op};
            std::vector<operation_t> *vec = &temp_bmap->at(temp_key).history_log_vec;
            vec->emplace_back(vec_val);
        }
    }

    void Add(size_t pd_index, int64_t quot, uint8_t rem) {
        capacity++;
        add_to_all_history(pd_index, quot, rem);
        add_to_bucket_history(pd_index, quot, rem);

        time_stamp++;
        auto find_res = Find(pd_index, quot, rem);
        assert(find_res);
        assert(Find(pd_index, quot, rem));
    }

    int Find(item_key_t temp_key) const {
        if (temp_key.pd_index == static_cast<uint64_t>(-1))
            return 0;
        bool all_map_res = false;
        if (all_history.count(temp_key)) {
            all_map_res = all_history.at(temp_key).reps;
        }
        bool bucket_map_res = false;
        auto temp_map = &bucket_umap_array[temp_key.pd_index];
        if (temp_map->count(temp_key)) {
            bucket_map_res = temp_map->at(temp_key).reps;
        }
        assert(bucket_map_res == all_map_res);

        return all_map_res;
    }


    int Find(size_t pd_index, int64_t quot, uint8_t rem) const {
        item_key_t temp_key = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        return Find(temp_key);
    }

    void remove_from_all_history(size_t pd_index, int64_t quot, uint8_t rem, operations op_kind) {
        item_key_t temp_key = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        assert(all_history.count(temp_key));
        /*
        bool is_new_item = (all_history.reps(temp_key) == 0);
        if (is_new_item) {
            operation_t vec_val = {time_stamp, Add_op};
            vector<operation_t> new_vec = {vec_val};
            item_val_t new_val = {1, new_vec};
            all_history[temp_key] = new_val;
        } else {
*/

        auto *count_pointer = &all_history[temp_key].reps;
        *count_pointer -= 1;

        operation_t vec_val = {time_stamp, op_kind};
        std::vector<operation_t> *vec = &all_history[temp_key].history_log_vec;
        vec->emplace_back(vec_val);
    }

    void remove_from_bucket_history(size_t pd_index, int64_t quot, uint8_t rem, operations op_kind) {
        auto relative_time_stamp = bucket_array_time_stamp[pd_index];
        bucket_array_time_stamp[pd_index]++;

        item_key_t temp_key = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        bucket_map::bucket_umap_t *temp_bmap = &bucket_umap_array[pd_index];
        assert(temp_bmap->count(temp_key));
        auto *count_pointer = &temp_bmap->at(temp_key).reps;
        *count_pointer -= 1;

        operation_t vec_val = {relative_time_stamp, op_kind};
        std::vector<operation_t> *vec = &temp_bmap->at(temp_key).history_log_vec;
        vec->emplace_back(vec_val);
    }

    void Remove(item_key_t temp_key, operations op_kind = operations::Remove_op) {
        Remove(temp_key.pd_index, temp_key.quot, temp_key.rem, op_kind);
    }

    void Remove(size_t pd_index, int64_t quot, uint8_t rem, operations op_kind = operations::Remove_op) {
        assert(Find(pd_index, quot, rem));
        assert(capacity);
        capacity--;

        remove_from_all_history(pd_index, quot, rem, op_kind);
        remove_from_bucket_history(pd_index, quot, rem, op_kind);

        time_stamp++;
    }

    bool is_first_better_for_pop(int64_t q1, uint8_t r1, int64_t q2, uint8_t r2) const {
        if (using_flipped_quot) {
            return (q1 != q2) ? q1 > q2 : r1 < r2;
        }
        return (q1 != q2) ? q1 < q2 : r1 < r2;
    }

    /**
     * No popping is performed.
     * @param pd_index
     * @return
     */
    auto get_pop_element(size_t pd_index) const -> item_key_t {
        int64_t res_quot = 0;
        uint8_t res_rem = 0;

        bool pd_index_is_empty = true;
        auto temp_map = bucket_umap_array[pd_index];
        for (const auto &el : temp_map) {
            if (el.second.reps) {
                if (pd_index_is_empty) {
                    res_quot = el.first.quot;
                    res_rem = el.first.rem;
                    pd_index_is_empty = false;
                }
                auto temp_q = el.first.quot;
                auto temp_rem = el.first.rem;
                if (is_first_better_for_pop(temp_q, temp_rem, res_quot, res_rem)) {
                    res_quot = temp_q;
                    res_rem = temp_rem;
                }
            }
        }

        if (pd_index_is_empty)
            return {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        return {pd_index, static_cast<uint64_t>(res_quot), static_cast<uint64_t>(res_rem)};
    }

    void print_bucket_log_assuming_no_deletions(uint64_t pd_index, uint8_t quot, uint8_t rem) const {
        std::cout << __FILE__ << __LINE__ << std::endl;
    }

    void print_element_history(uint64_t pd_index, uint8_t quot, uint8_t rem) const {
        item_key_t itemKey = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        if (all_history.count(itemKey) == 0) {
            std::cout << "The element was not inserted to this HistoryLog " << std::endl;
        }
        auto val = all_history.at(itemKey);
        auto item_vec = val.history_log_vec;
        std::cout << std::string(80, '=') << std::endl;
        std::cout << "current time stamp is: " << time_stamp << std::endl;
        std::cout << "time_stamp\t\t Kind " << std::endl;
        for (const auto &op : item_vec) {
            std::cout << op.time_stamp << "\t\t" << op.kind << std::endl;
        }
        std::cout << std::string(80, '=') << std::endl;
    }

    void print_bucket_log(uint64_t pd_index) const {
        assert(pd_index < number_of_buckets);
        bucket_map::bucket_umap_t bucket_map = bucket_umap_array[pd_index];
        for (const auto &temp_pair : bucket_map) {
            std::cout << std::string(80, '*') << std::endl;
            std::cout << "quot: \t" << ((uint64_t) temp_pair.first.quot) << std::endl;
            std::cout << "rem: \t" << ((uint64_t) temp_pair.first.rem) << std::endl;
            print_element_history_log_vec(pd_index, temp_pair.first.quot, temp_pair.first.rem);
        }
    }

    void print_element_history_log_vec(uint64_t pd_index, uint8_t quot, uint8_t rem) const {
        item_key_t itemKey = {pd_index, static_cast<uint64_t>(quot), static_cast<uint64_t>(rem)};
        if (all_history.count(itemKey) == 0) {
            std::cout << "The element was not inserted to this HistoryLog " << std::endl;
        }
        auto val = all_history.at(itemKey);
        auto item_vec = val.history_log_vec;
        std::cout << std::string(32, '.') << std::endl;
        std::cout << "time_stamp\t\t Kind " << std::endl;
        size_t c = 0;
        for (const auto &op : item_vec) {
            std::cout << c++ << ")\t" << op.time_stamp << "\t\t" << op.kind << std::endl;
        }
        std::cout << "current reps: \t" << val.reps << std::endl;
        std::cout << std::string(32, '.') << std::endl;
    }


    auto pop(size_t pd_index) -> item_key_t {
        auto pop_res = get_pop_element(pd_index);
        if (pop_res.pd_index != static_cast<uint64_t>(-1)) {
            assert(pop_res.pd_index == pd_index);
            Remove(pop_res, operations::Pop_op);
        }
        return pop_res;
    }


private:
    static bool cmp_helper(int64_t q1, uint8_t r1, int64_t q2, uint8_t r2) {
        return (q1 != q2) ? q1 < q2 : r1 < r2;
    }

    auto get_min_element(size_t pd_index) const -> item_key_t {
        int64_t min_quot = 0x1111'1111'1111'1111;
        uint8_t min_rem = 255;

        auto temp_map = bucket_umap_array[pd_index];
        for (const auto &el : temp_map) {
            if (el.second.reps) {
                auto temp_q = el.first.quot;
                auto temp_rem = el.first.rem;
                if (cmp_helper(temp_q, temp_rem, min_quot, min_rem)) {
                    min_quot = temp_q;
                    min_rem = temp_rem;
                }
            }
        }

        return {pd_index, static_cast<uint64_t>(min_quot), static_cast<uint64_t>(min_rem)};
    }

    auto get_max_element(size_t pd_index) const -> item_key_t {
        int64_t max_quot = 0;
        uint8_t max_rem = 0;

        bool pd_index_is_empty = true;
        auto temp_map = bucket_umap_array[pd_index];
        for (const auto &el : temp_map) {
            if (el.second.reps) {
                pd_index_is_empty = false;
                auto temp_q = el.first.quot;
                auto temp_rem = el.first.rem;
                if (cmp_helper(max_quot, max_rem, temp_q, temp_rem)) {
                    max_quot = temp_q;
                    max_rem = temp_rem;
                }
            }
        }

        if (pd_index_is_empty)
            return {static_cast<uint64_t>(-1), static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};
        return {pd_index, static_cast<uint64_t>(max_quot), static_cast<uint64_t>(max_rem)};
    }

    auto pop_min(size_t pd_index) -> item_key_t {
        auto pop_res = get_min_element(pd_index);
        Remove(pd_index, pop_res.quot, pop_res.rem, operations::Pop_op);
        return pop_res;
    }

    auto pop_max(size_t pd_index) -> item_key_t {
        auto pop_res = get_max_element(pd_index);
        Remove(pd_index, pop_res.quot, pop_res.rem, operations::Pop_op);
        return pop_res;
    }


    auto only_get_pop_element_min(size_t pd_index) const -> item_key_t {
        return get_min_element(pd_index);
    }

    auto only_get_pop_element_max(size_t pd_index) const -> item_key_t {
        return get_max_element(pd_index);
    }

    //    auto get_bucket_history(size_t pd_index) const -> vector<log_vec_key_t> {
    //        return pd_log_vec_array[pd_index];
    //    }
    //
    //    auto get_element_history(size_t pd_index, int64_t quot, uint8_t rem) const -> vector<log_vec_key_t> {
    //        auto temp_history = pd_log_vec_array[pd_index];
    //    }
};

inline auto spare_pop_res_to_item_key_t(uint64_t pop_res) -> item_key_t {
    auto pop_rem = pop_res & 255;
    auto pop_spare_quot = (pop_res >> 8ul);
    assert(pop_spare_quot <= 25);
    auto pop_quot = 24 - pop_spare_quot;
    return {0, pop_quot, pop_rem};
}

const item_key_t rand_item_key_simple(item_key_t lim_key);

const item_key_t rand_item_key_simple(uint64_t max_pd_index, uint64_t max_quot = 25, uint64_t max_rem = 255);

#endif//FILTERS_HISTORYLOG_HPP
