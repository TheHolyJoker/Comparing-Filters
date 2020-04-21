//
// Created by tomer on 11/4/19.
//
/**
 * This file as an attempt to create simple permutation, using the fact that
 * a multiplicative group of integers modulo any prime is cyclic.
 * The prime is p = (2**31 - 1).
 * Given element x (in the group and there for smaller than p) The operation on x that will attempt
 * to act as a permutation is multiplying x by one of the group generator r, also known as a primitive root.
 *
 */

#ifndef CLION_CODE_PERMUTATION_H
#define CLION_CODE_PERMUTATION_H

#include <zconf.h>
#include <cstdint>

#include "../Global_functions/macros.h"


/**
 * Primitive roots and their inverse modulo (1<<31) - 1.
 */
static ulong primitive_root_array[5] = {21759941, 84476145, 96692034, 161856978, 168441100};
static ulong inv_primitive_root_array[5] = {2059219856, 1309127614, 1139066240, 1665930690, 1904044902};


class Permutation {
    size_t mod, mult_const, mult_inv_const;
    const bool is_naive;
public:
    Permutation(size_t mod);

    Permutation(size_t mod, bool naive_perm);

    uint32_t get_perm(size_t el);

    uint64_t get_perm64(uint64_t el);

    uint32_t get_perm_inv(size_t el);

};


#endif //CLION_CODE_PERMUTATION_H
