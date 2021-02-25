/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD512_SPLIT_HPP
#define FILTERS_PD512_SPLIT_HPP

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

    auto validate_header(const __m512i *pd) -> bool;

    inline void print_headers(const __m512i *pd) {
        // constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        std::cout << "h0: " << bin_print_header_spaced(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced(h1) << std::endl;
    }

    inline void print_headers_masked(const __m512i *pd) {
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;
        std::cout << "h0: " << bin_print_header_spaced(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced(h1) << std::endl;
    }
}// namespace v_pd512

namespace pd512 {
    auto count_ones_up_to_the_kth_zero(const __m512i *x, size_t k = 51) -> size_t;

    auto count_zeros_up_to_the_kth_one(const __m512i *x, size_t k) -> size_t;

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

    inline int popcnt128(__m128i n) {
        const __m128i n_hi = _mm_unpackhi_epi64(n, n);
        return _mm_popcnt_u64(_mm_cvtsi128_si64(n)) + _mm_popcnt_u64(_mm_cvtsi128_si64(n_hi));
    }

    auto get_capacity_naive_with_OF_bit(const __m512i *x) -> size_t;

    inline auto get_capacity(const __m512i *pd) -> int {
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;
        const size_t total_pop = _mm_popcnt_u64(h0) + _mm_popcnt_u64(h1);
        assert(total_pop == 50);
        const int att = (h1 == 0) ? 14ul - _lzcnt_u64(h0) : (128 - 50 - _lzcnt_u64(h1));
        // assert(att == get_capacity_naive_with_OF_bit(pd));
        return (h1 == 0) ? 14ul - _lzcnt_u64(h0) : (128 - 50 - _lzcnt_u64(h1));
        // uint64_t header_end;
        // memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1, 5);
        // constexpr uint64_t mask = (1ULL << 37u) - 1u;
        // header_end &= mask;
        // return (header_end == 0) ? pd_popcount_att_helper_start(pd) : (128 - 51 - _lzcnt_u64(header_end) + 1);
    }

    // inline __m128i popcnt8(__m128i n) {
    //     static const __m128i popcount_mask = _mm_set1_epi8(0x0F);
    //     static const __m128i popcount_table = _mm_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
    //     const __m128i pcnt0 = _mm_shuffle_epi8(popcount_table, _mm_and_si128(n, popcount_mask));
    //     const __m128i pcnt1 = _mm_shuffle_epi8(popcount_table, _mm_and_si128(_mm_srli_epi16(n, 4), popcount_mask));
    //     return _mm_add_epi8(pcnt0, pcnt1);
    // }

    inline bool pd_full_naive(const __m512i *pd) {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        return 1 & (header_end >> (101 - 1 - 64));
    }
    inline bool pd_full(const __m512i *pd) {
        const bool att = _mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 16;
        assert(att == (get_capacity(pd) == 51));
        assert(att == pd_full_naive(pd));
        return (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 16);

        // uint64_t header_end;
        // memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
        //        sizeof(header_end));
        // return 1 & (header_end >> (50 + 51 - 64 - 1));
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

    inline bool is_full(const __m512i *pd) {
        assert(pd_full(pd) == pd_full_naive(pd));
        assert((pd_popcount(pd) == 51) == pd_full(pd));
        return pd_full(pd);
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
        assert(popcount128(header) == 50);
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

    inline bool pd_find_50_v11_backup(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        // const uint64_t h0 = ((uint64_t *) pd)[0];
        // const uint64_t h1 = ((uint64_t *) pd)[1];

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        // assert(h0_att == h0);
        // assert(h1_att == h1);
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
            return v & (_blsmsk_u64(_mm_extract_epi64(_mm512_castsi512_si128(*pd), 0)) >> 1ul);

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

    inline bool pd_find_50_v11(int64_t quot, uint8_t rem, const __m512i *pd) {

        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);

        if ((_blsr_u64(v) == 0)) {
            if (v << quot) {
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
            return v & (_blsmsk_u64(_mm_extract_epi64(_mm512_castsi512_si128(*pd), 0)) >> 1ul);
        } else if (quot < pop - 1) {
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop - 1) {
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t diff = next_leading_one_index - leading_one_index - 1;
            const uint64_t temp = (63 - next_leading_one_index) + 1;
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop) {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop + 1) {
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t diff = _tzcnt_u64(h1 >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else {
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - pop - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v12(int64_t quot, uint8_t rem, const __m512i *pd) {
        const __m512i target = _mm512_set1_epi8(rem);
        const int64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        if ((_blsr_u64(v) == 0)) {
            // const unsigned __int128 mask_helper1 = ((unsigned __int128) v) << quot;
            const unsigned __int128 x = ((unsigned __int128) v) << quot;
            const long long int hi = x >> 64;
            const long long int lo = x;
            const __m128i v128 = __m128i{lo, hi};
            constexpr __m128i header_mask = __m128i{-1, (UINT64_C(1) << (101 - 64)) - 1};
            const __m128i header128 = _mm_and_si128(_mm512_castsi512_si128(*pd), header_mask);
            bool att1 = _mm_test_all_zeros(header128, v128);

            // const __int128 mask_helper = (((unsigned __int128) v) << quot) - 1;
            const __int128 y = (((unsigned __int128) v) << quot) - 1;
            const long long int hi2 = y >> 64;
            const long long int lo2 = y;
            const __m128i mask = __m128i{lo2, hi2};
            int pop_res = popcnt128(_mm_and_si128(header128, mask));
            bool att2 = popcnt128(_mm_and_si128(header128, mask)) == quot;
            return att1 && att2;

            // bool att = att1 && att2;
            // bool to_print = (att != pd_find_50_v1(quot, rem, pd));
            // if (to_print) {
            //     const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
            //     const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
            //     // const unsigned __int128 shifted_header = header >> (quot + zero_count);
            //     // std::cout << v_pd512::bin_print_header_spaced128(header) << std::endl;
            //     // std::cout << v_pd512::bin_print_header_spaced128(shifted_header) << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced(h0) << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced(h1) << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced(v) << std::endl;
            //     bool is_case_one = (v << quot);
            //     std::cout << is_case_one << std::endl;

            //     if (is_case_one) {
            //         const int64_t val_mask = v << quot;
            //         std::cout << "val_mask: " << val_mask << std::endl;
            //     } else {
            //         std::cout << "bahhh" << std::endl;
            //     }

            //     // std::cout << v_pd512::bin_print_header_spaced(v) << std::endl;
            //     // std::cout << v_pd512::bin_print_header_spaced(mask_m1) << std::endl;
            //     assert(0);
            // }
            // assert(att == pd_find_50_v1(quot, rem, pd));
            // return att1 && att2;
        }
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);


        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(_mm_extract_epi64(_mm512_castsi512_si128(*pd), 0)) >> 1ul);
        } else if (quot < pop - 1) {
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop - 1) {
            // std::cout << "h2" << std::endl;
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t diff = next_leading_one_index - leading_one_index - 1;
            const uint64_t temp = (63 - next_leading_one_index) + 1;
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop) {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else if (quot == pop + 1) {
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t diff = _tzcnt_u64(h1 >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        } else {
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - pop - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v17(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            if (v << quot) {
                const int64_t mask = v << quot;
                const bool att = (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
                assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const int64_t pop = _mm_popcnt_u64(h0);
                const uint64_t index = (_tzcnt_u64(v) + quot) & 63;
                const int64_t mask = (1ULL << index);
                const bool att = (!(h1 & mask)) && (_mm_popcnt_u64(h1 & (mask - 1)) == (quot - pop));
                assert(att == pd_find_50_v1(quot, rem, pd));

                // const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                // const unsigned __int128 header = (*h) & kLeftoverMask;
                // const unsigned __int128 mask = ((unsigned __int128) v) << quot;

                // const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
                // assert(att == pd_find_50_v1(quot, rem, pd));
                // return (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
            }
        }

        const int64_t pop = _mm_popcnt_u64(h0);

        if (quot == 0) {
            // std::cout << "h0" << std::endl;
            return v & (_blsmsk_u64(h0) >> 1ul);
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

    inline bool pd_find_50_v18(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
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
            return v & (_blsmsk_u64(h0) >> 1ul);
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
        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v4(quot, rem, pd));
        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v7(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v3(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v6(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v8(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v9(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v10(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v11(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v12(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v17(quot, rem, pd));
        assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v18(quot, rem, pd));
        // return pd_find_50_v4(quot, rem, pd);
        // return pd_find_50_v11(quot, rem, pd);
        // return pd_find_50_v12(quot, rem, pd);
        // return pd_find_50_v11(quot, rem, pd);
        return pd_find_50_v18(quot, rem, pd);
        // return pd_find_50_v18(quot, rem, pd);
        // return pd_find_50_v10(quot, rem, pd);
        // return pd_find_body_v2(quot, rem, pd);
        // return pd_find_50_v8(quot, rem, pd);
        // return pd_find_50_v6(quot, rem, pd);
    }

    inline bool did_pd_overflowed_naive(const __m512i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        return h_array[1] & (1ULL << (101 - 64));
    }
    inline bool did_pd_overflowed(const __m512i *pd) {
        return (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 32);
        // bool res = _mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 32;
        // assert(res == did_pd_overflowed_naive(pd));
        // return (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 32);
    }
    inline void set_overflow_bit(__m512i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        h_array[1] |= (1ULL) << (101 - 64);
        assert(did_pd_overflowed(pd));
    }
    inline void clear_overflow_bit(__m512i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        // assert(h_array[1] & (1ULL) << (101 - 64));
        assert(did_pd_overflowed(pd));
        h_array[1] ^= (1ULL) << (101 - 64);
        assert(!did_pd_overflowed(pd));

        // h_array[1] &= (((1ULL << 101 - 64) - 1) | ~(((1ULL << 41) - 1)));
    }

    // inline bool did_pd_overflowed2(const __m512i *pd) {
    //     constexpr __m128i x = __m128i{0, 1ULL << 38ul};
    //     return _mm_extract_epi16(_mm_and_si128(_mm512_castsi512_si128(*pd), x), 6);
    //     // bool res = (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 64);
    //     // bool att = _mm_extract_epi16(_mm_and_si128(_mm512_castsi512_si128(*pd), x), 6);
    //     // assert(res == att);
    //     // return (_mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 64);
    // }
    inline bool pd_find_special_50(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd_find_50_v18(quot, rem, pd) || did_pd_overflowed(pd);
        // return pd_find_50_v18(quot, rem, pd) || did_pd_overflowed(pd);
        // return pd_find_50_v11(quot, rem, pd) || did_pd_overflowed(pd);
    }

    enum pd_Status {
        No,
        Yes,
        look_in_the_next_level
    };

    inline pd_Status pd_find_enums(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd_find_50_v18(quot, rem, pd) ? Yes : (did_pd_overflowed(pd) ? look_in_the_next_level : No);
        // return pd_find_50_v18(quot, rem, pd) || did_pd_overflowed(pd);
        // return pd_find_50_v18(quot, rem, pd) || did_pd_overflowed(pd);
        // return pd_find_50_v11(quot, rem, pd) || did_pd_overflowed(pd);
    }



    inline uint8_t get_last_quot_in_pd_naive_bf(const __m512i *pd) {
        /* This is complete garbage */
        assert(0);

        assert(false);
        return -1;
    }

    inline uint8_t get_last_quot_in_pd_super_naive(const __m512i *pd) {
        return count_ones_up_to_the_kth_zero(pd);
    }
    
    inline uint8_t get_last_quot_in_pd_naive(const __m512i *pd) {
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;

        if (h1 == 0) {
            uint64_t temp = (~h0) & (h0 >> 1);
            assert(temp);
            size_t first_one = _lzcnt_u64(h0);
            size_t temps_first_one = _lzcnt_u64(temp);
            assert(temps_first_one > first_one);
            return temps_first_one - first_one;
        }

        uint64_t temp = (~h1) & (h1 >> 1);
        if (temp == 0) {
            size_t end = _tzcnt_u64(~h1);
            size_t begin = _lzcnt_u64(~h0);
            return end + begin;
        }
        assert(_lzcnt_u64(temp) > _lzcnt_u64(h1));
        return _lzcnt_u64(temp) - _lzcnt_u64(h1);
    }

    inline uint8_t get_last_quot_in_pd(const __m512i *pd) {
        auto a = 50 - get_last_quot_in_pd_naive(pd);
        auto b = get_last_quot_in_pd_super_naive(pd);
        if (a != b) {
            v_pd512::print_headers_masked(pd);
        }
        assert(a == b);
        return 50 - get_last_quot_in_pd_naive(pd);
    }

    inline uint8_t get_last_qr_in_pd(const __m512i *pd) {
        uint64_t quot = 50 - get_last_quot_in_pd_naive(pd);
        constexpr int imm1 = 3;
        constexpr int imm2 = 15;
        const uint64_t rem = _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);
        return (quot << 8ul) | rem;
    }

    inline uint64_t pd_add_50_Wed(int64_t quot, char rem, __m512i *pd) {
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
            assert(false);
        }

        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
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
        return 1ul << 15;
    }


    inline auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool {
        /* This function has na error. */
        assert(quot < 50);
        // unsigned __int128 header = 0;
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        assert(popcount128(header) == 50);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        const int64_t pop = _mm_popcnt_u64(h0);
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;

        const uint64_t b_mask = ~((1ull << (begin_fingerprint)) - 1);
        const uint64_t end_mask = ((1ull << end_fingerprint) - 1);
        const uint64_t mask = b_mask & end_mask;
        assert((begin < end) ? mask : !mask);
        const uint64_t v_masked = v & mask;
        if (!v_masked) return false;

        const uint64_t i = _tzcnt_u64(v_masked);
        assert(pd_find_50(quot, rem, pd));
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        assert(popcount128(new_header) == 50);
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;


        memcpy(pd, &new_header, kBytes2copy);
        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        return true;
    }

    inline int my_cmp(int64_t quot1, char rem1, int64_t quot2, char rem2) {
        if (quot1 != quot2)
            return quot1 < quot2;
        return rem1 <= rem2;
    }

    inline uint64_t pd_swap(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(quot < 50);
        const size_t capacity = pd_popcount(pd);
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;

        assert(h1);
        const uint64_t temp = h1 << (64 - 37);
        assert((temp >> (64 - 37)) == h1);
        const uint64_t index = _lzcnt_u64(temp);

        const int64_t old_quot = get_last_quot_in_pd(pd);
        if (old_quot < quot) {
            // std::cout << "pd_swap 0" << std::endl;
            set_overflow_bit(pd);
            uint64_t res = (quot << 8u) | ((uint64_t) rem);
            assert(res);
            return res;
        }
        constexpr int imm1 = 3;
        constexpr int imm2 = 15;
        const uint8_t old_rem = _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);


        if ((old_quot == quot) && (old_rem <= rem)) {
            // std::cout << "pd_swap 1" << std::endl;
            set_overflow_bit(pd);
            uint64_t res = (quot << 8u) | ((uint64_t) rem);
            if (!res) {
                v_pd512::print_headers_masked(pd);
            }
            assert(res);
            return res;
            // return (quot << 8u) | ((uint8_t)rem);
        }

        const uint64_t old_qr = (old_quot << 8) | ((uint64_t) old_rem);
        const uint64_t new_qr = (quot << 8) | ((uint64_t) rem);
        if (old_qr <= new_qr) {
            v_pd512::bin_print(old_qr);
        }
        assert(old_qr > new_qr);

        const __m512i target = _mm512_set1_epi8(old_rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        assert(v & (1ULL << 63));

        //todo: change to -> remove last.
        conditional_remove(old_quot, old_rem, pd);
        pd_add_50_Wed(quot, rem, pd);
        set_overflow_bit(pd);
        uint64_t res = (old_quot << 8u) | ((uint64_t) old_rem);
        assert(res);
        // std::cout << "pd_swap 2" << std::endl;
        return res;
        // return (old_quot << 8u) | old_rem;
    }
    // insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket dictionary.
    // Returns false if the dictionary is full.

    inline uint64_t pd_conditional_add_50_db(int64_t quot, uint8_t rem, __m512i *pd) {
        bool is_full_cond = is_full(pd);
        if (is_full_cond) {
            // std::cout << "pd_swap" << std::endl;
            uint64_t res = pd_swap(quot, rem, pd);
            assert(res);
            uint64_t new_quot = (res >> 8);
            assert(new_quot < 50);
            return res;
        } else {
            // std::cout << "pd_add_wed" << std::endl;
            uint64_t res = pd_add_50_Wed(quot, rem, pd);
            assert(res);
            return res;
        }
        // return (is_full(pd)) ? pd_swap(quot, rem, pd) : pd_add_50_Wed(quot, rem, pd);
    }

    inline uint64_t pd_conditional_add_50(int64_t quot, uint8_t rem, __m512i *pd) {
        return pd_conditional_add_50_db(quot, rem, pd);
        // switch (is_full(pd)) {
        //     case true:
        //         return pd_swap(quot, rem, pd);
        //     case false:
        //         return pd_add_50_Wed(quot, rem, pd);
        //     default:
        //         break;
        // }
        // // return (is_full(pd)) ? pd_swap(quot, rem, pd) : pd_add_50_Wed(quot, rem, pd);
    }
    bool pd_add_50_old(int64_t quot, char rem, __m512i *pd);


    inline bool pd_add_special_50_v2(int64_t quot, char rem, __m512i *pd) {
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
            set_overflow_bit(pd);
            return false;
        }

        //should add cases to speed up the computation of the new header.
        const uint64_t end = select128(header, quot);
        // if (end < 64){

        // }

        unsigned __int128 high_header = (header << 1) & ~((((unsigned __int128) 1) << (end + 1)) - 1);
        unsigned __int128 low_header = header & ((((unsigned __int128) 1) << (end)) - 1);
        unsigned __int128 temp = high_header & low_header;
        assert(!temp);
        unsigned __int128 new_header = high_header | low_header;
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        memcpy(pd, &new_header, kBytes2copy);

        // const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        memmove(&((char *) pd)[kBytes2copy + end_fingerprint + 1],
                &((const char *) pd)[kBytes2copy + end_fingerprint],
                sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((char *) pd)[kBytes2copy + end_fingerprint] = rem;

        assert(pd_find_50(quot, rem, pd));
        return true;
    }

    inline bool pd_add_special_50(int64_t quot, char rem, __m512i *pd) {
        assert(quot < 50);

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(popcount128(header) == 50);
        // assert(popcount128(header) == 50);
        const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51) {
            set_overflow_bit(pd);
            // uint64_t *h_array = ((uint64_t *) pd);
            // h_array[1] |= (1ULL << (101 - 64));
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
        assert(popcount128(header) == 50 || popcount128(header) == 51);
        // assert(popcount128(new_header) == 50);
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

    inline bool pd_add_50_v0(int64_t quot, char rem, __m512i *pd) {
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
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
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
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        assert(popcount128(header) == 50);
        const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51)
            return false;


        const uint64_t end = select128(header, quot);

        unsigned __int128 high_header = (header << 1) & ~((((unsigned __int128) 1) << (end + 1)) - 1);
        unsigned __int128 low_header = header & ((((unsigned __int128) 1) << (end)) - 1);
        unsigned __int128 temp = high_header & low_header;
        assert(!temp);
        unsigned __int128 new_header = high_header | low_header;
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        memcpy(pd, &new_header, kBytes2copy);

        // const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        memmove(&((char *) pd)[kBytes2copy + end_fingerprint + 1],
                &((const char *) pd)[kBytes2copy + end_fingerprint],
                sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((char *) pd)[kBytes2copy + end_fingerprint] = rem;

        assert(pd_find_50(quot, rem, pd));
        return true;
    }


    auto remove_naive(int64_t quot, char rem, __m512i *pd) -> bool;

    inline auto remove(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(false);
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

    inline auto conditional_remove_old(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(quot < 50);
        // unsigned __int128 header = 0;
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        // auto temp = did_pd_overflowed(pd);
        // assert(pd_popcount(pd) == 50);
        // assert(popcount128(header) == 50 || popcount128(header) == 51);
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
        assert(popcount128(header) == 50);
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 102ul : 0;

        // assert(pd_popcount(pd) == 50);
        // new_header |= header >> (end - 1);
        // assert(popcount128(header) == 50);
        // assert(popcount128(header) == 50 || popcount128(header) == 51);
        // assert(popcount128(new_header) == 50);

        // bool did_overflow_before = did_pd_overflowed(pd);
        // if (did_overflow_before) {
        //     std::cout << v_pd512::bin_print_header_spaced128(header) << std::endl;
        //     std::cout << v_pd512::bin_print_header_spaced128(new_header) << std::endl;
        //     std::cout << std::string(80, '*') << std::endl;
        // }
        const bool did_overflow_before = did_pd_overflowed(pd);
        memcpy(pd, &new_header, kBytes2copy);
        assert((did_overflow_before) ? did_pd_overflowed(pd) : 1);

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

    inline auto get_capacity_without_OF_bit(const __m512i *pd) -> int {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1, 5);
        constexpr uint64_t mask = (1ULL << 37u) - 1u;
        header_end &= mask;
        return (header_end == 0) ? pd_popcount_att_helper_start(pd) : (128 - 51 - _lzcnt_u64(header_end) + 1);
    }


    // auto get_capacity(const __m512i *pd) -> int;


    auto validate_number_of_quotient(const __m512i *pd) -> bool;


    auto get_capacity_old(const __m512i *x) -> size_t;

    auto get_capacity_naive(const __m512i *x) -> size_t;


    auto get_name() -> std::string;
    ////New functions
    // inline auto is_full(const __m512i *x) -> bool {
    //     assert(get_capacity_naive(x) == get_capacity(x));
    //     return get_capacity(x) == 51;
    // }
}// namespace pd512


#endif// FILTERS_PD512_SPLIT_HPP


auto my_equal(__m512i x, __m512i y) -> bool;
