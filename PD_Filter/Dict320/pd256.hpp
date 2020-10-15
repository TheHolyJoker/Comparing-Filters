/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD256_HPP
#define FILTERS_PD256_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include "x86intrin.h"
#include <immintrin.h>

#define QUOT_SIZE22 (22)
#define CAPACITY26 (26)

namespace v_pd256 {
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

}// namespace v_pd256


namespace pd256 {

    auto validate_number_of_quotient(const __m256i *pd) -> bool;
    auto validate_number_of_quotient(uint64_t clean_header) -> bool;
    auto get_capacity_att(const __m256i *x) -> size_t;


    void validate_clz(int64_t quot, char rem, const __m256i *pd);

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select64(uint64_t x, int64_t j) {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    auto is_pd_full_naive(const __m256i *pd) -> bool;

    inline bool pd_full(const __m256i *pd) {
        assert(validate_number_of_quotient(pd));
        auto res = (((uint64_t *) pd)[0]) & (1ULL << 47);
        bool v_res = (get_capacity_att(pd) == CAPACITY26);
        assert((!!res) == (get_capacity_att(pd) == CAPACITY26));
        return res;
    }


    // inline int pd_popcount(const __m256i *pd) {
    //     return get_capacity_att(pd);
    // }

    inline auto get_clean_header(const __m256i *pd) -> uint64_t {
        auto res = (_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd))) & ((1ULL << 48) - 1);
        assert(validate_number_of_quotient(res));
        return (_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd))) & ((1ULL << 48) - 1);
    }

    inline auto get_capacity(const __m256i *pd) -> int {
        const uint64_t header = get_clean_header(pd);
        auto res = 42 - _lzcnt_u64(header);
        auto v_res = get_capacity_att(pd);
        assert(v_res == res);
        assert(res == get_capacity_att(pd));
        return res;
    }


    //Another way to compte begin and end.
    inline void test_get_begin_and_end(int64_t quot, uint8_t rem, const __m256i *pd) {
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
            v_pd256::bin_print_header(header);
            std::cout << std::endl;

            std::cout << "bin(shifted_header):\t";
            v_pd256::bin_print_header(shifted_header);
            std::cout << std::endl;

            std::cout << line_mid << std::endl;
            std::cout << "header:   " << v_pd256::bin_print_header_spaced(header) << std::endl;
            std::cout << "s_header: " << v_pd256::bin_print_header_spaced(shifted_header) << std::endl;


            std::cout << line << std::endl;


            // std::cout << "end2:\t " << end2 << std::endl;
            // std::cout << "end3:\t " << end3 << std::endl;
            // std::cout << "_tzcnt_u64(header >> begin): \t" << _tzcnt_u64(header >> begin) << std::endl;
            assert(0);
        }
        assert(my_end == end_valid);
    }


    inline uint32_t get_v(uint8_t rem, const __m256i *pd) {
        const __m256i target = _mm256_set1_epi8(rem);
        return _mm256_cmpeq_epu8_mask(target, *pd);
    }
    inline bool pd_find_26_ver9(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE22);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 6ul;
        if (!v) return false;

        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = get_clean_header(pd);

        if (v_off == 0) {
            const uint64_t mask = v << quot;
            return (_mm_popcnt_u64(h0 & (mask - 1)) == quot) && (!(h0 & mask));
        }

        if (quot == 0)
            return v & (_blsmsk_u64(h0) >> 1ul);

        uint64_t new_v = (v << quot) & ~h0;
        const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        //bzhi: Copy all bits from unsigned 64-bit integer a to dst, and reset (set to 0) the high bits in dst starting at index.
        const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
        const uint64_t v_mask = _blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits);
        // bool att = v_mask & new_v;
        return v_mask & new_v;
    }

    inline uint64_t mask_between_bits_naive5(uint64_t x) {
        uint64_t hi_bit = (x - 1) & x;
        uint64_t clear_hi = hi_bit - 1;
        uint64_t lo_set = (x - 1);
        uint64_t res = clear_hi ^ lo_set;// & (~x0)
        return res;
    }


    inline bool pd_find_26_ver17(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE22);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd);
        if (!v)
            return false;
        const uint64_t h0 = get_clean_header(pd);
        const uint64_t new_v = ((v >> 6ul) << quot) & ~h0;
        const uint64_t v_mask = (quot) ? mask_between_bits_naive5(_pdep_u64(3ul << (quot - 1), h0)) : (_blsmsk_u64(h0) >> 1ul);
        return v_mask & new_v;
    }
    
    inline bool pd_find_with_v(int64_t quot, uint64_t v, const __m256i *pd) {
        const uint64_t h0 = get_clean_header(pd);
        const uint64_t new_v = ((v >> 6ul) << quot) & ~h0;
        const uint64_t v_mask = (quot) ? mask_between_bits_naive5(_pdep_u64(3ul << (quot - 1), h0)) : (_blsmsk_u64(h0) >> 1ul);
        return v_mask & new_v;
    }


    inline bool pd_find_26(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(pd_find_26_ver9(quot, rem, pd) == pd_find_26_ver17(quot, rem, pd));
        return pd_find_26_ver9(quot, rem, pd);
    }


    inline bool pd_add_26(int64_t quot, char rem, __m256i *pd) {
        assert(quot < QUOT_SIZE22);
        const uint64_t header = get_clean_header(pd);
        if (pd_full(pd))
            return false;
        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= 48);
        const __m256i target = _mm256_set1_epi8(rem);
        constexpr unsigned kBytes2copy = 6;
        uint64_t new_header = header & ((1ULL << begin) - 1);
        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(_mm_popcnt_u64(new_header) == QUOT_SIZE22);
        // assert(select64(new_header, 32 - 1) - (32 - 1) == fill + 1);
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

        assert(pd_find_26(quot, rem, pd));
        return true;
    }

    auto remove_naive(int64_t quot, char rem, __m256i *pd) -> bool;

    inline auto remove(int64_t quot, char rem, __m256i *pd) -> bool {
        assert(false);
        // assert(quot < QUOT_SIZE22);
        // assert(pd_find_26(quot, rem, pd));
        // const uint64_t header = ((uint64_t *) pd)[0];
        // assert(_mm_popcnt_u64(header) == 24);
        // constexpr unsigned kBytes2copy = 6;

        // const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        // const uint64_t end = select64(header, quot);
        // assert(begin <= end);
        // assert(end <= 63);

        // uint64_t new_header = header & ((1ULL << begin) - 1);
        // new_header |= ((header >> end) << (end - 1));
        // assert(_mm_popcnt_u64(new_header) == 24);
        // //
        // memcpy(pd, &new_header, kBytes2copy);
        // const uint64_t begin_fingerprint = begin - quot;
        // const uint64_t end_fingerprint = end - quot;
        // assert(begin_fingerprint <= end_fingerprint);
        // assert(end_fingerprint <= 32);
        // uint64_t i = begin_fingerprint;
        // for (; i < end_fingerprint; ++i) {
        //     if (rem == ((const char *) pd)[kBytes2copy + i])
        //         break;
        // }
        // assert(rem == ((const char *) pd)[kBytes2copy + i]);

        // memmove(&((char *) pd)[kBytes2copy + i],
        //         &((const char *) pd)[kBytes2copy + i + 1],
        //         sizeof(*pd) - (kBytes2copy + i + 1));
        // // ((char *) pd)[kBytes2copy + i] = rem;

        // assert(pd_find_32(quot, rem, pd));
        return true;
    }

    inline auto conditional_remove(int64_t quot, char rem, __m256i *pd) -> bool {
        assert(false);

        assert(quot < 32);
        // assert(pd_find_32(quot, rem, pd));
        const uint64_t header = ((uint64_t *) pd)[0];
        assert(_mm_popcnt_u64(header) == 24);
        constexpr unsigned kBytes2copy = 6;

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

        assert(_mm_popcnt_u64(new_header) == 24);
        // memcpy(pd, &new_header, kBytes2copy);
        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));

        return true;
    }

    void print512(const __m256i *var);

    inline auto pd_popcount_att_helper_start(const __m256i *pd) -> int {
        uint64_t header;
        memcpy(&header, reinterpret_cast<const uint64_t *>(pd), 8);
        return 14ul - _lzcnt_u64(header);
    }

    auto validate_number_of_quotient(const __m256i *pd) -> bool;

    auto get_capacity_naive(const __m256i *x) -> size_t;

    auto get_name() -> std::string;
    ////New functions
    inline auto is_full(const __m256i *x) -> bool {
        assert(get_capacity_naive(x) == get_capacity(x));
        return get_capacity(x) == 32;
    }
}// namespace pd256


#endif// FILTERS_PD256_HPP

// auto my_equal(__m256i x, __m256i y) -> bool;
