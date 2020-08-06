//
// Created by tomer on 26/05/2020.
//

#ifndef MINIMAL_S_PD_FILTER_HASHUTIL_HPP
#define MINIMAL_S_PD_FILTER_HASHUTIL_HPP

#include <cstring>
#include <stdint.h>
#include <string>

namespace s_pd_filter {
    static const uint32_t SEED_1 = 1, SEED_2 = 42;

    //-----------------------------------------------------------------------------
    // MurmurHash3 was written by Austin Appleby, and is placed in the public
    // domain. The author hereby disclaims copyright to this source code.
    //-----------------------------------------------------------------------------
//    namespace MurmurHash {
    void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, void *out);

    void MurmurHash3_x86_128(const void *key, int len, uint32_t seed, void *out);

    void MurmurHash3_x64_128(const void *key, int len, uint32_t seed, void *out);
//    }
    //-----------------------------------------------------------------------------

    /*
* taken from
* https://burtleburtle.net/bob/hash/integer.html
* */
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

    inline uint32_t hashint2(uint32_t a) {
        a += ~(a << 15);
        a ^= (a >> 10);
        a += (a << 3);
        a ^= (a >> 6);
        a += ~(a << 11);
        a ^= (a >> 16);
        return a;
    }

    inline uint32_t hashint3(uint32_t a) {
        a = (a ^ 0xdeadbeef) + (a << 4);
        a = a ^ (a >> 10);
        a = a + (a << 7);
        a = a ^ (a >> 13);
        return a;
    }

    inline uint32_t hashint3(uint64_t a) {
        a = (a ^ 0xdeadbeef) + (a << 4);
        a = a ^ (a >> 10);
        a = a + (a << 7);
        a = a ^ (a >> 13);
        return a;
    }

    inline uint64_t hashint64(uint64_t a) {
        uint32_t x = hashint3(a);
        uint32_t y = a & 4294967295ul;
        x = hashint(x);
        y = hashint(y);
//        uint64_t res = (uint64_t) y | (((uint64_t) x) << 32ul);
        return (uint64_t) y | (((uint64_t) x) << 32ul);
//
//        a = (a^0xdeadbeef) + (a<<4);
//        a = a ^ (a>>10);
//        a = a + (a<<7);
//        a = a ^ (a>>13);
//        return a;
    }
    inline uint64_t hashint64_2(uint64_t a) {
        uint32_t x = hashint3(a);
        uint32_t y = a & 4294967295ul;
        x = hashint2(x);
        y = hashint2(y);
        return (uint64_t) y | (((uint64_t) x) << 32ul);
    }

    inline uint32_t my_hash(const uint32_t el, uint32_t seed) {
        uint32_t a = 0, b = 0;
        MurmurHash3_x86_32(&el, (int) (64), SEED_1, &a);
        MurmurHash3_x86_32(&el, (int) (64), SEED_2, &b);
        return a + seed * b;
    }

    inline uint64_t my_hash64(const uint64_t el, uint32_t seed) {
        uint64_t a = 0, b = 0;
        MurmurHash3_x86_32(&el, (int) (64), SEED_1, &a);
        MurmurHash3_x86_32(&el, (int) (64), SEED_2, &b);
        return a + seed * b;
    }

    inline uint32_t my_hash(const uint64_t el, uint32_t seed) {
        uint32_t a = 0, b = 0;
        MurmurHash3_x86_32(&el, (int) (64), SEED_1, &a);
        MurmurHash3_x86_32(&el, (int) (64), SEED_2, &b);
        return a + seed * b;
    }

    inline uint32_t my_hash(const char *elementP, uint32_t seed) {
        uint32_t a, b;
        MurmurHash3_x86_32(elementP, (int) (std::strlen(elementP)), SEED_1, &a);
        MurmurHash3_x86_32(elementP, (int) (std::strlen(elementP)), SEED_2, &b);
        return a + seed * b;
    }

    inline uint32_t my_hash(const std::string *elementP, uint32_t seed) {
//    assert(false);
        char const *cp = elementP->c_str();
        uint32_t a, b;
        MurmurHash3_x86_32(cp, (int) (strlen(cp)), SEED_1, &a);
        MurmurHash3_x86_32(cp, (int) (strlen(cp)), SEED_2, &b);
        return a + seed * b;
    }


}
#endif //MINIMAL_S_PD_FILTER_HASHUTIL_HPP
