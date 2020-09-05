/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD320_HPP
#define FILTERS_PD320_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <immintrin.h>
// #include "immintrin.h"
//#include "x86intrin.h"

namespace v_pd320 {
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

}// namespace v_pd320


namespace pd320 {
    void validate_clz(int64_t quot, char rem, const __m512i *pd);

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select64(uint64_t x, int64_t j) {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    auto is_pd_full_naive(const __m512i *pd) -> bool;

    inline bool pd_full(const __m512i *pd) {
        return (((uint64_t *) pd)[0]) & (1ULL << 63);
        // const uint64_t header = ((uint64_t *) pd)[0];
        // bool res = header & (1ULL << 63);
        // assert(res == is_pd_full_naive(pd));
        // return header & (1ULL << 63);
    }

    auto get_capacity_att(const __m512i *x) -> size_t;

    // inline int pd_popcount(const __m512i *pd) {
    //     return get_capacity_att(pd);
    // }

    inline auto get_capacity(const __m512i *pd) -> int {
        const uint64_t header = ((uint64_t *) pd)[0];
        assert(_mm_popcnt_u64(header) == 32);
        return 32 - _lzcnt_u64(header);
    }

    //Another way to compte begin and end.
    inline void test_get_begin_and_end(int64_t quot, uint8_t rem, const __m512i *pd) {
        const uint64_t header = ((uint64_t *) pd)[0];
        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0) - quot;
        const uint64_t end_valid = select64(header, quot) - quot;
        uint64_t my_begin = 0;
        uint64_t my_end = 0;
        uint64_t shifted_header = header;

        int caser = 0;
        if (quot == 0) {
            caser = 1;
            my_begin = 0;
            my_end = _tzcnt_u64(header);
            assert(header);
            assert(my_end < 64);
            assert(my_begin <= my_end);
        } else {
            caser = 2;
            uint64_t temp = select64(header, quot - 1) + 1;
            my_begin = temp - quot;
            shifted_header = header >> temp;
            assert(shifted_header);
            uint64_t temp2 = _tzcnt_u64(shifted_header);
            assert(temp2 < 32);
            my_end = my_begin + temp2;
            assert(my_begin <= my_end);
        }

        assert(my_begin == begin);
        if (my_end != end_valid) {
            size_t line_length = 120;
            std::string line = std::string(line_length, '*');
            std::string line_mid = std::string(line_length, '_');
            std::cout << line << std::endl;
            std::cout << "caser: \t\t\t" << caser << std::endl;
            std::cout << "quot:  \t\t\t" << quot << std::endl;
            std::cout << "begin: \t\t\t " << begin << std::endl;
            std::cout << "end_valid: \t\t" << end_valid << std::endl;
            std::cout << "my_end: \t\t" << my_end << std::endl;
            // std::cout << "header_begin: " << header_begin << std::endl;
            // std::cout << "header_end: " << header_end << std::endl;
            // std::cout << "header_begin - quot: " << header_begin - quot << std::endl;
            // std::cout << "header_end - quot: " << header_end - quot << std::endl;

            std::cout << line_mid << std::endl;

            std::cout << "header: " << header << std::endl;
            std::cout << "bin(header): \t\t";
            v_pd320::bin_print_header(header);
            std::cout << std::endl;

            std::cout << "bin(shifted_header):\t";
            v_pd320::bin_print_header(shifted_header);
            std::cout << std::endl;

            std::cout << line_mid << std::endl;
            std::cout << "header:   " << v_pd320::bin_print_header_spaced(header) << std::endl;
            std::cout << "s_header: " << v_pd320::bin_print_header_spaced(shifted_header) << std::endl;


            std::cout << line << std::endl;


            // std::cout << "end2:\t " << end2 << std::endl;
            // std::cout << "end3:\t " << end3 << std::endl;
            // std::cout << "_tzcnt_u64(header >> begin): \t" << _tzcnt_u64(header >> begin) << std::endl;
            assert(0);
        }
        assert(my_end == end_valid);
    }

    inline bool pd_find_32_ver1(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 32);

        const uint64_t header = ((uint64_t *) pd)[0];

        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0) - quot;
        const uint64_t end = select64(header, quot) - quot;
        assert(begin <= end);
        assert(end <= 32);

        if (begin == end)
            return false;

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        v = v >> 8;
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_32_ver2(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 32);

        const uint64_t header = ((uint64_t *) pd)[0];
        uint64_t begin_helper, end_helper;
        if (quot == 0) {
            begin_helper = 0;
            end_helper = _tzcnt_u64(header);
        } else {
            uint64_t temp = select64(header, quot - 1) + 1;
            begin_helper = temp - quot;
            end_helper = begin_helper + _tzcnt_u64(header >> temp);
        }
        if (begin_helper == end_helper) return false;

        assert(begin_helper <= end_helper);
        const uint64_t begin = begin_helper;
        const uint64_t end = end_helper;
        assert(begin <= end);
        assert(end <= 32);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        v = v >> 8;
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_32_ver3(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 32);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        v = v >> 8;
        if (v == 0) return false;

        const uint64_t header = ((uint64_t *) pd)[0];
        // const uint64_t valid_begin = (quot ? (select64(header, quot - 1) + 1) : 0) - quot;
        // const uint64_t valid_end = select64(header, quot) - quot;

        uint64_t begin_helper, end_helper;
        if (quot == 0) {
            begin_helper = 0;
            end_helper = _tzcnt_u64(header);
        } else {
            const uint64_t mask = (UINT64_C(3) << quot - 1);
            const uint64_t y = _pdep_u64(mask, header);
            uint64_t temp = _tzcnt_u64(y) + 1;
            // uint64_t valid_temp = select64(header, quot - 1) + 1;
            // assert(valid_temp == temp);
            begin_helper = temp - quot;
            end_helper = begin_helper + _tzcnt_u64(y >> temp);
        }
        if (begin_helper == end_helper) return false;

        return (begin_helper < end_helper) && ((v & ((UINT64_C(1) << end_helper) - 1)) >> begin_helper);
        // assert(begin_helper <= end_helper);
        // assert(begin_helper == valid_begin);
        // assert(end_helper == valid_end);
        // const uint64_t begin = begin_helper;
        // const uint64_t end = end_helper;
        // assert(begin <= end);
        // assert(end <= 32);


        /* very_slow:
        constexpr unsigned kBytes2copy = 8;
        uint64_t i = begin_helper;
        for (; i < end_helper; ++i) {
            if (rem == ((const char *) pd)[kBytes2copy + i])
                return true;
        }
        return false; */

        // const __m512i target = _mm512_set1_epi8(rem);
        // uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // v = v >> 8;
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }
    
    inline bool pd_find_32_ver4(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 32);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        v = v >> 8;
        if (!v) return false;

        const uint64_t header = ((uint64_t *) pd)[0];
        if (quot == 0) {
            const uint64_t end_helper = _tzcnt_u64(header);
            return (end_helper) && (v & ((UINT64_C(1) << end_helper) - 1));
        } else {
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), header);
            uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
        
    }

    inline bool pd_find_32(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd_find_32_ver4(quot, rem, pd);
        // round up to remove the header
        // constexpr unsigned kHeaderBytes = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(kHeaderBytes < sizeof(header));
        // v = v >> 8;
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;

        /* //Todo
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // return (v >> 1u) & v;
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
        // uint64_t v = quot;
        // round up to remove the header
        constexpr unsigned kHeaderBytes = (64 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kHeaderBytes < sizeof(header));
        v = v >> kHeaderBytes;
        return (v & ((UINT64_C(1) << end) - 1)) >> begin; */
    }


    inline bool pd_add_32(int64_t quot, char rem, __m512i *pd) {
        assert(quot < 32);
        const uint64_t header = ((uint64_t *) pd)[0];
        constexpr unsigned kBytes2copy = 8;
        assert(_mm_popcnt_u64(header) == 32);
        const unsigned fill = select64(header, 32 - 1) - (32 - 1);
        // assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 32) == pd_full(pd));
        if (fill == 32)
            return false;

        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= 63);
        const __m512i target = _mm512_set1_epi8(rem);

        uint64_t new_header = header & ((1ULL << begin) - 1);
        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(_mm_popcnt_u64(new_header) == 32);
        assert(select64(new_header, 32 - 1) - (32 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 32);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const char *) pd)[kBytes2copy + i])
                break;
        }
        assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));
        memmove(&((char *) pd)[kBytes2copy + i + 1],
                &((const char *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((char *) pd)[kBytes2copy + i] = rem;

        assert(pd_find_32(quot, rem, pd));
        return true;
        // //// jbapple: STOPPED HERE
        //.......
    }

    auto remove_naive(int64_t quot, char rem, __m512i *pd) -> bool;

    inline auto remove(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(quot < 32);
        assert(pd_find_32(quot, rem, pd));
        const uint64_t header = ((uint64_t *) pd)[0];
        assert(_mm_popcnt_u64(header) == 32);
        constexpr unsigned kBytes2copy = 8;

        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= 63);

        uint64_t new_header = header & ((1ULL << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        assert(_mm_popcnt_u64(new_header) == 32);

        memcpy(pd, &new_header, kBytes2copy);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 32);
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

        // assert(pd_find_32(quot, rem, pd));
        return true;
    }

    inline auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool {

        assert(quot < 32);
        // assert(pd_find_32(quot, rem, pd));
        const uint64_t header = ((uint64_t *) pd)[0];
        assert(_mm_popcnt_u64(header) == 32);
        constexpr unsigned kBytes2copy = 8;

        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= 63);

        uint64_t new_header = header & ((1ULL << begin) - 1);
        new_header |= ((header >> end) << (end - 1));

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 32);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const char *) pd)[kBytes2copy + i])
                break;
        }
        if ((i == end_fingerprint) || (rem != ((const char *) pd)[kBytes2copy + i])) {
            return false;
        }

        assert(_mm_popcnt_u64(new_header) == 32);
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

    auto validate_number_of_quotient(const __m512i *pd) -> bool;

    auto get_capacity_naive(const __m512i *x) -> size_t;

    auto get_name() -> std::string;
    ////New functions
    inline auto is_full(const __m512i *x) -> bool {
        assert(get_capacity_naive(x) == get_capacity(x));
        return get_capacity(x) == 32;
    }
}// namespace pd320


#endif// FILTERS_PD320_HPP

auto my_equal(__m512i x, __m512i y) -> bool;
