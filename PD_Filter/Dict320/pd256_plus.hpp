/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD256_PLUS_HPP
#define FILTERS_PD256_PLUS_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <immintrin.h>
#include <x86intrin.h>

#define QUOT_SIZE25 (25)
#define CAPACITY25 (25)
#define MAX(x, y) (((x) < (y)) ? y : x)
#define MIN(x, y) (((x) > (y)) ? y : x)

#define MSK(p) ((1ULL << p) - 1)
#define BITWISE_DISJOINT(a, b) (!((a) & (b)))


// #define DECODE(pd) (Lookup_Table[(_mm_extract_epi8(_mm256_castsi256_si128(*pd), 12) & 127)])

static constexpr uint8_t Table3_8[256] =
        {
                0,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                15,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                15,
                9,
                10,
                11,
                12,
                13,
                14,
                15,
                16,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                4,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                6,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                7,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                8,
                9,
                10,
                11,
                12,
                13,
                14,
                15,
                9,
                10,
                11,
                12,
                13,
                14,
                15,
                16,
                10,
                11,
                12,
                13,
                14,
                15,
                16,
                17,
};


namespace v_pd256_plus {

    void p_format_header(const __m256i *pd);

    void p_format_header(uint64_t header);

    void p_format_word(uint64_t x);

    auto to_bin(uint64_t x, size_t length) -> std::string;

    auto to_bin(uint64_t x) -> std::string;

    auto to_bin_reversed(uint64_t x) -> std::string;

    auto space_string(std::string s) -> std::string;

    auto bin_print_header_spaced(uint64_t header) -> std::string;

    auto bin_print_header_spaced2(uint64_t header) -> std::string;

    void print_headers(const __m256i *pd);

    inline void print_headers_masked(const __m256i *pd) {
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 1) & h1_mask;
        std::cout << "h0: " << bin_print_header_spaced2(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced2(h1) << std::endl;
    }

    inline void print_h1(bool mask, const __m256i *pd) {
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        // const uint64_t h0 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 0);
        uint64_t h1 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 1);
        if (mask)
            h1 &= h1_mask;
        // std::cout << "h0: " << bin_print_header_spaced2(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced2(h1) << std::endl;
    }

    void decode_by_table_validator();

    auto test_bit(size_t index, const __m256i *pd) -> bool;

    void print_hlb(const __m256i *pd);
}// namespace v_pd256_plus

namespace pd256_plus {
    auto validate_clz(int64_t quot, char rem, const __m256i *pd) -> bool;

    auto count_ones_up_to_the_kth_zero(const __m256i *x, size_t k = 51) -> size_t;

    auto count_ones_up_to_the_kth_zero(uint64_t word, size_t k) -> size_t;

    auto count_zeros_up_to_the_kth_one(const __m256i *x, size_t k) -> size_t;

    auto count_zeros_up_to_the_kth_one(uint64_t word, size_t k) -> size_t;


    auto validate_clz_helper(int64_t quot, char rem, const __m256i *pd) -> bool;

    auto validate_number_of_quotient(const __m256i *pd) -> bool;

    auto validate_number_of_quotient(uint64_t header) -> bool;

    auto validate_number_of_quotient_from_clean_header(uint64_t c_header) -> bool;

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select64(uint64_t x, int64_t j) {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    // returns the number of zeros before the jth (counting from 0) set bit of x
    inline uint64_t nth64(uint64_t x, int64_t j) {
        const uint64_t y = select64(x, j);
        assert(y < 64);
        const uint64_t z = x & ((UINT64_C(1) << y) - 1);
        return y - _mm_popcnt_u64(z);
    }

    inline int popcount64(uint64_t x) {
        return _mm_popcnt_u64(x);
    }

    auto get_capacity(const __m256i *pd) -> int;

    inline bool pd_full(const __m256i *pd) {
        // static int counter = 0;
        // counter++;
        return _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & (1ULL << (63 - (8)));
        // size_t cap = get_capacity(pd);
        // bool v_res = get_capacity(pd) == 25;
        // assert((!!res) == v_res);
        // return res;
        // return _mm_extract_epi16(_mm256_castsi256_si128(*pd), 6) & 240;
        // 16 + 32 + 64 + 128 == 240
    }


    inline uint8_t get_last_byte(const __m256i *pd) {
        uint8_t x;
        memcpy(&x, &((uint8_t *) pd)[31], 1);
        return x;

        //Old function:
        // return _mm_extract_epi8(_mm256_extracti64x2_epi64(*pd, 3), 15);
    }

    inline auto read_rem_by_index(size_t index, const __m256i *pd) -> uint8_t {

        // uint64_t *arr = ((uint64_t *) pd);
        // v_pd256_plus::p_format_word(arr[0]);
        // v_pd256_plus::p_format_word(arr[1]);
        // v_pd256_plus::p_format_word(arr[2]);
        // v_pd256_plus::p_format_word(arr[3]);
        assert(index < CAPACITY25);
        return ((uint8_t *) pd)[7U + index];
    }


    /**
     * @brief Get the header last byte object
     * This name was chosen for historical reasons. It actually returns header first byte.
     * @param pd 
     * @return uint8_t 
     */
    inline uint8_t get_header_last_byte(const __m256i *pd) {
        return _mm_cvtsi128_si32(_mm256_castsi256_si128(*pd)) & 63;
    }

    /**
     * @brief Get the header meta bits object
     * 
     * @param pd 
     * @return uint8_t return the first 6 bits of pd.
     * The first 5 bits encode the last none empty quot of this pd,
     * and the 6th bit indicate whether or not this pd overflowed. 
     */
    inline uint8_t get_header_meta_bits(const __m256i *pd) {
        return _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
    }


    inline uint64_t get_header(const __m256i *pd) {
        return ((uint64_t *) pd)[0] & ((1ULL << 56) - 1);
    }
    inline uint64_t get_header_smid(const __m256i *pd) {
        return _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & ((1ULL << 56) - 1);
    }

    inline uint64_t compute_last_quot_naive(const __m256i *pd) {
        size_t capacity = get_capacity(pd);
        if (capacity == 0)
            return 0;
        return count_ones_up_to_the_kth_zero(pd, capacity - 1);
    }

    uint64_t get_last_quot_after_future_swap_naive(int64_t new_quot, const __m256i *pd);

    inline uint64_t get_clean_header(const __m256i *pd) {
        // uint64_t res = (_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) >> 6) & ((1ULL << 50) - 1);
        // auto temp = _mm_popcnt_u64(res);
        // assert(_mm_popcnt_u64(res) == QUOT_SIZE25);
        return ((((uint64_t *) pd)[0]) >> 6ul) & ((1ULL << 50ul) - 1);
    }

    inline uint64_t clean_header(uint64_t header) {
        uint64_t res = ((header) >> 6) & ((1ULL << 50) - 1);
        // if (_mm_popcnt_u64(res) != QUOT_SIZE25){
        //     auto pop = _mm_popcnt_u64(res);
        //     std::cout << "pop: " << pop << std::endl;
        //     v_pd256_plus::p_format_header(header);
        //     assert(0);
        // }
        // assert(_mm_popcnt_u64(res) == QUOT_SIZE25);
        return res;
    }

    inline uint64_t decode_last_quot_naivest(const __m256i *pd) {
        uint64_t c_header = get_clean_header(pd);
        size_t capacity = get_capacity(pd);
        assert(capacity <= CAPACITY25);
        assert(capacity == get_capacity(pd));
        if (capacity == 0)
            return 0;
        return count_ones_up_to_the_kth_zero(c_header, capacity - 1);
    }

    inline uint64_t decode_last_quot_safe(const __m256i *pd) {
        return decode_last_quot_naivest(pd);
        // if (!pd_full(pd)) {
        //     uint64_t c_header = get_clean_header(pd);
        //     bool to_print = (c_header != MSK(25));
        //     auto capacity = count_zeros_up_to_the_kth_one(c_header, QUOT_SIZE25);
        //     assert(capacity == get_capacity(pd));
        //     if (capacity == 0) {
        //         if (to_print)
        //             {v_pd256_plus::p_format_header(pd);}
        //         return 0;
        //     }
        //     return count_ones_up_to_the_kth_zero(pd, capacity);
        // }
        // return _mm_cvtsi128_si32(_mm256_castsi256_si128(*pd)) & 31;
    }

    inline uint64_t decode_last_quot(const __m256i *pd) {
        return ((uint64_t *) pd)[0] & 31;
        // uint64_t v_res = decode_last_quot_safe(pd);
        // assert(res == v_res);
        // return res;
    }

    inline bool validate_decoding(const __m256i *pd) {
        auto a = decode_last_quot(pd);
        auto b = decode_last_quot_safe(pd);
        if (a != b) {
            v_pd256_plus::p_format_header(pd);
            std::cout << "a: " << a << std::endl;
            std::cout << "b: " << b << std::endl;
            auto c = decode_last_quot_safe(pd);
            auto d = decode_last_quot_safe(pd);
        }
        assert(a == b);
        return true;
    }

    inline uint64_t compute_last_quot_for_full_pd(const __m256i *pd) {
        assert(pd_full(pd));
        // assert(compute_last_quot_validate(pd));
        uint64_t header = ~(get_header(pd) << 8ul);
        auto res = (QUOT_SIZE25) -_lzcnt_u64(header);
        assert(res == compute_last_quot_naive(pd));
        return res;
        // return (QUOT_SIZE25) -_lzcnt_u64(header);
    }

    auto get_capacity_naive_with_OF_bit(const __m256i *x) -> size_t;

    auto get_specific_quot_capacity_wrapper(int64_t quot, const __m256i *pd) -> int;

    inline auto get_last_zero_index_naive(const __m256i *pd) -> uint8_t {
        assert(pd_full(pd));
        auto temp = count_ones_up_to_the_kth_zero(pd, CAPACITY25 - 1);
        return temp + CAPACITY25 - 1;
    }

    /**
     * @brief Get the last zero index:
     * returns the index of the last zero in the header.
     * 
     * @param pd 
     * @return uint8_t 
     */
    inline auto get_last_zero_index(const __m256i *pd) -> uint8_t {
        //Q:How many ones are there before the 51th zero? A:last_quot
        //Therefore, the index of the 51th zero is last_quot + 51 (-1 because we are starting to reps from zero).
        assert(pd_full(pd));
        auto res = decode_last_quot(pd) + CAPACITY25 - 1;// starting to reps from 0.
        // uint64_t res2 = Lookup_Table[(_mm_extract_epi8(_mm256_castsi256_si128(*pd), 12) & 127)];// starting to reps from 0.
        // uint64_t v_res = get_last_zero_index_naive(pd);
        // assert(res2 == v_res);
        assert(res == get_last_zero_index_naive(pd));
        return res;
    }

    inline auto get_last_quot_capacity_naive(int64_t last_quot, const __m256i *pd) -> uint8_t {
        return get_specific_quot_capacity_wrapper(last_quot, pd);
    }

    auto get_specific_quot_capacity_naive(int64_t quot, const __m256i *pd) -> int;

    auto get_specific_quot_capacity_naive2(int64_t quot, const __m256i *pd) -> int;

    inline auto get_last_quot_capacity_att(const __m256i *pd) -> uint8_t {
        uint64_t header = get_clean_header(pd) >> 14ul;
        size_t index = _lzcnt_u64(header);
        assert(index < 64);
        uint64_t header2 = ~(header << (index + 1));
        size_t index2 = _lzcnt_u64(header2);
        uint64_t header3 = ~(header2 << (index2));
        auto res = _lzcnt_u64(header3);
        assert(res != 64);
        assert(res == get_specific_quot_capacity_wrapper(decode_last_quot(pd), pd));
        return res;

        // res = (res != 64) ? res : 1;
        // auto v_res = get_specific_quot_capacity_wrapper(decode_last_quot(pd), pd);
    }


    inline auto compute_next_last_quot_naive(const __m256i *pd) -> uint8_t {
        size_t capacity = get_capacity(pd);
        if (capacity < 2)
            return 0;
        return count_ones_up_to_the_kth_zero(get_clean_header(pd), capacity - 1 - 1);
    }

    inline bool validate_compute_next_last_quot_att(uint8_t res, const __m256i *pd) {
        auto v_res = compute_next_last_quot_naive(pd);
        assert(v_res == res);
        return true;
    }

    /**
     * @brief an attempt to write a function that does not assume the pd is full
     * 
     * @param pd 
     * @return uint8_t 
     */
    inline auto compute_next_last_quot_att(const __m256i *pd) -> uint8_t {
        int static t_counter = 0;
        t_counter++;
        // uint64_t header1 = get_clean_header(pd) >> 14ul;
        // uint64_t header2 = get_clean_header(pd) << 14ul;
        // uint64_t res2 = compute_next_last_quot_helper1(header2);
        // uint64_t res1 = compute_next_last_quot_helper1(header1);
        uint64_t header = get_clean_header(pd) << 14ul;
        size_t index = _lzcnt_u64(header);
        assert(index < 64);
        uint64_t header2 = ~(header << (index + 1));
        size_t index2 = _lzcnt_u64(header2);// counting empty ones + 1 from the end.
        uint64_t header3 = (header2 << (index2 + 1));
        size_t index3 = _lzcnt_u64(header3);//connting second sequence of ones.
        auto res = QUOT_SIZE25 - 1 - index2 - index3;
        auto v_res = compute_next_last_quot_naive(pd);
        if (v_res != res) {
            v_pd256_plus::p_format_header(pd);
            std::cout << "res: " << res << std::endl;
            std::cout << "v_res: " << v_res << std::endl;
            std::cout << "t_counter: " << t_counter << std::endl;
            assert(0);
        }
        assert(validate_compute_next_last_quot_att(res, pd));
        //    assert(get_last_quot_after_future_swap_naive(0, pd));
        return res;

        // res = (res != 64) ? res : 1;
        // auto v_res = get_specific_quot_capacity_wrapper(decode_last_quot(pd), pd);
    }
    inline auto compute_curr_last_quot_att(const __m256i *pd) -> uint8_t {
        uint64_t header = get_clean_header(pd) >> 14ul;
        size_t index = _lzcnt_u64(header);
        assert(index < 64);
        uint64_t header2 = ~(header << (index + 1));
        size_t index2 = _lzcnt_u64(header2) + 1;// counting empty ones + 1 from the end.

        auto res = QUOT_SIZE25 - index2;
        assert(res == compute_last_quot_naive(pd));
        // auto v_res = compute_last_quot_naive(pd);
        // assert(v_res == res);
        return res;
    }


    /**
     * @brief Get the last quot capacity object
     * returns the capacity of the highest non empty quot.
     * @param last_quot 
     * @param pd 
     * @return uint8_t 
     */
    inline auto get_last_quot_capacity(int64_t last_quot, const __m256i *pd) -> uint8_t {
        //todo: fix this:
        return get_last_quot_capacity_att(pd);
        // return get_specific_quot_capacity_wrapper(last_quot, pd);

        /* //         assert(pd_full(pd));
//         assert(last_quot < QUOT_SIZE25);
//         const uint64_t last_zero_index = last_quot + (CAPACITY25 - 1);
//         const uint64_t rel_index = last_zero_index & 63;
//         assert(last_zero_index > 64);
// 
//         const uint64_t h1 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 1);
//         uint64_t h1_masked = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 1) & ((1ULL << (rel_index)) - 1);
//         const auto first_one_index = _lzcnt_u64(h1_masked);
//         const auto first_one_index_lsb = 63 - _lzcnt_u64(h1_masked);
//         assert(first_one_index_lsb < rel_index);
//         auto res = rel_index - first_one_index_lsb;
// #ifndef NDEBUG
//         auto v_res = get_specific_quot_capacity_wrapper(last_quot, pd);
//         if (v_res != res) {
//             v_pd256_plus::print_headers(pd);
//             v_pd256_plus::print_hlb(pd);
//             assert(false);
//         }
// #endif// DEBUG
// 
//         assert(res == get_specific_quot_capacity_wrapper(last_quot, pd));
//         return res; */
    }


    inline bool pd_empty(const __m256i *pd) {
        assert(0);
        const uint64_t header = get_clean_header(pd);
        auto res = (header == (1ULL << 25) - 1);
        auto v_res = get_capacity(pd) == 0;
        assert(res == v_res);
        return res;
    }

    inline uint8_t get_last_quot_for_full_pd_without_deletions(const __m256i *pd) {
        //todo ???????
        return count_ones_up_to_the_kth_zero(pd);
    }

    //    inline uint8_t get_last_qr_in_pd(const __m256i *pd) {
    //        assert(0);
    //        return -1;
    //        uint64_t quot = 50 - decode_last_quot(pd);
    //        constexpr int imm1 = 3;
    //        constexpr int imm2 = 15;
    //        const uint64_t rem = _mm_extract_epi8(_mm256_extracti64x2_epi64(*pd, imm1), imm2);
    //        return (quot << 8ul) | rem;
    //    }

    inline uint64_t get_last_quot_after_future_swap_ext_slow(int64_t new_quot, const __m256i *pd) {
        assert(pd_full(pd));
        uint64_t curr_last_q = decode_last_quot(pd);
        if (new_quot > curr_last_q)
            return curr_last_q;

        if (new_quot == curr_last_q) {
            assert(get_specific_quot_capacity_wrapper(curr_last_q, pd));
            return curr_last_q;
        }

        if (get_specific_quot_capacity_wrapper(curr_last_q, pd) > 1)
            return curr_last_q;
        while (new_quot < curr_last_q) {
            curr_last_q--;
            if (get_specific_quot_capacity_wrapper(curr_last_q, pd) > 0)
                return curr_last_q;
        }
        return new_quot;
    }

    inline uint64_t get_last_quot_after_future_swap_naive(int64_t new_quot, const __m256i *pd) {
        assert(pd_full(pd));
        const uint64_t curr_last_q = decode_last_quot(pd);

        if (new_quot >= curr_last_q)
            return curr_last_q;

        const uint64_t header = get_clean_header(pd);
        auto att = count_ones_up_to_the_kth_zero(pd, CAPACITY25 - 1 - 1);
        auto temp_min = MIN(new_quot, curr_last_q);
        auto min_q = MIN(temp_min, att);
        if (min_q == att)
            att = temp_min;
        auto v_att = get_last_quot_after_future_swap_ext_slow(new_quot, pd);
        assert(att == v_att);
        return att;
    }

    inline uint64_t find_next_zero_from_msb(const uint64_t last_zero_index, const __m256i *pd) {
        assert(last_zero_index < (64 - 8));
        const uint64_t header = get_header(pd);
        const uint64_t temp = (~header) & ((1ULL << last_zero_index) - 1);
        uint64_t temp_index = _lzcnt_u64(temp);
        assert(temp_index != 64);
        return 63 - temp_index;
    }

    inline uint64_t find_next_zero_from_msb(const __m256i *pd) {
        return find_next_zero_from_msb(get_last_zero_index(pd) + 6, pd);
    }

    /**
     * @brief Get the last quot after future swap object
     * 
     * this function is never called for the first time the pd gets full.
     * Meaning, the quot bits are already valid.
     * @param new_quot 
     * @param pd 
     * @return uint64_t 
     */
    inline uint64_t get_last_quot_after_future_swap(int64_t new_quot, const __m256i *pd) {
        assert(pd_full(pd));
        const uint64_t curr_last_q = decode_last_quot(pd);
        assert(validate_decoding(pd));
        if (new_quot >= curr_last_q)
            return curr_last_q;

        const uint64_t last_zero_index = get_last_zero_index(pd) + 6;
        const uint64_t next_zero_index = find_next_zero_from_msb(last_zero_index, pd);
        assert(last_zero_index > next_zero_index);
        auto diff = last_zero_index - next_zero_index - 1;
        uint64_t nom = curr_last_q - diff;
        uint64_t res = MAX(new_quot, nom);

        //validation of nom (disregarding of new_quot):
        // auto a = count_zeros_up_to_the_kth_one(get_clean_header(pd), nom);
        // auto b = count_zeros_up_to_the_kth_one(get_clean_header(pd), nom + 1);
        // assert(b - a > 0);

        assert(res == get_last_quot_after_future_swap_naive(new_quot, pd));
        return res;
    }

    inline bool pd_find_25_ver0(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < 25);

        const __m256i target = _mm256_set1_epi8(rem);
        uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v)
            return false;

        const uint64_t header = get_clean_header(pd);
        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0) - quot;
        const uint64_t end = select64(header, quot) - quot;
        assert(begin <= end);
        assert(end <= QUOT_SIZE25);

        if (begin == end)
            return false;

        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_25_ver7(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;


        const int64_t h0 = get_clean_header(pd);

        if (_blsr_u64(v) == 0) {
            const int64_t mask = v << quot;
            return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
        }
        if (quot == 0)
            return v & (_blsmsk_u64(h0) >> 1ul);

        uint64_t new_v = (v << quot) & ~h0;
        const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
        const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
        // bool att = v_mask & new_v;
        return v_mask & new_v;
    }

    inline bool pd_find_25_ver8(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;


        const int64_t h0 = get_clean_header(pd);
        uint64_t new_v = (v << quot) & ~h0;

        if (_blsr_u64(new_v) == 0) {
            return _mm_popcnt_u64(h0 & (new_v - 1)) == quot;
        } else {
            if (quot) {
                const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
                const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
                const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
                const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
                // bool att = v_mask & new_v;
                return v_mask & new_v;
            } else {
                return v & (_blsmsk_u64(h0) >> 1ul);
            }
        }
    }

    inline bool pd_find_25_ver9(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
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

    inline bool pd_find_25_ver10(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = get_clean_header(pd);

        if (v_off == 0) {
            const uint64_t mask = v << quot;
            return (_mm_popcnt_u64(h0 & (mask - 1)) == quot) && (!(h0 & mask));
        }

        if (quot) {
            uint64_t new_v = (v << quot) & ~h0;
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
            const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
            // bool att = v_mask & new_v;
            return v_mask & new_v;
        } else {
            return v & (_blsmsk_u64(h0) >> 1ul);
        }
    }

    inline bool pd_find_25_ver11(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = get_clean_header(pd);

        if (v_off == 0) {
            const uint64_t mask = v << quot;
            return (_mm_popcnt_u64(h0 & (mask - 1)) == quot) && (!(h0 & mask));
        } else if (_blsr_u64(v_off) == 0) {
            // const uint64_t new_v = v & ~(h0 >> quot);
            const uint64_t mask1 = (v_off << quot) & (~h0);
            const uint64_t mask2 = ((v ^ v_off) << quot) & (~h0);
            const bool res = ((_mm_popcnt_u64(h0 & (mask1 - 1)) == quot) || (_mm_popcnt_u64(h0 & (mask2 - 1)) == quot));
            assert(res == pd_find_25_ver0(quot, rem, pd));
            return ((_mm_popcnt_u64(h0 & (mask1 - 1)) == quot) || (_mm_popcnt_u64(h0 & (mask2 - 1)) == quot));
        }

        if (quot) {
            uint64_t new_v = (v << quot) & ~h0;
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
            const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
            // bool att = v_mask & new_v;
            return v_mask & new_v;
        } else {
            return v & (_blsmsk_u64(h0) >> 1ul);
        }
    }

    inline bool pd_find_25_ver12(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;


        const int64_t h0 = get_clean_header(pd);
        uint64_t new_v = (v << quot) & ~h0;
        uint64_t new_v_off = _blsr_u64(new_v);

        if (new_v_off == 0)
            return _mm_popcnt_u64(h0 & (new_v - 1)) == quot;
        else if (_blsr_u64(new_v_off) == 0) {
            bool res = (_mm_popcnt_u64(h0 & ((new_v_off) -1)) == quot) ||
                       (_mm_popcnt_u64(h0 & ((new_v ^ new_v_off) - 1)) == quot);
            assert(res == pd_find_25_ver0(quot, rem, pd));
            return res;
            // return _mm_popcnt_u64(h0 & (new_v - 1)) == quot;
        } else {
            if (quot) {
                const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
                const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
                const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
                const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
                // bool att = v_mask & new_v;
                return v_mask & new_v;
            } else {
                return v & (_blsmsk_u64(h0) >> 1ul);
            }
        }
    }

    inline bool pd_find_25_ver13_db(uint64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = (_mm256_cmpeq_epu8_mask(target, *pd) >> 7ul);
        const uint64_t v2 = ((uint64_t)(_mm256_cmpeq_epu8_mask(target, *pd) >> 7ULL)) << quot;
        // // const uint64_t v3 = (_mm256_cmpeq_epu8_mask(target, *pd) << quot) >> 7ul;
        // // if ((v << quot) != v2) {
        // //     std::cout << std::string(80, '*') << std::endl;
        // //     v_pd256_plus::p_format_word(v);
        // //     v_pd256_plus::p_format_word(v << quot);
        // //     v_pd256_plus::p_format_word(v2);
        // //     v_pd256_plus::p_format_word(v3);
        // //     std::cout << std::string(80, '-') << std::endl;
        // //     std::cout << "v        : \t" << v << std::endl;
        // //     std::cout << "v << quot: \t" << (v << quot) << std::endl;
        // //     std::cout << "v2:      : \t" << v2 << std::endl;
        // //     std::cout << "v3:      : \t" << v3 << std::endl;
        // //     std::cout << "quot:    : \t" << quot << std::endl;
        // //     std::cout << std::string(80, '*') << std::endl;
        // // }
        // assert((v << quot) == v2);
        if (!v) return false;

        // if (!v2) {
        //     std::cout << "here" << std::endl;
        // }
        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = get_clean_header(pd);

        if (v_off == 0) {
            const uint64_t mask = v << quot;
            auto res = (_mm_popcnt_u64(h0 & (v2 - 1)) == quot) && (!(h0 & v2));
            auto v_res = (_mm_popcnt_u64(h0 & (mask - 1)) == quot) && (!(h0 & mask));
            assert(res == v_res);
            return (_mm_popcnt_u64(h0 & (v2 - 1)) == quot) && (!(h0 & v2));
        }

        if (quot == 0) {
            assert(v == v2);
            return v & (_blsmsk_u64(h0) >> 1ul);
        }
        uint64_t new_v = (v << quot) & ~h0;
        uint64_t new_v2 = v2;
        const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
        const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
        auto res = v_mask & new_v2;
        auto v_res = v_mask & new_v;
        assert(res == v_res);
        return v_mask & v2;
    }

    inline bool pd_find_25_ver13(uint64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        // const uint64_t v = (_mm256_cmpeq_epu8_mask(target, *pd) >> 7ul);
        const uint64_t v2 = ((uint64_t)(_mm256_cmpeq_epu8_mask(target, *pd) >> 7ULL)) << quot;

        if (!v2) return false;

        const uint64_t v_off = _blsr_u64(v2);
        const uint64_t h0 = get_clean_header(pd);

        if (v_off == 0) {
            return (_mm_popcnt_u64(h0 & (v2 - 1)) == quot) && (!(h0 & v2));
        }

        if (quot == 0) {
            return v2 & (_blsmsk_u64(h0) >> 1ul);
        }
        const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
        const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
        return v_mask & v2;
    }

    inline bool pd_find_25_ver14_db(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const int64_t h0 = get_clean_header(pd);
        uint64_t new_v = (v << quot) & ~h0;
        const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        const uint64_t m0 = (~_bzhi_u64(-1, 0 - 1));
        const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
        const uint64_t v_mask = _blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits);
        // bool att = v_mask & new_v;
        return v_mask & new_v;
    }

    inline uint64_t mask_between_bits_naive(uint64_t x) {
        uint64_t hi_bit = (x - 1) & x;
        uint64_t lo_bit = hi_bit ^ x;
        uint64_t mid_lo = hi_bit - 1;
        uint64_t lo = (lo_bit - 1) | lo_bit;
        uint64_t res = mid_lo ^ lo;
        assert((res & (~x)) == res);
        return res;
    }

    inline uint64_t mask_between_bits_naive2(uint64_t x) {
        uint64_t hi_bit = (x - 1) & x;
        uint64_t clear_hi = hi_bit - 1;
        uint64_t lo_set = ((x - 1) | x);
        uint64_t res = (clear_hi ^ lo_set) & (~x);
        uint64_t v_res = mask_between_bits_naive(x);

        // if (res != v_res){

        //     v_pd256_plus::p_format_word(x);
        //     v_pd256_plus::p_format_word(res);
        //     v_pd256_plus::p_format_word(v_res);
        //     std::cout << std::string(80, '=') << std::endl;
        //     v_pd256_plus::p_format_word(hi_bit);
        //     v_pd256_plus::p_format_word(clear_hi);
        //     v_pd256_plus::p_format_word(lo_set);
        //     v_pd256_plus::p_format_word(res);
        //     std::cout << std::string(80, '=') << std::endl;
        //     assert(0);
        // }
        assert(res == mask_between_bits_naive(x));
        return res;
    }
    inline uint64_t mask_between_bits_naive3(uint64_t x) {
        auto res = _blsmsk_u64(x) ^ _blsmsk_u64(_blsr_u64(x) >> 1);
        assert(res == mask_between_bits_naive2(x));
        assert(res == mask_between_bits_naive(x));
        return res;
    }
    inline uint64_t mask_between_bits_naive5(uint64_t x) {
        uint64_t hi_bit = (x - 1) & x;
        uint64_t clear_hi = hi_bit - 1;
        uint64_t lo_set = (x - 1);
        uint64_t res = clear_hi ^ lo_set;// & (~x0)
        return res;
    }

    inline bool pd_find_25_ver15(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const uint64_t h0 = get_clean_header(pd);
        if (quot) {
            const uint64_t new_v = (v << quot) & ~h0;
            const uint64_t mask2 = 3ul << (quot - 1);
            const uint64_t two_bits_mask = _pdep_u64(mask2, h0);
            const uint64_t v_mask = mask_between_bits_naive5(two_bits_mask);
            return v_mask & new_v;
        }
        return v & (_blsmsk_u64(h0) >> 1ul);
    }

    inline bool pd_find_25_ver16(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const uint64_t h0 = get_clean_header(pd);
        const uint64_t new_v = (v << quot) & ~h0;
        const uint64_t v_mask = (quot) ? mask_between_bits_naive5(_pdep_u64(3ul << (quot - 1), h0)) : (_blsmsk_u64(h0) >> 1ul);
        return v_mask & new_v;
    }

    inline bool pd_find_25_ver17(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd);
        if (!v)
            return false;
        const uint64_t h0 = get_clean_header(pd);
        const uint64_t new_v = ((v >> 7ul) << quot) & ~h0;
        const uint64_t v_mask = (quot) ? mask_between_bits_naive5(_pdep_u64(3ul << (quot - 1), h0)) : (_blsmsk_u64(h0) >> 1ul);
        return v_mask & new_v;
    }

    inline bool pd_losse_find_ver1(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;


        const int64_t h0 = get_clean_header(pd);
        // uint64_t new_v = (v << quot) & ~h0;

        if (_blsr_u64(v) == 0) {
            const int64_t mask = v << quot;
            return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
        } else {
            return true;
        }
    }

    inline bool pd_losse_find_ver2(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE25);

        const __m256i target = _mm256_set1_epi8(rem);
        uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const int64_t h0 = get_clean_header(pd);
        uint64_t new_v = (v << quot) & ~h0;

        if (_blsr_u64(new_v) == 0) {
            return (_mm_popcnt_u64(h0 & (new_v - 1)) == quot);
        } else {
            return true;
        }
    }

    inline bool pd_losse_find_ver3(int64_t quot, uint8_t rem, const __m256i *pd) {
        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;
        if (!v) return false;

        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = get_clean_header(pd);

        if (v_off == 0) {
            const uint64_t mask = v << quot;
            return (_mm_popcnt_u64(h0 & (mask - 1)) == quot) && (!(h0 & mask));
        } else
            return true;
    }


    inline bool pd_find_25(int64_t quot, uint8_t rem, const __m256i *pd) {
#ifndef NDEBUG
        if (pd_find_25_ver0(quot, rem, pd) != pd_find_25_ver13(quot, rem, pd)) {
            pd_find_25_ver13(quot, rem, pd);
            pd_find_25_ver13(quot, rem, pd);
        }
#endif// NDEBUG

        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver7(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver8(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver9(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver10(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver12(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver11(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver13(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver15(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver16(quot, rem, pd));
        assert(pd_find_25_ver0(quot, rem, pd) == pd_find_25_ver17(quot, rem, pd));
        // return pd_find_25_ver17(quot, rem, pd);
        return pd_find_25_ver9(quot, rem, pd);
        // return pd_find_25_ver13(quot, rem, pd);
    }

    enum pd_Status {
        No = 0,
        Yes = 1,
        look_in_the_next_level = 2,
        Error = -1
    };


    inline bool did_pd_overflowed(const __m256i *pd) {
        return !(((uint64_t *) pd)[0] & 32);
    }


    inline void set_overflow_bit(__m256i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        h_array[0] |= 32;
        h_array[0] ^= 32;
        assert(did_pd_overflowed(pd));
    }

    inline void clear_overflow_bit(__m256i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        h_array[0] |= 32;
        // h_array[0] ^= 32;
        assert(!did_pd_overflowed(pd));
    }


    inline void encode_last_quot_new(int64_t last_quot, __m256i *pd) {
        //check this
        uint8_t byte_to_write = last_quot | (get_header(pd) & (32 + 64 + 128));
        memcpy(pd, &byte_to_write, 1);
        assert(decode_last_quot(pd) == last_quot);
    }

    inline void update_max_quot_not_overflowing(int64_t nom_quot, int64_t curr_quot, __m256i *pd) {
        // assert(!pd_full(pd));
        if (nom_quot <= curr_quot)
            return;
        uint8_t first_byte = nom_quot | (get_header(pd) & (32 + 64 + 128));
        memcpy(pd, &first_byte, 1);
        assert(decode_last_quot(pd) == nom_quot);
    }

    inline void update_max_quot_not_overflowing(int64_t nom_quot, __m256i *pd) {
        uint64_t curr_q = decode_last_quot(pd);
        update_max_quot_not_overflowing(nom_quot, curr_q, pd);
    }

    inline void update_max_quot(int64_t nom_quot, __m256i *pd) {
        uint64_t curr_q = decode_last_quot(pd);
        if (pd_full(pd)) {
            if (nom_quot >= curr_q)
                return;

            // Did not test that the last 3 bits of the first byte were not changed.
            uint8_t first_byte = nom_quot | (get_header(pd) & (32 + 64 + 128));
            memcpy(pd, &first_byte, 1);
            assert(decode_last_quot(pd) == nom_quot);
        } else {
            if (nom_quot <= curr_q)
                return;
            uint8_t first_byte = nom_quot | (get_header(pd) & (32 + 64 + 128));
            memcpy(pd, &first_byte, 1);
            assert(decode_last_quot(pd) == nom_quot);
        }
    }

    inline void update_max_quot(int64_t new_quot, int64_t old_quot, __m256i *pd) {
        update_max_quot(MIN(new_quot, old_quot), pd);
    }


    inline bool cmp_qr_naive(uint16_t qr, const __m256i *pd) {
        const uint64_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        if ((hfb & 32))
            return false;
        uint16_t old_q = (hfb & 31);
        if (old_q < (qr >> 8))
            return true;
        else if (old_q > (qr >> 8)) {
            return false;
        } else {
            return get_last_byte(pd) < (qr & 255);
        }
    }

    inline bool cmp_qr_naive(int64_t quot, uint8_t rem, const __m256i *pd) {
        return cmp_qr_naive((quot << 8) | rem, pd);
    }

    inline pd_Status get_status_after_l1_failed(uint16_t qr, const __m256i *pd) {
        // static int counter = 0;
        // counter++;
        // static int c1 = 0;
        // static int c2 = 0;
        // static int c3 = 0;
        // static int c4 = 0;
        bool of_res = did_pd_overflowed(pd);
        size_t r_counter = 0;
        const uint64_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        if ((hfb & 32)) {
            // c1++;
            // std::cout << "s0" << std::endl;
            return No;
        }
        assert(did_pd_overflowed(pd));
        uint16_t old_q = hfb;
        if (old_q < (qr >> 8)) {
            // c2++;
            // std::cout << "s1" << std::endl;
            return look_in_the_next_level;
        } else if (old_q > (qr >> 8)) {
            // c3++;
            // std::cout << "s2" << std::endl;
            return No;
        } else {
            // assert(qr >>);
            // c4++;
            // std::cout << "s3" << std::endl;
            //Fix this: should not be equality.
            return (get_last_byte(pd) <= (qr & 255)) ? look_in_the_next_level : No;
        }
    }

    inline pd_Status get_status_after_l1_failed(int64_t quot, uint8_t rem, const __m256i *pd) {
        return get_status_after_l1_failed((quot << 8) | ((uint64_t) rem), pd);
    }


    inline bool cmp_qr_smart0(uint16_t qr, const __m256i *pd) {
        assert(0);
        const uint16_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        const uint16_t old_qr = (hfb & 31) << 8 | get_last_byte(pd);
        return (hfb && 64) && (old_qr < qr);
    }

    inline bool cmp_qr_smart_slow(uint16_t qr, const __m256i *pd) {
        assert(0);

        // if (!(_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 32)){
        //     return false;
        // }

        const uint64_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        uint16_t old_qr = ((hfb & 31) << 8ul) | get_last_byte(pd);
        bool res = (hfb & 32) && (((hfb & 31) < (qr >> 8)) || (old_qr < qr));
        assert(res == cmp_qr_naive(qr, pd));
        return res;

        // assert(0);
        // uint8_t x, y;
        // memcpy(&x, &((uint8_t *) pd)[12], 1);
        // memcpy(&y, &((uint8_t *) pd)[63], 1);
        // return (x & 128) && ((Lookup_Table[x & 127] << 8 | y) < qr);
    }

    inline bool cmp_qr_smart(uint16_t qr, const __m256i *pd) {
        // assert(0);
        // const uint64_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        if (_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 32) {
            return false;
        }
        const uint64_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 31;
        const uint16_t old_qr = (hfb << 8ul) | get_last_byte(pd);
        // assert((old_qr < qr) == cmp_qr_naive(qr, pd));
        return old_qr < qr;
    }
    /**
     * @brief indicate if we should solely look for the element in the next level. 
     * 
     * @param qr 
     * @param pd 
     * @return true Look only in the second level.
     * @return false Look only in this PD.
     */
    inline bool cmp_qr1(uint16_t qr, const __m256i *pd) {
        if (((uint64_t *) pd)[0] & 32) {
            return false;
        }
        const uint64_t hfb = ((uint64_t *) pd)[0] & 31;
        const uint16_t old_qr = (hfb << 8ul) | get_last_byte(pd);
        return old_qr < qr;
    }

    inline bool cmp_qr2(uint16_t qr, const __m256i *pd) {
        const uint64_t hfb = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        return ((hfb << 8ul) | get_last_byte(pd)) < qr;
        //   return (hfb & 32) && (((hfb & 31) << 8ul) | get_last_byte(pd)) < qr;
    }

    inline pd_Status pd_find0(int64_t quot, uint8_t rem, const __m256i *pd) {
        if (pd_find_25(quot, rem, pd))
            return Yes;

        return get_status_after_l1_failed(quot, rem, pd);
    }

    inline pd_Status pd_find1(int64_t quot, uint8_t rem, const __m256i *pd) {
        if (pd_find_25(quot, rem, pd))
            return Yes;

        const uint64_t first_byte = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        return ((first_byte & 31) <= quot) ? look_in_the_next_level : No;
        // return ((first_byte & 32) && ((first_byte & 31) <= quot)) ? look_in_the_next_level : No;
    }

    // inline pd_Status pd_find3(int64_t quot, uint8_t rem, const __m256i *pd) {

    //     if (!(_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 32))
    //         return (pd_find_25(quot, rem, pd))
    //     if (pd_find_25(quot, rem, pd))
    //         return Yes;

    //     const uint64_t first_byte = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
    //     return ((first_byte & 32) && ((first_byte & 31) <= quot)) ? look_in_the_next_level : No;
    // }

    inline pd_Status pd_find2(int64_t quot, uint8_t rem, const __m256i *pd) {
        if (!pd_find_25(quot, rem, pd)) {
            const uint64_t first_byte = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
            return ((first_byte & 32) && ((first_byte & 31) <= quot)) ? look_in_the_next_level : No;
        }
        return Yes;
    }

    inline pd_Status losse_find(int64_t quot, uint8_t rem, const __m256i *pd) {
        if (pd_losse_find_ver3(quot, rem, pd))
            return Yes;

        const uint64_t first_byte = _mm_cvtsi128_si64(_mm256_castsi256_si128(*pd)) & 63;
        return ((first_byte & 32) && ((first_byte & 31) <= quot)) ? look_in_the_next_level : No;
    }


    inline void write_header_db(uint64_t begin, uint64_t end, int64_t quot, __m256i *pd) {
        validate_number_of_quotient(pd);
        static int counter = 0;
        counter++;
        std::cout << std::string(80, '-') << std::endl;
        std::cout << "Before:  \t";
        v_pd256_plus::p_format_header(pd);
        const uint64_t hMetaBits = get_header_meta_bits(pd);
        const uint64_t old_header = get_clean_header(pd);

        uint64_t nh0 = old_header;
        std::cout << "op0:     \t";
        v_pd256_plus::p_format_header(nh0);
        uint64_t nh1 = nh0 & MSK(begin);
        uint64_t new_header = old_header & MSK(begin);

        std::cout << "op1:     \t";
        v_pd256_plus::p_format_header(nh1);
        uint64_t temp = (old_header >> end) << (end + 1);

        std::cout << "temp:     \t";
        v_pd256_plus::p_format_header(temp);

        uint64_t nh2 = nh1 | temp;
        std::cout << "op2:     \t";
        v_pd256_plus::p_format_header(nh2);

        new_header |= ((old_header >> end) << (end + 1));
        new_header <<= 6;

        uint64_t nh3 = nh2 << 6;
        std::cout << "op3:     \t";
        v_pd256_plus::p_format_header(nh3);

        uint64_t new_quot = MAX(hMetaBits & 31, quot);
        uint64_t new_mBits = new_quot + (hMetaBits & 32);
        assert(BITWISE_DISJOINT(new_mBits, new_header));

        uint64_t nh4 = nh3 | new_mBits;
        new_header |= new_mBits;

        std::cout << "op4:     \t";
        v_pd256_plus::p_format_header(nh4);

        std::cout << "After:   \t";
        v_pd256_plus::p_format_header(new_header);

        std::cout << std::string(80, '-') << std::endl;
        validate_number_of_quotient(new_header);
        memcpy(pd, &new_header, 7);
    }


    inline void write_header_with_comments(uint64_t begin, uint64_t end, int64_t quot, __m256i *pd) {
        // auto temp = get_capacity(pd);
        assert(get_capacity(pd) < CAPACITY25);
        // validate_number_of_quotient(pd);
        // static int counter = 0;
        // counter++;
        // std::cout << std::string(80, '-') << std::endl;
        // std::cout << "Before:  \t";
        // v_pd256_plus::p_format_header(pd);
        const uint64_t hMetaBits = get_header_meta_bits(pd);
        const uint64_t old_header = get_clean_header(pd);

        // uint64_t nh0 = old_header;
        // std::cout << "op0:     \t";
        // v_pd256_plus::p_format_header(nh0);
        // uint64_t nh1 = nh0 & MSK(begin);
        uint64_t new_header = old_header & MSK(begin);

        // std::cout << "op1:     \t";
        // v_pd256_plus::p_format_header(nh1);
        // uint64_t temp = (old_header >> end) << (end + 1);

        // std::cout << "temp:     \t";
        // v_pd256_plus::p_format_header(temp);

        // uint64_t nh2 = nh1 | temp;
        // std::cout << "op2:     \t";
        // v_pd256_plus::p_format_header(nh2);

        new_header |= ((old_header >> end) << (end + 1));
        new_header <<= 6;

        // uint64_t nh3 = nh2 << 6;
        // std::cout << "op3:     \t";
        // v_pd256_plus::p_format_header(nh3);

        uint64_t new_quot = MAX(hMetaBits & 31, quot);
        uint64_t new_mBits = new_quot + (hMetaBits & 32);
        assert(BITWISE_DISJOINT(new_mBits, new_header));

        // uint64_t nh4 = nh3 | new_mBits;
        new_header |= new_mBits;

        // std::cout << "op4:     \t";
        // v_pd256_plus::p_format_header(nh4);

        // std::cout << "After:   \t";
        // v_pd256_plus::p_format_header(new_header);

        // std::cout << std::string(80, '-') << std::endl;
        assert(new_header < MSK(56));

        assert(validate_number_of_quotient(new_header));
        memcpy(pd, &new_header, 7);
        assert(validate_number_of_quotient(pd));
    }


    /**
     * @brief adds a single zero after the (quot - 1 )'th one. i.e in begin index.
     * 
     * @param begin 
     * @param end 
     * @param quot 
     * @param pd 
     */
    inline void write_header(uint64_t begin, uint64_t end, int64_t quot, __m256i *pd) {
        return write_header_with_comments(begin, end, quot, pd);
        assert(get_capacity(pd) < CAPACITY25);
        const uint64_t hMetaBits = get_header_meta_bits(pd);
        const uint64_t old_header = get_clean_header(pd);

        uint64_t new_header = old_header & MSK(begin);


        new_header |= ((old_header >> end) << (end + 1));
        new_header <<= 6;


        uint64_t new_quot = MAX(hMetaBits & 31, quot);
        uint64_t new_mBits = new_quot + (hMetaBits & 32);
        assert(BITWISE_DISJOINT(new_mBits, new_header));

        new_header |= new_mBits;
        assert(new_header < MSK(56));

        assert(validate_number_of_quotient(new_header));
        memcpy(pd, &new_header, 7);
        assert(validate_number_of_quotient(pd));
    }

    inline void delete_from_header_without_updating_max_quot(uint64_t begin, uint64_t end, int64_t quot, __m256i *pd) {
        assert(begin < end);
        const uint64_t hMetaBits = get_header_meta_bits(pd);
        const uint64_t old_header = get_clean_header(pd);

        const uint64_t header = get_header(pd);
        const uint64_t mask = MSK(begin + 6);
        uint64_t lo = header & mask;
        uint64_t hi = (header >> 1ul) & ~mask;
        uint64_t new_header = lo | hi;

        assert(BITWISE_DISJOINT(hi, lo));
        assert(new_header < MSK(56));
        auto curr_pop = _mm_popcnt_u64(new_header >> 6ul);
        auto old_pop = _mm_popcnt_u64(old_header);
        assert(curr_pop == old_pop);
        //        assert(validate_number_of_quotient(pd));
        assert(validate_number_of_quotient(new_header));

        memcpy(pd, &new_header, 7);

        assert(validate_number_of_quotient(pd));
    }

    inline void remove_one_zero_from_last_quot(int64_t quot, __m256i *pd) {
        // static int counter = 0;
        // counter++;
        // static int quot20_counter = 0;
        // if (quot == 20){
        //     quot20_counter++;
        // }
        assert(pd_full(pd));
        const uint64_t old_header = get_clean_header(pd);
        const uint64_t header = get_header(pd);

        const uint64_t mask_index = 6 + CAPACITY25 + quot - 1;
        bool zero_in_index = !((1ULL << mask_index) & header);
        if (!zero_in_index) {
            bool zero_in_index2 = !((1ULL << (mask_index + 1)) & header);
            bool zero_in_index3 = !((1ULL << (mask_index + 2)) & header);
            v_pd256_plus::p_format_word(((uint64_t *) pd)[0]);
            assert(0);
        }
        assert(zero_in_index);
        const uint64_t mask = MSK(mask_index);

        uint64_t lo = header & mask;
        uint64_t hi = (header >> 1ul) & ~mask;
        uint64_t new_header = lo | hi;
        auto curr_pop = _mm_popcnt_u64(new_header >> 6ul);
        auto old_pop = _mm_popcnt_u64(old_header);
        assert(curr_pop == old_pop);
        assert(validate_number_of_quotient(new_header));
        memcpy(pd, &new_header, 7);
        assert(validate_number_of_quotient(pd));
    }

    /**
     * @brief 
     * This function is "private" i.e should not be called before saving relevant data. 
     * If the pd is full, som data will be lost.
     * This function updates last quot, but does not updates the overflow bit.
     * 
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void pd_add_25_core(int64_t quot, uint8_t rem, __m256i *pd) {
        constexpr unsigned kBytes2copy = 7;
        const uint64_t header = get_clean_header(pd);

        const uint64_t begin = quot ? (select64(header, quot - 1) + 1) : 0;
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= CAPACITY25 + QUOT_SIZE25);
        // const __m256i target = _mm256_set1_epi8(rem);
        assert(validate_number_of_quotient(pd));
        write_header(begin, end, quot, pd);
        assert(validate_number_of_quotient(pd));

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const uint8_t *) pd)[kBytes2copy + i]) break;
        }
        assert((i == end_fingerprint) ||
               (rem <= ((const uint8_t *) pd)[kBytes2copy + i]));

        assert(validate_number_of_quotient(pd));
        memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
                &((const uint8_t *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        // In previous versions, changed 1 to 2, Because the last byte contained something when the pd is not full
        ((uint8_t *) pd)[kBytes2copy + i] = rem;
        assert(validate_number_of_quotient(pd));
    }

    inline void pd_add_50_not_full_after(int64_t quot, uint8_t rem, __m256i *pd) {
        assert(quot < QUOT_SIZE25);
        assert(!pd_full(pd));
        pd_add_25_core(quot, rem, pd);
    }

    inline void pd_add_50_for_swap(int64_t quot, uint8_t rem, __m256i *pd) {
        assert(quot < QUOT_SIZE25);
        pd_add_25_core(quot, rem, pd);
    }


    inline void pd_add_50_full_after(int64_t quot, uint8_t rem, __m256i *pd) {
        //std::cout << "A_fa" << std::endl;
        assert(quot < 50);
        assert(!pd_full(pd));
        pd_add_50_for_swap(quot, rem, pd);
    }


    inline void pd_add_50_only_rem(uint8_t rem, size_t quot_capacity, __m256i *pd) {
        constexpr unsigned kBytes2copy = 7;

        // const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t begin_fingerprint = CAPACITY25 - quot_capacity;
        const uint64_t end_fingerprint = CAPACITY25;
        assert(begin_fingerprint < end_fingerprint);
        assert(end_fingerprint <= CAPACITY25);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const uint8_t *) pd)[kBytes2copy + i]) break;
        }
        assert((i == end_fingerprint) ||
               (rem <= ((const uint8_t *) pd)[kBytes2copy + i]));

        memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
                &((const uint8_t *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((uint8_t *) pd)[kBytes2copy + i] = rem;
    }

    /**
     * @brief Removes biggest element (by lexicographic order).
     * pd must be full.
     * Does not update the meta bits.
     * Does not delete the last remainder, only decrease quot capacity by 1.
     * @param quot 
     * @param rem 
     * @param pd 
     * @return true the currect quot capacity is positive.
     * @return false the currect quot capacity is 0.
     */
    inline auto remove_biggest_element(int64_t quot, uint8_t rem, __m256i *pd) -> bool {
        // static int counter = 0;
        // counter++;
        // std::cout << std::string(80, '*') << std::endl;
        // std::cout << "counter:       \t\t" << counter << std::endl;
        // if (!pd_find_25(quot, rem, pd)) {
        //     v_pd256_plus::p_format_header(pd);
        //     std::cout << "quot: " << quot << std::endl;
        //     std::cout << "rem: " << ((uint16_t) rem) << std::endl;
        //     (pd_find_25(quot, rem, pd));
        // }
        // assert(pd_find_25(quot, rem, pd));
        // assert(pd_full(pd));
        // constexpr uint64_t maskNonHeaderBits = ~(((1ULL << 25) - 1) << 6);
        // uint64_t full_header = get_header(pd);
        // std::cout << "old_h:         \t\t";
        // v_pd256_plus::p_format_word(full_header);
        // std::cout << "old_ch:        \t\t";
        // v_pd256_plus::p_format_word(header);

        uint64_t last_zero_index = get_last_zero_index(pd);
        uint64_t header = get_clean_header(pd);
        assert(BITWISE_DISJOINT(header, (1ULL << last_zero_index)));
        bool curr_quot_still_pos = !(header & (1ULL << (last_zero_index - 1)));
        const uint64_t mask = MSK(last_zero_index);
        uint64_t lo = header & mask;
        uint64_t hi = ((header >> 1ul) & ~mask);// | (1ULL << last_zero_index);
        assert(BITWISE_DISJOINT(lo, hi));

        // std::cout << "hi:            \t\t";
        // v_pd256_plus::p_format_word(hi);
        // std::cout << "lo:            \t\t";
        // v_pd256_plus::p_format_word(lo);
        // size_t p1 = _mm_popcnt_u64(lo);
        // size_t p2 = _mm_popcnt_u64(hi);
        // uint64_t temp_h = (hi | lo) | (1ULL << last_zero_index);
        // uint64_t new_clean_h = (hi | lo);
        // size_t p3 = _mm_popcnt_u64(temp_h);
        // size_t p4 = _mm_popcnt_u64(new_clean_h);
        // std::cout << std::endl;
        // std::cout << "p1:            \t\t" << p1 << std::endl;
        // std::cout << "p2:            \t\t" << p2 << std::endl;
        // std::cout << "p3:            \t\t" << p3 << std::endl;
        // std::cout << "p4:            \t\t" << p3 << std::endl;
        // std::cout << std::endl;
        // assert(!BITWISE_DISJOINT(temp_h, (1ULL << last_zero_index)));
        // assert(p3 == p4);
        //suppose to fail (because the last bit) . [maybe?]
        // assert(validate_number_of_quotient_from_clean_header(new_clean_h));

        uint64_t new_header = (((hi | lo) << 6) | get_header_meta_bits(pd)) & MSK(56);
        memcpy(pd, &new_header, 7);
        return curr_quot_still_pos;
        // new_header
        // size_t p5 = _mm_popcnt_u64(new_header);
        // assert(p5 == QUOT_SIZE25);
        // new_header = new_header | get_header_meta_bits(pd);
        // new_header &= MSK(55);
        // size_t p6 = _mm_popcnt_u64(new_header);
        // assert(p6 + 1 == QUOT_SIZE25);
        // std::cout << "new_ch:        \t\t";
        // v_pd256_plus::p_format_word(new_clean_h);
        // std::cout << "new_h:         \t\t";
        // v_pd256_plus::p_format_word(new_header);
        // v_pd256_plus::p_format_header(pd);
        // std::cout << std::string(80, '*') << std::endl;
    }

    inline uint64_t pd_swap_short(int64_t quot, uint8_t rem, __m256i *pd) {
        // static int counter = 0;
        // static int c1 = 0;
        // static int c2 = 0;
        // static int c3 = 0;
        // counter++;
        set_overflow_bit(pd);
        // uint64_t valid_after_swap_quot = get_last_quot_after_future_swap(quot, pd);
        // uint64_t before_header_last_byte = get_header_last_byte(pd);

        const uint64_t last_quot = decode_last_quot(pd);
        // const uint64_t before_last_quot = decode_last_quot(pd);
        assert(validate_decoding(pd));
        if (last_quot < quot) {
            // c1++;
            return (quot << 8u) | ((uint64_t) rem);
        } else if (last_quot == quot) {
            // c2++;
            const uint64_t old_rem = get_last_byte(pd);
            if (old_rem < rem)//todo can be <= instead of <.
                return (quot << 8u) | ((uint64_t) rem);

            size_t quot_capacity = get_last_quot_capacity(quot, pd);
            assert(get_specific_quot_capacity_wrapper(quot, pd));
            pd_add_50_only_rem(rem, quot_capacity, pd);
            return (last_quot << 8) | old_rem;
        } else {
            // c3++;
            const uint64_t old_rem = get_last_byte(pd);
            uint64_t valid_after_swap_quot = get_last_quot_after_future_swap(quot, pd);
            // auto lq_cap = get_specific_quot_capacity_naive2(last_quot, pd);
            assert(get_specific_quot_capacity_naive2(last_quot, pd));
            // assert(get_last_quot_capacity(pd));
            bool res = remove_biggest_element(last_quot, old_rem, pd);
            // uint64_t new_last_quot = last_quot;
            // if (res)
            //     assert(get_specific_quot_capacity_naive2(last_quot, pd));
            // if (!res) {
            //     new_last_quot = compute_last_quot_for_full_pd(pd);
            //     new_last_quot = MAX(new_last_quot, quot);
            // }

            // if (new_last_quot != valid_after_swap_quot) {
            //     v_pd256_plus::p_format_header(pd);
            //     std::cout << "res:            \t" << res << std::endl;
            //     std::cout << "new_last_quot:  \t" << new_last_quot << std::endl;
            //     std::cout << "v_after_swap:   \t" << valid_after_swap_quot << std::endl;
            //     std::cout << "quot:           \t" << quot << std::endl;
            // }
            // assert(new_last_quot == valid_after_swap_quot);
            pd_add_50_for_swap(quot, rem, pd);

            encode_last_quot_new(valid_after_swap_quot, pd);
            assert(decode_last_quot(pd) == valid_after_swap_quot);
            return (last_quot << 8) | old_rem;
        }
    }

    inline uint64_t pd_conditional_add_50(int64_t quot, uint8_t rem, __m256i *pd) {
        // static int counter = 0;
        // static int c1 = 0;
        // static int c2 = 0;
        // static int c3 = 0;
        // counter++;
        assert(validate_decoding(pd));
        const uint64_t header = get_header(pd);
        // const uint64_t header_last_byte = _mm_extract_epi8(_mm256_castsi256_si128(*pd), 12);
        // const uint64_t before_add_last_quot = decode_last_quot(pd);
        // const uint64_t before_add_capacity = get_capacity(pd);
        uint16_t last_h_two_bits = (header >> 54) & 3;
        if (last_h_two_bits == 0) {
            // c1++;
            assert(get_capacity(pd) < CAPACITY25 - 1);
            assert(validate_decoding(pd));
            assert(validate_number_of_quotient(pd));
            pd_add_50_not_full_after(quot, rem, pd);
            assert(validate_number_of_quotient(pd));
            update_max_quot_not_overflowing(quot, pd);
            assert(validate_number_of_quotient(pd));
            assert(validate_decoding(pd));
            // auto temp = pd_find_25(quot, rem, pd);
            // if (!temp) {
            //     std::cout << std::string(80, '*') << std::endl;
            //     v_pd256_plus::p_format_header(pd);
            //     std::cout << std::string(80, '*') << std::endl;
            //     pd_find_25(quot, rem, pd);
            //     pd_find_25(quot, rem, pd);
            //     pd_find_25(quot, rem, pd);
            //     assert(0);
            // }
            assert(pd_find_25(quot, rem, pd));
            assert(validate_decoding(pd));
            return (1 << 15);
        } else if (last_h_two_bits == 1) {
            // c2++;
            // std::cout << "ca_2" << std::endl;
            //std::cout << "before adding" << std::endl;
            // v_pd256_plus::print_h1(0, pd);
            // v_pd256_plus::print_headers_masked(pd);
            // //std::cout << std::endl;
            // const uint64_t last_quot_capacity = get_specific_quot_capacity_wrapper(last_quot, pd);
            const uint64_t last_quot = decode_last_quot(pd);
            assert(get_specific_quot_capacity_wrapper(last_quot, pd));
            pd_add_50_full_after(quot, rem, pd);
            // assert(res == (1 << 15));
            assert(validate_number_of_quotient(pd));
            update_max_quot_not_overflowing(last_quot, quot, pd);
            assert(validate_number_of_quotient(pd));
            // const uint64_t new_last_quot = decode_last_quot(pd);
            assert(get_specific_quot_capacity_wrapper(decode_last_quot(pd), pd));
            assert((decode_last_quot(pd) == quot) || (decode_last_quot(pd) == last_quot));
            assert(pd_find_25(quot, rem, pd));
            assert(get_capacity(pd) >= 0);
            assert(get_capacity(pd) == CAPACITY25);
            assert(validate_decoding(pd));
            return (1 << 15);
            // const uint64_t new_header_last_byte = get_header_last_byte(pd);

            //std::cout << "after adding" << std::endl;
            // v_pd256_plus::print_h1(0, pd);
            //std::cout << std::string(84, '-') << std::endl;

            // auto temp = pd_find_50(quot, rem, pd);
            // if (pd_find_50(quot, rem, pd) != Yes) {
            //     pd_find_50(quot, rem, pd);
            //     assert(0);
            // }
        } else {
            // c3++;
            // std::cout << "ca_3" << std::endl;
            assert(pd_full(pd));
            assert(validate_decoding(pd));
            assert(validate_number_of_quotient(pd));
            auto res = pd_swap_short(quot, rem, pd);
            assert(validate_number_of_quotient(pd));
            assert(validate_decoding(pd));
            return res;
        }
    }


    inline bool delete_from_non_overflowing_pd_core(int64_t quot, uint8_t rem, __m256i *pd) {
        assert(quot < QUOT_SIZE25);
        constexpr unsigned kBytes2copy = 7;
        const uint64_t header = get_clean_header(pd);

        const uint64_t begin = quot ? (select64(header, quot - 1) + 1) : 0;
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= CAPACITY25 + QUOT_SIZE25);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= CAPACITY25);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const uint8_t *) pd)[kBytes2copy + i])
                break;
        }
        if ((i == end_fingerprint) || (rem != ((const uint8_t *) pd)[kBytes2copy + i])) {
            if (pd_find_25(quot, rem, pd)) assert(false);
            return false;
        }

        assert(validate_number_of_quotient(pd));
        delete_from_header_without_updating_max_quot(begin, end, quot, pd);
        assert(validate_number_of_quotient(pd));

        memmove(&((uint8_t *) pd)[kBytes2copy + i],
                &((const uint8_t *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        return true;
    }

    inline bool delete_from_non_overflowing_pd(int64_t quot, uint8_t rem, __m256i *pd) {
        assert(!did_pd_overflowed(pd));
        const uint64_t last_quot = decode_last_quot(pd);
        if (last_quot > quot) {
            return delete_from_non_overflowing_pd_core(quot, rem, pd);
        } else if (last_quot == quot) {
            const uint64_t valid_after_swap_quot = compute_next_last_quot_att(pd);
            bool res = delete_from_non_overflowing_pd_core(quot, rem, pd);
            if (res && (valid_after_swap_quot != quot))
                encode_last_quot_new(valid_after_swap_quot, pd);
            return res;
        }
        assert(0);
        return -42;
    }

    inline bool delete_from_overflowing_pd_quot_equal_to_last_helper(int64_t quot, uint8_t rem, __m256i *pd) {
        // static int counter = 0;
        // counter++;
        uint8_t last_rem = get_last_byte(pd);
        if (last_rem < rem) {
            // std::cout << "Del OF 2.1" << std::endl;
            return false;
        } else if (last_rem == rem) {
            auto res = remove_biggest_element(quot, rem, pd);
            return true;
        }

        const __m256i target = _mm256_set1_epi8(rem);
        const uint32_t v = (_mm256_cmpeq_epu8_mask(target, *pd));
        if (!v) {
            // std::cout << "Del OF 2.2" << std::endl;
            return false;
        }

        /*uint64_t header = get_clean_header(pd) >> 14ul;
            size_t index = _lzcnt_u64(header);
            assert(index < 64);
            uint64_t header2 = ~(header << (index + 1));
            size_t index2 = _lzcnt_u64(header2);// counting empty ones + 1 from the end.
            size_t index2_att = QUOT_SIZE25 - last_quot;
            assert(index2 == index2_att);
            uint64_t header3 = (header2 << (index2 + 1));
            size_t index3 = _lzcnt_u64(header3);//connting second sequence of ones.
            auto res = QUOT_SIZE25 - 1 - index2 - index3;
*/

        uint64_t aligned_header = get_clean_header(pd) << (14ul + QUOT_SIZE25 - quot);
        bool fast_find_res = _lzcnt_u32(v) < _lzcnt_u64(aligned_header);
        assert(fast_find_res == pd_find_25(quot, rem, pd));
        if (!fast_find_res) {
            // std::cout << "Del OF 2.4" << std::endl;
            return false;
        }

        // std::cout << "Del OF 2.5" << std::endl;
        // std::cout << "quot: " << quot << std::endl;
        // v_pd256_plus::p_format_word(((uint64_t *)pd)[0]);
        remove_one_zero_from_last_quot(quot, pd);
        // v_pd256_plus::p_format_word(((uint64_t *)pd)[0]);
        const size_t index = 31 - _lzcnt_u32(v) - 7;
        assert(read_rem_by_index(index, pd) == rem);
        memmove(&((uint8_t *) pd)[7U + index],
                &((const uint8_t *) pd)[7U + index + 1],
                sizeof(*pd) - (7U + index + 1));
        return true;
        // std::cout << "Del OF 2.3" << std::endl;
        // return fast_find_res;
    }

    inline bool delete_from_overflowing_pd(int64_t quot, uint8_t rem, __m256i *pd) {
        //todo: remove this call
        // bool find_res = pd_find_25(quot, rem, pd);
        const uint64_t last_quot = decode_last_quot(pd);
        if (quot < last_quot) {
            //todo: remove validations

            // std::cout << "Del OF 1" << std::endl;
            auto res = delete_from_non_overflowing_pd_core(quot, rem, pd);
            // if (find_res && !res) {
            //     auto res2 = delete_from_non_overflowing_pd_core(quot, rem, pd);
            //     assert(0);
            // }
            // if (!res) {
            //     assert(!find_res);
            //     //                assert(!pd_find_25(quot, rem, pd));
            // }
            assert(res || (!pd_find_25(quot, rem, pd)));
            return res;
        } else if (last_quot == quot) {
            return delete_from_overflowing_pd_quot_equal_to_last_helper(quot, rem, pd);
        }
        assert(0);
        return 0;
    }

    inline bool delete_element_wrapper(int64_t quot, uint8_t rem, __m256i *pd) {
        // bool find_res = pd_find_25(quot, rem, pd);
        const uint64_t last_quot = decode_last_quot(pd);
        if (quot > last_quot) {
            // std::cout << "Del wrap 1" << std::endl;
            // assert(!find_res);
            return false;
        } else if (!did_pd_overflowed(pd)) {
            // std::cout << "Del wrap 2" << std::endl;
            auto res = delete_from_non_overflowing_pd(quot, rem, pd);
            // if (!res) assert(!find_res);
            return res;
        } else {
            // std::cout << "Del wrap 3" << std::endl;
            auto res = delete_from_overflowing_pd(quot, rem, pd);
            // if (!res) assert(!find_res);
            return res;
        }
    }

    /**
     * @brief Adding an element that was popped right after a deletion of an element from the pd.
     * Todo: This function can be much more efficient because of the following assumption: removed_quot <= quot.
     * 
     * Assuming that:
     * 1) removed_quot >= last_quot
     * 2) During the pop deletion the header was not updated.
     * 
     * Using 'quot' and 'removed_quot' one could easily update the header by: 
     * 1) fliping exactly two bits. (removed_quot < quot). 
     * 2) Doing nothing (removed_quot == quot). 
     * 
     * 
     * @param quot 
     * @param rem 
     * @param removed_quot 
     * @param removed_rem 
     * @param pd 
     */
    inline void pd_pop_add(int64_t quot, uint8_t rem, int64_t removed_quot, uint8_t removed_rem, __m256i *pd) {
        assert(removed_quot <= quot);
        pd_add_25_core(quot, rem, pd);
        if (removed_quot < quot) {
            encode_last_quot_new(quot, pd);
        }
        assert(pd_find_25(quot, rem, pd));
    }

    /**
     * @brief This function is called after a deletion causing a pd to become not full. 
     * (and therefore to not be marked as overflowing pd as well.)  
     * 
     * @param pd 
     */
    inline void update_quot_after_pop(__m256i *pd) {
        uint64_t last_quot = compute_curr_last_quot_att(pd);
        encode_last_quot_new(last_quot, pd);
    }


    inline auto read_last_rem(const __m256i *pd) -> uint8_t {
        auto capacity = get_capacity(pd);
        return read_rem_by_index(capacity - 1, pd);
    }

    void print512(const __m256i *var);

    auto get_name() -> std::string;


    inline pd_Status pd256_plus_count_core(int64_t quot, uint8_t rem, const __m256i *pd, int ind = 0) {
        static size_t total_counter = 0;
        static size_t not_v = 0;
        static size_t yes_v = 0;
        static size_t q_cond0[2] = {0};// {must lookups, redundent lookups}.
        static size_t q_cond1[2] = {0};
        static size_t q_cond2[2] = {0};
        static size_t qq_cond0[2] = {0};// {must lookups, redundent lookups}.
        static size_t qq_cond1[2] = {0};
        static size_t qq_cond2[2] = {0};
        static size_t blsr_arr[51] = {0};
        static size_t new_blsr_arr[51] = {0};
        static size_t two_power[2] = {0};
        static size_t part3 = 0;
        static size_t new_v_redundant = 0;
        static size_t new_v_mask_counter = 0;
        static size_t new_v_changed = 0;
        // memcpy()

        if (ind == 1) {
            total_counter = 0;
            not_v = 0;
            yes_v = 0;
            q_cond0[0] = q_cond0[1] = 0;
            q_cond1[0] = q_cond1[1] = 0;
            q_cond2[0] = q_cond2[1] = 0;
            qq_cond0[0] = qq_cond0[1] = 0;
            qq_cond1[0] = qq_cond1[1] = 0;
            qq_cond2[0] = qq_cond2[1] = 0;
            new_blsr_arr[0] = new_blsr_arr[1] = 0;
            std::fill(blsr_arr, blsr_arr + 51, 0);
            std::fill(new_blsr_arr, new_blsr_arr + 51, 0);
            two_power[0] = two_power[1] = 0;
            part3 = 0;
            new_v_redundant = 0;
            new_v_mask_counter = 0;
            new_v_changed = 0;
        }
        if (ind == 2) {
            auto line = std::string(84, '-');
            std::cout << line << std::endl;
            std::cout << "total:      \t" << total_counter << std::endl;
            std::cout << "not_v:      \t" << not_v << std::endl;
            // std::cout << "yes_v:     \t" << yes_v << std::endl;
            std::cout << "q_cond0:   \t(" << q_cond0[0] << ", " << q_cond0[1] << ")" << std::endl;
            std::cout << "q_cond1    \t(" << q_cond1[0] << ", " << q_cond1[1] << ")" << std::endl;
            std::cout << "q_cond2    \t(" << q_cond2[0] << ", " << q_cond2[1] << ")" << std::endl;
            std::cout << "qq_cond0:  \t(" << qq_cond0[0] << ", " << qq_cond0[1] << ")" << std::endl;
            std::cout << "qq_cond1   \t(" << qq_cond1[0] << ", " << qq_cond1[1] << ")" << std::endl;
            std::cout << "qq_cond2   \t(" << qq_cond2[0] << ", " << qq_cond2[1] << ")" << std::endl;

            std::cout << "newBlsr:   \t(" << new_blsr_arr[0] << ", " << new_blsr_arr[1] << ")" << std::endl;
            // std::cout << "two_power: \t(" << two_power[0] << ", " << two_power[1] << ")" << std::endl;
            std::cout << "newVredund: \t" << new_v_redundant << std::endl;
            std::cout << "new_v_mask: \t" << new_v_mask_counter << std::endl;
            std::cout << "newVchange: \t" << new_v_changed << std::endl;
            std::cout << "part3:      \t" << part3 << std::endl;
            std::cout << "blsr_arr:  \t("
                      << blsr_arr[0] << ", "
                      << blsr_arr[1] << ", "
                      << blsr_arr[2] << ", "
                      << blsr_arr[3] << ", "
                      << blsr_arr[4] << ", "
                      << blsr_arr[5] << ", "
                      << blsr_arr[6] << ")"
                      << std::endl;
            std::cout << "nBlsrArr:  \t("
                      << new_blsr_arr[0] << ", "
                      << new_blsr_arr[1] << ", "
                      << new_blsr_arr[2] << ", "
                      << new_blsr_arr[3] << ", "
                      << new_blsr_arr[4] << ", "
                      << new_blsr_arr[5] << ", "
                      << new_blsr_arr[6] << ")"
                      << std::endl;
            std::cout << line << std::endl;
        }
        total_counter++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < 50);
        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 7ul;

        q_cond0[cmp_qr_naive(quot, rem, pd)]++;
        qq_cond0[decode_last_quot(pd) < quot]++;

        size_t r_counter = 0;
        if (!v) {
            // std::cout << "r0" << std::endl;
            not_v++;
            return get_status_after_l1_failed(quot, rem, pd);
            // return (pd_full(pd) && cmp_qr_naive(quot, rem, pd)) ? look_in_the_next_level : No;
        }

        q_cond1[cmp_qr_naive(quot, rem, pd)]++;
        qq_cond1[decode_last_quot(pd) < quot]++;

        yes_v++;
        auto blsr_popcount = _mm_popcnt_u64(v);
        blsr_arr[blsr_popcount]++;

        const int64_t h0 = get_clean_header(pd);
        uint64_t new_v = (v << quot) & ~h0;
        auto new_blsr_popcount = _mm_popcnt_u64(new_v);
        new_blsr_arr[new_blsr_popcount]++;

        if ((v << quot) != new_v) new_v_changed++;
        if (!new_v) new_v_mask_counter++;

        const uint64_t v_off = _blsr_u64(new_v);
        if (v_off == 0) {
            if (_blsr_u64(v) == 0) new_v_redundant++;
            // std::cout << "r1" << std::endl;
            // new_blsr_arr[1]++;
            auto temp = (_mm_popcnt_u64(h0 & (new_v - 1)) == quot);
            assert(temp == pd_find_25_ver0(quot, rem, pd));
            auto temp2 = get_status_after_l1_failed(quot, rem, pd);
            return (_mm_popcnt_u64(h0 & (new_v - 1)) == quot) ? Yes : get_status_after_l1_failed(quot, rem, pd);
        } else if (_blsr_u64(v_off) == 0) {
            if (_blsr_u64(_blsr_u64(v)) == 0) new_v_redundant++;
            // std::cout << "r2" << std::endl;
            // new_blsr_arr[2]++;
            const bool res =
                    (_mm_popcnt_u64(h0 & (v_off - 1)) == quot) || (_mm_popcnt_u64(h0 & ((v_off ^ new_v) - 1)) == quot);
            assert(res == pd_find_25_ver0(quot, rem, pd));
            return res ? Yes : get_status_after_l1_failed(quot, rem, pd);
        }
        new_v_redundant++;
        // const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        // const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        // if (_blsr_u64(v) == 0) {
        //     const uint64_t mask = _lrotl(v, quot);
        //     if (v << quot) {
        //         two_power[0]++;
        //         // const uint64_t mask = v << quot;
        //         return ((!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot)) ? Yes : cmp_qr_return_pd_status(quot, rem, pd);
        //     } else {
        //         two_power[1]++;
        //         // const uint64_t mask = v >> (64 - quot);
        //         return ((!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot)) ? Yes : cmp_qr_return_pd_status(quot, rem, pd);
        //     }
        // }
        q_cond2[cmp_qr_naive(quot, rem, pd)]++;
        qq_cond2[decode_last_quot(pd) < quot]++;

        part3++;

        if (quot) {
            // std::cout << "r3" << std::endl;
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            const uint64_t h_cleared_quot_plus_one_set_bits = _blsr_u64(h_cleared_quot_set_bits);
            const uint64_t v_mask = ((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(h_cleared_quot_plus_one_set_bits)) & (~h0));
            // bool att = v_mask & new_v;
            return (v_mask & new_v) ? Yes : get_status_after_l1_failed(quot, rem, pd);
        } else {
            // std::cout << "r4" << std::endl;
            return (v & (_blsmsk_u64(h0) >> 1ul)) ? Yes : get_status_after_l1_failed(quot, rem, pd);
        }
    }

    inline pd_Status pd256_plus_count(int64_t quot, uint8_t rem, const __m256i *pd, int ind = 0) {
        pd_Status res = pd256_plus_count_core(quot, rem, pd, ind);
        pd_Status v_res0 = pd_find0(quot, rem, pd);
        assert(v_res0 == res);
        // // pd_Status v_res1 = pd_find1(quot, rem, pd);
        // if (res != v_res0) {
        //     std::cout << "res: " << res << std::endl;
        //     std::cout << "v_res0: " << v_res0 << std::endl;
        //     std::cout << "v_res1: " << v_res1 << std::endl;
        //     pd256_plus_count_core(quot, rem, pd);
        //     assert(0);
        // }
        return res;
    }

}// namespace pd256_plus


#endif// FILTERS_PD256_PLUS_HPP
