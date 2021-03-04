/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_TS_PD512_HPP
#define FILTERS_TS_PD512_HPP

#include "pd512.hpp"
#include "pd512_plus.hpp"
#include <algorithm>
#include <iomanip>

// #define MAX(x, y) (((x) < (y)) ? y : x)
// #define MIN(x, y) (((x) > (y)) ? y : x)

#define MSK(p) ((1ULL << p) - 1)
#define BITWISE_DISJOINT(a, b) (!((a) & (b)))


namespace v_ts_pd512 {
    template<typename T>
    void zero_array(T *a, size_t a_size) {
        for (size_t i = 0; i < a_size; i++) {
            a[i] = 0;
        }
    }


    size_t popcount_up_to_index(uint64_t word, size_t index);

    size_t popcount_up_to_index(uint64_t *arr, size_t a_size, size_t index);

    bool v_write_header_cond12(uint64_t h0, size_t index, uint64_t quot);

    bool validate_write_header(int64_t quot, uint64_t index, __m512i *pd);

    // bool v_write_header_cond12_without_quot(uint64_t h0, size_t index, uint64_t quot);
    void print_body(const __m512i *pd);

    /**
     * @brief Computes the symmetric difference between two pocket dictionaries.
     * 
     * @param pd1 
     * @param pd2 
     * @return size_t 
     */
    size_t pd_distance(const __m512i *pd1, const __m512i *pd2);

    size_t val_pd_distance(int64_t quot, uint8_t rem, const __m512i *old_pd, const __m512i *new_pd);

    size_t array_sym_diff_single(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size);

    size_t array_sym_diff(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size);


    bool validate_body_add(size_t end_fingerprint, uint8_t rem, __m512i *pd);


    bool val_AWF(int64_t quot, uint8_t rem, const __m512i *pd);

    bool val_add_when_full(int64_t quot, uint8_t rem, const __m512i *pd);

    size_t get_last_occupied_quot_naive(const __m512i *pd);

    bool compare_headers(const __m512i *pd1, const __m512i *pd2);

    bool compare_bodies(const __m512i *pd1, const __m512i *pd2);

    void diff_bodies(const __m512i *pd1, const __m512i *pd2, size_t *diff_arr);


    bool validate_evicted_qr(int64_t quot, uint8_t rem, int64_t evict_quot, uint8_t evict_rem, __m512i *pd);
}// namespace v_ts_pd512

namespace ts_pd512 {

    constexpr size_t QUOTS = 50;
    constexpr size_t MAX_CAPACITY = 51;
    constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;


    inline bool pd_full(const __m512i *pd) {
        uint8_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint8_t *>(pd) + 12, 1);
        return header_end & 16;
    }

    /**
     * @brief Shift 'a' 8 bits left.
     * @param a 
     * @return __m512i 
     */
    inline __m512i shift_left(__m512i a) {
        constexpr __m512i idx = {433757350076153919, 1012478732780767239, 1591200115485380623, 2169921498189994007,
                                 2748642880894607391, 3327364263599220775, 3906085646303834159, 4484807029008447543};
        constexpr uint64_t mask = 18446744073709551614ULL;
        __m512i res = _mm512_maskz_permutexvar_epi8(mask, idx, a);
        return res;
    }

    auto get_specific_quot_capacity_naive2(int64_t quot, const __m512i *pd) -> int;


    inline size_t get_last_occupied_quot_for_full_pd_db(const __m512i *pd) {
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);// 101-64 == 37; 64 - 37 == 27;
        assert(pd_full(pd));
        const uint64_t h1_att = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) << 26u;
        const uint64_t clear_h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_const_mask;
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) << 27u;
        auto pop0 = _mm_popcnt_u64(h1_att);
        auto pop1 = _mm_popcnt_u64(h1);
        auto pop2 = _mm_popcnt_u64(clear_h1);
        assert(_mm_popcnt_u64(h1) == _mm_popcnt_u64(clear_h1));
        assert(_mm_popcnt_u64(h1) == _mm_popcnt_u64(h1 << 1) + 1);

        auto res = QUOTS - _lzcnt_u64(~h1);
        assert(v_ts_pd512::get_last_occupied_quot_naive(pd) == res);
        return QUOTS - _lzcnt_u64(~h1);
    }

    inline size_t get_last_occupied_quot_for_full_pd(const __m512i *pd) {
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);// 101-64 == 37; 64 - 37 == 27;
        assert(pd_full(pd));
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) << 27u;
        return QUOTS - _lzcnt_u64(~h1);
    }

    // inline bool no_GE_occupied_quots_for_full_pd(int64_t quot, const __m512i *pd){
    //     assert(pd_full(pd));
    //     constexpr uint64_t const_mask = (1ULL << 37) - 1;
    //     const uint64_t mask = mask & ();

    // }

    inline bool find(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd512::pd_find_50(quot, rem, pd);
    }


    // inline auto conditional_remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
    //     return pd512::conditional_remove(quot, rem, pd);
    // }
    inline auto insertion_from_not_sorted_body_naive(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        assert(quot < 50);

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        assert(pd512::popcount128(header) == 50);
        // constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        const uint64_t begin = quot ? (pd512::select128(header, quot - 1) + 1) : 0;
        const uint64_t end = pd512::select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v0 = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v0)
            return false;
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> (13ul + begin_fingerprint);
        uint64_t i = _tzcnt_u64(v) + begin_fingerprint;

        /*         std::cout << "i: " << i << std::endl;
        std::cout << "v: " << v << std::endl; 
        // v_pd512_plus::p_format_word(v);
        v_pd512::bin_print(v);
        v_pd512_plus::print_headers_extended(pd);
        v_ts_pd512::print_body(pd);
 */
        if (i >= end_fingerprint)
            return false;

        assert(pd512::pd_find_50(quot, rem, pd));
        assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));

        assert(pd512::popcount128(header) == 50);

        memcpy(pd, &new_header, kBytes2copy);
        memmove(&((uint8_t *) pd)[kBytes2copy + i],
                &((const uint8_t *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));

        return true;
    }


    inline auto remove_from_not_sorted_body_naive(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        assert(quot < 50);

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        assert(pd512::popcount128(header) == 50);
        // constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        const uint64_t begin = quot ? (pd512::select128(header, quot - 1) + 1) : 0;
        const uint64_t end = pd512::select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v0 = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v0)
            return false;
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> (13ul + begin_fingerprint);
        uint64_t i = _tzcnt_u64(v) + begin_fingerprint;

        /*         std::cout << "i: " << i << std::endl;
        std::cout << "v: " << v << std::endl; 
        // v_pd512_plus::p_format_word(v);
        v_pd512::bin_print(v);
        v_pd512_plus::print_headers_extended(pd);
        v_ts_pd512::print_body(pd);
 */
        if (i >= end_fingerprint)
            return false;

        assert(pd512::pd_find_50(quot, rem, pd));
        assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));

        assert(pd512::popcount128(header) == 50);

        memcpy(pd, &new_header, kBytes2copy);
        memmove(&((uint8_t *) pd)[kBytes2copy + i],
                &((const uint8_t *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));

        return true;
    }

    inline auto remove_from_not_sorted_body_new(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        assert(quot < 50);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v) {
        //     return false;
        // }

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        if (_blsr_u64(v) == 0) {
            const uint64_t i = _tzcnt_u64(v);

            const bool find_res = (!(header & (((unsigned __int128) 1) << (quot + i)))) &&
                                  (pd512::popcount128(header & (((unsigned __int128) 1 << (quot + i)) - 1)) == quot);

            if (find_res) {
                assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

                const uint64_t shift = i + quot;
                unsigned __int128 new_header = header & ((((unsigned __int128) 1) << shift) - 1);
                new_header |= ((header >> (shift + 1)) << (shift));
                // new_header |= ((header >> shift) << (shift - 1));

                assert(pd512::popcount128(header) == 50);

                assert(pd512::validate_number_of_quotient(pd));
                memcpy(pd, &new_header, kBytes2copy);
                assert(pd512::validate_number_of_quotient(pd));

                memmove(&((uint8_t *) pd)[kBytes2copy + i],
                        &((const uint8_t *) pd)[kBytes2copy + i + 1],
                        sizeof(*pd) - (kBytes2copy + i + 1));
            }
            return find_res;
        }

        const uint64_t begin = quot ? (pd512::select128(header, quot - 1) + 1) : 0;
        const uint64_t end = pd512::select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        uint64_t v1 = v >> begin_fingerprint;
        uint64_t i = _tzcnt_u64(v1) + begin_fingerprint;

        if (i >= end_fingerprint)
            return false;

        assert(pd512::pd_find_50(quot, rem, pd));
        assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

        const uint64_t shift = i + quot;
        // assert(shift == begin);
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << shift) - 1);
        new_header |= ((header >> (shift + 1)) << (shift));
        // new_header |= ((header >> shift) << (shift - 1));

        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        // new_header |= ((header >> end) << (end - 1));

        assert(pd512::popcount128(header) == 50);

        assert(pd512::validate_number_of_quotient(pd));
        memcpy(pd, &new_header, kBytes2copy);
        assert(pd512::validate_number_of_quotient(pd));

        memmove(&((uint8_t *) pd)[kBytes2copy + i],
                &((const uint8_t *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));

        return true;
    }

    /* inline auto remove_from_not_sorted_body_without128_bits_op(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v) {
        //     return false;
        // }

        if (_blsr_u64(v) == 0) {
            const uint64_t i = _tzcnt_u64(v);

            const bool find_res = (!(header & (((unsigned __int128) 1) << (quot + i)))) &&
                                  (pd512::popcount128(header & (((unsigned __int128) 1 << (quot + i)) - 1)) == quot);

            if (find_res) {
                assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

                const uint64_t shift = i + quot;
                unsigned __int128 new_header = header & ((((unsigned __int128) 1) << shift) - 1);
                new_header |= ((header >> (shift + 1)) << (shift));
                // new_header |= ((header >> shift) << (shift - 1));

                assert(pd512::popcount128(header) == 50);

                assert(pd512::validate_number_of_quotient(pd));
                memcpy(pd, &new_header, kBytes2copy);
                assert(pd512::validate_number_of_quotient(pd));

                memmove(&((uint8_t *) pd)[kBytes2copy + i],
                        &((const uint8_t *) pd)[kBytes2copy + i + 1],
                        sizeof(*pd) - (kBytes2copy + i + 1));
            }
            return find_res;
        }
    }
 */

    inline void remove_from_not_sorted_body_with_assertion(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(quot < 50);
        assert(pd512::pd_find_50(quot, rem, pd));

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        assert(pd512::popcount128(header) == 50);

        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (_blsr_u64(v) == 0) {
            // std::cout << "r0" << std::endl;
            uint64_t i = _tzcnt_u64(v);
            assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

            const uint64_t shift = i + quot;
            unsigned __int128 new_header = header & ((((unsigned __int128) 1) << shift) - 1);
            new_header |= ((header >> (shift + 1)) << (shift));

            // new_header |= ((header >> shift) << (shift - 1));

            //validation
            /* const uint64_t begin = quot ? (pd512::select128(header, quot - 1) + 1) : 0;
            const uint64_t end = pd512::select128(header, quot);
            assert(begin <= end);
            // assert(shift == begin);


            unsigned __int128 new_header_valid = header & ((((unsigned __int128) 1) << begin) - 1);
            new_header_valid |= ((header >> end) << (end - 1));


            // bool cond = (new_header_valid == new_header);
            // if (!cond) {
            //     std::cout << "here!" << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced128(header) << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced128(new_header_valid ^ new_header) << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced128(new_header) << std::endl;
            //     std::cout << v_pd512::bin_print_header_spaced128(new_header_valid) << std::endl;
            // }
            // assert(cond);
            */

            assert(pd512::validate_number_of_quotient(pd));
            memcpy(pd, &new_header, kBytes2copy);
            assert(pd512::validate_number_of_quotient(pd));

            memmove(&((uint8_t *) pd)[kBytes2copy + i],
                    &((const uint8_t *) pd)[kBytes2copy + i + 1],
                    sizeof(*pd) - (kBytes2copy + i + 1));
            return;
        }
        // std::cout << "r1" << std::endl;
        const uint64_t begin = quot ? (pd512::select128(header, quot - 1) + 1) : 0;
        const uint64_t end = pd512::select128(header, quot);
        assert(begin <= end);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        uint64_t v1 = v >> begin_fingerprint;
        uint64_t i = _tzcnt_u64(v1) + begin_fingerprint;

        assert(i < end_fingerprint);
        assert(pd512::pd_find_50(quot, rem, pd));
        assert(rem == ((const uint8_t *) pd)[kBytes2copy + i]);

        const uint64_t shift = i + quot;
        // assert(shift == begin);
        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << shift) - 1);
        new_header |= ((header >> (shift + 1)) << (shift));

        assert(pd512::popcount128(header) == 50);

        assert(pd512::validate_number_of_quotient(pd));
        memcpy(pd, &new_header, kBytes2copy);
        assert(pd512::validate_number_of_quotient(pd));

        memmove(&((uint8_t *) pd)[kBytes2copy + i],
                &((const uint8_t *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
    }

    /**
     * @brief Try to remove the element from the filter.
     * 
     * @param quot 
     * @param rem 
     * @param pd 
     * @return true 
     * @return false 
     */
    inline auto remove_attempt_db(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        static int count = 0;
        static int c2 = 0;
        count++;
        const __m512i old_pd = *pd;
        bool find_res = find(quot, rem, pd);
        auto res = remove_from_not_sorted_body_naive(quot, rem, pd);
        if (!res) {
            c2++;
            assert(!find(quot, rem, pd));
            return res;
        }

        auto diff = v_ts_pd512::val_pd_distance(quot, rem, &old_pd, pd);
        // auto diff = v_ts_pd512::pd_distance(pd, &old_pd);
        // if (diff != 1){
        // }
        assert(diff == 1);
        return res;
    }

    inline auto remove_attempt(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        return remove_from_not_sorted_body_new(quot, rem, pd);
    }

    inline auto remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        // return remove_from_not_sorted_body_new(quot, rem, pd);
        return remove_from_not_sorted_body_naive(quot, rem, pd);
        // return remove_attempt(quot, rem, pd);
    }

    /**
     * @brief 
     * assertion on the element being in the filter.
     * @param quot 
     * @param rem 
     * @param pd 
     * @return true 
     * @return false 
     */
    inline void remove_with_assertion(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(find(quot, rem, pd));
        __m512i old_pd = *pd;
        assert(pd512::validate_number_of_quotient(pd));
        remove_from_not_sorted_body_with_assertion(quot, rem, pd);
        assert(pd512::validate_number_of_quotient(pd));

        //        auto diff = v_ts_pd512::pd_distance(pd, &old_pd);
        // auto res = v_ts_pd512::pd_distance(pd, &old_pd);
        assert(v_ts_pd512::pd_distance(pd, &old_pd));
        //        assert(res);
        //        return res;
    }


    inline void body_add_naive(size_t end_fingerprint, uint8_t rem, __m512i *pd) {
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        memmove(&((uint8_t *) pd)[kBytes2copy + end_fingerprint + 1],
                &((const uint8_t *) pd)[kBytes2copy + end_fingerprint],
                sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((uint8_t *) pd)[kBytes2copy + end_fingerprint] = rem;
    }

    inline void body_add(size_t end_fingerprint, uint8_t rem, __m512i *pd) {
        // assert(v_ts_pd512::validate_body_add(end_fingerprint, rem, pd));
        // v_ts_pd512::print_body(pd);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        __m512i pd_start = *pd;
        ((uint8_t *) &pd_start)[kBytes2copy + end_fingerprint] = rem;

        const __m512i shifted_pd = shift_left(*pd);
        const uint64_t mask = ~MSK(kBytes2copy + end_fingerprint + 1);
        // const __mmask64 blend_mask = mask;
        *pd = _mm512_mask_blend_epi8(mask, pd_start, shifted_pd);
        // v_ts_pd512::print_body(pd);
    }

    inline void body_add2(size_t end_fingerprint, uint8_t rem, __m512i *pd) {
        static int counter = 0;
        counter++;
        // assert(v_ts_pd512::validate_body_add(end_fingerprint, rem, pd));
        assert(pd512::validate_number_of_quotient(pd));
        __m512i old_pd = *pd;

        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        const uint64_t mask = ~MSK(kBytes2copy + end_fingerprint + 1);
        const __m512i shifted_pd = shift_left(*pd);
        ((uint8_t *) pd)[kBytes2copy + end_fingerprint] = rem;
        *pd = _mm512_mask_blend_epi8(mask, *pd, shifted_pd);
        assert(pd512::validate_number_of_quotient(pd));

        /* if (!pd512::validate_number_of_quotient(pd)) {
            v_pd512_plus::print_headers(&old_pd);
            v_pd512_plus::print_headers(pd);
            std::cout << "end_fingerprint:  " << end_fingerprint << std::endl;
            std::cout << "mask:             " << mask << std::endl;
            std::cout << "counter:          " << counter << std::endl;
            // std::cout << "quot:          " << quot << std::endl;
            // std::cout << "end:           " << end << std::endl;
            // std::cout << "index:         " << index << std::endl;
            // std::cout << "val_index:     " << val_index << std::endl;
            // std::cout << "pre_val_index: " << pre_val_index << std::endl;

            assert(0);
        } */
    }

    /**
     * @brief This function helps insertions. When "popcount(h0) == quot". and pd is not full.
     * 
     * @param quot 
     * @param end 
     * @param pd 
     */
    inline void write_header_naive_end64(int64_t quot, uint64_t end, __m512i *pd) {
        assert(pd512::validate_number_of_quotient(pd));
        assert(end == 64);
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);
        uint64_t *pd64 = (uint64_t *) pd + 1;
        uint64_t h1_low = ((pd64[0] << 1) & h1_const_mask);
        memcpy(pd64, &h1_low, 5);
        assert(pd512::validate_number_of_quotient(pd));
    }

    inline void write_header_naive(int64_t quot, uint64_t end, __m512i *pd) {
        //This functions takes 36 lines in godbolt.
        assert(pd512::validate_number_of_quotient(pd));
        
        // I'm assuming that the case "quot == 0" was dealt with, previously.
        assert(quot);

        /* if (quot == 0) {
            const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);
            memcpy(pd64 + 1, &low_h1, 5);
            pd64[0] <<= 1u;
            assert(pd512::validate_number_of_quotient(pd));
            return;
        } */
        const __m512i old_pd = *pd;
        // v_pd512_plus::print_headers(pd);

        // constexpr uint64_t h1_mask = (1ULL << 37) - 1; // round(37 / 8) = 5.
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);
        uint64_t *pd64 = (uint64_t *) pd;


        if (end == 63) {
            constexpr uint64_t mask63 = (1ull << 63);
            pd64[0] ^= mask63;
            uint64_t h1_low = ((pd64[1] << 1) & h1_const_mask) | 1;
            memcpy(pd64 + 1, &h1_low, 5);
            assert(pd512::validate_number_of_quotient(pd));
            return;
        }

        uint64_t h0 = pd64[0];
        uint64_t h1 = pd64[1];

        //can not happend as long as quot > 0.
        assert(end > 0);
        uint64_t h0_mask = MSK(end);
        uint64_t h0_low = h0 & h0_mask;
        assert(end < 64);
        uint64_t h0_high = (h0 >> end) << (end + 1);
        uint64_t h1_low = ((h1 << 1) & h1_const_mask) | (h0 >> 63u);
        pd64[0] = h0_low | h0_high;
        memcpy(pd64 + 1, &h1_low, 5);
        // const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);

        assert(pd512::validate_number_of_quotient(pd));

        /* if (!pd512::validate_number_of_quotient(pd)) {
            v_pd512_plus::print_headers_extended(&old_pd);
            v_pd512_plus::print_headers_extended(pd);
            assert(0);
        } */
        // v_pd512_plus::print_headers(pd);
        // assert(pd512::validate_number_of_quotient(pd));
    }

    inline void write_header_att(int64_t quot, uint64_t end, __m512i *pd) {
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        // const uint64_t end = select128(header, quot);


        unsigned __int128 high_header = (header << 1) & ~((((unsigned __int128) 1) << (end + 1)) - 1);
        unsigned __int128 low_header = header & ((((unsigned __int128) 1) << (end)) - 1);
        assert(!(high_header & low_header));
        unsigned __int128 new_header = high_header | low_header;
        memcpy(pd, &new_header, kBytes2copy);

        // assert(popcount128(new_header) == 50);
        // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
    }

    /**
     * @brief adds a single zero *before* the (quot)'th one, and shifts the header.
     * 
     * I assume the following on `index`:
     * 1) popcount(header[:index]) == quot - 1
     * 2) popcount(header[:index + 1]) == quot
     * (Meaning header[index] == 1).
     * 3) index < 64.
     * 
     * @param quot is only used for validations.
     * @param index 
     * @param pd 
     */
    inline void write_header_error(uint64_t index, __m512i *pd) {
        assert(0);
        assert(index < 64);//Condition 3.
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        assert(h0 & (1ULL << index));//Condition 1 + 2.
        // v_write_header_cond12()

        //this is wrong.
        const uint64_t h1 = ((_mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask) << 1u) | (h0 >> 63u);

        uint64_t h0_mask = MSK(index);
        uint64_t h0_lower = h0 & h0_mask;
        // to AND then shift or vice versa?
        uint64_t new_h0 = h0_lower | ((h0 << 1u) & ~h0_mask);
        // uint64_t new_h1 = (h1 << 1u) | (h0 >> 63);
        memcpy(pd, &new_h0, 8);
        memcpy(&((uint8_t *) pd)[8], &h1, 5);
    }

    /**
     * @brief adds a single zero *before* the (quot)'th one, and shifts the header.
     * 
     * I assume the following on `index`:
     * 1) popcount(header[:index]) == quot - 1
     * 2) popcount(header[:index + 1]) == quot
     * (Meaning header[index] == 1).
     * 3) index < 64.
     * 
     * @param quot is only used for validations.
     * @param index (index := select(pd, quot))
     * @param pd 
     */
    inline void write_header(uint64_t index, __m512i *pd) {
        //Alternative to write_header6.
        assert(0);
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);

        uint64_t *p_h1 = &((uint64_t *) pd)[1];
        const uint64_t h1_low = ((*p_h1 << 1) | (h0 >> 63u)) & h1_mask;
        const uint64_t h1_high = *p_h1 & ~h1_mask;

        *p_h1 = (h1_low | h1_high);


        const uint64_t h0_mask = MSK(index);
        const uint64_t h0_lower = h0 & h0_mask;

        const uint64_t new_h0 = h0_lower | ((h0 & ~h0_mask) << 1u);

        memcpy(pd, &new_h0, 8);
        //   *(p_h1-1) = new_h0;
    }

    inline void write_header6(uint64_t index, __m512i *pd) {
        assert(pd512::validate_number_of_quotient(pd));
        // v_pd512_plus::print_headers(pd);
        // constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);

        uint64_t *pd64 = (uint64_t *) pd;
        const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);

        const uint64_t h0_mask = MSK(index);
        const uint64_t h0_lower = pd64[0] & h0_mask;

        // pd64[0] = h0_lower | ((pd64[0] << 1u) & ~h0_mask);
        pd64[0] = h0_lower | ((pd64[0] & ~h0_mask) << 1u);
        memcpy(pd64 + 1, &low_h1, 5);
        // v_pd512_plus::print_headers(pd);

        assert(pd512::validate_number_of_quotient(pd));
    }

    inline void write_header8(uint64_t index, __m512i *pd) {
        assert(pd512::validate_number_of_quotient(pd));
        
        uint64_t *pd64 = (uint64_t *) pd;
        const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);
        memcpy(pd64 + 1, &low_h1, 5);

        const uint64_t h0_mask = MSK(index);
        const uint64_t h0_lower = pd64[0] & h0_mask;

        pd64[0] = h0_lower | ((pd64[0] & ~h0_mask) << 1u);
        //This saves one command, but does not work when index == 63. 
        // pd64[0] = h0_lower | ((pd64[0] >> index) << (index + 1));
        
        //   pd64[0] = (pd64[0] & MSK(index)) | ((pd64[0] >> index) << (index - 1));
    
        assert(pd512::validate_number_of_quotient(pd));
    }


    inline void add_when_not_full_with_zero_quot(uint8_t rem, __m512i *pd) {
        // const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        uint64_t *pd64 = (uint64_t *) pd;
        const uint64_t end = _tzcnt_u64(pd64[0]);
        const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);
        memcpy(pd64 + 1, &low_h1, 5);
        pd64[0] <<= 1u;
        assert(pd512::validate_number_of_quotient(pd));
        // body_add(end, rem, pd);
        // assert(v_ts_pd512::validate_body_add(end, rem, pd));
        // body_add2(end, rem, pd);
        body_add_naive(end, rem, pd);
        return;
    }

    /**
     * @brief
     * 
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void add_when_not_full_db(int64_t quot, uint8_t rem, __m512i *pd) {
        // static int j_arr[4];
        assert(!pd_full(pd));
        assert(quot < 50);

        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);

        if (quot == 0) {
            // j_arr[0]++;
            // std::cout << "j0: " << std::endl;
            assert(pd512::validate_number_of_quotient(pd));
            const uint64_t end = _tzcnt_u64(h0);
            write_header_naive(0, end, pd);
            // assert(v_ts_pd512::validate_body_add(end, rem, pd));
            // body_add2(end, rem, pd);
            body_add_naive(end, rem, pd);
            assert(pd512::validate_number_of_quotient(pd));
            return;
        }

        const uint64_t pop = _mm_popcnt_u64(h0);

        if (quot < pop) {
            static int c = 0;
            c++;

            /** In this case, we do not need to read the rest of the header. (we do have to rewrite to it)*/

            // j_arr[1]++;
            const uint64_t end = pd512::select64(h0, quot);

            // std::cout << "j1: " << std::endl;
            // // if (end != quot) {
            // std::cout << "pop:  " << pop << std::endl;
            // std::cout << "quot: " << quot << std::endl;
            // std::cout << "end:  " << end << std::endl;
            // std::cout << "c:    " << c << std::endl;
            // }

            write_header_naive(quot, end, pd);
            // assert(v_ts_pd512::validate_body_add(end - quot, rem, pd));
            // body_add2(end - quot, rem, pd);
            body_add_naive(end - quot, rem, pd);
            return;
            /** 
             * In this case, we do not need to read the rest of the header, or even do select (we might have to do select if the h0 last bit is not set.) , we have the select result. 
             * If we don't have to keep the remainders sorted, we can write only to the second block of the header.
             * 
            // } else if (pop == quot) {
            
            //     write_header_pop_equal_quot(end, pd);
            //     body_add2(end - quot, rem, pd);
            //     return;
            //     }
            */
        } else if (pop < quot) {
            /** We already have the popcount result, so ranking here is easier. */
            // std::cout << "j2: \t" << std::endl;
            /* j_arr[2]++;
            std::cout << "j1: \t"
                      << "j_arr[2]: " << j_arr[2] << std::endl;
            std::cout << "*** " << __FILE__ << ":" << __LINE__ << " *** " << std::endl; */
            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_const_mask;
            const uint64_t end = pd512::select64(h1, quot - pop);

            //header
            assert(pd512::validate_number_of_quotient(pd));

            const uint64_t h1_mask = MSK(end);
            const uint64_t h1_low = h1 & h1_mask;
            const uint64_t h1_high = (h1 >> end) << (end + 1);
            const uint64_t new_h1 = h1_low | h1_high;

            memcpy(&((uint64_t *) pd)[1], &new_h1, 5);
            assert(pd512::validate_number_of_quotient(pd));

            const uint64_t index = 64 + end - quot;
            body_add_naive(index, rem, pd);
            assert(pd512::validate_number_of_quotient(pd));

            assert(find(quot, rem, pd));

            return;
        } else {
// std::cout << "j3: " << std::endl;

/* assert(quot == pop);
            j_arr[3]++;
            assert(quot > 0);
            std::cout << "j2: " << std::endl;
            std::cout << "j_arr[3]: " << j_arr[3] << std::endl; */
#ifndef NDEBUG
            const uint64_t pre_val_index = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot);
#endif// NDEBUG
            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
            const uint64_t end = _tzcnt_u64(h1);
            // const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
            // const uint64_t rel_end = _lzcnt_u64(h1);
            //header
            // v_pd512_plus::print_headers(pd);
            assert(pd512::validate_number_of_quotient(pd));
            write_header_naive_end64(quot, 64, pd);
            assert(pd512::validate_number_of_quotient(pd));

            // v_pd512_plus::print_headers(pd);

            //body
            // const uint64_t end = _tzcnt_u64(h1) - 1;
            const uint64_t index = (64 - pop) + end;
#ifndef NDEBUG
            const uint64_t val_index = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot) - 1;
            assert(index == val_index);
#endif// DEBUG

            /* bool cond = (index == pre_val_index);
            if (!cond) {
                std::cout << "pop:           " << pop << std::endl;
                std::cout << "quot:          " << quot << std::endl;
                std::cout << "end:           " << end << std::endl;
                std::cout << "index:         " << index << std::endl;
                std::cout << "val_index:     " << val_index << std::endl;
                std::cout << "pre_val_index: " << pre_val_index << std::endl;
                v_pd512_plus::print_headers(pd);
                assert(0);
            }
 */
            // v_ts_pd512::print_body(pd);
            // assert(v_ts_pd512::validate_body_add(index, rem, pd));
            // body_add2(index, rem, pd);
            body_add_naive(index, rem, pd);
            /* // body_add_naive(index, rem, pd);
            // v_ts_pd512::print_body(pd);
            // if (!find(quot, rem, pd)){
            //     std::cout << "pop:           " << pop << std::endl;
            //     std::cout << "quot:          " << quot << std::endl;
            //     std::cout << "end:           " << end << std::endl;
            //     std::cout << "index:         " << index << std::endl;
            //     std::cout << "val_index:     " << val_index << std::endl;
            //     std::cout << "pre_val_index: " << pre_val_index << std::endl;
            //     v_pd512_plus::print_headers(pd);
            //     assert(0);

            // } */
            assert(find(quot, rem, pd));
        }
    }

    /**
     * @brief Inserting a new element when pd is not full. 
     * 
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void add_when_not_full(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(!pd_full(pd));
        assert(quot < 50);

        // FIXME: uncomment.
        if (quot == 0) {
            add_when_not_full_with_zero_quot(rem, pd);
            return;
        }


        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t pop = _mm_popcnt_u64(h0);
        if (quot < pop) {
            const uint64_t end = pd512::select64(h0, quot);
            assert(v_ts_pd512::validate_write_header(quot, end, pd));
            write_header6(end, pd);
            // write_header_naive(quot, end, pd);
            body_add_naive(end - quot, rem, pd);
            return;
        } else {
            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_const_mask;
            const uint64_t end = pd512::select64(h1, quot - pop);

            //header
            assert(pd512::validate_number_of_quotient(pd));

            const uint64_t h1_mask = MSK(end);
            const uint64_t h1_low = h1 & h1_mask;
            const uint64_t h1_high = (h1 >> end) << (end + 1);
            const uint64_t new_h1 = h1_low | h1_high;

            memcpy(&((uint64_t *) pd)[1], &new_h1, 5);
            assert(pd512::validate_number_of_quotient(pd));

            //body
            const uint64_t index = 64 + end - quot;
            //TODO: Fix body_add functions, and find out why they work in other places, but not here.
            body_add_naive(index, rem, pd);

            assert(pd512::validate_number_of_quotient(pd));
            assert(find(quot, rem, pd));

            return;
        }
    }

    /**
     * @brief 
     * split - indicates that a deletion is performed, and than insertion. (without optimizations).
     * uniform - indicates a random element is deleted (without optimizations).
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void add_pd_full_naive_split_uniform(int64_t quot, uint8_t rem, __m512i *pd) {
        // static int counter = 0;
        // std::cout << "counter: " << counter++ << std::endl;
        assert(pd_full(pd));
        assert(pd512::get_capacity_naive(pd) == MAX_CAPACITY);
        assert(quot < 50);
        // constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        const size_t rand_rem_index = rand() % MAX_CAPACITY;
        // const size_t rand_rem_index = 0;
        int64_t rand_quot = pd512_plus::count_ones_up_to_the_kth_zero(pd, rand_rem_index);
        assert(kBytes2copy + rand_rem_index < 64);
        uint8_t rand_rem = ((uint8_t *) pd)[kBytes2copy + rand_rem_index];
        /*
        std::cout << "rand_rem_index is:  " << rand_rem_index << std::endl;
        std::cout << "expected quot is:   " << rand_quot << std::endl;
        std::cout << "correct quot is:    ";
        bool res_arr[QUOTS];
        for (size_t i = 0; i < QUOTS; i++) {
            auto temp = find(i, rand_rem, pd);
            if (temp) { std::cout << i << "\t"; }
            res_arr[i] = temp;
        }
        std::cout << std::endl;
        v_pd512_plus::print_headers_extended(pd);

        */

        assert(find(rand_quot, rand_rem, pd));
        bool res = remove_from_not_sorted_body_naive(rand_quot, rand_rem, pd);
        assert(res);
        add_when_not_full(quot, rem, pd);
    }

    inline void get_first_qr(__m512i *pd, uint64_t *evict_quot, uint64_t *evict_rem) {
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        size_t temp = _tzcnt_u64(~h0);
        *evict_quot = temp;
        *evict_rem = ((uint8_t *) pd)[kBytes2copy];
    }

    inline void get_evict_qr(int64_t quot, uint8_t rem, __m512i *pd, uint64_t *evict_quot, uint64_t *evict_rem) {
        //        static int c = 0;
        //        c++;
        auto cap = pd512::get_capacity_naive(pd);

        if (quot == 0) {
            get_first_qr(pd, evict_quot, evict_rem);

            //validation.
#ifndef NDEBUG
            for (size_t i = 0; i < *evict_quot; i++) {
                assert(get_specific_quot_capacity_naive2(i, pd) == 0);
            }
            assert(find(*evict_quot, *evict_rem, pd));
#endif// DEBUG
            return;
        }

        //TODO: speed this function up ( can probably be done using manual inlining of the current function)
        //TODO: speed this codeblock up.
        size_t quot_cap = get_specific_quot_capacity_naive2(quot, pd);
        //easy case:
        if (quot_cap > 0) {
            *evict_quot = quot;
            size_t evict_rem_index = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot - 1);
            *evict_rem = ((uint8_t *) pd)[kBytes2copy + evict_rem_index];
            assert(find(*evict_quot, *evict_rem, pd));
            return;
        } else {
            size_t cap_upto_quot = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot - 1);
            size_t cap_upto_quot_plus1 = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot);
            if (cap_upto_quot != cap_upto_quot_plus1) {
                auto sanity = get_specific_quot_capacity_naive2(quot, pd);
                get_specific_quot_capacity_naive2(quot, pd);
                assert(0);
            }
            assert(cap_upto_quot == cap_upto_quot_plus1);

            // uint64_t evict_quot_att = pd512_plus::count_ones_up_to_the_kth_zero(pd, cap_upto_quot_plus1);
            *evict_quot = pd512_plus::count_ones_up_to_the_kth_zero(pd, cap_upto_quot_plus1);
            if (*evict_quot == QUOTS) {
                get_first_qr(pd, evict_quot, evict_rem);
                return;
            }


            //validation.
            for (size_t i = quot; i < *evict_quot; i++) {
                assert(get_specific_quot_capacity_naive2(i, pd) == 0);
            }

            assert(get_specific_quot_capacity_naive2(*evict_quot, pd));
            // uint64_t evict_rem_att = ((uint8_t *) pd)[kBytes2copy + cap_upto_quot_plus1];
            // *evict_rem = ((uint8_t *) pd)[kBytes2copy + cap_upto_quot_plus1 + 1];
            *evict_rem = ((uint8_t *) pd)[kBytes2copy + cap_upto_quot];
            assert(find(*evict_quot, *evict_rem, pd));
            return;
        }
    }

    /**
     * @brief 
     * QLRU is evacuation policy. when trying to insert in element x:= (x_q, x_r), to a full pd, 
     * We will first delete element y:= (y_q, y_r), where y_q is the minimal quotient in the pd satisfying 
     * q_x <= y_q.
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void add_pd_full_naive_split_qlru(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd_full(pd));
        assert(pd512::get_capacity_naive(pd) == MAX_CAPACITY);
        assert(quot < 50);

        uint64_t evict_quot = 64;
        uint64_t evict_rem = 64;

        get_evict_qr(quot, rem, pd, &evict_quot, &evict_rem);

        assert(find(evict_quot, evict_rem, pd));
        remove_with_assertion(evict_quot, evict_rem, pd);

        add_when_not_full(quot, rem, pd);
        assert(find(quot, rem, pd));
    }

    inline void add_when_full_with_zero_quot(uint8_t rem, __m512i *pd) {
        // assert(quot == 0);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);

        //Hard case: i.e. no element with quot zero.
        if (h0 & 1) {
            //body
            ((uint8_t *) pd)[kBytes2copy] = rem;

            // const uint64_t pre_pop = _mm_popcnt_u64(h0);
            //header
            // Turns first one to zero, and first zero to one.
            // Logically, we delete the first remainder, and add a new one, whose matching quotinet is zero.

            const uint64_t first_zero_index = _tzcnt_u64(~h0);
            // first_zero_index can not be 64. If it is 64, then the header contains to many 1's (63 which is more than QUOTS)
            assert(first_zero_index < 64);
            const uint64_t xor_mask = (1ULL) | (1ULL << first_zero_index);
            assert(!(h0 & (1ULL << first_zero_index)));
            ((uint64_t *) pd)[0] ^= xor_mask;

            // const uint64_t post_pop = _mm_popcnt_u64(h0);
            // assert(pre_pop == post_pop);

        } else {
            const uint64_t zero_quot_cap = _tzcnt_u64(h0);
            // const uint64_t end_fingerprint = end - quot; (quot is zero.)
            // assert(end == end_fingerprint);

            memmove(&((uint8_t *) pd)[kBytes2copy],
                    &((const uint8_t *) pd)[kBytes2copy + 1],
                    zero_quot_cap - 1);// How many bytes to move. If end == 1, then no byte should be removed.
            ((uint8_t *) pd)[kBytes2copy + zero_quot_cap - 1] = rem;
        }
    }

    inline void add_when_full_by_deleting_first_qr(int64_t quot, uint8_t rem, __m512i *pd) {
        /**Finding qr to evict*/
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        size_t temp = _tzcnt_u64(~h0);
        int64_t evict_quot = temp;
        uint8_t evict_rem = ((uint8_t *) pd)[kBytes2copy];

        assert(find(evict_quot, evict_rem, pd));

        /**Removing that qr, and inserting the new one.*/
        //TODO: HARD: merge the writing of those two.
        remove_with_assertion(evict_quot, evict_rem, pd);
        add_when_not_full(quot, rem, pd);

        assert(find(quot, rem, pd));
    }

    /**
     * @brief Hard case.
     * 
     * TODO: inlining and change args, to pre computed values.
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void add_when_full_to_empty_quot_LT_pop(int64_t quot, uint8_t rem, __m512i *pd) {
        // static int counter = 0;
        // std::cout << "k: (" << counter++ << ")\t";
        assert(get_specific_quot_capacity_naive2(quot, pd) == 0);

        // This case should be handled prior.
        assert(quot <= get_last_occupied_quot_for_full_pd(pd));
        /* 
        const size_t last_occupied_quot = get_last_occupied_quot_for_full_pd(pd);
        if (last_occupied_quot < quot) {
            add_when_full_by_deleting_first_qr(quot, rem, pd);
            return;
        }
 */

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        const uint64_t pop = _mm_popcnt_u64(h0);
        const uint64_t end = pd512::select64(h0, quot);
        const size_t evict_rem_index = end - quot;

        const uint64_t end_mask = ~MSK(end);
        uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + evict_rem_index];

        if ((end_mask & h0) != end_mask) {
            // std::cout << "1:\t"
            //           << "*** " << __FILE__ << ":" << __LINE__ << " *** " << std::endl;

            size_t relative_next_non_empty_quots = _tzcnt_u64(~(h0 >> end));

            assert(quot + relative_next_non_empty_quots < 64);
            uint64_t evict_q_att = quot + relative_next_non_empty_quots;
            assert(get_specific_quot_capacity_naive2(evict_q_att, pd));

            assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));

            assert(end > 0);
            const uint64_t xor_mask = ((1ULL) | (1ULL << relative_next_non_empty_quots)) << end;

            assert(pd512::validate_number_of_quotient(pd));
            ((uint64_t *) pd)[0] ^= xor_mask;
            assert(pd512::validate_number_of_quotient(pd));

            ((uint8_t *) pd)[kBytes2copy + evict_rem_index] = rem;
        } else {
            // std::cout << "2:\t"
            //           << "*** " << __FILE__ << ":" << __LINE__ << " *** " << std::endl;
            size_t h1_non_occupied_quots_count = _tzcnt_u64(~h1);
            size_t evict_q_att = quot + (64 - end) + h1_non_occupied_quots_count;
            // size_t evict_q_att = (63 - end) + h1_non_occupied_quots_count - quot;
            assert(get_specific_quot_capacity_naive2(evict_q_att, pd));
            assert(get_specific_quot_capacity_naive2(evict_q_att - 1, pd) == 0);

            assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));


            const uint64_t xor_mask = 1ULL << end;
            const uint64_t h1_xor_mask = 1ULL << h1_non_occupied_quots_count;
            assert(pd512::validate_number_of_quotient(pd));
            ((uint64_t *) pd)[0] ^= xor_mask;
            ((uint64_t *) pd)[1] ^= h1_xor_mask;
            assert(pd512::validate_number_of_quotient(pd));
            ((uint8_t *) pd)[kBytes2copy + evict_rem_index] = rem;
        }
    }

    inline void add_when_full_quot_lt_pop(int64_t quot, uint8_t rem, __m512i *pd) {
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t begin = pd512::select64(h0, quot - 1);
        const uint64_t end = pd512::select64(h0, quot);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(h0 & (1ULL << end));
        const bool is_quot_not_empty = !(h0 & (1ULL << (begin + 1)));

#ifndef NDEBUG
        const uint64_t db_end = pd512::select64(h0, quot);
        const auto v_quot_capi = get_specific_quot_capacity_naive2(quot, pd);
        const bool is_quot_not_empty_val = !(h0 & (1ULL << (db_end - 1)));
        assert(is_quot_not_empty_val == is_quot_not_empty);
#endif// NDEBUG \
        //TODO: we do not need to compute both "begin" and "end". "begin" ?suffice?.(it does not - we dont know how many bytes to move.)
        if (is_quot_not_empty) {
            // std::cout << ".1" << std::endl;

            // size_t rem_index = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot - 1);
            uint64_t evict_q_att = quot;
            uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1];

            assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));

            memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                    &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 2],
                    end_fingerprint - begin_fingerprint - 1);// How many bytes to copy.
            //TODO: end_fingerprint +- 1
            ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
            return;
        } else {
            // std::cout << ".2" << std::endl;
            assert(begin + 1 == end);
            add_when_full_to_empty_quot_LT_pop(quot, rem, pd);
            return;
        }
    }

    inline void add_when_full_quot_gt_pop(uint64_t pop, int64_t quot, uint8_t rem, __m512i *pd) {
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        const uint64_t begin = pd512::select64(h1, quot - pop - 1);
        const uint64_t end = pd512::select64(h1, quot - pop);
        const uint64_t begin_fingerprint = 64 + begin - quot;
        const uint64_t end_fingerprint = 64 + end - quot;
        const size_t evict_rem_index = end_fingerprint;
        assert(begin_fingerprint < MAX_CAPACITY);

        const bool is_quot_not_empty = !(h1 & (1ULL << (begin + 1)));

        if (is_quot_not_empty) {
            uint64_t evict_q_att = quot;
            uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1];

            assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));

            memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                    &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 2],
                    end_fingerprint - begin_fingerprint - 1);// How many bytes to copy.
            ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
            return;
        } else {
            assert(begin + 1 == end);
            // size_t zero_count_up_to_begin = begin_fingerprint + 1;
            // auto temp = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot);
            // assert(zero_count_up_to_begin == pd512_plus::count_zeros_up_to_the_kth_one(pd, quot));


            size_t relative_next_non_empty_quots = _tzcnt_u64(~(h1 >> end));

            assert(quot + relative_next_non_empty_quots < QUOTS);
            uint64_t evict_q_att = quot + relative_next_non_empty_quots;
            assert(get_specific_quot_capacity_naive2(evict_q_att, pd));
            assert(get_specific_quot_capacity_naive2(evict_q_att - 1, pd) == 0);

            assert(end > 0);
            const uint64_t xor_mask = ((1ULL) | (1ULL << relative_next_non_empty_quots)) << end;

            assert(pd512::validate_number_of_quotient(pd));
            ((uint64_t *) pd)[1] ^= xor_mask;
            assert(pd512::validate_number_of_quotient(pd));
            assert(get_specific_quot_capacity_naive2(quot, pd));

            ((uint8_t *) pd)[kBytes2copy + evict_rem_index] = rem;

            return;
        }
    }

    inline void add_when_full_quot_eq_pop(int64_t quot, uint8_t rem, __m512i *pd) {
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        size_t begin = 63 - _lzcnt_u64(h0);
        size_t end = 64 + _tzcnt_u64(h1);
        if (begin + 1 < end) {

            const uint64_t begin_fingerprint = begin - quot;
            const uint64_t end_fingerprint = end - quot;

            uint64_t evict_q_att = quot;
            uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1];

            assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));
            memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                    &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 2],
                    end_fingerprint - begin_fingerprint - 1);// How many bytes to copy.
            ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
            return;
        } else {
            /** If we reach here, the following is true:
             * 1) begin == 63
             * 2) end == 64
            */
            assert(begin == 63);
            assert(end == 64);
            size_t relative_next_non_empty_quots = _tzcnt_u64(~(h1));
            assert(quot + relative_next_non_empty_quots < QUOTS);
            uint64_t evict_q_att = quot + relative_next_non_empty_quots;
            assert(get_specific_quot_capacity_naive2(evict_q_att, pd));
            assert(get_specific_quot_capacity_naive2(evict_q_att - 1, pd) == 0);

            assert(end > 0);
            // const uint64_t xor_mask = ((1ULL) | (1ULL << relative_next_non_empty_quots)) << end;

            //TODO *TEST THIS*:
            // set lowest off bit:
            assert(pd512::validate_number_of_quotient(pd));

            // v_pd512_plus::print_headers_extended(pd);
            const uint64_t h1_xor_mask = (1ULL << relative_next_non_empty_quots) | 1ULL;
            // uint64_t old_h0 = ((uint64_t *) pd)[0];
            // uint64_t old_h1 = ((uint64_t *) pd)[1];
            ((uint64_t *) pd)[1] ^= h1_xor_mask;
            // v_pd512_plus::print_headers_extended(pd);
            assert(pd512::validate_number_of_quotient(pd));

            // ((uint64_t *) pd)[1] ^= xor_mask;
            assert(get_specific_quot_capacity_naive2(quot, pd));

            ((uint8_t *) pd)[kBytes2copy + (end - quot)] = rem;

            return;
        }
    }

    inline void add_when_full(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd_full(pd));
        assert(quot < 50);

        if (quot == 0) {
            add_when_full_with_zero_quot(rem, pd);
            return;
        }

        const size_t last_occupied_quot = get_last_occupied_quot_for_full_pd(pd);

        if (last_occupied_quot < quot) {
            add_when_full_by_deleting_first_qr(quot, rem, pd);
            return;
        }

        //TODO: the following case: "(last_occupied_quot == quot)"
        /* else if (last_occupied_quot == quot){
            ((uint8_t *) pd)[63] = rem;
            return;
        } */


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t pop = _mm_popcnt_u64(h0);

        /***Trying to delete an element with the same quot - in order to minimize the number of writing we do*/

        if (quot < pop) {
            return add_when_full_quot_lt_pop(quot, rem, pd);
        } else if (pop < quot) {
            return add_when_full_quot_gt_pop(pop, quot, rem, pd);
        } else {
            assert(pop == quot);
            add_when_full_quot_eq_pop(quot, rem, pd);
        }
    }

    inline void add_when_full_old(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd_full(pd));
        assert(quot < 50);

        if (quot == 0) {
            // std::cout << "h0" << std::endl;
            add_when_full_with_zero_quot(rem, pd);
            return;
        }

        const size_t last_occupied_quot = get_last_occupied_quot_for_full_pd(pd);

        if (last_occupied_quot < quot) {
            add_when_full_by_deleting_first_qr(quot, rem, pd);
            return;
        }

        //TODO: the following case: "(last_occupied_quot == quot)"
        /* else if (last_occupied_quot == quot){
            ((uint8_t *) pd)[63] = rem;
            return;
        } */


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t pop = _mm_popcnt_u64(h0);

        /***Trying to delete an element with the same quot - in order to minimize the number of writing we do*/

        if (quot < pop) {
            // std::cout << "h1";

            // const uint64_t begin = (quot ? (pd512::select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
            const uint64_t begin = pd512::select64(h0, quot - 1);
            const uint64_t end = pd512::select64(h0, quot);
            const uint64_t begin_fingerprint = begin - quot;
            const uint64_t end_fingerprint = end - quot;
            assert(h0 & (1ULL << end));
            const bool is_quot_not_empty = !(h0 & (1ULL << (begin + 1)));

#ifndef NDEBUG
            const uint64_t db_end = pd512::select64(h0, quot);
            const auto v_quot_capi = get_specific_quot_capacity_naive2(quot, pd);
            const bool is_quot_not_empty_val = !(h0 & (1ULL << (db_end - 1)));
            assert(is_quot_not_empty_val == is_quot_not_empty);
#endif// NDEBUG \
        //TODO: we do not need to compute both "begin" and "end". "begin" ?suffice?.(it does not - we dont know how many bytes to move.)
            if (is_quot_not_empty) {
                // std::cout << ".1" << std::endl;

                // size_t rem_index = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot - 1);
                uint64_t evict_q_att = quot;
                uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1];

                assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));

                memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                        &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 2],
                        end_fingerprint - begin_fingerprint - 1);// How many bytes to copy.
                //TODO: end_fingerprint +- 1
                ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
                return;
            } else {
                // std::cout << ".2" << std::endl;
                assert(begin + 1 == end);
                add_when_full_to_empty_quot_LT_pop(quot, rem, pd);
                return;
            }
        } else if (pop < quot) {
            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
            const uint64_t begin = pd512::select64(h1, quot - pop - 1);
            const uint64_t end = pd512::select64(h1, quot - pop);
            const uint64_t begin_fingerprint = 64 + begin - quot;
            const uint64_t end_fingerprint = 64 + end - quot;
            const size_t evict_rem_index = end_fingerprint;
            assert(begin_fingerprint < MAX_CAPACITY);

            const bool is_quot_not_empty = !(h1 & (1ULL << (begin + 1)));

            if (is_quot_not_empty) {
                uint64_t evict_q_att = quot;
                uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1];

                assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));

                memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                        &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 2],
                        end_fingerprint - begin_fingerprint - 1);// How many bytes to copy.
                ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
                return;
            } else {
                assert(begin + 1 == end);
                // size_t zero_count_up_to_begin = begin_fingerprint + 1;
                // auto temp = pd512_plus::count_zeros_up_to_the_kth_one(pd, quot);
                // assert(zero_count_up_to_begin == pd512_plus::count_zeros_up_to_the_kth_one(pd, quot));


                size_t relative_next_non_empty_quots = _tzcnt_u64(~(h1 >> end));

                assert(quot + relative_next_non_empty_quots < QUOTS);
                uint64_t evict_q_att = quot + relative_next_non_empty_quots;
                assert(get_specific_quot_capacity_naive2(evict_q_att, pd));
                assert(get_specific_quot_capacity_naive2(evict_q_att - 1, pd) == 0);

                assert(end > 0);
                const uint64_t xor_mask = ((1ULL) | (1ULL << relative_next_non_empty_quots)) << end;

                assert(pd512::validate_number_of_quotient(pd));
                ((uint64_t *) pd)[1] ^= xor_mask;
                assert(pd512::validate_number_of_quotient(pd));
                assert(get_specific_quot_capacity_naive2(quot, pd));

                ((uint8_t *) pd)[kBytes2copy + evict_rem_index] = rem;

                return;
            }
        } else {
            assert(pop == quot);
            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
            size_t begin = 63 - _lzcnt_u64(h0);
            size_t end = _tzcnt_u64(h1);
            if (begin + 1 < end) {

                const uint64_t begin_fingerprint = begin - quot;
                const uint64_t end_fingerprint = end - quot;

                uint64_t evict_q_att = quot;
                uint64_t evict_r_att = ((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1];

                assert(v_ts_pd512::validate_evicted_qr(quot, rem, evict_q_att, evict_r_att, pd));
                memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                        &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 2],
                        end_fingerprint - begin_fingerprint - 1);// How many bytes to copy.
                ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
                return;
            } else {

                size_t relative_next_non_empty_quots = _tzcnt_u64(~(h1 >> end));
                assert(quot + relative_next_non_empty_quots < QUOTS);
                uint64_t evict_q_att = quot + relative_next_non_empty_quots;
                assert(get_specific_quot_capacity_naive2(evict_q_att, pd));
                assert(get_specific_quot_capacity_naive2(evict_q_att - 1, pd) == 0);

                assert(end > 0);
                // const uint64_t xor_mask = ((1ULL) | (1ULL << relative_next_non_empty_quots)) << end;

                //TODO *TEST THIS*:
                // set lowest off bit:
                assert(pd512::validate_number_of_quotient(pd));
                ((uint64_t *) pd)[1] |= ((uint64_t *) pd)[1] + 1;
                assert(pd512::validate_number_of_quotient(pd));

                // ((uint64_t *) pd)[1] ^= xor_mask;
                assert(get_specific_quot_capacity_naive2(quot, pd));

                ((uint8_t *) pd)[kBytes2copy + (end - quot)] = rem;

                return;
            }
        }
    }


    /*inline void add_att_core(int64_t quot, uint8_t rem, __m256i *pd) {
        constexpr unsigned kBytes2copy = 7;
        const uint64_t header = get_clean_header(pd);

        const uint64_t begin = quot ? (select64(header, quot - 1) + 1) : 0;
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= CAPACITY25 + QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        assert(validate_number_of_quotient(pd));
        write_header(begin, end, quot, pd);
        assert(validate_number_of_quotient(pd));

        __m256i pd_start = *pd;
        const __m256i shifted_pd = shift_left(*pd);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const uint8_t *) pd)[kBytes2copy + i]) break;
        }
        ((uint8_t *) &pd_start)[kBytes2copy + i] = rem;
        const uint32_t mask = ~MSK(kBytes2copy + i + 1);
        const __mmask32 blend_mask = mask;

        assert((i == end_fingerprint) ||
               (rem <= ((const uint8_t *) pd)[kBytes2copy + i]));

        assert(validate_number_of_quotient(pd));
        *pd = _mm256_mask_blend_epi8(blend_mask, pd_start, shifted_pd);

        // memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
        //         &((const uint8_t *) pd)[kBytes2copy + i],
        //         sizeof(*pd) - (kBytes2copy + i + 1));
        // In previous versions, changed 1 to 2, Because the last byte contained something when the pd is not full
        // ((uint8_t *) pd)[kBytes2copy + i] = rem;
        // assert(validate_number_of_quotient(pd));
    }*/


    inline void write_header_pop_equal_quot(uint64_t index, __m512i *pd) {
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        //TODO: switch order of if here.
        if (index == 63) {
            assert(h0 >> 63u);
            constexpr uint64_t mask63 = MSK(63);
            uint64_t *pd_64 = &((uint64_t *) pd)[0];
            pd_64[0] &= mask63;

            const uint64_t low = (pd_64[1] & h1_mask) | 1;
            const uint64_t high = pd_64[1] & ~h1_mask;
            pd_64[1] = low | high;
            return;
        }
        write_header6(index, pd);
    }


    inline bool pd_add_50(int64_t quot, char rem, __m512i *pd) {
        assert(quot < 50);
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        assert(pd512::popcount128(header) == 50);
        const unsigned fill = pd512::select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd512::pd_popcount(pd)));
        assert((fill == 51) == pd512::pd_full(pd));
        if (fill == 51) {
            add_when_full(quot, rem, pd);
            return true;
        }


        const uint64_t end = pd512::select128(header, quot);

        unsigned __int128 high_header = (header << 1) & ~((((unsigned __int128) 1) << (end + 1)) - 1);
        unsigned __int128 low_header = header & ((((unsigned __int128) 1) << (end)) - 1);
        unsigned __int128 temp = high_header & low_header;
        assert(!temp);
        unsigned __int128 new_header = high_header | low_header;
        assert(pd512::popcount128(new_header) == 50);
        assert(pd512::select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        new_header |= (pd512::did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        memcpy(pd, &new_header, kBytes2copy);

        // const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        memmove(&((char *) pd)[kBytes2copy + end_fingerprint + 1],
                &((const char *) pd)[kBytes2copy + end_fingerprint],
                sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((char *) pd)[kBytes2copy + end_fingerprint] = rem;

        assert(pd512::pd_find_50(quot, rem, pd));
        return true;
    }

    inline bool add(int64_t quot, uint8_t rem, __m512i *pd) {
        const bool res = pd_full(pd);
        (!res) ? add_when_not_full(quot, rem, pd) : add_when_full(quot, rem, pd);
        assert(find(quot, rem, pd));
        return res;
        // if (!pd_full(pd)) {
        //     add_when_not_full(quot, rem, pd);
        //     return false;
        // }
        // add_when_full(quot, rem, pd);
        // assert(find(quot, rem, pd));
        // return true;
    }

    inline bool add_old(int64_t quot, uint8_t rem, __m512i *pd) {
        if (pd_full(pd)) {
            assert(v_ts_pd512::val_AWF(quot, rem, pd));
            add_when_full(quot, rem, pd);
            assert(find(quot, rem, pd));
            // return false;
        } else {
#ifndef NDEBUG
            std::cout << "Bad bench" << std::endl;
            assert(0);
            __m512i old_pd = *pd;
            bool cond = find(quot, rem, pd);
#endif// NDEBUG
            add_when_not_full(quot, rem, pd);
            assert(find(quot, rem, pd));
            // add_when_not_full_db(quot, rem, pd);
            assert(v_ts_pd512::pd_distance(pd, &old_pd) == 1);
            return true;
        }
        return true;
    }

}// namespace ts_pd512


#endif// FILTERS_TS_PD512_HPP


/*    size_t h1_non_occupied_quots_count = _tzcnt_u64(~(h1 >> end));
                size_t evict_q_att = quot + h1_non_occupied_quots_count;
                assert(evict_q_att < QUOTS);
                // size_t evict_q_att = (63 - end) + h1_non_occupied_quots_count - quot;
                assert(get_specific_quot_capacity_naive2(evict_q_att, pd));
                assert(get_specific_quot_capacity_naive2(evict_q_att - 1, pd) == 0);

#ifndef NDEBUG
                uint64_t evict_q_val = 64;
                uint64_t evict_r_val = 64;

                get_evict_qr(quot, rem, pd, &evict_q_val, &evict_r_val);

                assert(evict_q_att == evict_q_val);
                // assert(evict_r_att == evict_r_val);
#endif// NDEBUG
                const uint64_t xor_mask = 1ULL << end;
                const uint64_t h1_xor_mask = 1ULL << h1_non_occupied_quots_count;
                assert(pd512::validate_number_of_quotient(pd));
                ((uint64_t *) pd)[0] ^= xor_mask;
                ((uint64_t *) pd)[1] ^= h1_xor_mask;
                assert(pd512::validate_number_of_quotient(pd));
                ((uint8_t *) pd)[kBytes2copy + evict_rem_index] = rem;

                // add_pd_full_naive_split_qlru(quot, rem, pd);

                // add_when_full_to_empty_quot_LT_pop(quot, rem, pd);

            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_const_mask;
            const uint64_t end = pd512::select64(h1, quot - pop);

            //header
            assert(pd512::validate_number_of_quotient(pd));

            const uint64_t h1_mask = MSK(end);
            const uint64_t h1_low = h1 & h1_mask;
            const uint64_t h1_high = (h1 >> end) << (end + 1);
            const uint64_t new_h1 = h1_low | h1_high;

            memcpy(&((uint64_t *) pd)[1], &new_h1, 5);
            assert(pd512::validate_number_of_quotient(pd));

            //body
            const uint64_t index = 64 + end - quot;
            //TODO: Fix body_add functions, and find out why they work in other places, but not here.
            body_add_naive(index, rem, pd);

            assert(pd512::validate_number_of_quotient(pd));
            assert(find(quot, rem, pd));

            return; 

            //FIXME: deal with this case.
            // std::cout << "h2" << std::endl;
            // add_pd_full_naive_split_qlru(quot, rem, pd);
            // add_pd_full_naive_split_uniform(quot, rem, pd);
            // if ((quot - pop) == 1){

            // } */
