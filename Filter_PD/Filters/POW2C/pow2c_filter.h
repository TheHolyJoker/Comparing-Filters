//
// Created by tomer on 11/4/19.
//

#ifndef CLION_CODE_POW2C_FILTER_H
#define CLION_CODE_POW2C_FILTER_H

#include "../../PD/Non_Contiguous_PD/PD.h"
#include "../../Hash/Hash.h"
#include "../../Hash/Permutation.h"
#include "../../Hash/static_hashing.h"

using namespace PD_Hash;

static vector<uint32_t> perm_vec;

class pow2c_filter {
    vector<PD> pd_vec;
    size_t capacity;
    const size_t size, fp_size, interval_length, single_pd_capacity;
    const size_t quotient_length, pd_index_length;
    Hash h;
    Permutation perm;

public:
    pow2c_filter(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size);

//    ~pow2c_filter();

    bool lookup(string *s);

    void insert(string *s);

    void remove(string *s);

    bool lookup(uint32_t s);

    void insert(uint32_t s);

    void remove(uint32_t s);

private:
    static size_t index_perm(size_t index) {
        return perm_vec[index];
    }

public:

    inline void split(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
        *remainder = hash_index & MASK(fp_size);
        hash_index >>= fp_size;
        *quotient = hash_index & MASK(quotient_length);
        hash_index >>= quotient_length;
        *pd_index = hash_index & MASK(pd_index_length);

    }

    inline void
    wrap_split_string_att_wrong(string *s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                                D_TYPE *r2) {
        /*uint32_t hash_res = my_hash(s, HASH_SEED);
        uint32_t h1 = hash_res << 1ul;
        uint32_t h2 = (naive_perm(hash_res) << 1ul) | 1u;

        *r1 = h1 & MASK(fp_size);
        *r2 = h2 & MASK(fp_size);
        h1 >>= fp_size;
        h2 >>= fp_size;

        *q1 = h1 % (interval_length);
        *q2 = h2 % (interval_length);
        h1 >>= quotient_length;
        h2 >>= quotient_length;

        *pd_index1 = h1 % pd_vec.size();
        *pd_index2 = h2 % pd_vec.size();*/
//        ulong h1 = h.hash(s) << 1ul;
        ulong temp = my_hash(s, HASH_SEED);
        ulong h1 = temp << 1ul;
        ulong h2 = (temp << 1ul) | 1u;
        *r1 = h1 & MASK(fp_size);
        *r2 = h2 & MASK(fp_size);
//        *r2 = *r1 | 1ul;
        h1 >>= fp_size;
        h2 >>= fp_size;

        *q1 = h1 % (interval_length);
        *q2 = h2 % (interval_length);
//        *q2 = *q1;
        h1 >>= quotient_length;
        h2 >>= quotient_length;

        *pd_index1 = h1 % pd_vec.size();
//        *pd_index2 = naive_perm(h2) % pd_vec.size();
//        *pd_index2 = index_perm(*pd_index1);
        *pd_index2 = (*pd_index1 + 1) % pd_vec.size();
//        *pd_index2 = naive_perm(*pd_index1) % pd_vec.size();
        assert(*pd_index1 < pd_vec.size());
        assert(*pd_index2 < pd_vec.size());
    }

    inline void
    wrap_split_string_att(string *s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                          D_TYPE *r2) {
        ulong h1 = h.hash(s) << 1ul;
        *r1 = h1 & MASK(fp_size);
        *r2 = *r1 | 1ul;
//        cout << *r1 << ", " << *r2 << endl;
        h1 >>= fp_size;
        *q1 = h1 % (interval_length);
        *q2 = *q1;
        h1 >>= quotient_length;
        *pd_index1 = h1 % pd_vec.size();
        *pd_index2 = index_perm(*pd_index1);
        if (*pd_index1 == *pd_index2) {
            cout << "same pd indexes" << endl;
            cout << *pd_index1 << endl;
        }
        assert(*pd_index1 < pd_vec.size());
        assert(*pd_index2 < pd_vec.size());
    }

    inline void wrap_split_string(string *s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                                  D_TYPE *r2) {
        size_t hash1 = h.hash(s) << 1ul;
        size_t hash2 = (perm.get_perm(h.hash(s)) << 1ul) | 1ul;
        split(hash1, pd_index1, q1, r1);
        split(hash1, pd_index1, q2, r2);
    }

    inline void wrap_split32(uint32_t s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                             D_TYPE *r2) {
        size_t hash1 = h.hash32(s) << 1ul;
        size_t hash2 = (perm.get_perm(h.hash32(s)) << 1ul) | 1ul;
        split(hash1, pd_index1, q1, r1);
        split(hash2, pd_index2, q2, r2);
    }

    /**For generic testing.*/
    bool is_const_size() { return false; }
};


static size_t count_perm_vec_fixed_point() {
    size_t counter = 0;
    for (size_t i = 0; i < perm_vec.size(); ++i) {
        if (perm_vec[i] == i) {
//            cout << i << " is a fixed point." << endl;
            counter++;
        };
    }
    return counter;
}


#endif //CLION_CODE_POW2C_FILTER_H
