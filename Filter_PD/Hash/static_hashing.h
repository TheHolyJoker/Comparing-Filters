//
// Created by tomer on 11/26/19.
//

#ifndef CLION_CODE_STATIC_HASHING_H
#define CLION_CODE_STATIC_HASHING_H


//#include "MurmurHash3.h"
#include "../../Shared_files/MurmurHash3.h"

#include "../Global_functions/basic_functions.h"
#include "../Global_functions/macros.h"
#include <iostream>
#include <cstring>
#include <random>
#include <assert.h>

//template<typename T>
//uint32_t hash(uint32_t)

//static const uint32_t SEED_1 = random() , SEED_2 = random();
static const uint32_t SEED_1 = 1, SEED_2 = 42;

inline uint32_t my_hash(const uint32_t el, uint32_t seed) {
    uint32_t a = 0, b = 0;
    MurmurHash3_x86_32(&el, (int) (64), SEED_1, &a);
    MurmurHash3_x86_32(&el, (int) (64), SEED_2, &b);
    return a + seed * b;
}

inline uint32_t my_hash(const char *elementP, uint32_t seed) {
    uint32_t a, b;
    MurmurHash3_x86_32(elementP, (int) (strlen(elementP)), SEED_1, &a);
    MurmurHash3_x86_32(elementP, (int) (strlen(elementP)), SEED_2, &b);
    return a + seed * b;
}

inline uint32_t my_hash(const string *elementP, uint32_t seed) {
//    assert(false);
    char const *cp = elementP->c_str();
    uint32_t a, b;
    MurmurHash3_x86_32(cp, (int) (strlen(cp)), SEED_1, &a);
    MurmurHash3_x86_32(cp, (int) (strlen(cp)), SEED_2, &b);
    return a + seed * b;
}

template<typename T, typename S>
inline void hash2_choice(S x, T *index1, T *index2) {
//    uint32_t a = 0, b = 0;
    MurmurHash3_x86_32(&x, (int) (64), SEED_1, index1);
    MurmurHash3_x86_32(&x, (int) (64), SEED_2, index2);
//    return a + seed * b;
}


inline uint32_t hash32(uint32_t el, uint32_t seed) {
    uint32_t a = 0, b = 0;
    MurmurHash3_x86_32(&el, (int) (32), SEED_1, &a);
    MurmurHash3_x86_32(&el, (int) (32), SEED_2, &b);
    return a + seed * b;
}

inline uint32_t naive_perm(uint32_t n) {
    n += 4061379055;
    n = ((n << 11u) | (n >> 21u)) + 3038993865;
    return ((n << 4u) | (n >> 27u)) ^ 2419120842u;
}

/*
 * taken from https://burtleburtle.net/bob/hash/integer.html*/
inline uint32_t hashint(uint32_t a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

inline uint32_t hashint2(uint32_t a) {
    a += ~(a << 15);
    a ^= (a >> 10);
    a += (a << 3);
    a ^= (a >> 6);
    a += ~(a << 11);
    a ^= (a >> 16);
    return a
}
/*
 * Robert Jenkins' 96 bit Mix Function
 */
inline uint32_t mix(int a, int b, int c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}
//uint32_t rotl32 (uint32_t value, unsigned int count) {
//    return value << count | value >> (32 - count);
//}


#endif //CLION_CODE_STATIC_HASHING_H