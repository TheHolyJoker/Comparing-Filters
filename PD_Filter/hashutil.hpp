//
// Created by tomer on 26/05/2020.
//

#ifndef MINIMAL_S_PD_FILTER_HASHUTIL_HPP
#define MINIMAL_S_PD_FILTER_HASHUTIL_HPP

#include <cstring>
#include <stdint.h>
#include <string>
#include "../cuckoofilter/src/hashutil.h"

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
        x = hashint2(x);
        y = hashint(y);
        //        uint64_t res = (uint64_t) y | (((uint64_t) x) << 32ul);
        return (uint64_t)y | (((uint64_t)x) << 32ul);
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
        x = hashint(x);
        y = hashint2(y);
        return (uint64_t)y | (((uint64_t)x) << 32ul);
    }

    inline uint32_t my_hash(const uint32_t el, uint32_t seed) {
        uint32_t a = 0, b = 0;
        MurmurHash3_x86_32(&el, (int)(64), SEED_1, &a);
        MurmurHash3_x86_32(&el, (int)(64), SEED_2, &b);
        return a + seed * b;
    }

    inline uint64_t my_hash64(const uint64_t el, uint32_t seed) {
        uint64_t a = 0, b = 0;
        MurmurHash3_x86_32(&el, (int)(64), SEED_1, &a);
        MurmurHash3_x86_32(&el, (int)(64), SEED_2, &b);
        return a + seed * b;
    }

    inline uint32_t my_hash(const uint64_t el, uint32_t seed) {
        uint32_t a = 0, b = 0;
        MurmurHash3_x86_32(&el, (int)(64), SEED_1, &a);
        MurmurHash3_x86_32(&el, (int)(64), SEED_2, &b);
        return a + seed * b;
    }

    inline uint32_t my_hash(const char *elementP, uint32_t seed) {
        uint32_t a, b;
        MurmurHash3_x86_32(elementP, (int)(std::strlen(elementP)), SEED_1, &a);
        MurmurHash3_x86_32(elementP, (int)(std::strlen(elementP)), SEED_2, &b);
        return a + seed * b;
    }

    inline uint32_t my_hash(const std::string *elementP, uint32_t seed) {
        //    assert(false);
        char const *cp = elementP->c_str();
        uint32_t a, b;
        MurmurHash3_x86_32(cp, (int)(strlen(cp)), SEED_1, &a);
        MurmurHash3_x86_32(cp, (int)(strlen(cp)), SEED_2, &b);
        return a + seed * b;
    }

    /* Taken from Morton filter repository*/

    // Based on code in the public domain (MurmurHash3a)
    // See https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp 
    // 92cf370
    // The disavowing of the copyright is reproduced below and applies only to MurmurHash3:
    //-----------------------------------------------------------------------------
    // MurmurHash3 was written by Austin Appleby, and is placed in the public
    // domain. The author hereby disclaims copyright to this source code.
    // Template it to make it usable with vector types such as vN_u64

    /* template <class T>
    inline T hash64N(T ks) { // Bit mix from MurmurHash64/CLHash
        ks ^= ks >> 33ul;
        ks *= 0xff51afd7ed558ccdULL;
        ks ^= ks >> 33ul;
        ks *= 0xc4ceb9fe1a85ec53ULL;
        ks ^= ks >> 33ul;
        return ks;
    } */
    // Based on code in the public domain (MurmurHash3a)
    /* template <class T>
    inline T hash32N(T ks) { // Bit mix from MurmurHash32
        ks ^= ks >> 16;
        ks *= 0x85ebca6b;
        ks ^= ks >> 13;
        ks *= 0xc2b2ae35;
        ks ^= ks >> 16;
        return ks;
    } */

    namespace cuckoofilter {

        class HashUtil {
        public:
            // Bob Jenkins Hash
            static uint32_t BobHash(const void *buf, size_t length, uint32_t seed = 0);
            static uint32_t BobHash(const std::string &s, uint32_t seed = 0);

            // Bob Jenkins Hash that returns two indices in one call
            // Useful for Cuckoo hashing, power of two choices, etc.
            // Use idx1 before idx2, when possible. idx1 and idx2 should be initialized to seeds.
            static void BobHash(const void *buf, size_t length, uint32_t *idx1,
                uint32_t *idx2);
            static void BobHash(const std::string &s, uint32_t *idx1, uint32_t *idx2);

            // MurmurHash2
            static uint32_t MurmurHash(const void *buf, size_t length, uint32_t seed = 0);
            static uint32_t MurmurHash(const std::string &s, uint32_t seed = 0);

            // SuperFastHash
            static uint32_t SuperFastHash(const void *buf, size_t len);
            static uint32_t SuperFastHash(const std::string &s);

            // Null hash (shift and mask)
            static uint32_t NullHash(const void *buf, size_t length, uint32_t shiftbytes);

            // Wrappers for MD5 and SHA1 hashing using EVP
            static std::string MD5Hash(const char *inbuf, size_t in_length);
            static std::string SHA1Hash(const char *inbuf, size_t in_length);

        private:
            HashUtil();
        };
    }; //cuckoofilter 
};

#endif //MINIMAL_S_PD_FILTER_HASHUTIL_HPP
