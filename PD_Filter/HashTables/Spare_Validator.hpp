//
// Created by tomer on 8/18/20.
//
/** This a an attempt to implement hash table that supports:
 *  Insertions.
 *  Membership queries, and also some from of successor query.
 *  Deletions (while maintaining the history of the hash_table)
 *  
 * Each element x in the hash table has several fields:
 * @pd_index. Denoting the index of the pd in which x was stored in the first level.
 * @quot.
 * @rem.
 * 
 * We would like to be able to compute successor (or minimum) of all elements with the same pd_index according to lexicographic order of
 * (quot, rem). 
 * In other words, given a pd_index, return an element y in the hash table where:
 * for any element z in the hash table: one of the following is true:
 * 1) z_quot > y_quot  
 * 2) z_quot = y_quot AND z_rem >= y_rem.
 */
#ifndef FILTERS_SPARE_VALIDATOR_HPP
#define FILTERS_SPARE_VALIDATOR_HPP


#include "../../hashutil.h"
#include "../L2_pd/twoDimPD.hpp"
#include "../basic_function_util.h"
#include "../macros.h"
#include <unordered_set>
#include <vector>

namespace SV {
    enum operations {
        Add_op = 1,
        Remove_op = -1,
        Pop_op = -2
    };
}

typedef std::tuple<int64_t, uint8_t> qr_key_t;
typedef std::tuple<size_t, SV::operations> log_vec_key_t;
//typedef unordered_map<qr_key_t, int> my_map_t;
typedef vector<log_vec_key_t> log_vec_t;

namespace qr_map {

    // typedef std::tuple<int, char, char> key;

    struct key_hash : public std::unary_function<qr_key_t, std::size_t> {
        std::size_t operator()(const qr_key_t &k) const {
            auto res = (std::get<0>(k) << 8ul) | std::get<1>(k);
            // res = (res << 1ul) | std::get<2>(k);
            return res;
        }
    };

    struct key_equal : public std::binary_function<qr_key_t, qr_key_t, bool> {
        bool operator()(const qr_key_t &v0, const qr_key_t &v1) const {
            return (
                    std::get<0>(v0) == std::get<0>(v1) &&
                    std::get<1>(v0) == std::get<1>(v1));
            // std::get<2>(v0) == std::get<2>(v1));
        }
    };


    typedef std::unordered_map<const qr_key_t, size_t, key_hash, key_equal> map_t;

}// namespace qr_map


class Spare_Validator {
public:
    size_t capacity{0};
    size_t time_stamp = 0;
    //    const size_t max_spare_capacity;
    qr_map::map_t *pd_map_array;
    //    my_map_t *pd_map_array;
    log_vec_t *pd_log_vec_array;

    explicit Spare_Validator(size_t number_of_buckets_in_l1) {
        pd_map_array = new qr_map::map_t[number_of_buckets_in_l1];
        pd_log_vec_array = new log_vec_t[number_of_buckets_in_l1];
    }

    ~Spare_Validator() {
        delete[] pd_map_array;
        delete[] pd_log_vec_array;
    }

    static auto wrap_count(const qr_map::map_t *temp_map, const qr_key_t &temp_key) -> int {
        if (temp_map->count(temp_key))
            return temp_map->at(temp_key);
        return 0;
    }


    void Add(size_t pd_index, int64_t quot, uint8_t rem) {
        capacity++;
        auto temp_log_vec = &pd_log_vec_array[pd_index];
        qr_map::map_t *temp_map = &pd_map_array[pd_index];
        qr_key_t temp_key = std::make_tuple(quot, rem);
        int count_res = wrap_count(temp_map, temp_key);
        temp_map->operator[](temp_key) += 1;
        auto after_count = wrap_count(temp_map, temp_key);
        assert(after_count == count_res + 1);

        //        if (count_res == 0) {
        //            temp_map->insert({temp_key, 1});
        ////            temp_map[temp_key] = 1;
        //        } else {
        //            temp_map->operator[](temp_key) += 1;
        //            auto after_count = wrap_count(temp_map, temp_key);
        //            assert(after_count == count_res + 1);
        //        }
        //        auto count_res2 = temp_map.reps(temp_key);

        log_vec_key_t temp_log_vec_key = std::make_tuple(time_stamp, SV::operations::Add_op);
        temp_log_vec->push_back(temp_log_vec_key);
        time_stamp++;

        auto find_res = Find(pd_index, quot, rem);
        assert(find_res);
        assert(Find(pd_index, quot, rem));
    }

    int Find(size_t pd_index, int64_t quot, uint8_t rem) const {
        auto temp_map = &pd_map_array[pd_index];
        qr_key_t temp_key = std::make_tuple(quot, rem);
        return wrap_count(temp_map, temp_key);
    }

    void Remove(size_t pd_index, int64_t quot, uint8_t rem, SV::operations op_kind = SV::operations::Remove_op) {
        assert(Find(pd_index, quot, rem));
        assert(capacity);

        capacity--;
        auto temp_log_vec = &pd_log_vec_array[pd_index];
        auto temp_map = &pd_map_array[pd_index];
        qr_key_t temp_key = std::make_tuple(quot, rem);
        int count_res = wrap_count(temp_map, temp_key);
        assert(count_res);
        temp_map->operator[](temp_key) -= 1;

        log_vec_key_t temp_log_vec_key = std::make_tuple(time_stamp, op_kind);
        temp_log_vec->push_back(temp_log_vec_key);
        time_stamp++;
    }

    static bool cmp_helper(int64_t q1, uint8_t r1, int64_t q2, uint8_t r2) {
        return (q1 != q2) ? q1 < q2 : r1 < r2;
    }

    auto get_min_element(size_t pd_index) const -> qr_key_t {
        int64_t min_quot = 0x1111'1111'1111'1111;
        uint8_t min_rem = 255;

        auto temp_map = pd_map_array[pd_index];
        //        auto it = temp_map.begin();
        for (const auto &el : temp_map) {
            if (el.second) {
                auto temp_key = el.first;
                auto temp_q = std::get<0>(temp_key);
                auto temp_rem = std::get<1>(temp_key);
                if (cmp_helper(temp_q, temp_rem, min_quot, min_rem)) {
                    min_quot = temp_q;
                    min_rem = temp_rem;
                }
            }
        }

        return std::make_tuple(min_quot, min_rem);
    }


    auto pop(size_t pd_index) -> qr_key_t {
        auto pop_res = get_min_element(pd_index);
        auto quot = get<0>(pop_res);
        auto rem = get<1>(pop_res);
        if (quot <= 25)
            Remove(pd_index, quot, rem, SV::operations::Pop_op);

        return pop_res;


    }
};


#endif//FILTERS_SPARE_VALIDATOR_HPP
