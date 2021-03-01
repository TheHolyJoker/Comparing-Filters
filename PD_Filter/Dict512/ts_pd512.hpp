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
    size_t popcount_up_to_index(uint64_t word, size_t index);

    size_t popcount_up_to_index(uint64_t *arr, size_t a_size, size_t index);

    bool v_write_header_cond12(uint64_t h0, size_t index, uint64_t quot);
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

    template<typename T>
    void zero_array(T *a, size_t a_size) {
        for (size_t i = 0; i < a_size; i++) {
            a[i] = 0;
        }

        /* for (size_t i = 0; i < a_size; i++) {
            std::cout << "i: " << a[i] << std::endl;
        } */
    }
}// namespace v_ts_pd512

namespace ts_pd512 {

    constexpr size_t QUOTS = 50;
    constexpr size_t MAX_CAPACITY = 51;
    constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;


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

    inline bool find(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd512::pd_find_50(quot, rem, pd);
    }

    // inline auto conditional_remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
    //     return pd512::conditional_remove(quot, rem, pd);
    // }

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

        // Consider using this function instead of this for.
        // __mmask64 _mm512_cmp_epu8_mask (__m512i a, __m512i b, const int imm8)

        // for (; i < end_fingerprint; ++i) {
        //     if (rem <= ((const uint8_t *) pd)[kBytes2copy + i])
        //         break;
        // }

        // if ((i == end_fingerprint) || (rem != ((const uint8_t *) pd)[kBytes2copy + i])) {
        //     return false;
        // }

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

    /**
     * @brief Try to remove the element from the filter.
     * 
     * @param quot 
     * @param rem 
     * @param pd 
     * @return true 
     * @return false 
     */
    inline auto remove_attempt(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        static int count = 0;
        static int c2 = 0;
        count++;
        const __m512i old_pd = *pd;
        bool find_res = find(quot, rem, pd);
        auto res = remove_from_not_sorted_body_naive(quot, rem, pd);
        if (!res){
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

    inline auto remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        return remove_attempt(quot, rem, pd);
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
    inline auto remove_with_assertion(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        assert(find(quot, rem, pd));
        __m512i old_pd = *pd;
        auto res = remove_from_not_sorted_body_naive(quot, rem, pd);
        auto diff = v_ts_pd512::pd_distance(pd, &old_pd);
        assert(diff == 1);
        assert(res);
        return res;
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
     * @brief This function helps insertions. When "popcount(h0) == quot". 
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
        // if (pd64[0] & (1ULL << 63)) {
        //     pd64[0] ^= (1ULL << 63);
        //     uint64_t h1_low = ((pd64[1] << 1) & h1_const_mask) | 1;
        //     memcpy(pd64 + 1, &h1_low, 5);
        //     // return;
        // } else {
        //
        // return;
        // }
        //
        // size_t leading_zeros = _lzcnt_u64(pd64[0]);
        // if (leading_zeros == 0) {
        //     v_pd512_plus::print_headers(pd);
        //     assert(0);
        // }
        // assert(leading_zeros > 0);
        // uint64_t xor_mask = (3ULL << (63 - 1)) >> (leading_zeros - 1);
        // pd64[0] ^= xor_mask;
        // uint64_t h1_low = ((pd64[1] << 1) & h1_const_mask);
        // memcpy(pd64 + 1, &h1_low, 5);
        //
        // assert()
    }

    inline void write_header_naive(int64_t quot, uint64_t end, __m512i *pd) {
        assert(pd512::validate_number_of_quotient(pd));
        // v_pd512_plus::print_headers(pd);

        // constexpr uint64_t h1_mask = (1ULL << 37) - 1; // round(37 / 8) = 5.
        constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);
        uint64_t *pd64 = (uint64_t *) pd;

        if (quot == 0) {
            const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);
            memcpy(pd64 + 1, &low_h1, 5);
            pd64[0] <<= 1u;
            return;
        }
        if (end == 63) {
            constexpr uint64_t mask63 = (1ull << 63);
            pd64[0] ^= mask63;
            uint64_t h1_low = ((pd64[1] << 1) & h1_const_mask) | 1;
            memcpy(pd64 + 1, &h1_low, 5);
            return;
            // pd64[1] =
        } else if (end == 64) {
        }
        // constexpr uint64_t h1_const_mask = ((1ULL << (101 - 64)) - 1);

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

        // v_pd512_plus::print_headers(pd);
        assert(pd512::validate_number_of_quotient(pd));
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
     * @param index 
     * @param pd 
     */
    inline void write_header(uint64_t index, __m512i *pd) {
        //Not sure if this fucntion is ok or not.
        assert(0);
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);

        uint64_t *p_h1 = &((uint64_t *) pd)[1];
        const uint64_t h1_low = ((*p_h1 << 1) | (h0 >> 63u)) & h1_mask;
        const uint64_t h1_high = *p_h1 & ~h1_mask;

        *p_h1 = (h1_low | h1_high);


        const uint64_t h0_mask = MSK(index);
        const uint64_t h0_lower = h0 & h0_mask;

        const uint64_t new_h0 = h0_lower | ((h0 << 1u) & ~h0_mask);

        memcpy(pd, &new_h0, 8);
        //   *(p_h1-1) = new_h0;
    }

    inline void write_header6(uint64_t index, __m512i *pd) {
        assert(pd512::validate_number_of_quotient(pd));
        v_pd512_plus::print_headers(pd);

        // constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);

        uint64_t *pd64 = (uint64_t *) pd;
        const uint64_t low_h1 = ((pd64[1] << 1)) | (pd64[0] >> 63u);

        const uint64_t h0_mask = MSK(index);
        const uint64_t h0_lower = pd64[0] & h0_mask;

        pd64[0] = h0_lower | ((pd64[0] << 1u) & ~h0_mask);
        memcpy(pd64 + 1, &low_h1, 5);
        v_pd512_plus::print_headers(pd);
        assert(pd512::validate_number_of_quotient(pd));
    }


    inline void add_when_not_full_with_zero_quot(uint8_t rem, __m512i *pd) {
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t end = _tzcnt_u64(h0);
        write_header_naive(0, end, pd);
        // body_add(end, rem, pd);
        // assert(v_ts_pd512::validate_body_add(end, rem, pd));
        // body_add2(end, rem, pd);
        body_add_naive(end, rem, pd);
        return;
    }

    /**
     * @brief FIXME: this function contains errors. Specifically, 
     * I think that when an element is inserted another element might be deleted.
     * 
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void add_when_not_full(int64_t quot, uint8_t rem, __m512i *pd) {
        // static int j_arr[4];
        assert(!pd512::pd_full(pd));
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
            // assert(v_ts_pd512::validate_body_add(index, rem, pd));
            //TODO: Fix body_add functions, and find out why they work in other places, but not here.
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

            // const uint64_t pre_val_index = pd512::count_zeros_up_to_the_kth_one(pd, quot + 1);
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
            const uint64_t val_index = pd512::count_zeros_up_to_the_kth_one(pd, quot + 1) - 1;
            assert(index == val_index);
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

    inline void add_when_full_split_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        static int counter = 0;
        std::cout << "counter: " << counter++ << std::endl;
        assert(pd512::pd_full(pd));
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

    inline void add_when_full_with_zero_quot(uint8_t rem, __m512i *pd) {
        //TODO: test all cases in this function.
        // assert(quot == 0);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);

        //Hard case: i.e. no element with quot zero.
        if (h0 & 1) {
            //body
            ((uint8_t *) pd)[kBytes2copy] = rem;

            const uint64_t pre_pop = _mm_popcnt_u64(h0);
            //header
            // Turns first one to zero, and first zero to one.
            // Logically, we delete the first remainder, and add a new one, whose matching quotinet is zero.

            const uint64_t first_zero_index = _tzcnt_u64(~h0);
            const uint64_t xor_mask = (1ULL) | (1ULL << first_zero_index);
            assert(!(h0 & (1ULL << first_zero_index)));
            ((uint64_t *) pd)[0] ^= xor_mask;

            const uint64_t post_pop = _mm_popcnt_u64(h0);
            assert(pre_pop == post_pop);

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

    inline void add_when_full_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd512::pd_full(pd));
        assert(quot < 50);
        // constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        if (quot == 0) {
            add_when_full_with_zero_quot(rem, pd);
            return;
        }


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t pop = _mm_popcnt_u64(h0);

        /***Trying to delete an element with the same quot - in order to minimize the number of writing we do*/
        if (quot <= pop) {
            // const uint64_t begin = (quot ? (pd512::select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
            const uint64_t begin = pd512::select64(h0, quot - 1);
            const uint64_t end = pd512::select64(h0, quot);
            const uint64_t begin_fingerprint = begin - quot;
            const uint64_t end_fingerprint = end - quot;
            assert(h0 & (1ULL << end));

            const bool is_quot_not_empty = (h0 & (1ULL << (end - 1)));
            if (is_quot_not_empty) {
                assert(begin + 1 < end);
                assert(get_specific_quot_capacity_naive2(quot, pd));

                memmove(&((uint8_t *) pd)[kBytes2copy + begin_fingerprint],
                        &((const uint8_t *) pd)[kBytes2copy + begin_fingerprint + 1],
                        end_fingerprint - begin_fingerprint - 1);// How many bytes to move.
                ((uint8_t *) pd)[kBytes2copy + end_fingerprint - 1] = rem;
                return;
            } else {

                //we are looking for the first remainder whose quotient is the smallest among those bigger than `quot`.
                assert(begin + 1 == end);

                const uint64_t pre_pop = _mm_popcnt_u64(h0);


                const uint64_t rel_first_zero_index = _tzcnt_u64(~(h0 >> end));
                const uint64_t first_zero_index = rel_first_zero_index + end;

                // TODO: This assertion will occur.
                //word overflow.  No remainder's quot are in h0. (it is in h1).
                assert(first_zero_index < 64);

                const uint64_t xor_mask = ((1ULL) | (1ULL << rel_first_zero_index)) << end;
                assert(!(h0 & (1ULL << first_zero_index)));
                ((uint64_t *) pd)[0] ^= xor_mask;

                const uint64_t post_pop = _mm_popcnt_u64(h0);
                assert(pre_pop == post_pop);
                ((uint8_t *) pd)[kBytes2copy + begin_fingerprint] = rem;
            }
        } else {
            add_when_full_split_naive(quot, rem, pd);
            // if ((quot - pop) == 1){

            // }
        }
    }

    inline void add_when_full(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd512::pd_full(pd));
        assert(quot < 50);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        const size_t rand_rem_index = rand() % MAX_CAPACITY;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t pop = _mm_popcnt_u64(h0);


        if (quot <= pop) {
            /** In this case, we do not need to read the rest of the header. (we do have to rewrite to it)*/
            const uint64_t end = pd512::select64(h0, quot);

            write_header_naive(quot, end, pd);
            // write_header6(end, pd);
            body_add_naive(end - quot, rem, pd);
            // body_add2(end - quot, rem, pd);
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
        } else {
            /** We already have the popcount result, so ranking here is easier. */
            constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);
            const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
            const uint64_t end = pd512::select64(h1, quot - pop);

            //header
            const uint64_t h1_low = (h1 << 1);
            memcpy(&((uint64_t *) pd)[1], &h1_low, 5);

            const uint64_t index = 64 + end - quot;
            body_add_naive(index, rem, pd);
            // body_add2(index, rem, pd);
            return;
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


    inline bool add(int64_t quot, uint8_t rem, __m512i *pd) {
        // static int count_arr[2];
        // add_when_full_split_naive(quot, rem, pd);
        if (pd512::pd_full(pd)) {
            // count_arr[0]++;
            // std::cout << "h0:\t ";
            // std::cout << "count_arr[0]: " << count_arr[0] << std::endl;
            add_when_full_split_naive(quot, rem, pd);
        } else {
            // count_arr[1]++;
            // std::cout << "h1:\t ";
            // std::cout << "count_arr[1]: " << count_arr[1] << std::endl;
            __m512i old_pd = *pd;
            bool cond = find(quot, rem, pd);
            add_when_not_full(quot, rem, pd);

            auto res = v_ts_pd512::pd_distance(pd, &old_pd);
            if (res != 1) {
                std::cout << "old capacity: " << pd512::get_capacity_naive(&old_pd) << std::endl;
                std::cout << "new capacity: " << pd512::get_capacity_naive(pd) << std::endl;
                std::cout << "quot: " << quot << std::endl;
                std::cout << "rem: " << ((uint64_t) rem) << std::endl;
                std::cout << "res:  " << res << std::endl;
                v_pd512_plus::print_headers_extended(&old_pd);
                v_ts_pd512::print_body(&old_pd);
                v_pd512_plus::print_headers_extended(pd);
                v_ts_pd512::print_body(pd);
                v_ts_pd512::pd_distance(pd, &old_pd);
            }
            assert(res == 1);
        }

        // pd512::pd_add_50(quot, rem, pd);
        // if (!find(quot, rem, pd)) {
        //     std::cout << "count_arr[0]: " << count_arr[0] << std::endl;
        //     std::cout << "count_arr[1]: " << count_arr[1] << std::endl;
        //     assert(0);
        // }
        assert(find(quot, rem, pd));
        return true;
    }

}// namespace ts_pd512


namespace ts_pd512 {

    inline void evict_random_element(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(0);
    }

    /**
     * @brief Finding the last `remainder` in the "run" of the quotient `quot`, and replacing it with rem.
     * This functions only changes one byte in the body. (Does not change the header).
     *  
     * Assertion error if there is no element in the filter with quotient `quot`.
     * This function probably induce non-uniform evacuation policy (w.r.t. insertion time), and therefore will probably be replaced.  
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void evict_random_element_with_the_same_quot_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(ts_pd512::get_specific_quot_capacity_naive2(quot, pd));
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        assert(pd512::popcount128(header) == 50);
        const unsigned fill = pd512::select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd512::pd_popcount(pd)));
        assert((fill == 51) == pd512::pd_full(pd));


        const uint64_t end = pd512::select128(header, quot);
        const uint64_t end_fingerprint = end - quot;

        // memmove(&((uint8_t *) pd)[kBytes2copy + end_fingerprint + 1],
        //         &((const uint8_t *) pd)[kBytes2copy + end_fingerprint],
        //         sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((uint8_t *) pd)[kBytes2copy + end_fingerprint] = rem;

        assert(find(quot, rem, pd));
        // return true;
    }


    inline void evict_random_element_with_new_quot_super_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        size_t index = (rand() % QUOTS);

        const size_t rand_quot = pd512::count_ones_up_to_the_kth_zero(pd, index);
        //todo: add in assertion here of something.
        const uint8_t rand_rem = ((uint8_t *) pd)[kBytes2copy + index];

        //todo: smart remove here
        remove(rand_quot, rand_rem, pd);
        add(quot, rem, pd);
    }


    inline void evict_random_element_with_the_same_quot(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(get_specific_quot_capacity_naive2(quot, pd));
        const __m512i target = _mm512_set1_epi8(rem);
        // compare_less or equal.
        uint64_t v = _mm512_cmp_epu8_mask(target, *pd, 2) >> 13ul;
    }

    inline void evict_random_element_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd512::pd_full_naive(pd));
        if (get_specific_quot_capacity_naive2(quot, pd)) {
            evict_random_element_with_the_same_quot_naive(quot, rem, pd);
            return;
        }
    }

}// namespace ts_pd512

#endif// FILTERS_TS_PD512_HPP
