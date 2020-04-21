//
// Created by tomer on 11/25/19.
//

#ifndef CLION_CODE_GEN_2POWER_H
#define CLION_CODE_GEN_2POWER_H

#include "../../PD/Non_Contiguous_PD/PD.h"
#include "../../PD/Naive_PD/naive_PD.h"
#include "../../PD/Naive_PD/naive_PD.h"
#include "../../PD/Contiguous_PD/cg_PD.h"
#include "../../Hash/Hash.h"
#include "../../Hash/Permutation.h"
#include "../../Hash/static_hashing.h"

#define GEN_TO_PRINT (0)

template<class D>
class gen_2Power {
    vector<D> pd_vec;
    vector<uint> pd_capacity_vec;
    size_t capacity;
    const size_t size, fp_size, interval_length, single_pd_capacity;
    const size_t quotient_length, pd_index_length;
    const bool is_const_size_indicator;
//    const uint32_t xor_array[2];
//    const uint32_t rotate_array[2];
//    Hash h;
//    Permutation perm;
public:
//    gen_2Power(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size);

    gen_2Power(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size, bool const_size = false);

//    gen_2Power(size_t max_number_of_elements, size_t error_power_inv, size_t level1_counter_size,
//    size_t level2_counter_size, double level1_load_factor, double level2_load_factor);

//    ~gen_2Power();
//    ~gen_2Power();
//    ~pow2c_filter();

    auto lookup(string *s) -> bool;

    void insert(string *s);

    void remove(string *s);

    auto lookup(uint32_t s) -> bool;

    void insert(uint32_t s);

    void remove(uint32_t s);

    inline void wrap_split(string *s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                           D_TYPE *r2) {
        auto h = my_hash(s, HASH_SEED);
        split(h, pd_index1, pd_index2, q1, q2, r1, r2);
    }

    inline void wrap_split(uint32_t s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                           D_TYPE *r2) {
        auto h = my_hash(s, HASH_SEED);
        if (GEN_TO_PRINT) printf("Hash res is %u\n", h);
        split(h, pd_index1, pd_index2, q1, q2, r1, r2);
    }

    inline void split(ulong h, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                      D_TYPE *r2) {
        ulong h1 = h << 1u;
//        ulong h2 = h1 | 1u;
//        ulong h2 = (h << 1u) | 1u;

        *r1 = h1 & MASK(fp_size);
        *r2 = (h1 & MASK(fp_size)) | 1u;
        h1 >>= fp_size;
//        h2 >>= fp_size;

        *q1 = h1 % (interval_length);
        *q2 = h1 % (interval_length);
        h1 >>= quotient_length;
//        h2 >>= quotient_length;

        *pd_index1 = h1 % pd_vec.size();
        *pd_index2 = (*pd_index1 + 1) % pd_vec.size();
//        *pd_index2 = naive_perm(h2) % pd_vec.size();

//        if (*pd_index1 == *pd_index2) {
//            cout << "same pd indexes" << endl;
//            printf("H:%zu, h1:%zu, h2:%zu\n", h, h1, h2);
//            cout << *pd_index1 << endl;
//        }
        if (DB) assert(*pd_index1 < pd_vec.size());
        if (DB) assert(*pd_index2 < pd_vec.size());
    }

    auto is_const_size() -> bool { return is_const_size_indicator; }
};


#endif //CLION_CODE_GEN_2POWER_H
