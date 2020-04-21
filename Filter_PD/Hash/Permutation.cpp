//
// Created by tomer on 11/4/19.
//

#include <cassert>
#include <cstdlib>
#include "Permutation.h"

Permutation::Permutation(size_t mod) : mod(mod), is_naive(false) {
//    assert (mod <= P31);
//    size_t index = rand() % 5;
    size_t index = 0;
    mult_const = primitive_root_array[index];
    mult_inv_const = inv_primitive_root_array[index];
    //todo: translate from python the general case.
}

Permutation::Permutation(size_t mod, bool naive_perm) : mod(mod), mult_const(0), mult_inv_const(0), is_naive(true) {}

uint32_t Permutation::get_perm(size_t el) {
    if (is_naive)
        return (el + 1) % mod;
//    if (DB) assert (el < mod);
    return (el * mult_const) % mod;
}

uint64_t Permutation::get_perm64(uint64_t el) {
    if (is_naive)
        return (el + 1) % mod;
    //    if (DB) assert (el < mod);
    return (el * mult_const) % mod;
}

uint32_t Permutation::get_perm_inv(size_t el) {
    if (is_naive){
        (el == 0) ? mod - 1: el - 1;
    }
//    if (DB) assert (el < mod);
    return (el * mult_inv_const) % mod;
}

//uint32_t Permutation::operator()(size_t el) {
//    assert (el < mod);
//    return el*mu;
//}
