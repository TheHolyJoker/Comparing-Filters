//
// Created by tomer on 7/12/20.
//

#ifndef FILTERS_PD512_WRAPPER_HPP
#define FILTERS_PD512_WRAPPER_HPP

#include <string>
#include "pd512.hpp"
#include <cstddef>
#include <memory>
/*
extern "C" {
#include "pd512.h"
}
*/

//using namespace pd512;

static const size_t capacity = 50u;
//static uint counter = 0;

class pd512_wrapper {
    __m512i x = {(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
//    __m512i x;

public:

    pd512_wrapper(size_t m, size_t f, size_t l) {}

    pd512_wrapper() {
        long int a[8] = {(INT64_C(1) << 50) - 1,0,0,0,0,0,0,0};
        memcpy(&x, a, 64);
    }
/*
//        x = = {(INT64_C(1) << 50) - 1,0,0,0,0,0,0,0};
//        x = {(INT64_C(1) << 50) - 1,0,0,0,0,0,0,0};
//        x = {static_cast<ulong>((INT64_C(1) << 50) - 1),0,0,0,0,0,0,0};
//        ulong a[8] = {static_cast<ulong>(~INT64_C(0)),0,0,0,0,0,0,0};
//        auto b = (INT64_C(1) << 50) - 1;
//        ulong a[8] = {INT64_C(0),0,0,0,0,0,0,0};
//        static_assert(sizeof(ulong) == 8, "");
//        ulong a[8] = {(INT64_C(1) << 50) - 1,0,0,0,0,0,0,0};
//        memcpy(&x, a, 64);
//        x = _mm512_setzero_epi32();
//        x = _mm512_setzero_ps();
//        {~INT64_C(0), 0, 0, 0, 0, 0, 0, 0};
//        std::cout << "pd512 constructor" << std::endl;
//    }
*/
    /*explicit pd512_wrapper(){
//        cout << "here " << counter++ << endl;
        ulong a[8] = {(INT64_C(1) << 50) - 1,0,0,0,0,0,0,0};
        memcpy(&x, a, 64);
//
//        x = {(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    }
*/

    ~pd512_wrapper() {}

    auto lookup(int64_t quot, char rem) -> bool {
        return pd512::pd_find_50(quot % capacity, rem, &x);
    }

    auto insert(int64_t quot, char rem) -> bool {
        return pd512::pd_add_50(quot % capacity, rem, &x);
    }

    void remove(int64_t quot, char rem) {
        assert(false);
    }

    auto conditional_remove(int64_t quot, char rem) -> bool {
        if (lookup(quot % capacity, rem)) {
            remove(quot % capacity, rem);
            return true;
        }
        return false;
    }

    auto is_full() -> bool {
        return get_capacity() == 50;
    }

    auto get_capacity() -> size_t {
        unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, &x, kBytes2copy);
        // Number of bits to keep. Requires little-endianness
        const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        return pd512::popcount128(header);

    }

    auto get_name() -> std::string {
        return "pd512 ";
    }

    void *operator new(std::size_t size) {
//        int ok = posix_memalign((void**)&Table, sizeof(Bucket) * CHAR_BIT, sizeof(Bucket) * num_of_buckets);
//        int ok = posix_memalign((void**)&pd512_wrapper, )
//        return posix_memalign()
        auto res = _mm_malloc(size, alignof(pd512_wrapper));
        assert(res != NULL);
        return res;
//        return _mm_malloc(size, alignof(pd512_wrapper));
    }

    void *operator new[](std::size_t size) {
        return _mm_malloc(size, alignof(pd512_wrapper));
    }

    void operator delete(void *ptr) {
        _mm_free(ptr);
    }

    void operator delete[](void *ptr) {
        _mm_free(ptr);
    }

};


#endif //FILTERS_PD512_WRAPPER_HPP
