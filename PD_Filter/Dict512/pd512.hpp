/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD512_HPP
#define FILTERS_PD512_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <immintrin.h>
// #include "immintrin.h"
//#include "x86intrin.h"

namespace v_pd512 {
    template<typename T>
    void bin_print(T x) {
        if (x == 0) {
            std::cout << '0' << std::endl;
            return;
        }
        size_t slot_size = sizeof(T) * CHAR_BIT;
        uint64_t b = 1ULL << (slot_size - 1u);
        while ((!(b & x)) and (b)) {
            b >>= 1ul;
        }
        assert(b);
        while (b) {
            std::string temp = (b & x) ? "1" : "0";
            std::cout << temp;
            b >>= 1ul;
        }
        std::cout << std::endl;
    }
    void bin_print_header(uint64_t header);

    auto bin_print_header_spaced(uint64_t header) -> std::string;

    auto bin_print_header_spaced128(unsigned __int128 header) -> std::string;


}// namespace v_pd512

namespace pd512 {
    auto validate_clz(int64_t quot, char rem, const __m512i *pd) -> bool;

    auto validate_clz_helper(int64_t quot, char rem, const __m512i *pd) -> bool;

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select64(uint64_t x, int64_t j) {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select128(unsigned __int128 x, int64_t j) {
        const int64_t pop = _mm_popcnt_u64(x);
        if (j < pop)
            return select64(x, j);
        return 64 + select64(x >> 64, j - pop);
    }

    inline uint64_t select128withPop64(unsigned __int128 x, int64_t j, int64_t pop) {
        if (j < pop) return select64(x, j);
        return 64 + select64(x >> 64, j - pop);
    }

    // returns the number of zeros before the jth (counting from 0) set bit of x
    inline uint64_t nth64(uint64_t x, int64_t j) {
        const uint64_t y = select64(x, j);
        assert(y < 64);
        const uint64_t z = x & ((UINT64_C(1) << y) - 1);
        return y - _mm_popcnt_u64(z);
    }


    // returns the number of zeros before the jth (counting from 0) set bit of x
    inline uint64_t nth128(unsigned __int128 x, int64_t j) {
        const uint64_t y = select128(x, j);
        assert(y < 128);
        const unsigned __int128 z = x & ((((unsigned __int128) 1) << y) - 1);
        return y - _mm_popcnt_u64(z) - _mm_popcnt_u64(z >> 64);
    }

    /*Stackoverflow: https://stackoverflow.com/a/40528716/5381404  */
    inline int lzcnt_u128(unsigned __int128 u) {
        uint64_t hi = u >> 64;
        uint64_t lo = u;
        lo = (hi == 0) ? lo : -1ULL;
        return _lzcnt_u64(hi) + _lzcnt_u64(lo);
    }

    inline int tzcnt_u128(unsigned __int128 u) {
        uint64_t hi = u >> 64;
        uint64_t lo = u;
        lo = (hi == 0) ? lo : -1ULL;
        return _tzcnt_u64(hi) + _tzcnt_u64(lo);
    }
    inline int popcount64(uint64_t x) {
        return _mm_popcnt_u64(x);
    }

    inline int popcount128(unsigned __int128 x) {
        const uint64_t hi = x >> 64;
        const uint64_t lo = x;
        return popcount64(lo) + popcount64(hi);
    }

    inline bool pd_full(const __m512i *pd) {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        return 1 & (header_end >> (50 + 51 - 64 - 1));
    }

    inline int pd_popcount(const __m512i *pd) {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        constexpr uint64_t kLeftoverMask = (UINT64_C(1) << (50 + 51 - 64)) - 1;
        header_end = header_end & kLeftoverMask;
        const int result = 128 - 51 - _lzcnt_u64(header_end) + 1;
        return result;
    }


    // find an 8-bit value in a pocket dictionary with quotients in [0,64) and 49 values
    bool pd_find_64(int64_t quot, char rem, const __m512i *pd);


    inline bool pd_find_50_v1(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v4(int64_t quot, uint8_t rem, const __m512i *pd) {
        /* Does not work */
        assert(0);
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        uint64_t v2 = _mm512_cmpeq_epu8_mask(target, *pd) >> (13ul + (quot - 1));

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);

        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        bool att = (v2 & ((UINT64_C(1) << (end + quot)) - 1)) >> (begin + quot);
        bool res = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        assert(att == res);
        assert((v2 & ((UINT64_C(1) << (end + quot)) - 1)) >> (begin + quot));
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline int pd_find_body_elimination64(int64_t quot, uint8_t rem, const __m512i *pd, uint64_t v) {
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        bool res = pd_find_50_v1(quot, rem, pd);

        const int64_t zero_count = _tzcnt_u64(v);

        bool is_last_index_one = header & ((1ULL << (quot + zero_count)));
        if (is_last_index_one) {
            assert(res == false);
            return 0;
            /* counter++;
            if (res) {
                const unsigned __int128 shifted_header = header >> (quot + zero_count);
                std::cout << v_pd512::bin_print_header_spaced128(header) << std::endl;
                std::cout << v_pd512::bin_print_header_spaced128(shifted_header) << std::endl;
                // std::cout << v_pd512::bin_print_header_spaced(mask) << std::endl;
                // std::cout << v_pd512::bin_print_header_spaced(mask_m1) << std::endl;
                assert(0);
            }
            assert(res == false);
            return 0; */
        } else if (quot + zero_count < 64) {
            const int64_t h0 = ((uint64_t *) pd)[0];
            bool att = _mm_popcnt_u64(h0 & ((1ULL << (quot + zero_count)) - 1)) == quot;
            assert(att == res);
            return att;
        } else {
            return -1;
        }
    }

    inline int pd_find_body_elimination(int64_t quot, uint8_t rem, const __m512i *pd, uint64_t v) {
        assert(v);
        // static int counter = 0;
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        bool res = pd_find_50_v1(quot, rem, pd);

        const int64_t v_pop = _mm_popcnt_u64(v);
        if (v_pop > 1) return -1;


        const int64_t zero_count = _tzcnt_u64(v);
        bool is_last_index_one = header & (((unsigned __int128) 1) << (quot + zero_count));
        if (is_last_index_one) {
            assert(res == false);
            return 0;
            /* counter++;
            if (res) {
                const unsigned __int128 shifted_header = header >> (quot + zero_count);
                std::cout << v_pd512::bin_print_header_spaced128(header) << std::endl;
                std::cout << v_pd512::bin_print_header_spaced128(shifted_header) << std::endl;
                // std::cout << v_pd512::bin_print_header_spaced(mask) << std::endl;
                // std::cout << v_pd512::bin_print_header_spaced(mask_m1) << std::endl;
                assert(0);
            }
            assert(res == false);
            return 0; */
        } else if (quot + zero_count < 64) {
            const int64_t h0 = ((uint64_t *) pd)[0];
            bool att = _mm_popcnt_u64(h0 & ((1ULL << (quot + zero_count)) - 1)) == quot;
            assert(att == res);
            return att;
        } else {
            return -1;
        }
        /* bool c = quot + zero_count < 63;// CHECK THIS!
        if (c) {
            // counter++;
            const int64_t h0 = ((uint64_t *) pd)[0];
            const int64_t mask_p1 = h0 & ((1ULL << (quot + zero_count + 1)) - 1);
            const int64_t mask = h0 & ((1ULL << (quot + zero_count)) - 1);
            // const int64_t mask_m1 = h0 & ((1ULL << (quot + zero_count - 1)) - 1);

            const int64_t mid_p1 = _mm_popcnt_u64(mask_p1);
            const int64_t mid = _mm_popcnt_u64(mask);
            // const int64_t mid_m1 = _mm_popcnt_u64(mask_m1);
            // assert(mid == mid_m1);
            // v_pd512::bin_print_header_spaced(mask);

            // bool att = (_mm_popcnt_u64(mask) == quot);
            // if (mid == quot - 1){
            //     std::cout << "A1: "<< std::endl;
            // }
            // else if (mid == quot)
            // {
            //     std::cout << "A2: "<< std::endl;
            // }
            // bool att = (mid == quot - 1) || (mid == quot);
            // bool att3 = (mid == quot +1);
            bool att2 = (mid == quot);
            bool att_helper = (mid_p1 == mid);
            bool att = (mid_p1 == quot) && att_helper;
            // bool att = (mid_m1 == quot - 1) || (mid != mid_m1);
            if (att != res) {
                std::cout << std::string(80, '*') << std::endl;
                std::cout << "counter: " << counter << std::endl;
                std::cout << std::string(80, '*') << std::endl;
                std::cout << v_pd512::bin_print_header_spaced(h0) << std::endl;
                std::cout << v_pd512::bin_print_header_spaced(mask_p1) << std::endl;
                std::cout << v_pd512::bin_print_header_spaced(mask) << std::endl;
                // std::cout << v_pd512::bin_print_header_spaced(mask_m1) << std::endl;
                std::cout << std::string(80, '*') << std::endl;


                // std::cout << "log2(v): " << quot << std::endl;
                std::cout << "quot: " << quot << std::endl;
                std::cout << "zero_count: " << zero_count << std::endl;
                std::cout << "quot + zero_count: " << quot + zero_count << std::endl;
                std::cout << "mid_p1: " << mid_p1 << std::endl;
                std::cout << "mid: " << mid << std::endl;
                // std::cout << "mid_m1: " << mid_m1 << std::endl;
                std::cout << std::string(80, '*') << std::endl;
                std::cout << "res: " << res << std::endl;
                std::cout << "att: " << att << std::endl;
                std::cout << "att_helper: " << att_helper << std::endl;
                std::cout << "att2: " << att2 << std::endl;
                // std::cout << "att3: " << att3 << std::endl;
            }
            assert(att == res);
            return (_mm_popcnt_u64(mask) == quot);
            // return (_mm_popcnt_u64(mask) == quot - 1) || (_mm_popcnt_u64(mask) == quot);
        }
        // else if (v == 1) {
        // }

        const int64_t pop = _mm_popcnt_u64(v);
        if (pop == 1) {
            const int64_t zero_count = _tzcnt_u64(v);
            if (quot + zero_count < 64) {
                const int64_t h0 = ((uint64_t *) pd)[0];
                const int64_t mask = h0 & ((1ULL << (quot + zero_count)) - 1);
                return _mm_popcnt_u64(mask) == quot;
            }
        }

        return -1; */
    }

    inline bool pd_find_50_v5(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        // if (!v || pd_find_body_elimination(quot, rem, pd, v)) return false;
        if (!v) return false;

        // if (quot == 0){

        // }

        const int64_t v_pop = _mm_popcnt_u64(v);
        if (v_pop == 1) {
            const int64_t zero_count = _tzcnt_u64(v);
            bool c1 = quot > 0;
            bool c2 = quot + zero_count < 63;// CHECK THIS!
            bool c = c1 && c2;
            if (c) {
                // counter++;
                const int64_t h0 = ((uint64_t *) pd)[0];
                const int64_t mask_p1 = h0 & ((1ULL << (quot + zero_count + 1)) - 1);
                const int64_t mask = h0 & ((1ULL << (quot + zero_count)) - 1);
                const int64_t mask_m1 = h0 & ((1ULL << (quot + zero_count - 1)) - 1);

                const int64_t mid_p1 = _mm_popcnt_u64(mask_p1);
                const int64_t mid = _mm_popcnt_u64(mask);
                const int64_t mid_m1 = _mm_popcnt_u64(mask_m1);
                // assert(mid == mid_m1);
                // v_pd512::bin_print_header_spaced(mask);

                // bool att = (_mm_popcnt_u64(mask) == quot);
                // if (mid == quot - 1){
                //     std::cout << "A1: "<< std::endl;
                // }
                // else if (mid == quot)
                // {
                //     std::cout << "A2: "<< std::endl;
                // }
                // bool att = (mid == quot - 1) || (mid == quot);
                // bool att3 = (mid == quot +1);
                bool att2 = (mid == quot);
                bool att_helper = (mid_p1 == mid);
                bool att = (mid_p1 == quot) && att_helper;
                // bool att = (mid_m1 == quot - 1) || (mid != mid_m1);
                bool res = pd_find_50_v1(quot, rem, pd);
                if (att != res) {
                    std::cout << std::string(80, '*') << std::endl;
                    // std::cout << "counter: " << counter << std::endl;
                    std::cout << std::string(80, '*') << std::endl;
                    std::cout << v_pd512::bin_print_header_spaced(h0) << std::endl;
                    std::cout << v_pd512::bin_print_header_spaced(mask_p1) << std::endl;
                    std::cout << v_pd512::bin_print_header_spaced(mask) << std::endl;
                    std::cout << v_pd512::bin_print_header_spaced(mask_m1) << std::endl;
                    std::cout << std::string(80, '*') << std::endl;


                    // std::cout << "log2(v): " << quot << std::endl;
                    std::cout << "quot: " << quot << std::endl;
                    std::cout << "zero_count: " << zero_count << std::endl;
                    std::cout << "quot + zero_count: " << quot + zero_count << std::endl;
                    std::cout << "mid_p1: " << mid_p1 << std::endl;
                    std::cout << "mid: " << mid << std::endl;
                    std::cout << "mid_m1: " << mid_m1 << std::endl;
                    std::cout << std::string(80, '*') << std::endl;
                    std::cout << "res: " << res << std::endl;
                    std::cout << "att: " << att << std::endl;
                    std::cout << "att_helper: " << att_helper << std::endl;
                    std::cout << "att2: " << att2 << std::endl;
                    // std::cout << "att3: " << att3 << std::endl;
                }
                assert(att == res);
                return (_mm_popcnt_u64(mask) == quot);
                // return (_mm_popcnt_u64(mask) == quot - 1) || (_mm_popcnt_u64(mask) == quot);
            }
        }
        // else if (v == 1) {
        // }

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);

        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        // bool att = (v2 & ((UINT64_C(1) << (end + quot)) - 1)) >> (begin + quot);
        bool res = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // assert(att == res);
        // assert((v2 & ((UINT64_C(1) << (end + quot)) - 1)) >> (begin + quot));
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v6(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        // if (_mm_popcnt_u64(v) == 1) {
        if (_blsr_u64(v) == 0) {
            const int64_t zero_count = _tzcnt_u64(v);
            return (!(header & (((unsigned __int128) 1) << (quot + zero_count)))) &&
                   (popcount128(header & (((unsigned __int128) 1 << (quot + zero_count)) - 1)) == quot);
            // bool is_last_index_one = header & (((unsigned __int128) 1) << (quot + zero_count));
            // bool cond2 = popcount128(header & (((unsigned __int128) 1 << (quot + zero_count)) - 1)) == quot;
            // return (!is_last_index_one && cond2);
            // bool att = (!is_last_index_one && cond2);
            // assert(pd_find_50_v1(quot, rem, pd) == att);

            // return (header & (((unsigned __int128) 1) << (quot + zero_count))) &&
            // bool is_last_index_one = header & (((unsigned __int128) 1) << (quot + zero_count));
            // if (is_last_index_one) {
            //     assert(res == false);
            //     return 0;
            // }
        }
        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;

        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;

        // bool att = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // assert(att == res);
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }


    inline bool pd_find_50_v7(int64_t quot, uint8_t rem, const __m512i *pd) {
        static int total_calls = 0;
        static int c0 = 0;
        static int c1 = 0;
        // static int c1 = 0;
        total_calls++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) {
            c0++;
            return false;
        }

        if ((_mm_popcnt_u64(v) == 1) && (_tzcnt_u64(v) + quot < 64)) {
            c1++;
            const int64_t zero_count = _tzcnt_u64(v);
            const int64_t h0 = ((uint64_t *) pd)[0];
            const bool is_last_index_one = h0 & (1ULL << (quot + zero_count));
            const bool att = _mm_popcnt_u64(h0 & ((1ULL << (quot + zero_count)) - 1)) == quot;
            assert(((!is_last_index_one) && att) == pd_find_50_v1(quot, rem, pd));
            return (!is_last_index_one) && att;
        }

        if ((rand() % 1000000) == 0) {
            std::cout << "v7: " << std::endl;
            std::cout << "total_calls: " << total_calls << std::endl;
            std::cout << "c0: " << c0 << std::endl;
            std::cout << "c1: " << c1 << std::endl;
            std::cout << "ratio: " << (1.0 * c0 / total_calls) << "\t";
            std::cout << "ratio: " << (1.0 * c1 / total_calls) << std::endl;
        }

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;

        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // bool att = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // assert(att == res);
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v8(int64_t quot, uint8_t rem, const __m512i *pd) {
        // static int total_calls = 0;
        // static int c0 = 0;
        // static int c1 = 0;
        // static int c2 = 0;
        // total_calls++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) {
            // c0++;
            return false;
        }

        if (_blsr_u64(v) == 0) {
            const int64_t zero_count = _tzcnt_u64(v);
            if (zero_count + quot < 64) {
                // c1++;
                const int64_t h0 = ((uint64_t *) pd)[0];
                // return (!(h0 & (1ULL << (quot + zero_count)))) && (_mm_popcnt_u64(_bzhi_u64(h0, quot + zero_count)) == quot);
                return (!(h0 & (1ULL << (quot + zero_count)))) && (_mm_popcnt_u64(h0 & ((1ULL << (quot + zero_count)) - 1)) == quot);
                /* const bool is_last_index_one = h0 & (1ULL << (quot + zero_count));
                // const int64_t temp = h0 & ((1ULL << (quot + zero_count)) - 1);
                // const int64_t temp2 = _bzhi_u64(h0, quot + zero_count);
                // assert(temp == temp2);
                const bool att = _mm_popcnt_u64(h0 & ((1ULL << (quot + zero_count)) - 1)) == quot;
                const bool att2 = _mm_popcnt_u64(_bzhi_u64(h0, quot + zero_count)) == quot;
                assert(att == att2);
                assert(((!is_last_index_one) && att) == pd_find_50_v1(quot, rem, pd));
                return (!is_last_index_one) && att; */
            } else {
                // c2++;
                const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                const unsigned __int128 header = (*h);
                return (!(header & (((unsigned __int128) 1) << (quot + zero_count)))) &&
                       (popcount128(header & (((unsigned __int128) 1 << (quot + zero_count)) - 1)) == quot);
            }
        }

        // if ((rand() % 1000000) == 0) {
        //     std::cout << "v8: " << std::endl;
        //     std::cout << "total_calls: " << total_calls << std::endl;
        //     std::cout << "c0: " << c0 << std::endl;
        //     std::cout << "c1: " << c1 << std::endl;
        //     std::cout << "c2: " << c2 << std::endl;
        //     std::cout << "ratio: " << (1.0 * c0 / total_calls) << "\t";
        //     std::cout << "ratio: " << (1.0 * c1 / total_calls) << "\t";
        //     std::cout << "ratio: " << (1.0 * c2 / total_calls) << std::endl;
        // }

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;

        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // bool att = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // assert(att == res);
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v9(int64_t quot, uint8_t rem, const __m512i *pd) {

        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v)
            return false;


        if ((_blsr_u64(v) == 0) && (v << quot)) {
            const int64_t h0 = ((uint64_t *) pd)[0];
            // const unsigned __int128 *h = (const unsigned __int128 *) pd;
            // const unsigned __int128 header = (*h);
            const int64_t mask = v << quot;
            const bool att = (!(h0 & mask)) && _mm_popcnt_u64(h0 & (mask - 1)) == quot;
            assert(att == pd_find_50_v1(quot, rem, pd));
            return (!(h0 & mask)) && _mm_popcnt_u64(h0 & (mask - 1)) == quot;
        }


        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;

        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v10(int64_t quot, uint8_t rem, const __m512i *pd) {

        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v)
            return false;


        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;


        if ((_blsr_u64(v) == 0)) {
            if (v << quot) {
                const int64_t h0 = ((uint64_t *) pd)[0];
                // const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // const unsigned __int128 header = (*h);
                const int64_t mask = v << quot;
                const bool att = (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const unsigned __int128 mask = ((unsigned __int128) v) << quot;
                const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
            }
        }


        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;

        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v11(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = ((uint64_t *) pd)[0];
        const uint64_t h1 = ((uint64_t *) pd)[1];
        if ((_blsr_u64(v) == 0)) {
            if (v << quot) {
                // const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // const unsigned __int128 header = (*h);
                const int64_t mask = v << quot;
                const bool att = (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const unsigned __int128 *h = (const unsigned __int128 *) pd;
                constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                const unsigned __int128 header = (*h) & kLeftoverMask;
                const unsigned __int128 mask = ((unsigned __int128) v) << quot;
                const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
            }
        }

        const int64_t pop = _mm_popcnt_u64(h0);

        if (quot == 0) {
            return v & (_blsmsk_u64(((uint64_t *) pd)[0]) >> 1ul);

            // std::cout << "h0" << std::endl;
            // const uint64_t end = _tzcnt_u64(h0);
            // return (end) && (v & ((UINT64_C(1) << end) - 1));
            // bool res = (end) && (v & ((UINT64_C(1) << end) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));

        } else if (quot < pop - 1) {
            // std::cout << "h1" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else if (quot == pop - 1) {
            // std::cout << "h2" << std::endl;
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t diff = next_leading_one_index - leading_one_index - 1;
            const uint64_t temp = (63 - next_leading_one_index) + 1;
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_end = (63 - leading_one_index) - quot;
            // const uint64_t my_begin = (63 - next_leading_one_index) - quot + 1;
            // assert(my_begin <= my_end);
            // assert(my_begin == begin);
            // assert(my_end == end);
            // assert(begin + diff == end);
            // assert(temp == my_begin + quot);
            // return true;
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else if (quot == pop) {
            // std::cout << "h3" << std::endl;
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_begin = (63 - _lzcnt_u64(h0)) - quot + 1;
            // const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
            // assert(my_begin + diff == end);
            // assert(my_begin == begin);
            // assert(my_end == end);

            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else if (quot == pop + 1) {
            // std::cout << "h4" << std::endl;
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t diff = _tzcnt_u64(h1 >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_begin = 64 - quot + temp;
            // const uint64_t my_end = my_begin + _tzcnt_u64(h1 >> temp);
            // assert(my_begin == begin);
            // assert(my_end == end);
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else {
            // std::cout << "h5" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - pop - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_begin = temp - quot + 64;
            // const uint64_t my_end = my_begin + diff;
            // assert(my_begin == begin);
            // assert(my_end == end);
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        }
    }

    inline bool pd_find_50_v18(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = ((uint64_t *) pd)[0];
        const uint64_t h1 = ((uint64_t *) pd)[1];
        if (_blsr_u64(v) == 0) {
            if (v << quot) {
                // const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // const unsigned __int128 header = (*h);
                const int64_t mask = v << quot;
                const bool att = (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const unsigned __int128 *h = (const unsigned __int128 *) pd;
                constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                const unsigned __int128 header = (*h) & kLeftoverMask;
                const unsigned __int128 mask = ((unsigned __int128) v) << quot;
                const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
            }
        }

        const int64_t pop = _mm_popcnt_u64(h0);

        if (quot == 0) {
            // std::cout << "h0" << std::endl;
            return v & (_blsmsk_u64(((uint64_t *) pd)[0]) >> 1ul);
        } else if (quot < pop) {
            // std::cout << "h1" << std::endl;
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            // std::cout << "h2" << std::endl;

            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            // std::cout << "h3" << std::endl;

            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v19(int64_t quot, uint8_t rem, const __m512i *pd) {
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        // std::cout << "aSHSHNLBJB" << std::endl;

        if (!v) return false;

        // const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);

        // return (_blsr_u64(v) != 0) || (((unsigned __int128) v) << quot)
        // const uint64_t h1 = ((uint64_t *) pd)[1];

        if (_blsr_u64(v) == 0) {
            if (v << quot) {
                const uint64_t h0 = ((uint64_t *) pd)[0];
                const int64_t mask = v << quot;
                const bool att = (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const unsigned __int128 *h = (const unsigned __int128 *) pd;
                constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                const unsigned __int128 header = (*h) & kLeftoverMask;
                const unsigned __int128 mask = ((unsigned __int128) v) << quot;
                const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
            }
        } else {
            return true;
        }
    }


    inline bool pd_find_50_v5_validation(int64_t quot, uint8_t rem, const __m512i *pd) {
        static int counter = 0;
        /* Does not work */
        // assert(0);
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v || pd_find_body_elimination(quot, rem, pd, v)) return false;


        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);

        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        bool res = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        assert(res == pd_find_50_v1(quot, rem, pd));
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v2(int64_t quot, uint8_t rem, const __m512i *pd) {
        //// This is not relevant for pd512.
        assert(0);
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        if (quot == 0) {
            const uint64_t end = _tzcnt_u64(header);
            bool res = (end) && (v & ((UINT64_C(1) << end) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));
            return (end) && (v & ((UINT64_C(1) << end) - 1));
        } else {
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), header);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }


    inline bool pd_find_50_v3_validation(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const uint64_t h0 = ((uint64_t *) pd)[0];
        const uint64_t h1 = ((uint64_t *) pd)[1];
        const int64_t pop = _mm_popcnt_u64(h0);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;


        if (quot == 0) {
            // std::cout << "h0" << std::endl;
            const uint64_t end = _tzcnt_u64(h0);
            bool res = (end) && (v & ((UINT64_C(1) << end) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));

            return (end) && (v & ((UINT64_C(1) << end) - 1));
        } else if (quot < pop - 1) {
            // std::cout << "h1" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop - 1) {
            // std::cout << "h2" << std::endl;
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t diff = next_leading_one_index - leading_one_index - 1;
            const uint64_t temp = (63 - next_leading_one_index) + 1;
            const uint64_t my_end = (63 - leading_one_index) - quot;
            const uint64_t my_begin = (63 - next_leading_one_index) - quot + 1;
            assert(my_begin <= my_end);
            assert(my_begin == begin);
            assert(my_end == end);
            assert(begin + diff == end);
            assert(temp == my_begin + quot);
            // return true;
            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop) {
            // std::cout << "h3" << std::endl;
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            const uint64_t my_begin = (63 - _lzcnt_u64(h0)) - quot + 1;
            const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
            assert(my_begin + diff == end);
            assert(my_begin == begin);
            assert(my_end == end);

            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop + 1) {
            // std::cout << "h4" << std::endl;
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t diff = _tzcnt_u64(h1 >> temp);
            const uint64_t my_begin = 64 - quot + temp;
            const uint64_t my_end = my_begin + _tzcnt_u64(h1 >> temp);
            assert(my_begin == begin);
            assert(my_end == end);
            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else {
            // std::cout << "h5" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - pop - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - quot + 64;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);
            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50_v1(quot, rem, pd));
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v3(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = ((uint64_t *) pd)[0];
        const uint64_t h1 = ((uint64_t *) pd)[1];
        const int64_t pop = _mm_popcnt_u64(h0);

        if (quot == 0) {
            // std::cout << "h0" << std::endl;
            const uint64_t end = _tzcnt_u64(h0);
            return (end) && (v & ((UINT64_C(1) << end) - 1));
            // bool res = (end) && (v & ((UINT64_C(1) << end) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));

        } else if (quot < pop - 1) {
            // std::cout << "h1" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else if (quot == pop - 1) {
            // std::cout << "h2" << std::endl;
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t diff = next_leading_one_index - leading_one_index - 1;
            const uint64_t temp = (63 - next_leading_one_index) + 1;
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_end = (63 - leading_one_index) - quot;
            // const uint64_t my_begin = (63 - next_leading_one_index) - quot + 1;
            // assert(my_begin <= my_end);
            // assert(my_begin == begin);
            // assert(my_end == end);
            // assert(begin + diff == end);
            // assert(temp == my_begin + quot);
            // return true;
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else if (quot == pop) {
            // std::cout << "h3" << std::endl;
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_begin = (63 - _lzcnt_u64(h0)) - quot + 1;
            // const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
            // assert(my_begin + diff == end);
            // assert(my_begin == begin);
            // assert(my_end == end);

            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else if (quot == pop + 1) {
            // std::cout << "h4" << std::endl;
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t diff = _tzcnt_u64(h1 >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_begin = 64 - quot + temp;
            // const uint64_t my_end = my_begin + _tzcnt_u64(h1 >> temp);
            // assert(my_begin == begin);
            // assert(my_end == end);
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        } else {
            // std::cout << "h5" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - pop - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // const uint64_t my_begin = temp - quot + 64;
            // const uint64_t my_end = my_begin + diff;
            // assert(my_begin == begin);
            // assert(my_end == end);
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50_v1(quot, rem, pd));
        }
    }

    bool pd_find_50_old(int64_t quot, uint8_t rem, const __m512i *pd);

    inline bool pd_find_body_v1(int64_t quot, uint8_t rem, const __m512i *pd) {
        // std::cout << "tomer" << std::endl;
        // static int total_calls = 0;
        // static int c0 = 0;
        // static int c1 = 0;
        // static int c2 = 0;
        // total_calls++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) {
            // c0++;
            return false;
        }

        if (_mm_popcnt_u64(v) == 1) {
            const int64_t zero_count = _tzcnt_u64(v);
            if (zero_count + quot < 64) {
                // c1++;
                const int64_t h0 = ((uint64_t *) pd)[0];
                return (!(h0 & (1ULL << (quot + zero_count)))) && _mm_popcnt_u64(_bzhi_u64(h0, quot + zero_count)) == quot;
                /* const bool is_last_index_one = h0 & (1ULL << (quot + zero_count));
                // const int64_t temp = h0 & ((1ULL << (quot + zero_count)) - 1);
                // const int64_t temp2 = _bzhi_u64(h0, quot + zero_count);
                // assert(temp == temp2);
                const bool att = _mm_popcnt_u64(h0 & ((1ULL << (quot + zero_count)) - 1)) == quot;
                const bool att2 = _mm_popcnt_u64(_bzhi_u64(h0, quot + zero_count)) == quot;
                assert(att == att2);
                assert(((!is_last_index_one) && att) == pd_find_50_v1(quot, rem, pd));
                return (!is_last_index_one) && att; */
            } else {
                // c2++;
                const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                const unsigned __int128 header = (*h);
                return (!(header & (((unsigned __int128) 1) << (quot + zero_count)))) &&
                       (popcount128(header & (((unsigned __int128) 1 << (quot + zero_count)) - 1)) == quot);
            }
        }
        return true;
    }

    inline bool pd_find_body_v2(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        // return v && (!(*pd & ))
        if (_mm_popcnt_u64(v) == 1) {
            const int64_t zero_count = _tzcnt_u64(v);
            const int64_t h0 = ((uint64_t *) pd)[0];
            return (zero_count + quot >= 64) || ((!(h0 & (1ULL << (quot + zero_count)))) && _mm_popcnt_u64(_bzhi_u64(h0, quot + zero_count)) == quot);
        } else {
            return v != 0;
        }
    }

    inline bool pd_find_50(int64_t quot, uint8_t rem, const __m512i *pd) {
        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v5(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v3(quot, rem, pd));
        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v4(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v6(quot, rem, pd));
        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v7(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v8(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v9(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v10(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v11(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v18(quot, rem, pd));
        // return pd_find_50_v4(quot, rem, pd);
        // return pd_find_50_v11(quot, rem, pd);
        return pd_find_50_v11(quot, rem, pd);
        // return pd_find_50_v18(quot, rem, pd);
        // return pd_find_50_v10(quot, rem, pd);
        // return pd_find_body_v2(quot, rem, pd);
        // return pd_find_50_v8(quot, rem, pd);
        // return pd_find_50_v6(quot, rem, pd);
    }

    inline bool did_pd_overflowed_naive(const __m512i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        return h_array[1] & (1ULL << (102 - 64));
    }
    inline bool did_pd_overflowed(const __m512i *pd) {
        return (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 64);
        // return did_pd_overflowed_naive(pd);
        // bool att = (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 64);
        // assert(att == res);
        // if (att != res){
        //     std::cout << "att: " << att << std::endl;
        //     std::cout << "res: " << res << std::endl;
        //     assert(0);
        // }
        // assert(did_pd_overflowed_naive(pd) == (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 64));
        // uint64_t *h_array = ((uint64_t *) pd);
        // bool res =  h_array[1] & (1ULL << (102 - 64));

        // constexpr int index = 6;
        // __m128i header = _mm512_castsi512_si128(*pd);
        // bool att = (_mm_extract_epi16(header, index) & 64);
        // assert(att == res);
        // return (_mm_extract_epi16(header, index) & 64);
        // return _mm256_extract_epi16(*pd, 6) & 64;
        // constexpr __m512i set_bit = __m512i{0, INT64_C(1) << 38ul, 0, 0, 0, 0, 0, 0};
        // return (_mm512_maskz_and_epi32(3, *pd, set_bit));
        // _mm512_mask_cmpeq_epi16_mask (*pd, set_bit, __m512i b)
        // return _mm512_cmpeq_epu8_mask(*pd, set_bit) ;
        // return _mm512_cvtsi512_si32(_mm512_alignr_epi32(*pd, *pd, 102));
        // return ((uint64_t *) pd)[2] & 274877906944ULL;
        // uint64_t *h_array = ((uint64_t *) pd);
        // return h_array[1] & (1ULL << (102 - 64));
    }

    inline bool pd_find_special_50(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd_find_50_v11(quot, rem, pd) || did_pd_overflowed(pd);
    }

    // insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket dictionary.
    // Returns false if the dictionary is full.
    bool pd_add_50_old(int64_t quot, char rem, __m512i *pd);


    inline bool pd_add_special_50(int64_t quot, char rem, __m512i *pd) {
        assert(quot < 50);

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        assert(popcount128(header) == 50);
        const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51) {
            uint64_t *h_array = ((uint64_t *) pd);
            h_array[1] |= (1ULL << (102 - 64));
            return false;
        }

        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);


        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        // const uint64_t before_shift = _mm512_cmp_epi8_mask(target, *pd, 2);
        // uint64_t i = _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = (begin == end) ? end_fingerprint : _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        uint64_t i = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

        assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));

        memmove(&((char *) pd)[kBytes2copy + i + 1],
                &((const char *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((char *) pd)[kBytes2copy + i] = rem;

        assert(pd_find_50(quot, rem, pd));
        return true;
        // //// jbapple: STOPPED HERE
        // const __m512i target = _mm512_set1_epi8(rem);
        // uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // // round up to remove the header
        // v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }


    inline bool pd_add_50(int64_t quot, char rem, __m512i *pd) {
        assert(quot < 50);
        // The header has size 50 + 51
        // unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(kBytes2copy < sizeof(header));
        // memcpy(&header, pd, kBytes2copy);

        // Number of bits to keep. Requires little-endianness
        // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        // const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        // header = header & kLeftoverMask;

        assert(popcount128(header) == 50);
        const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51)
            return false;

        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);


        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        // const uint64_t before_shift = _mm512_cmp_epi8_mask(target, *pd, 2);
        // uint64_t i = _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        // uint64_t i = (begin == end) ? end_fingerprint : _tzcnt_u64(((before_shift >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        uint64_t i = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

        assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));

        memmove(&((char *) pd)[kBytes2copy + i + 1],
                &((const char *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((char *) pd)[kBytes2copy + i] = rem;

        assert(pd_find_50(quot, rem, pd));
        return true;
        // //// jbapple: STOPPED HERE
        // const __m512i target = _mm512_set1_epi8(rem);
        // uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // // round up to remove the header
        // v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }


    auto remove_naive(int64_t quot, char rem, __m512i *pd) -> bool;

    inline auto remove(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(quot < 50);
        assert(pd_find_50(quot, rem, pd));
        // The header has size 50 + 51
        // unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        assert(popcount128(header) == 50);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(kBytes2copy < sizeof(header));
        // memcpy(&header, pd, kBytes2copy);
        // auto my_temp = popcount128(header);
        // std::cout << "my_temp: " << my_temp << std::endl;
        // Number of bits to keep. Requires little-endianness
        // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        // const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        // header = header & kLeftoverMask;
        // assert(popcount128(header) == 50);
        //// const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        //// assert((fill <= 14) || (fill == pd_popcount(pd)));
        //// assert((fill == 51) == pd_full(pd));
        //// if (fill == 51)
        ////    return false;

        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        // assert(end <= 50 + 51);
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));

        assert(popcount128(new_header) == 50);
        //// assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);


        memcpy(pd, &new_header, kBytes2copy);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const char *) pd)[kBytes2copy + i])
                break;
        }
        assert(rem == ((const char *) pd)[kBytes2copy + i]);

        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        // ((char *) pd)[kBytes2copy + i] = rem;

        // assert(pd_find_50(quot, rem, pd));
        return true;
    }

    inline auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(quot < 50);
        // unsigned __int128 header = 0;
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        assert(popcount128(header) == 50);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(kBytes2copy < sizeof(header));
        // memcpy(&header, pd, kBytes2copy);
        // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        // const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        // header = header & kLeftoverMask;
        // assert(popcount128(header) == 50);
        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        /* const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        assert(begin <= end);
        assert(end <= 51); */

        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        // Consider using this function instead of this for.
        // __mmask64 _mm512_cmp_epu8_mask (__m512i a, __m512i b, const int imm8)

        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const char *) pd)[kBytes2copy + i])
                break;
        }

        if ((i == end_fingerprint) || (rem != ((const char *) pd)[kBytes2copy + i])) {
            // std::cout << "pd512::CR" << std::endl;
            return false;
        }

        assert(pd_find_50(quot, rem, pd));
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        // new_header |= header >> (end - 1);
        assert(popcount128(new_header) == 50);

        memcpy(pd, &new_header, kBytes2copy);
        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        return true;
    }


    void print512(const __m512i *var);


    inline auto pd_popcount_att_helper_start(const __m512i *pd) -> int {
        uint64_t header;
        memcpy(&header, reinterpret_cast<const uint64_t *>(pd), 8);
        return 14ul - _lzcnt_u64(header);
    }

    inline auto get_capacity(const __m512i *pd) -> int {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1, 5);
        constexpr uint64_t mask = (1ULL << 37u) - 1u;
        header_end &= mask;
        return (header_end == 0) ? pd_popcount_att_helper_start(pd) : (128 - 51 - _lzcnt_u64(header_end) + 1);
    }


    auto validate_number_of_quotient(const __m512i *pd) -> bool;


    auto get_capacity_old(const __m512i *x) -> size_t;

    auto get_capacity_naive(const __m512i *x) -> size_t;

    auto get_name() -> std::string;
    ////New functions
    inline auto is_full(const __m512i *x) -> bool {
        assert(get_capacity_naive(x) == get_capacity(x));
        return get_capacity(x) == 51;
    }
}// namespace pd512


#endif// FILTERS_PD512_HPP


auto my_equal(__m512i x, __m512i y) -> bool;
