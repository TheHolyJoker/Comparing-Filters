//
// Created by tomer on 9/16/18.
//

#ifndef QF_FILTER_HASH_H
#define QF_FILTER_HASH_H

//#include "MurmurHash3.h"
#include "../../Shared_files/MurmurHash3.h"
#include "../GlobalFunctions/globalFunction.h"
#include <iostream>
#include <cstring>
#include <random>
#include <assert.h>

namespace QF_Hash {
#define DEFAULT_SEED 2
#define SECOND_SEED 42
#define HASH_BLOCK_TYPE uint32_t
#define HASH_BLOCK_SIZE (32)
#define MAX_MULTI (1ULL<<10)


    class Hash {

        size_t size, multiConst;
//    vector<size_t> multiConst;

    public:
        Hash();

        explicit Hash(size_t size);

//    Hash(size_t size, size_t maxRemainderLength);

        uint32_t hash(const char *elementP) const;

        uint32_t hash(const string *elementP) const;

        uint32_t hash(uint32_t el);

        uint32_t dataHash(const string *elementP) const;

        /**
         * Calculate the fingerprint on an element (elementP) using double hashing.
         * @param elementP
         * @param size
         * @return
         */
        uint32_t calc(const char *elementP) const;

        uint32_t calc(const string *elementP) const;

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
#endif //QF_FILTER_HASH_H
