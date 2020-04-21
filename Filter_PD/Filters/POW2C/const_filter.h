//
// Created by tomer on 11/9/19.
//

#ifndef CLION_CODE_CONST_FILTER_H
#define CLION_CODE_CONST_FILTER_H

#include "../../PD/Constant_size_PD/const_PD.h"
#include "../../Hash/Hash.h"
#include "../../Hash/Permutation.h"

using namespace PD_Hash;

class const_filter {
    vector<const_PD> pd_vec;
    size_t size, capacity;
    size_t fp_size, interval_length, single_pd_capacity;
    size_t quotient_length, pd_index_length;
    Hash h;
    Permutation perm;
public:
    const_filter(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size);

    bool lookup(string *s);

    bool lookup(uint32_t s);

    void insert(string *s);

    void insert(uint32_t s);

    void remove(string *s);

    void remove(uint32_t s);

    inline void split(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
        *remainder = hash_index & MASK(fp_size);
        hash_index >>= fp_size;
        *quotient = hash_index & MASK(quotient_length);
        hash_index >>= quotient_length;
        *pd_index = hash_index & MASK(pd_index_length);
    }

    inline void wrap_split32(uint32_t s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                             D_TYPE *r2) {
        size_t hash1 = h.hash32(s) << 1ul;
        size_t hash2 = (perm.get_perm(h.hash32(s)) << 1ul) | 1ul;
        split(hash1, pd_index1, q1, r1);
        split(hash2, pd_index2, q2, r2);
    }

    size_t get_occupancy();

    size_t find_min_occupancy();

    size_t find_max_occupancy();

    size_t count_min_occupancy();

    size_t count_max_occupancy();

    double absolute_mean_deviation();

    double squared_deviation();

    void print_stats();

    /**For generic testing.*/
    bool is_const_size() { return true; }
};


#endif //CLION_CODE_CONST_FILTER_H
