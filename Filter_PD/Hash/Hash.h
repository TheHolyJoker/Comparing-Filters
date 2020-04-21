//
// Created by tomer on 9/16/18.
//

#ifndef ADAPTIVEBF_HASH_H
#define ADAPTIVEBF_HASH_H

//#include "MurmurHash3.h"
#include "../../Shared_files/MurmurHash3.h"
#include "../Global_functions/basic_functions.h"
#include "../Global_functions/macros.h"
#include <iostream>
#include <cstring>
#include <random>
#include <assert.h>


//using namespace std;

namespace PD_Hash {
    class Hash {

        size_t size, multiConst;
//    vector<size_t> multiConst;

    public:
        Hash();

        explicit Hash(size_t size);

//    Hash(size_t size, size_t maxRemainderLength);


//    uint32_t hash(uint32_t x) const;

        uint32_t hash(const char *elementP) const;

        uint32_t hash(string *elementP) const;

        uint32_t dataHash(string *elementP) const;

        uint64_t hash64(uint64_t el);

        inline uint32_t hash32(uint32_t el) {
            uint32_t a = 0, b = 0;
            MurmurHash3_x86_32(&el, (int) (64), DEFAULT_SEED, &a);
            MurmurHash3_x86_32(&el, (int) (64), SECOND_SEED, &b);
            return a + multiConst * b;
        }

        /**
         * Calculate the fingerprint on an element (elementP) using double hashing.
         * @param elementP
         * @param size
         * @return
         */
        uint32_t calc(const char *elementP) const;

        uint32_t calc(string *elementP) const;

        uint32_t operator()(const char *elementP) const;

        uint32_t operator()(string *elementP) const;

//    size_t calc(string *elementP, HASH_BLOCK_TYPE *dataArr, size_t length) const;
//
//    size_t calc(string *elementP, uint16_t *dataArr, size_t length) const;
//
//    size_t lengthHash(string *elementP, uint32_t *dataArr, size_t arraySize) const;
//
//    size_t getMultiConst() const;

    };

/**
 *
 * @param a
 * @param b
 * @return natural random number in the range [a,b]
 */
    size_t rangedUniformDistribution(size_t a, size_t b);

}
#endif //ADAPTIVEBF_HASH_H
