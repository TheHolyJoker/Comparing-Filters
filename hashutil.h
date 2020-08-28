#ifndef HASHUTIL_H_
#define HASHUTIL_H_

#include <climits>
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>

#include "Hash_functions/wyhash.h"
#include "Hash_functions/xxhash64.h"
#include <random>
#include <vector>
// #include "Hash_functions/woothash.h"

namespace hashing {
    // See Martin Dietzfelbinger, "Universal hashing and k-wise independent random
    // variables via integer arithmetic without primes".
    class TwoIndependentMultiplyShift {
        unsigned __int128 multiply_, add_;

    public:
        TwoIndependentMultiplyShift() {
            ::std::random_device random;
            for (auto v : {&multiply_, &add_}) {
                *v = random();
                for (int i = 1; i <= 4; ++i) {
                    *v = *v << 32;
                    *v |= random();
                }
            }
        }

        inline uint64_t operator()(uint64_t key) const {
            return (add_ + multiply_ * static_cast<decltype(multiply_)>(key)) >> 64;
        }
        auto get_name() const -> string{
            return "TwoIndependentMultiplyShift";
        }
    };

    class SimpleMixSplit {

    public:
        uint64_t seed;

        SimpleMixSplit() {
            ::std::random_device random;
            seed = random();
            seed <<= 32;
            seed |= random();
        }

        inline static uint64_t murmur64(uint64_t h) {
            h ^= h >> 33;
            h *= UINT64_C(0xff51afd7ed558ccd);
            h ^= h >> 33;
            h *= UINT64_C(0xc4ceb9fe1a85ec53);
            h ^= h >> 33;
            return h;
        }

        inline uint64_t operator()(uint64_t key) const {
            return murmur64(key + seed);
        }
    };

    class my_xxhash64 {
        uint64_t seed;

    public:
        my_xxhash64() {
            seed = random();
        }
        inline uint64_t operator()(uint64_t key) const {
            return XXHash64::hash(&key, 8, seed);
        }
        auto get_name() const -> string{
            return "xxhash64";
        }
    };

    class my_wyhash64 {
        uint64_t seed;

    public:
        my_wyhash64() {
            seed = random();
        }
        inline uint64_t operator()(uint64_t key) const {
            return wyhash64(key, seed);
        }

        auto get_name() const -> string{
            return "wyhash64";
        }
        
    };

    inline uint32_t hashint(uint32_t a) {
        a = (a + 0x7ed55d16) + (a << 12);
        a = (a ^ 0xc761c23c) ^ (a >> 19);
        a = (a + 0x165667b1) + (a << 5);
        a = (a + 0xd3a2646c) ^ (a << 9);
        a = (a + 0xfd7046c5) + (a << 3);
        a = (a ^ 0xb55a4f09) ^ (a >> 16);
        return a;
    }

    inline uint32_t hashint(uint64_t a) {
        a = (a + 0x7ed55d16) + (a << 12);
        a = (a ^ 0xc761c23c) ^ (a >> 19);
        a = (a + 0x165667b1) + (a << 5);
        a = (a + 0xd3a2646c) ^ (a << 9);
        a = (a + 0xfd7046c5) + (a << 3);
        a = (a ^ 0xb55a4f09) ^ (a >> 16);
        return a;
    }


}// namespace hashing

#endif// CUCKOO_FILTER_HASHUTIL_H_
