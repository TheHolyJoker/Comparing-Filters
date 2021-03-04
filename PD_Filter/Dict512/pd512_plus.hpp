/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_PD512_PLUS_HPP
#define FILTERS_PD512_PLUS_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <immintrin.h>
#include <x86intrin.h>

#define QUOT_SIZE (50)
#define MAX(x, y) (((x) < (y)) ? y : x)
#define MIN(x, y) (((x) > (y)) ? y : x)

static constexpr uint8_t __attribute__((aligned(64))) Lookup_Table[128] = {
        50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
        49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50,
        45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46,
        47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48,
        44, 44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 39, 38, 38, 37, 37,
        36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
        31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

// #define DECODE(pd) (Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)])

namespace v_pd512_plus {

    auto to_bin(uint64_t x) -> std::string;

    auto to_bin_reversed(uint64_t x) -> std::string;

    auto to_bin_reversed(const unsigned __int128 header) -> std::string;

    auto bin_print_header_spaced(uint64_t header) -> std::string;

    auto bin_print_header_spaced2(uint64_t header) -> std::string;

    void print_headers(const __m512i *pd);

    void print_headers_extended(const __m512i *pd);

    inline void print_headers_masked(const __m512i *pd) {
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;
        std::cout << "h0: " << bin_print_header_spaced2(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced2(h1) << std::endl;
    }

    inline void print_h1(bool mask, const __m512i *pd) {
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        // const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (mask)
            h1 &= h1_mask;
        // std::cout << "h0: " << bin_print_header_spaced2(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced2(h1) << std::endl;
    }

    void decode_by_table_validator();

    auto test_bit(size_t index, const __m512i *pd) -> bool;

    void print_hlb(const __m512i *pd);
}// namespace v_pd512_plus

namespace pd512_plus {


    auto count_ones_up_to_the_kth_zero(const __m512i *x, size_t k = 51) -> size_t;

    auto count_zeros_up_to_the_kth_one(const __m512i *x, size_t k) -> size_t;

    auto validate_clz(int64_t quot, char rem, const __m512i *pd) -> bool;

    auto validate_clz_helper(int64_t quot, char rem, const __m512i *pd) -> bool;

    auto validate_number_of_quotient(const __m512i *pd) -> bool;
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

    inline bool pd_full(const __m512i *pd) {
        return _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 240;
        // return _mm_extract_epi16(_mm512_castsi512_si128(*pd), 6) & 240;
        // 16 + 32 + 64 + 128 == 240
    }
    
    inline uint8_t get_last_byte(const __m512i *pd) {
        uint8_t x;
        memcpy(&x, &((uint8_t *) pd)[63], 1);
        return x;

        //Old function:
        // return _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, 3), 15);
        
        
        
        // constexpr int imm1 = 3;
        // constexpr int imm2 = 15;
        // return _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);

        // uint8_t att8 = _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);
        // assert(att == att8);
        // uint64_t *h_array = ((uint64_t *) pd);
        // uint64_t res = h_array[7] >> (64 - 8);
        // assert(res == att);
        // return _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);
    }

    inline uint8_t get_header_last_byte(const __m512i *pd) {
        uint8_t x;
        memcpy(&x, &((uint8_t *) pd)[12], 1);
        return x;
        //Old function:
        // return _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
    }

    inline uint64_t decode_by_table2(const __m512i *pd) {
        // uint8_t x;
        // memcpy(&x, &((uint8_t *) pd)[12], 1);
        // return Lookup_Table[x & 127];

        //Old function:
        return Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
    }
    inline uint8_t get_hi_meta_bits(const __m512i *pd) {
        return _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 224;
    }

    inline unsigned __int128 get_header128(const __m512i *pd) {
        return ((const unsigned __int128 *) pd)[0];
    }
    auto get_capacity_naive_with_OF_bit(const __m512i *x) -> size_t;

    inline auto get_capacity_easy_case(const __m512i *pd) -> int {
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;
        const size_t total_pop = _mm_popcnt_u64(h0) + _mm_popcnt_u64(h1);
        assert(total_pop == QUOT_SIZE);
        const int att = (h1 == 0) ? 14ul - _lzcnt_u64(h0) : (128 - QUOT_SIZE - _lzcnt_u64(h1));
        // assert(att == get_capacity_naive_with_OF_bit(pd));
        return (h1 == 0) ? 14ul - _lzcnt_u64(h0) : (128 - QUOT_SIZE - _lzcnt_u64(h1));
    }

    // switch (header_meta_bits & 224) {
    //             case 0:
    //                 return get_capacity_easy_case(pd);
    //                 break;
    //             case (32):
    //                 break;
    //             case (64):
    //                 return get_capacity_easy_case(pd);
    //                 break;
    //             case (128):
    //                 break;
    //             case (160):
    //                 break;
    //             case (192):
    //                 break;
    //             case (224):
    //                 break;
    //             default:
    //                 assert(false);
    //                 break;
    auto get_capacity(const __m512i *pd) -> int;

    auto get_specific_quot_capacity(int64_t quot, const __m512i *pd) -> int;

    inline auto get_last_zero_index_naive(const __m512i *pd) -> uint8_t {
        assert(pd_full(pd));
        auto temp = count_ones_up_to_the_kth_zero(pd);
        return temp + 51 - 1;
    }

    inline auto get_last_zero_index(const __m512i *pd) -> uint8_t {
        //Q:How many ones are there before the 51th zero? A:last_quot
        //Therefore, the index of the 51th zero is last_quot + 51 (-1 because we are starting to reps from zero).
        assert(pd_full(pd));
        auto res = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)] + 50;// starting to reps from 0.
        // uint64_t res2 = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];// starting to reps from 0.
        // uint64_t v_res = get_last_zero_index_naive(pd);
        // assert(res2 == v_res);
        assert(res == get_last_zero_index_naive(pd));
        return Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)] + 50;

        // assert(Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)]);
        // assert(res == count_ones_up_to_the_kth_zero(pd));
    }

    inline auto get_last_quot_capacity(int64_t last_quot, const __m512i *pd) -> uint8_t {
        assert(pd_full(pd));
        assert(last_quot < 50);
        const uint64_t last_zero_index = last_quot + 50;
        const uint64_t rel_index = last_zero_index & 63;
        assert(last_zero_index > 64);

        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        uint64_t h1_masked = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & ((1ULL << (rel_index)) - 1);
        const auto first_one_index = _lzcnt_u64(h1_masked);
        const auto first_one_index_lsb = 63 - _lzcnt_u64(h1_masked);
        assert(first_one_index_lsb < rel_index);
        auto res = rel_index - first_one_index_lsb;
#ifndef NDEBUG
        auto v_res = get_specific_quot_capacity(last_quot, pd);
        if (v_res != res) {
            v_pd512_plus::print_headers(pd);
            v_pd512_plus::print_hlb(pd);
            assert(false);
        }
#endif// DEBUG

        assert(res == get_specific_quot_capacity(last_quot, pd));
        return res;
        return rel_index - first_one_index;
    }

    inline auto get_last_quot_capacity(const __m512i *pd) -> uint8_t {
        // assert(pd_full(pd));
        const uint8_t lhb = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        const int64_t last_quot = (lhb & (255 - 31)) ? Lookup_Table[lhb] : get_last_byte(pd);
        return get_last_quot_capacity(last_quot, pd);
    }

    inline bool pd_empty(const __m512i *pd) {
        constexpr uint64_t empty_h0 = (1ULL << 50) - 1;
        return _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0) == empty_h0;
    }

    inline uint8_t get_last_quot_for_full_pd_without_deletions(const __m512i *pd) {
        return count_ones_up_to_the_kth_zero(pd);
    }

    // inline uint64_t decode_by_table(const __m512i *pd) {
    //     const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
    //     assert(header_last_byte & (32 + 64 + 128));
    // }

    inline uint64_t decode_last_quot_by_hlb(uint64_t header_last_byte) {
        switch (header_last_byte & (32 + 64)) {
            case 0:
                if (header_last_byte & 16) {
                    assert(!(header_last_byte & 8));
                    return 49;
                }
                assert(false);
                return 4242;
            case 32:
                return (QUOT_SIZE - 1) - (4 - ((header_last_byte >> 3) & 3));
            case 64:
                return (QUOT_SIZE - 1) - ((header_last_byte >> 1) & 15) - 5;
            case 96:
                return (QUOT_SIZE - 1) - ((header_last_byte & 31) + 18);
            default:
                assert(false);
        }
        assert(false);
        return 4242;
    }

    inline uint64_t decode_last_quot(const __m512i *pd) {
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        // auto temp = (4 - ((header_last_byte >> 3) & 3));
        // const uint64_t hi_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & (32 + 64);
        // const uint64_t hi_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 224;
        switch (header_last_byte & (32 + 64)) {
            case 0:
                // std::cout << "d0" << std::endl;
                return (header_last_byte & 16) ? 49 : get_last_byte(pd);
                // break;
            case 32:
                // auto reduce_from_max_quot = 4 - ((header_last_byte >> 3) & 3);
                // std::cout << "d1" << std::endl;
                return (QUOT_SIZE - 1) - (4 - ((header_last_byte >> 3) & 3));
                // break;
            case 64:
                // auto reduce_from_max_quot = ((header_last_byte >> 1) & 15) + 5;
                // std::cout << "d2" << std::endl;
                return (QUOT_SIZE - 1) - ((header_last_byte >> 1) & 15) - 5;
                // break;
            case 96:
                // auto reduce_from_max_quot = ((header_last_byte  & 31) + 18);
                // std::cout << "d3" << std::endl;
                return (QUOT_SIZE - 1) - ((header_last_byte & 31) + 18);
                // break;
            default:
                assert(false);
                return 4242;
        }
        assert(false);

        // if (hi_meta_bits == 0)
        //     return get_last_quot_in_pd_naive_easy_case(pd);
        // else if (hi_meta_bits & 32) {
        //     const uint64_t att = QUOT_SIZE - (header_meta_bits >> 6);
        //     return att;
        // } else if (hi_meta_bits == 64) {
        //     const uint64_t reduce_from_max_quot = ((header_meta_bits >> 1) & 15) + 4;
        //     assert(reduce_from_max_quot >= 4);
        //     assert(reduce_from_max_quot <= 19);
        //     return QUOT_SIZE - (((header_meta_bits >> 1) & 15) + 4);
        // } else if (hi_meta_bits == 128) {
        //     const uint64_t reduce_from_max_quot = (header_meta_bits & 31) + 18;
        //     assert(reduce_from_max_quot >= 18);
        //     assert(reduce_from_max_quot <= 49);
        //     return QUOT_SIZE - ((header_meta_bits & 31) + 18);
        // } else {
        //     assert(false);
        // }
        // assert(false);
        // return 4242;
        // // switch (hi_meta_bits)
        // // {
        // // case 0:
        // //     return get_last_quot_in_pd_naive_easy_case(pd);
        // // case 64:
        // //     return QUOT_SIZE - (((header_meta_bits >> 1) & 15) + 4);
        // // case 128:
        // //     return QUOT_SIZE - ((header_meta_bits & 31) + 18);
        // // default:
        // //     return QUOT_SIZE - (header_meta_bits >> 6);
        // // }
    }

    inline uint64_t decode_last_quot_safe(const __m512i *pd) {
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);

        if ((header_last_byte & (32 + 64)) == 0) {
            auto temp = validate_number_of_quotient(pd);
            if (!temp) {
                auto header128 = get_header128(pd);
                constexpr uint64_t MASK64 = 18446744073709551615ULL;
                uint64_t hi = header128 >> 64;
                uint64_t lo = header128 & MASK64;
                std::cout << "hi: " << hi << std::endl;
                std::cout << "lo: " << lo << std::endl;
                v_pd512_plus::print_headers(pd);
                assert(false);
            }
            assert(validate_number_of_quotient(pd));
            auto capacity = count_zeros_up_to_the_kth_one(pd, QUOT_SIZE);
            if (capacity == 0)
                return 0;
            return count_ones_up_to_the_kth_zero(pd, capacity);
        }

        auto last_quot_by_count = count_ones_up_to_the_kth_zero(pd, 51);
        assert(last_quot_by_count <= 50);
        auto last_zero_index = (last_quot_by_count + 51) - 1;
        if (last_zero_index > 100) {
            v_pd512_plus::print_headers(pd);
            v_pd512_plus::print_headers_masked(pd);
        }
        assert(last_zero_index <= 100);

        auto meta_data = (header_last_byte >> 5) & 3;
        if (meta_data == 1) {
            assert(last_zero_index <= 98);
        } else if (meta_data == 2) {
            assert(last_zero_index <= 95);
        } else if (meta_data == 3) {
            assert(last_zero_index <= 94);
        } else {
            v_pd512_plus::print_headers(pd);
            v_pd512_plus::print_headers_masked(pd);
            assert(0);
        }

        return last_quot_by_count;
    }


    inline uint64_t decode_last_quot_safe_old(const __m512i *pd) {
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);

        //only for testing a single PD
        assert(!(header_last_byte & 128));

        if ((header_last_byte & (32 + 64 + 128)) == 0) {
            auto temp = validate_number_of_quotient(pd);
            if (!temp) {
                auto header128 = get_header128(pd);
                constexpr uint64_t MASK64 = 18446744073709551615ULL;
                uint64_t hi = header128 >> 64;
                uint64_t lo = header128 & MASK64;
                std::cout << "hi: " << hi << std::endl;
                std::cout << "lo: " << lo << std::endl;
                v_pd512_plus::print_headers(pd);
                assert(false);
            }
            assert(validate_number_of_quotient(pd));
            auto capacity = count_zeros_up_to_the_kth_one(pd, QUOT_SIZE);
            if (capacity == 0)
                return 0;
            return count_ones_up_to_the_kth_zero(pd, capacity);
        }

        auto last_quot_by_count = count_ones_up_to_the_kth_zero(pd, 51);
        assert(last_quot_by_count <= 50);
        auto last_zero_index = (last_quot_by_count + 51) - 1;
        if (last_zero_index > 100) {
            v_pd512_plus::print_headers(pd);
            v_pd512_plus::print_headers_masked(pd);
        }
        assert(last_zero_index <= 100);

        auto meta_data = (header_last_byte >> 5) & 3;
        if (meta_data == 1) {
            assert(last_zero_index <= 98);
        } else if (meta_data == 2) {
            assert(last_zero_index <= 95);
        } else if (meta_data == 3) {
            assert(last_zero_index <= 94);
        } else {
            v_pd512_plus::print_headers(pd);
            v_pd512_plus::print_headers_masked(pd);
            assert(0);
        }

        return last_quot_by_count;
    }
    // inline uint64_t decode_last_quot_in_pd_att(const __m512i *pd) {
    //     const uint64_t header_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
    //     const uint64_t hi_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 224;


    inline uint64_t decode_by_table(const __m512i *pd) {
        static constexpr uint8_t table[128] = {
                50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
                49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50,
                45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46,
                47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48,
                44, 44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 39, 38, 38, 37, 37,
                36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
                31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
                15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

        // constexpr uint8_t table[96] = {45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 44, 44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        // uint64_t lhb = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        // auto index = (_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127);
        // uint64_t res = table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
        // uint64_t v_res = decode_last_quot(pd);
        assert(table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)] == decode_last_quot(pd));
        return table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
    }

    inline uint64_t decode_by_table2_vec(const __m512i *pd) {
        return Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
    }


    inline bool validate_encoding_mb1(const __m512i *pd) {
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        const uint64_t all_meta_bits = header_last_byte & (224);
        const uint64_t meta_bits = header_last_byte & (32 + 64);
        assert(meta_bits == 32);

        uint64_t bits_to_read = (header_last_byte >> 3) & 3;
        uint64_t naive_last_quot = 49;
        switch (bits_to_read) {
            case 0:
                naive_last_quot = 45;
                break;
            case 1:
                naive_last_quot = 46;
                break;
            case 2:
                naive_last_quot = 47;
                break;
            case 3:
                naive_last_quot = 48;
                break;

            default:
                assert(false);
                break;
        }
        uint64_t last_quot = 49 - (4 - bits_to_read);
        assert(naive_last_quot == last_quot);
        // if (last_quot == 49) {
        //     uint64_t temp = (header_last_byte >> 3) & 15;
        //     assert(temp == 2);
        // } else
        if (last_quot == 48) {
            uint64_t temp = (header_last_byte >> 2) & 7;
            assert(temp == 6);
        } else if (last_quot == 47) {
            uint64_t temp = (header_last_byte >> 1) & 31;
            assert(temp == 26);
        } else if (last_quot == 46) {
            uint64_t temp = header_last_byte & 127;
            uint64_t temp2 = header_last_byte & 31;
            assert(temp == 46);
            assert(temp2 == 14);
        } else if (last_quot == 45) {
            uint64_t temp = header_last_byte & 31;
            assert(temp == 7);
        } else {
            v_pd512_plus::print_headers(pd);
            assert(false);
        }
        return true;
    }

    inline bool validate_encoding(const __m512i *pd) {
        uint64_t last_quot = decode_last_quot(pd);
        if (last_quot == 0) {
            auto temp = count_zeros_up_to_the_kth_one(pd, 1);
            auto capacity = count_zeros_up_to_the_kth_one(pd, 50);
            assert(capacity == temp);
            if (temp == 0) {
                assert(pd_empty(pd));
            }
            return true;
        }
        assert(last_quot < 50);
        if (!pd_full(pd)) {
            auto temp = count_zeros_up_to_the_kth_one(pd, last_quot + 1);
            auto index = temp + last_quot - 1;
            unsigned __int128 h128 = get_header128(pd);
            unsigned __int128 temp128 = (h128 >> index) & 3;
            if (temp128 != 2) {
                v_pd512_plus::print_headers(pd);
            }
            assert(temp128 == 2);
            return true;
        }

        assert(pd_full(pd));
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        const uint64_t all_meta_bits = header_last_byte & (224);
        const uint64_t meta_bits = header_last_byte & (32 + 64);
        if (all_meta_bits == 0) {
            auto temp = (header_last_byte >> 3);
            assert(temp == (temp & 3));
            if (temp != 2) {
                v_pd512_plus::print_headers(pd);
            }
            assert(temp == 2);
            return true;
        }
        if (meta_bits == 32) {
            assert(validate_encoding_mb1(pd));
            return true;
        }
        // uint64_t naive_last_quot = count_ones_up_to_the_kth_zero(pd, 51);
        // uint64_t decoded_last_quot = decode_last_quot(pd);
        // assert(naive_last_quot == decode_last_quot);
        // assert(decode_last_quot);
        //todo: complete this validation.
        return true;
    }
    //     if ((header_meta_bits & 240) == 0) {
    //         const uint64_t att = get_last_byte(pd);
    //         auto res = get_last_quot_in_pd_naive_easy_case(pd);
    //         assert(att == get_last_quot_in_pd_naive_easy_case(pd));
    //         return get_last_byte(pd);
    //     } else if ((header_meta_bits & 240) == 16) {
    //         const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
    //         const uint64_t temp = _lzcnt_u64(~(h1 << (64 - 36)));
    //         const uint64_t att = QUOT_SIZE - temp;
    //         assert(att == get_last_quot_in_pd_naive_easy_case(pd));
    //         return att;
    //     } else if (hi_meta_bits & 32) {
    //         const uint64_t att = QUOT_SIZE - (header_meta_bits >> 6);
    //         return att;
    //     } else if (hi_meta_bits == 64) {
    //         const uint64_t reduce_from_max_quot = ((header_meta_bits >> 1) & 15) + 4;
    //         assert(reduce_from_max_quot >= 4);
    //         assert(reduce_from_max_quot <= 19);
    //         return QUOT_SIZE - (((header_meta_bits >> 1) & 15) + 4);
    //     } else if (hi_meta_bits == 128) {
    //         const uint64_t reduce_from_max_quot = (header_meta_bits & 31) + 18;
    //         assert(reduce_from_max_quot >= 18);
    //         assert(reduce_from_max_quot <= 49);
    //         return QUOT_SIZE - ((header_meta_bits & 31) + 18);
    //     } else {
    //         assert(false);
    //     }
    //     assert(false);
    //     return 4242;
    // }

    inline uint64_t decode_last_quot_wrapper(const __m512i *pd) {
        assert(decode_by_table2(pd) == decode_by_table(pd));
        return Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
        // return decode_by_table(pd);
        // return decode_last_quot(pd);
    }

    inline uint64_t decode_last_quot_wrapper_db(const __m512i *pd) {
        // assert(decode_last_quot(pd) == decode_last_quot_safe(pd));
        // return decode_last_quot(pd);
        auto a = decode_last_quot(pd);
        auto b = decode_last_quot_safe(pd);
        // auto c = decode_last_quot_in_pd_att(pd);
        if (a != b) {
            v_pd512_plus::print_headers(pd);
            // v_pd512_plus::print_h1(0,pd);
        }
        assert(validate_encoding(pd));
        assert(a == b);
        if (a) {
            assert(get_specific_quot_capacity(a, pd));
        }
        // assert(a == c);
        return a;
    }

    inline uint8_t get_last_qr_in_pd(const __m512i *pd) {
        uint64_t quot = 50 - decode_last_quot_wrapper(pd);
        constexpr int imm1 = 3;
        constexpr int imm2 = 15;
        const uint64_t rem = _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);
        return (quot << 8ul) | rem;
    }

    inline uint64_t get_last_quot_after_future_swap_ext_slow(int64_t new_quot, const __m512i *pd) {
        assert(pd_full(pd));
        uint64_t curr_last_q = decode_last_quot_wrapper(pd);
        if (new_quot > curr_last_q)
            return curr_last_q;

        if (new_quot == curr_last_q) {
            assert(get_specific_quot_capacity(curr_last_q, pd));
            return curr_last_q;
        }

        if (get_specific_quot_capacity(curr_last_q, pd) > 1)
            return curr_last_q;
        while (new_quot < curr_last_q) {
            curr_last_q--;
            if (get_specific_quot_capacity(curr_last_q, pd) > 0)
                return curr_last_q;
        }
        return new_quot;
    }

    inline uint64_t get_last_quot_after_future_swap_naive(int64_t new_quot, const __m512i *pd) {
        assert(pd_full(pd));
        const uint64_t curr_last_q = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];

        if (new_quot >= curr_last_q)
            return curr_last_q;

        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);

        auto att = count_ones_up_to_the_kth_zero(pd, 50);
        auto temp_min = MIN(new_quot, curr_last_q);
        auto min_q = MIN(temp_min, att);
        if (min_q == att)
            att = temp_min;
        // auto att_cap = get_specific_quot_capacity(att, pd);
        auto v_att = get_last_quot_after_future_swap_ext_slow(new_quot, pd);
        // auto v_att_cap = get_specific_quot_capacity(v_att, pd);
        // if (att != v_att) {
        //     get_last_quot_after_future_swap_ext_slow(new_quot, pd);
        // }
        assert(att == v_att);
        return att;
    }

    inline uint64_t find_next_zero_from_msb(uint64_t last_zero_index, const __m512i *pd) {
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        const uint64_t rel_index = get_last_zero_index(pd) & 63;// equiv to % 64.
        const uint64_t temp = (~h1) & ((1ULL << rel_index) - 1);
        uint64_t temp_index = _lzcnt_u64(temp);
        return temp_index;
    }

    inline uint64_t get_last_quot_after_future_swap(int64_t new_quot, const __m512i *pd) {
        assert(pd_full(pd));
        const uint64_t curr_last_q = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
        if (new_quot >= curr_last_q)
            return curr_last_q;

        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        const uint64_t last_zero_index = get_last_zero_index(pd);
        assert(!v_pd512_plus::test_bit(last_zero_index, pd));
        const uint64_t rel_index = get_last_zero_index(pd) & 63;// equiv to % 64.
        if (last_zero_index > 64) {
            if (!(h1 & (1ULL << (rel_index - 1)))) {
                assert(get_specific_quot_capacity(curr_last_q, pd) > 1);
                uint64_t res = curr_last_q;
                assert(res == get_last_quot_after_future_swap_naive(new_quot, pd));
                return res;
                return curr_last_q;
            }
            uint64_t temp_index = find_next_zero_from_msb(last_zero_index, pd);
            uint64_t index_from_lsb = 63 - temp_index;
            assert(rel_index > index_from_lsb + 1);
            uint64_t diff = rel_index - index_from_lsb - 1;
            uint64_t nom = curr_last_q - diff;
            uint64_t res = MAX(new_quot, nom);
            assert(res == get_last_quot_after_future_swap_naive(new_quot, pd));
            return res;
        } else {
            assert(false);
            return 4242;
        }
        // const uint64_t ones_count = QUOT_SIZE - curr_last_q;
        // assert(ones_count <= 37);
        // const uint64_t masked_h1 = h1 & ((1ULL << (37 - ones_count)) - 1);


        // auto att = count_ones_up_to_the_kth_zero(pd, 50);
        // auto temp_min = MIN(new_quot, curr_last_q);
        // auto min_q = MIN(temp_min, att);
        // if (min_q == att)
        //     att = temp_min;
        // // auto att_cap = get_specific_quot_capacity(att, pd);
        // auto v_att = get_last_quot_after_future_swap_ext_slow(new_quot, pd);
        // // auto v_att_cap = get_specific_quot_capacity(v_att, pd);
        // // if (att != v_att) {
        // //     get_last_quot_after_future_swap_ext_slow(new_quot, pd);
        // // }
        // assert(att == v_att);
        // return att;
    }

    inline bool should_look_in_current_level_by_qr_safe(int64_t quot, uint8_t rem, const __m512i *pd) {
        const int64_t last_q = decode_last_quot_wrapper_db(pd);
        if (quot < last_q)
            return true;
        else if (quot > last_q)
            return false;
        else
            return (get_last_byte(pd) >= rem);
    }

    inline bool pd_find_50_v0(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        const uint64_t last_q = decode_last_quot_wrapper_db(pd);
        if (last_q < quot)
            return false;

        //v1 code.
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        // assert(popcount128(header) == 50);
        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    inline bool pd_find_50_v1(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        // assert(popcount128(header) == 50);
        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        if (begin == end) return false;
        assert(begin <= end);
        assert(end <= 51);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
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
            // //std::cout << "h0" << std::endl;
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            // //std::cout << "h1" << std::endl;
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            // //std::cout << "h2" << std::endl;

            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            // //std::cout << "h3" << std::endl;

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
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        // const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        // const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        const uint64_t h0 = ((uint64_t *)pd)[0];
        const uint64_t h1 = ((uint64_t *)pd)[1];
        if (_blsr_u64(v) == 0) {
            if (v << quot) {
                // const unsigned __int128 *h = (const unsigned __int128 *) pd;
                // const unsigned __int128 header = (*h);
                // const bool att = (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
                // assert(att == pd_find_50_v1(quot, rem, pd));
                const int64_t mask = v << quot;
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const unsigned __int128 *h = (const unsigned __int128 *) pd;
                constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
                const unsigned __int128 header = (*h) & kLeftoverMask;
                const unsigned __int128 mask = ((unsigned __int128) v) << quot;
                // const bool att = (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
                // assert(att == pd_find_50_v1(quot, rem, pd));
                return (!(header & mask)) && (popcount128(header & (mask - 1)) == quot);
            }
        }

        const int64_t pop = _mm_popcnt_u64(h0);

        if (quot == 0) {
            // //std::cout << "h0" << std::endl;
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            // //std::cout << "h1" << std::endl;
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            // //std::cout << "h2" << std::endl;

            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            // //std::cout << "h3" << std::endl;

            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }


    inline bool pd_find_50_v23(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            const uint64_t mask = _lrotl(v, quot);
            return (mask > v) ? ((!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot)) : ((!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot));
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {

            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {

            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v21_analyse(int64_t quot, uint8_t rem, const __m512i *pd) {
        static size_t v_count[2] = {0, 0};
        static size_t blsr_u64_count[4] = {0, 0, 0, 0};
        static size_t cond_1[2] = {0, 0};
        static size_t popcnt_cond[2] = {0, 0};

        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        bool res = true;
        if (!v) res = false;

        v_count[(v == 0)]++;


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            const uint64_t mask = _lrotl(v, quot);
            return (mask > v) ? ((!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot)) : ((!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot));
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {

            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {

            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v22(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            if (v << quot) {
                const uint64_t mask = v << quot;
                return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
            } else {
                const uint64_t mask = v >> (64 - quot);
                return (!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot);
            }
        }

        if (quot == 0)
            return v & (_blsmsk_u64(h0) >> 1ul);

        const int64_t pop = _mm_popcnt_u64(h0);
        const int cmp = (quot == pop) * 2 + (quot > pop);

        const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);

        const uint64_t mask_1 = (~_bzhi_u64(-1, quot - pop - 1));
        const uint64_t h_cleared_quot_set_bits_1 = _pdep_u64(mask_1, h1);

        const uint64_t helper = _lzcnt_u64(h0);
        const uint64_t temp = (63 - helper) + 1;
        const uint64_t diff = helper + _tzcnt_u64(h1);

        switch (cmp) {
            case 0:
                // const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
                // const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
                return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
                break;
            case 1:
                // const uint64_t mask_1 = (~_bzhi_u64(-1, quot - pop - 1));
                // const uint64_t h_cleared_quot_set_bits_1 = _pdep_u64(mask_1, h1);
                return (((_blsmsk_u64(h_cleared_quot_set_bits_1) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits_1))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
            case 2:
                // const uint64_t helper = _lzcnt_u64(h0);
                // const uint64_t temp = (63 - helper) + 1;
                // const uint64_t diff = helper + _tzcnt_u64(h1);
                return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            default:
                assert(false);
                break;
        }
        // } else if (quot < pop) {
        //     const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        //     const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        //     return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        // } else if (quot > pop) {
        //     const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
        //     const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
        //     return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        // } else {
        //     const uint64_t helper = _lzcnt_u64(h0);
        //     const uint64_t temp = (63 - helper) + 1;
        //     const uint64_t diff = helper + _tzcnt_u64(h1);
        //     return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        // }
        return false;
    }

    inline bool part2_helper(int64_t quot, uint64_t v, uint64_t h0, uint64_t h1) {
        if (v << quot) {
            const uint64_t mask = v << quot;
            return (!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot);
        } else {
            const uint64_t mask = v >> (64 - quot);
            return (!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot);
        }
    }

    inline bool part2_helper_v2(int64_t quot, uint64_t v, uint64_t h0, uint64_t h1) {
        if (v << quot) {
            const uint64_t mask = v << quot;
            return _mm_popcnt_u64(h0 & (mask - 1)) == quot;
        } else {
            const uint64_t mask = v >> (64 - quot);
            return _mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot;
        }
    }

    inline bool part3_helper_v3(int64_t quot, uint64_t v, uint64_t h0, uint64_t h1) {
        if (v << quot) {
            const uint64_t mask = v << quot;
            return _mm_popcnt_u64(h0 & (mask - 1)) == quot;
        } else {
            const uint64_t mask = v >> (64 - quot);
            return _mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot;
        }
    }

    inline bool part2_helper_v4(int64_t quot, uint64_t v, uint64_t h0, uint64_t h1) {
        if (v << quot) {
            const uint64_t mask = v << quot;
            return (_mm_popcnt_u64(h0 & (mask - 1)) == quot) && (!(h0 & mask));
        } else {
            const uint64_t mask = v >> (64 - quot);
            return (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot) && (!(h1 & mask)) ;
        }
    }
    inline bool pd_find_50_v25(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t v_off = _blsr_u64(v);
        // const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        // const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);

        const uint64_t h0 = ((uint64_t *)pd)[0];
        const uint64_t h1 = ((uint64_t *)pd)[1];
        if (v_off == 0) {
            return part2_helper(quot, v, h0, h1);
        } else if (_blsr_u64(v_off) == 0) {
            return part2_helper(quot, v_off, h0, h1) || part2_helper(quot, v ^ v_off, h0, h1);
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }


    inline bool pd_find_50_v26(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (v_off == 0) {
            return part2_helper(quot, v, h0, h1);
        } else if (_blsr_u64(v_off) == 0) {
            return part2_helper(quot, v_off, h0, h1) || part2_helper(quot, v ^ v_off, h0, h1);
        } else
            return (quot != 0) ? (v & (_blsmsk_u64(h0) >> 1ul)) : true;


        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v27(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        if (!v128) return false;
        // assert(v128);
        // unsigned __int128 h128 = ((unsigned __int128*) pd)[0];
        // if ((hp128[0] & v128) == v128) {
        //     if (pd_find_50_v0(quot, rem, pd)) {
        //         std::cout << v_pd512_plus::to_bin_reversed(hp128[0]) << std::endl;
        //         std::cout << v_pd512_plus::to_bin_reversed(v128) << std::endl;
        //         v_pd512_plus::print_headers(pd);
        //         // v_pd512_plus::to_bin_reversed(hp128[0]);
        //         // v_pd512_plus::to_bin_reversed(v128);
        //         assert(0);
        //     }
        //     return false;
        // }
        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (v_off == 0) {
            return part2_helper(quot, v, h0, h1);
        } else if (_blsr_u64(v_off) == 0) {
            return part2_helper(quot, v_off, h0, h1) || part2_helper(quot, v ^ v_off, h0, h1);
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v28(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v) return false;

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        // if (!v128) return false;

        const unsigned __int128 mask0 = v128 - 1;
        const unsigned __int128 mask1 = (v128 & (v128 - 1)) - 1;
        const unsigned __int128 mask2 = (mask1 & (mask1 - 1)) - 1;
        const unsigned __int128 mask3 = (mask2 & (mask2 - 1)) - 1;
        return (v && v128) && ((popcount128(*hp128 & mask0) == quot) ||
                               (popcount128(*hp128 & mask1) == quot) ||
                               (popcount128(*hp128 & mask2) == quot) ||
                               (popcount128(*hp128 & mask3) == quot) ||
                               (mask3 & (mask3 - 1)));
    }
    inline bool pd_find_50_v29(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v) return false;

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        // if (!v128) return false;

        const unsigned __int128 mask0 = v128 - 1;
        const unsigned __int128 mask1 = (v128 & (v128 - 1));
        // const unsigned __int128 mask2 = (v128 & (v128 - 1) & (v128 - 2));
        // const unsigned __int128 mask3 = (v128 & (v128 - 1) & (v128 - 2) & (v128 - 3));
        return (v && v128) && ((popcount128(*hp128 & mask0) == quot) ||
                               ((mask1 != 0) && ((popcount128(*hp128 & (mask1 - 1)) == quot) ||
                                                 (v128 & (v128 - 1) & (v128 - 2)))));
    }

    inline bool pd_find_50_v30(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        if (!v128) return false;

        const unsigned __int128 mask0 = v128 - 1;
        const unsigned __int128 mask1 = (v128 & (v128 - 1));
        return (v && v128) && ((popcount128(*hp128 & mask0) == quot) ||
                               ((mask1 != 0) && ((popcount128(*hp128 & (mask1 - 1)) == quot) ||
                                                 (v128 & (v128 - 1) & (v128 - 2)))));
    }
    inline bool pd_find_50_v31(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        if (!v128) return false;

        const unsigned __int128 mask0 = v128 - 1;
        const unsigned __int128 mask1 = (v128 & (v128 - 1));
        const unsigned __int128 mask2 = (mask1 & (mask1 - 1));
        const unsigned __int128 mask3 = (mask2 & (mask2 - 1));
        return (popcount128(*hp128 & mask0) == quot) ||
               ((mask1 != 0) && ((popcount128(*hp128 & (mask1 - 1)) == quot) ||
                                 ((mask2 != 0) && ((popcount128(*hp128 & (mask2 - 1)) == quot) || mask3))));
    }

    inline bool pd_find_50_v32_db(int64_t quot, uint8_t rem, const __m512i *pd) {
        // debug this.
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        // uint64_t v_old = v;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        // const uint64_t a = (h0 >> quot);
        // const uint64_t b = (h1 & ((1ULL << quot) - 1)) << (64 - quot);
        // if (a & b) {
        //     v_pd512_plus::print_headers(pd);
        //     std::cout << "a:  " << v_pd512_plus::bin_print_header_spaced2(a) << std::endl;
        //     std::cout << "b:  " << v_pd512_plus::bin_print_header_spaced2(b) << std::endl;
        //     std::cout << "ab: " << v_pd512_plus::bin_print_header_spaced2(a & b) << std::endl;
        // }
        // assert(!(a & b));
        // assert(!((h0 >> quot) & ((h1 & ((1ULL << quot) - 1)) >> (64 - quot))));
        // v &= ~((h0 >> quot) | ((h1 & ((1ULL << quot) - 1)) >> (64 - quot)));
        v &= ~((h0 >> quot) | ((h1 & ((1ULL << quot) - 1)) << (64 - quot)));
        if (!v)
            // if (pd_find_50_v1(quot, rem, pd)) {
            //     v_pd512_plus::print_headers(pd);
            //     std::cout << "v:  "<< v_pd512_plus::bin_print_header_spaced2(v) << std::endl;
            //     std::cout << "a:  "<< v_pd512_plus::bin_print_header_spaced2(a) << std::endl;
            //     std::cout << "b:  "<< v_pd512_plus::bin_print_header_spaced2(b) << std::endl;
            //     std::cout << "ab: "<< v_pd512_plus::bin_print_header_spaced2(a & b) << std::endl;
            //     assert(!pd_find_50_v1(quot, rem, pd));
            // }
            return false;


        const uint64_t v_off = _blsr_u64(v);

        if (v_off == 0) {
            return part2_helper_v2(quot, v, h0, h1);
        } else if (_blsr_u64(v_off) == 0) {
            return part2_helper_v2(quot, v_off, h0, h1) || part2_helper_v2(quot, v ^ v_off, h0, h1);
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v32(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & ((1ULL << 37) - 1);
        v &= ~((h0 >> quot) | ((h1 & ((1ULL << quot) - 1)) << (64 - quot)));
        if (!v)
            return false;

        const uint64_t v_off = _blsr_u64(v);

        if (v_off == 0) {
            return part2_helper_v2(quot, v, h0, h1);
        } else if (_blsr_u64(v_off) == 0) {
            return part2_helper_v2(quot, v_off, h0, h1) || part2_helper_v2(quot, v ^ v_off, h0, h1);
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline bool pd_find_50_v33(int64_t quot, uint8_t rem, const __m512i *pd) {
        //todo.
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return false;

        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & ((1ULL << 37) - 1);
        v &= ~((h0 >> quot) | ((h1 & ((1ULL << quot) - 1)) << (64 - quot)));
        if (!v)
            return false;

        const uint64_t v_off = _blsr_u64(v);

        return part2_helper_v2(quot, v, h0, h1) || part2_helper_v2(quot, v_off, h0, h1) || part2_helper_v2(quot, v ^ v_off, h0, h1) || part2_helper_v2(quot, _blsr_u64(v_off), h0, h1);
        // if (v_off == 0) {
        //     return part2_helper_v2(quot, v, h0, h1);
        // } else if (_blsr_u64(v_off) == 0) {
        //     return part2_helper_v2(quot, v_off, h0, h1) || part2_helper_v2(quot, v ^ v_off, h0, h1);
        // }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }


    inline bool pd_find_50_v28_db(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) {
            assert(!pd_find_50_v1(quot, rem, pd));
            return false;
        }

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        if (!v128) {
            assert(!pd_find_50_v1(quot, rem, pd));
            return false;
        }
        assert(v && v128);
        // const uint64_t v_hi = v128 >>64;
        // const uint64_t v_lo = v128;
        auto pop = popcount128(v128);
        const unsigned __int128 mask0 = v128 - 1;
        const unsigned __int128 mask1 = (v128 & (v128 - 1)) - 1;
        const unsigned __int128 mask2 = (v128 & (v128 - 1) & (v128 - 2)) - 1;
        const unsigned __int128 mask3 = (v128 & (v128 - 1) & (v128 - 2) & (v128 - 3)) - 1;
        return (v && v128) && ((popcount128(*hp128 & mask0) == quot) ||
                               (popcount128(*hp128 & mask1) == quot) ||
                               (popcount128(*hp128 & mask2) == quot) ||
                               (popcount128(*hp128 & mask3) == quot) ||
                               (v128 & (v128 - 1) & (v128 - 2) & (v128 - 3) & (v128 - 4)));
    }


    inline bool pd_find_50(int64_t quot, uint8_t rem, const __m512i *pd) {
#ifndef NDEBUG
        if (should_look_in_current_level_by_qr_safe(quot, rem, pd)) {
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v9(quot, rem, pd));
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v11(quot, rem, pd));
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v17(quot, rem, pd));
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v18(quot, rem, pd));
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v22(quot, rem, pd));
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v25(quot, rem, pd));
            if (pd_find_50_v1(quot, rem, pd) != pd_find_50_v32(quot, rem, pd)) {
                pd_find_50_v32(quot, rem, pd);
                pd_find_50_v32(quot, rem, pd);
                pd_find_50_v32(quot, rem, pd);
                assert(0);
            }
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v32(quot, rem, pd));
            if (pd_find_50_v1(quot, rem, pd) != pd_find_50_v27(quot, rem, pd)) {
                pd_find_50_v27(quot, rem, pd);
                pd_find_50_v27(quot, rem, pd);
                pd_find_50_v27(quot, rem, pd);
            }
            assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v27(quot, rem, pd));
            if (pd_find_50_v1(quot, rem, pd)) {
                assert(pd_find_50_v28(quot, rem, pd));
                assert(pd_find_50_v29(quot, rem, pd));
                assert(pd_find_50_v30(quot, rem, pd));
                assert(pd_find_50_v31(quot, rem, pd));
                // if (!pd_find_50_v28_db(quot, rem, pd)) {
                //     pd_find_50_v28_db(quot, rem, pd);
                //     pd_find_50_v28_db(quot, rem, pd);
                //     assert(0);
                // }
            }
        }

#endif// !NDEBUG

        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v26(quot, rem, pd));
        // assert(pd_find_50_v1(quot, rem, pd) == pd_find_50_v23(quot, rem, pd));
        // return pd_find_50_v25(quot, rem, pd);
        // return pd_find_50_v27(quot, rem, pd);
        // return pd_find_50_v28(quot, rem, pd);
        // return pd_find_50_v29(quot, rem, pd);
        return pd_find_50_v25(quot, rem, pd);
    }

    inline bool did_pd_overflowed_vec(const __m512i *pd) {
        return (_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 128);
    }

    inline bool did_pd_overflowed(const __m512i *pd) {
        // uint8_t x;
        // memcpy(&x, &((uint8_t *) pd)[12], 1);
        // return x & 128;
        return (_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 128);
    }

    inline bool did_pd_overflowed_att(const __m512i *pd) {
        uint8_t x;
        memcpy(&x, &((uint8_t *) pd)[12], 1);
        return x & 128;
    }

    inline bool did_pd_overflowed_wrapper(const __m512i *pd) {
        bool a = did_pd_overflowed_vec(pd);
        bool b = did_pd_overflowed_att(pd);
        if (a != b) {
            v_pd512_plus::print_hlb(pd);
            did_pd_overflowed_att(pd);
        }
        assert(a == b);
        return true;
    }

    inline void set_overflow_bit(__m512i *pd) {
        uint64_t *h_array = ((uint64_t *) pd);
        h_array[1] |= (1ULL) << (103 - 64);
        assert(did_pd_overflowed_vec(pd));
        assert(did_pd_overflowed_wrapper(pd));
    }
    inline void clear_overflow_bit(__m512i *pd) {
        assert(false);
        uint64_t *h_array = ((uint64_t *) pd);
        // assert(h_array[1] & (1ULL) << (101 - 64));
        assert(did_pd_overflowed(pd));
        h_array[1] ^= (1ULL) << (103 - 64);
        assert(!did_pd_overflowed(pd));

        // h_array[1] &= (((1ULL << 101 - 64) - 1) | ~(((1ULL << 41) - 1)));
    }

    inline void update_max_quot_when_pd_is_not_full(int64_t quot, __m512i *pd) {
        assert(!pd_full(pd));
        const uint64_t old_quot = get_last_byte(pd);
        if (quot <= old_quot)
            return;

        memcpy(&((uint8_t *) pd)[63], &quot, 1);
        assert(get_last_byte(pd) == quot);
    }

    inline void encode_last_quot_helper_mb1(int64_t last_quot, __m512i *pd) {
        uint64_t decrease_by = QUOT_SIZE - last_quot - 1;
        assert(1 <= decrease_by);
        assert(decrease_by <= 4);
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        uint64_t old_word = header_last_byte;
        uint64_t word = header_last_byte | 32;
        if (decrease_by == 1) {
            // word |= 32;
            memcpy(&((uint8_t *) pd)[12], &word, 1);
            assert(validate_encoding(pd));
        } else if (decrease_by == 2) {
            word |= 31;
            word ^= (2 + 8);
            memcpy(&((uint8_t *) pd)[12], &word, 1);
            assert(validate_encoding(pd));
        } else if (decrease_by == 3) {
            word |= 31;
            word ^= 17;
            memcpy(&((uint8_t *) pd)[12], &word, 1);
            assert(validate_encoding(pd));
        } else if (decrease_by == 4) {
            word &= 128;
            word |= (32 + 7);
            memcpy(&((uint8_t *) pd)[12], &word, 1);
            assert(validate_encoding(pd));
        } else {
            assert(0);
        }
        memcpy(&((uint8_t *) pd)[12], &old_word, 1);
    }

    inline void encode_last_quot_only_decrease(int64_t last_quot, __m512i *pd) {
        // v_pd512_plus::print_headers(pd);
        uint64_t decrease_by = QUOT_SIZE - last_quot - 1;
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        uint64_t word = header_last_byte;
        if (decrease_by == 0) {
            assert(0);
        }
        if (decrease_by <= 4) {
            encode_last_quot_helper_mb1(last_quot, pd);
            uint64_t bits_to_read = 4 - decrease_by;
            assert(bits_to_read <= 3);
            word &= 7;
            word |= (bits_to_read << 3);
            word |= 32;

        } else if (decrease_by <= 20) {
            uint64_t decrease_by_with_offset = decrease_by - 5;
            assert(decrease_by_with_offset <= 15);
            uint64_t a = header_last_byte & 1;
            uint64_t b = 64;
            uint64_t c = decrease_by_with_offset << 1u;
            uint64_t d = (a & b) || (a & c) || (b & c);
            assert(!d);
            word = (header_last_byte & 1) | 64 | (decrease_by_with_offset << 1u);
        } else {
            uint64_t decrease_by_with_offset = decrease_by - 18;
            // uint64_t a = header_last_byte & 1;
            uint64_t b = 32 + 64;
            uint64_t c = decrease_by_with_offset;
            uint64_t d = !(b & c);
            assert(d);
            word = (32 + 64) | decrease_by_with_offset;
        }
        const uint64_t OF_bit = header_last_byte & 128;
        word |= OF_bit;
        //std::cout << std::string(80, '-') << std::endl;
        // v_pd512_plus::print_h1(0, pd);
        memcpy(&((uint8_t *) pd)[12], &word, 1);
        // v_pd512_plus::print_h1(0, pd);
        //std::cout << std::string(80, '-') << std::endl;
        // v_pd512_plus::print_headers(pd);
        assert(decode_last_quot_wrapper(pd) == last_quot);
        assert(validate_encoding(pd));
    }

    inline void encode_last_quot_when_full_for_the_first_time(int64_t old_last_quot, int64_t new_quot, __m512i *pd) {
        const uint64_t new_last_quot = (new_quot < old_last_quot) ? old_last_quot : new_quot;
        assert(new_last_quot == MAX(new_quot, old_last_quot));
        if (new_last_quot == 49) {
            uint64_t hi_meta_bits = get_hi_meta_bits(pd);
            auto temp_value = hi_meta_bits & (32 + 64);
            assert(temp_value == 0);
            uint64_t header_last_byte = get_header_last_byte(pd);
            if (header_last_byte & 8) {
                // auto h_last_byte
                v_pd512_plus::print_headers(pd);
                assert(0);
            }
            assert(!(header_last_byte & 8));
            return;
        }
        encode_last_quot_only_decrease(new_last_quot, pd);
    }

    inline void encode_after_swap_3(int64_t correct_q, __m512i *pd) {
        if (correct_q == 49) {
            uint64_t hi_meta_bits = get_hi_meta_bits(pd);
            auto temp_value = hi_meta_bits & (32 + 64);
            assert(temp_value == 0);
            uint64_t header_last_byte = get_header_last_byte(pd);
            assert(!(header_last_byte & 8));
            assert(get_specific_quot_capacity(correct_q, pd));
            return;
        }
        encode_last_quot_only_decrease(correct_q, pd);
    }


    inline void decrease_pd_max_quot_after_swap_insertion(int64_t possible_max_quot, int64_t old_quot, __m512i *pd) {
        // set_overflow_bit(pd);
        // assert(pd_full(pd));
        //Todo: FIX THIS! (might work but probably cause many weird problems)
        //Todo: this should be computed before the deletion. depeneding on the next zero place.
        //currently, the PD state is not valid.
        const uint64_t new_quot = get_last_quot_for_full_pd_without_deletions(pd);
        if (new_quot == old_quot) {
            //todo: fix this. (failed)
            assert(decode_last_quot_wrapper(pd) == new_quot);
            return;
        }
        assert(new_quot < old_quot);
        assert(possible_max_quot <= new_quot);
        encode_last_quot_only_decrease(new_quot, pd);
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
        No = 0,
        Yes = 1,
        look_in_the_next_level = 2,
        Error = -1
    };

    inline pd_Status pd_find_enums(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd_find_50_v18(quot, rem, pd) ? Yes : (did_pd_overflowed(pd) ? look_in_the_next_level : No);
        // return pd_find_50_v18(quot, rem, pd) || did_pd_overflowed(pd);
        // return pd_find_50_v18(quot, rem, pd) || did_pd_overflowed(pd);
        // return pd_find_50_v11(quot, rem, pd) || did_pd_overflowed(pd);
    }


    inline pd_Status pd_find_50_case2_helper(int64_t quot, uint8_t rem, const __m512i *pd) {
        // int last_byte = get_last_byte(pd);
        // int actual_rem = rem;
        // int cmp8 = get_last_byte(pd) < rem;
        // int cmp_int = last_byte < actual_rem;
        // assert(cmp8 == cmp_int);
        if (pd_full(pd)) {
            if (get_last_byte(pd) < rem) {
                //std::cout << "_1" << std::endl;
                return did_pd_overflowed(pd) ? look_in_the_next_level : No;
            } else if (get_last_byte(pd) == rem) {
                //std::cout << "_2" << std::endl;
                return Yes;
            } else {
                //std::cout << "_3" << std::endl;
                return pd_find_50_v18(quot, rem, pd) ? Yes : No;
            }
        }
        //std::cout << "_4" << std::endl;
        // //std::cout << std::endl;
        return pd_find_50_v18(quot, rem, pd) ? Yes : No;
    }

    // inline bool should_look_in_CURRENT_pd(int64_t quot, uint8_t rem, const __m512i *pd) {
    //     if (!pd_full(pd))
    //         return quot <= Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];

    //     const uint64_t last_q = decode_last_quot(pd);
    //     const uint64_t last_rem = get_last_qr_in_pd
    //     const int64_t last_q = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
    //     return (last_q < quot) || ((last_q == quot) && (get_last_byte(pd) < rem));
    // }


    inline bool should_look_in_next_level_by_qr(int64_t quot, uint8_t rem, const __m512i *pd) {
#ifndef COUNT
        assert(pd_full(pd));
#endif// !COUNT \
        // const uint64_t last_q = decode_last_quot(pd);
        const int64_t last_q = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
        return (last_q < quot) || ((last_q == quot) && (get_last_byte(pd) < rem));
    }

    inline pd_Status cmp_qr_return_pd_status(int64_t quot, uint8_t rem, const __m512i *pd) {
        // const uint64_t last_q = decode_last_quot(pd);
        const int64_t last_q = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
        return ((last_q < quot) || ((last_q == quot) && (get_last_byte(pd) < rem))) ? look_in_the_next_level : No;
    }

    inline pd_Status call_this_function_after_search_in_pd_failed(int64_t quot, uint8_t rem, const __m512i *pd) {
        return (did_pd_overflowed(pd) && (decode_by_table2(pd) <= quot)) ? look_in_the_next_level : No;
    }

    inline uint16_t get_last_qr_smart(const __m512i *pd) {
        return (!did_pd_overflowed(pd)) ? 12800 : (decode_by_table2(pd) << 8 | get_last_byte(pd));
    }

    inline bool cmp_qr_smart0(uint16_t qr, const __m512i *pd) {
        // uint8_t x, y;
        // memcpy(&x, &((uint8_t *) pd)[12], 1);
        // memcpy(&y, &((uint8_t *) pd)[63], 1);
        // return (x & 128) && ((Lookup_Table[x & 127] << 8 | y) < qr);
        return ((decode_by_table2(pd) << 8 | get_last_byte(pd)) < qr) && did_pd_overflowed(pd);
        // return did_pd_overflowed(pd) && ((decode_by_table2(pd) << 8 | get_last_byte(pd)) < qr);
        // const uint16_t last_h = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        // return (last_h & 128) || ((Lookup_Table[last_h & 127] << 8 | get_last_byte(pd)) < qr);
    }

    inline bool cmp_qr_smart(uint16_t qr, const __m512i *pd) {
        uint8_t x, y;
        memcpy(&x, &((uint8_t *) pd)[12], 1);
        memcpy(&y, &((uint8_t *) pd)[63], 1);
        return (x & 128) && (((Lookup_Table[x & 127] << 8) | y) < qr);
        // return ((decode_by_table2(pd) << 8 | get_last_byte(pd)) < qr) && did_pd_overflowed(pd);
        // return did_pd_overflowed(pd) && ((decode_by_table2(pd) << 8 | get_last_byte(pd)) < qr);
        // const uint16_t last_h = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        // return (last_h & 128) || ((Lookup_Table[last_h & 127] << 8 | get_last_byte(pd)) < qr);
    }

    inline bool cmp_qr_smart2(uint16_t qr, const __m512i *pd) {
        const uint8_t x = ((uint8_t *) pd)[12];
        return (x & 128) && ((Lookup_Table[x & 127] << 8 | (((uint8_t *) pd)[63])) < qr);
    }

    inline bool cmp_qr_smart3(uint16_t qr, const __m512i *pd) {
        
        uint8_t x;
        memcpy(&x, &((uint8_t *) pd)[12], 1);
        if (!(x & 128))
            return false;
        if ((qr >> 8) < Lookup_Table[x & 127])
            return false;
        else if ((qr >> 8) > Lookup_Table[x & 127])
            return true;
        else
            return (qr & 255) > get_last_byte(pd);
        
    }

    inline bool cmp_qr_smart3(int64_t quot, uint8_t rem, const __m512i *pd) {
        
        uint8_t x;
        memcpy(&x, &((uint8_t *) pd)[12], 1);
        if (!(x & 128))
            return false;
        if (quot < Lookup_Table[x & 127])
            return false;
        else if (quot > Lookup_Table[x & 127])
            return true;
        else
            return (rem & 255) > get_last_byte(pd);
        
    }
    inline bool cmp_qr_smart4(int64_t quot, uint8_t rem, const __m512i *pd) {
        
        uint8_t x,y;
        memcpy(&x, &((uint8_t *) pd)[12], 1);
        memcpy(&y, &((uint8_t *) pd)[63], 1);

        return (x & 128) && ((quot > Lookup_Table[x & 127]) || ((quot == Lookup_Table[x & 127]) && (rem > y)));
    }


    inline bool pd_minimal_find_50_v2(int64_t quot, uint8_t rem, const __m512i *pd) {
        return should_look_in_next_level_by_qr(quot, rem, pd) || pd_find_50_v18(quot, rem, pd);
    }

    inline bool pd_minimal_find_50_v3(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        return quot <= decode_by_table2(pd);
    }

    inline bool pd_minimal_find_50_v4(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        return v;

        // return quot <= decode_by_table2(pd);
    }

    inline bool pd_minimal_find_50_v5(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        return v ^ quot;

        // return quot <= decode_by_table2(pd);
    }


    inline pd_Status pd_find2(int64_t quot, uint8_t rem, const __m512i *pd) {
        pd_full(pd) ? (should_look_in_next_level_by_qr ? (did_pd_overflowed(pd) && look_in_the_next_level) : pd_find_50(quot, rem, pd)) : pd_find_50(quot, rem, pd);
        if (pd_find_50(quot, rem, pd))
            return Yes;
        return (should_look_in_next_level_by_qr(quot, rem, pd) && did_pd_overflowed(pd)) ? look_in_the_next_level : No;
    }

    inline pd_Status pd_find1(int64_t quot, uint8_t rem, const __m512i *pd) {
        // if (!should_look_in_next_level_by_qr(quot, rem, pd))
        //     pd_find_50_old(quot, rem, pd);
        if (pd_find_50(quot, rem, pd))
            return Yes;
        return (did_pd_overflowed(pd) && (decode_by_table2(pd) <= quot)) ? look_in_the_next_level : No;
        // return should_look_in_next_level_by_qr(quot, rem, pd) ? (did_pd_overflowed(pd) ? look_in_the_next_level : No) : pd_find_50_v18(quot, rem, pd) ? Yes: No;
        // const __m512i target = _mm512_set1_epi8(rem);
        // const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v){
        //     return (did_pd_overflowed(pd) && should_look_in_next_level_by_qr(quot, rem, pd)) ? look_in_the_next_level : No;
        // }
        // if (did_pd_overflowed){
        //     return should_look_in_next_level_by_qr(quot, rem, pd) ? look_in_the_next_level : (pd_find_50_v18(quot, rem, pd) ? Yes : No);
        // }
        // return ((!should_look_in_next_level_by_qr(quot, rem, pd)) && (pd_find_50_v18(quot, rem, pd))) ? Yes: No;
        // const uint64_t last_quot = decode_last_quot_wrapper(pd);
        // int cmp = (quot < last_quot) + (quot == last_quot) * 2;
        // switch (cmp) {
        //     case 1:
        //         //std::cout << "l1" << std::endl;
        //         return pd_find_50_v18(quot, rem, pd) ? Yes : No;
        //     case 0:
        //         //std::cout << "l0" << std::endl;
        //         return did_pd_overflowed(pd) ? look_in_the_next_level : No;
        //     case 2:
        //         //std::cout << "l2";
        //         return pd_find_50_case2_helper(quot, rem, pd);
        //     default:
        //         break;
        // }
        // return Error;
    }

    inline pd_Status pd_find3(int64_t quot, uint8_t rem, const __m512i *pd) {
        return (pd_find_50(quot, rem, pd)) ? Yes : call_this_function_after_search_in_pd_failed(quot, rem, pd);
        // return (did_pd_overflowed(pd) && (decode_by_table2(pd) <= quot)) ? look_in_the_next_level : No;
    }

    inline pd_Status pd_find4(int64_t quot, uint8_t rem, const __m512i *pd) {
        if (pd_find_50(quot, rem, pd))
            return Yes;
        return (cmp_qr_smart4(quot, rem, pd)) ? look_in_the_next_level : No;
     }

    inline bool pd_plus_rNs(int64_t quot, uint64_t h0, uint64_t h1, uint64_t v) {
        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return v & (_blsmsk_u64(h0) >> 1ul);
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v;
        } else if (quot > pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return (((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop));
        } else {
            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
        }
    }

    inline pd_Status find_rem_with_last_q(int64_t quot, uint8_t rem, uint64_t h1, uint64_t v, const __m512i *pd) {
        if (rem > get_last_byte(pd))
            return (pd_full(pd)) ? look_in_the_next_level : No;

        return (_lzcnt_u64(v) <= get_last_quot_capacity(quot, pd) + 13) ? Yes : No;
    }

    // inline pd_Status pd_plus_rNs(int64_t quot, uint64_t h0, uint64_t h1, uint64_t v) {
    //     const int64_t pop = _mm_popcnt_u64(h0);
    //     if (quot == 0) {
    //         return (v & (_blsmsk_u64(h0) >> 1ul)) ? Yes : No;
    //     } else if (quot < pop) {
    //         const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
    //         const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
    //         return ((((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v) ? Yes : No;
    //     } else if (quot > pop) {
    //
    //         const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
    //         const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
    //         return ((((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop))) ? Yes : No;
    //     } else {
    //
    //         const uint64_t helper = _lzcnt_u64(h0);
    //         const uint64_t temp = (63 - helper) + 1;
    //         const uint64_t diff = helper + _tzcnt_u64(h1);
    //         return (diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1))) ? Yes : No;
    //     }
    // }

    inline pd_Status pd_plus_find_v1(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return call_this_function_after_search_in_pd_failed(quot, rem, pd);


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            assert(false);
            const uint64_t mask = _lrotl(v, quot);
            if (v < mask) {
                // const uint64_t mask = v << quot;
                return ((!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot)) ? Yes : call_this_function_after_search_in_pd_failed(quot, rem, pd);
            } else {
                // const uint64_t mask = v >> (64 - quot);
                return ((!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot)) ? Yes : call_this_function_after_search_in_pd_failed(quot, rem, pd);
            }
        }

        const int cmp = (decode_by_table2(pd) < quot) + (decode_by_table2(pd) == quot) * 2;
        switch (cmp) {
            case 0:
                return pd_plus_rNs(quot, h0, h1, v) ? Yes : No;
            case 1:
                // return look_in_the_next_level;
                return (pd_full(pd)) ? look_in_the_next_level : No;
            case 2:
                return find_rem_with_last_q(quot, rem, h1, v, pd);
            default:
                assert(false);
                return Error;
                // break;
        }
        // if (decode_by_table2(pd) < quot) return pd_full(pd) ? look_in_the_next_level : No;


        // const int64_t pop = _mm_popcnt_u64(h0);
        // if (quot == 0) {
        //     return (v & (_blsmsk_u64(h0) >> 1ul)) ? Yes : No;
        // } else if (quot < pop) {
        //     const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
        //     const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
        //     return ((((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v) ? Yes : No;
        // } else if (quot > pop) {
        //     const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
        //     const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
        //     return ((((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop))) ? Yes : No;
        // } else {
        //     const uint64_t helper = _lzcnt_u64(h0);
        //     const uint64_t temp = (63 - helper) + 1;
        //     const uint64_t diff = helper + _tzcnt_u64(h1);
        //     return (diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1))) ? Yes : No;
        // }
    }
    inline pd_Status pd_plus_find_v2(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (!v) return (pd_full(pd) && should_look_in_next_level_by_qr(quot, rem, pd)) ? look_in_the_next_level : No;


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            const uint64_t mask = _lrotl(v, quot);
            if (v << quot) {
                // const uint64_t mask = v << quot;
                return ((!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot)) ? Yes : cmp_qr_return_pd_status(quot, rem, pd);
            } else {
                // const uint64_t mask = v >> (64 - quot);
                return ((!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot)) ? Yes : cmp_qr_return_pd_status(quot, rem, pd);
            }
        }

        const int cmp = (decode_by_table2(pd) < quot) + (decode_by_table2(pd) == quot) * 2;
        switch (cmp) {
            case 0:
                return pd_plus_rNs(quot, h0, h1, v) ? Yes : No;
            case 1:
                // return look_in_the_next_level;
                return (pd_full(pd)) ? look_in_the_next_level : No;
            case 2:
                return find_rem_with_last_q(quot, rem, h1, v, pd);
            default:
                assert(false);
                return Error;
                // break;
        }
    }

    inline pd_Status pd_plus_find_v2_count(int64_t quot, uint8_t rem, const __m512i *pd, int ind = 0) {
        static size_t total_counter = 0;
        static size_t not_v = 0;
        static size_t yes_v = 0;
        static size_t q_cond0[2] = {0};// {must lookups, redundent lookups}.
        static size_t q_cond1[2] = {0};
        static size_t q_cond2[2] = {0};
        static size_t qq_cond0[2] = {0};// {must lookups, redundent lookups}.
        static size_t qq_cond1[2] = {0};
        static size_t qq_cond2[2] = {0};
        static size_t new_blsr_arr[2] = {0};
        static size_t blsr_arr[51] = {0};
        static size_t two_power[2] = {0};
        static size_t part3 = 0;
        static size_t v128_counter = 0;
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
            two_power[0] = two_power[1] = 0;
            part3 = 0;
            v128_counter = 0;
        }
        if (ind == 2) {
            auto line = std::string(84, '-');
            std::cout << line << std::endl;
            std::cout << "total:      \t" << total_counter << std::endl;
            std::cout << "not_v:      \t" << not_v << std::endl;
            // std::cout << "yes_v:     \t" << yes_v << std::endl;
            std::cout << "q_cond0:    \t(" << q_cond0[0] << ", " << q_cond0[1] << ")" << std::endl;
            std::cout << "q_cond1     \t(" << q_cond1[0] << ", " << q_cond1[1] << ")" << std::endl;
            std::cout << "q_cond2     \t(" << q_cond2[0] << ", " << q_cond2[1] << ")" << std::endl;
            std::cout << "qq_cond0:   \t(" << qq_cond0[0] << ", " << qq_cond0[1] << ")" << std::endl;
            std::cout << "qq_cond1    \t(" << qq_cond1[0] << ", " << qq_cond1[1] << ")" << std::endl;
            std::cout << "qq_cond2    \t(" << qq_cond2[0] << ", " << qq_cond2[1] << ")" << std::endl;

            std::cout << "newBlsr:    \t(" << new_blsr_arr[0] << ", " << new_blsr_arr[1] << ")" << std::endl;
            // std::cout << "two_power: \t(" << two_power[0] << ", " << two_power[1] << ")" << std::endl;
            std::cout << "v128:       \t" << v128_counter << std::endl;
            std::cout << "part3:      \t" << part3 << std::endl;
            std::cout << "blsr_arr:   \t("
                      << blsr_arr[0] << ", "
                      << blsr_arr[1] << ", "
                      << blsr_arr[2] << ", "
                      << blsr_arr[3] << ", "
                      << blsr_arr[4] << ", "
                      << blsr_arr[5] << ", "
                      << blsr_arr[6] << ")"
                      << std::endl;
            std::cout << line << std::endl;
        }
        total_counter++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        q_cond0[should_look_in_next_level_by_qr(quot, rem, pd)]++;
        qq_cond0[decode_last_quot(pd) < quot]++;


        if (!v) {
            not_v++;
            return (pd_full(pd) && should_look_in_next_level_by_qr(quot, rem, pd)) ? look_in_the_next_level : No;
        }
        q_cond1[should_look_in_next_level_by_qr(quot, rem, pd)]++;
        qq_cond1[decode_last_quot(pd) < quot]++;

        yes_v++;
        auto blsr_popcount = _mm_popcnt_u64((v));
        blsr_arr[blsr_popcount]++;

        unsigned __int128 v128 = ((const unsigned __int128) v) << quot;
        const unsigned __int128 *hp128 = (const unsigned __int128 *) pd;
        v128 &= ~(*hp128);
        if (!v128) v128_counter++;
        const uint64_t v_off = _blsr_u64(v);
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (v_off == 0) {
            new_blsr_arr[0]++;
            return part2_helper(quot, v, h0, h1) ? Yes : cmp_qr_return_pd_status(quot, rem, pd);

        } else if (_blsr_u64(v_off) == 0) {
            new_blsr_arr[1]++;
            return (part2_helper(quot, v_off, h0, h1) || part2_helper(quot, v ^ v_off, h0, h1)) ? Yes : cmp_qr_return_pd_status(quot, rem, pd);
        }

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
        q_cond2[should_look_in_next_level_by_qr(quot, rem, pd)]++;
        qq_cond2[decode_last_quot(pd) < quot]++;

        part3++;

        const int cmp = (decode_by_table2(pd) < quot) + (decode_by_table2(pd) == quot) * 2;
        switch (cmp) {
            case 0:
                return pd_plus_rNs(quot, h0, h1, v) ? Yes : No;
            case 1:
                // return look_in_the_next_level;
                return (pd_full(pd)) ? look_in_the_next_level : No;
            case 2:
                return find_rem_with_last_q(quot, rem, h1, v, pd);
            default:
                assert(false);
                return Error;
                // break;
        }
    }

    inline int pd_plus_find_v3(int64_t quot, uint8_t rem, const __m512i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return (pd_full(pd) && should_look_in_next_level_by_qr(quot, rem, pd)) << 1;

        if (decode_by_table2(pd) < quot) return pd_full(pd) << 1;


        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        if (_blsr_u64(v) == 0) {
            if (v << quot) {
                const uint64_t mask = v << quot;
                return ((!(h0 & mask)) && (_mm_popcnt_u64(h0 & (mask - 1)) == quot));
            } else {
                const uint64_t mask = v >> (64 - quot);
                return ((!(h1 & mask)) && (_mm_popcnt_u64(h0) + _mm_popcnt_u64(h1 & (mask - 1)) == quot));
            }
        }

        const int64_t pop = _mm_popcnt_u64(h0);
        if (quot == 0) {
            return (v & (_blsmsk_u64(h0) >> 1ul));
        } else if (quot < pop) {
            const uint64_t mask = (~_bzhi_u64(-1, quot - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h0);
            return ((((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h0)) >> quot) & v);
        } else if (quot > pop) {

            const uint64_t mask = (~_bzhi_u64(-1, quot - pop - 1));
            const uint64_t h_cleared_quot_set_bits = _pdep_u64(mask, h1);
            return ((((_blsmsk_u64(h_cleared_quot_set_bits) ^ _blsmsk_u64(_blsr_u64(h_cleared_quot_set_bits))) & (~h1)) >> (quot - pop)) & (v >> (64 - pop)));
        } else {

            const uint64_t helper = _lzcnt_u64(h0);
            const uint64_t temp = (63 - helper) + 1;
            const uint64_t diff = helper + _tzcnt_u64(h1);
            return (diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1)));
        }
    }


    inline int pd_find_50_count(int64_t quot, uint8_t rem, const __m512i *pd) {
        if (pd_find_50_v18(quot, rem, pd))
            return 1;

        uint64_t res = 0;
        if (!did_pd_overflowed(pd))
            res |= 2;
        if (!should_look_in_next_level_by_qr(quot, rem, pd))
            res |= 4;

        return res;
    }

    inline void write_header_naive(uint64_t begin, uint64_t end, const unsigned __int128 header, __m512i *pd) {
        const uint64_t old_header_last_byte = get_header_last_byte(pd);
        const uint64_t old_header_meta_bits = get_hi_meta_bits(pd);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        new_header &= kLeftoverMask;

        uint64_t meta_bits = (old_header_last_byte >> 5) & 3;
        static constexpr uint8_t temp_table[4] = {0, 2, 4, 5};
        uint64_t how_many_extra_bits_to_encode = temp_table[meta_bits];
        uint64_t new_byte_mask = (1ULL << (5 - how_many_extra_bits_to_encode)) - 1;


        // switch (meta_bits) {
        //     case 0:
        //         how_many_extra_bits_to_encode = 0;
        //         break;
        //     case 1:
        //         how_many_extra_bits_to_encode = 2;
        //         break;
        //     case 2:
        //         how_many_extra_bits_to_encode = 4;
        //         break;
        //     case 3:
        //         how_many_extra_bits_to_encode = 5;
        //         break;
        //
        //     default:
        //         break;
        // }
        if (how_many_extra_bits_to_encode == 5)
            assert(new_byte_mask == 0);

        uint64_t old_last_byte = old_header_last_byte;
        uint64_t new_last_byte = (new_header >> 96);
        assert(new_last_byte < 256);
        uint64_t lo = new_last_byte & new_byte_mask;
        uint64_t hi = old_last_byte & ~new_byte_mask;
        assert(!(lo & hi));
        uint64_t final_last_byte = lo | hi;

        memcpy(pd, &new_header, 12);
        memcpy(&((uint8_t *) pd)[12], &final_last_byte, 1);
        assert(old_header_meta_bits == get_hi_meta_bits(pd));

        // uint64_t header_last_byte_low_bits
        // uint64_t header_last_byte_shifted = (new_header >> 96);
        // uint64_t header_last_byte_low_bits = header_last_byte_shifted & header_last_byte_mask;
        // uint64_t header_last_byte_high_bits = header_last_byte_shifted & ~header_last_byte_mask;

        // assert(header_last_byte_low_bits <= 31);
        // uint64_t header_last_byte_hi_bits = get_hi_meta_bits(pd);
        // assert((header_last_byte_low_bits & header_last_byte_hi_bits) == 0);
        // uint64_t header_last_byte = header_last_byte_low_bits | header_last_byte_hi_bits;
        // assert(popcount128(new_header) == 50);
        // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        //redundent
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        // memcpy(pd, &new_header, 12);
        // memcpy(&((uint8_t *) pd)[12], &final_last_byte, 1);
        // assert(old_header_meta_bits == get_hi_meta_bits(pd));
    }

    inline void write_header_naive_delete(uint64_t begin, uint64_t end, const unsigned __int128 header, __m512i *pd) {
        const uint64_t old_header_last_byte = get_header_last_byte(pd);
        const uint64_t old_header_meta_bits = get_hi_meta_bits(pd);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        new_header &= kLeftoverMask;

        uint64_t meta_bits = (old_header_last_byte >> 5) & 3;
        uint64_t how_many_extra_bits_to_encode = 42;
        switch (meta_bits) {
            case 0:
                how_many_extra_bits_to_encode = 0;
                break;
            case 1:
                how_many_extra_bits_to_encode = 2;
                break;
            case 2:
                how_many_extra_bits_to_encode = 4;
                break;
            case 3:
                how_many_extra_bits_to_encode = 5;
                break;

            default:
                break;
        }
        uint64_t new_byte_mask = (1ULL << (5 - how_many_extra_bits_to_encode)) - 1;
        if (how_many_extra_bits_to_encode == 5)
            new_byte_mask = 0;

        uint64_t old_last_byte = old_header_last_byte;
        uint64_t new_last_byte = (new_header >> 96);
        assert(new_last_byte < 256);
        uint64_t lo = new_last_byte & new_byte_mask;
        uint64_t hi = old_last_byte & ~new_byte_mask;
        assert(!(lo & hi));
        uint64_t final_last_byte = lo | hi;

        memcpy(pd, &new_header, 12);
        memcpy(&((uint8_t *) pd)[12], &final_last_byte, 1);
        assert(old_header_meta_bits == get_hi_meta_bits(pd));

        // uint64_t header_last_byte_low_bits
        // uint64_t header_last_byte_shifted = (new_header >> 96);
        // uint64_t header_last_byte_low_bits = header_last_byte_shifted & header_last_byte_mask;
        // uint64_t header_last_byte_high_bits = header_last_byte_shifted & ~header_last_byte_mask;
        //
        // assert(header_last_byte_low_bits <= 31);
        // uint64_t header_last_byte_hi_bits = get_hi_meta_bits(pd);
        // assert((header_last_byte_low_bits & header_last_byte_hi_bits) == 0);
        // uint64_t header_last_byte = header_last_byte_low_bits | header_last_byte_hi_bits;
        // assert(popcount128(new_header) == 50);
        // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        //redundent
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        // memcpy(pd, &new_header, 12);
        // memcpy(&((uint8_t *) pd)[12], &final_last_byte, 1);
        // assert(old_header_meta_bits == get_hi_meta_bits(pd));
    }


    inline void write_header(uint64_t begin, uint64_t end, const unsigned __int128 header, __m512i *pd) {
        return write_header_naive(begin, end, header, pd);
        /* const uint64_t old_header_last_byte = get_header_last_byte(pd);
        const uint64_t old_header_meta_bits = get_hi_meta_bits(pd);
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        new_header &= kLeftoverMask;

        uint64_t header_last_byte_low_bits = (new_header >> 96);
        assert(header_last_byte_low_bits <= 31);
        uint64_t header_last_byte_hi_bits = get_hi_meta_bits(pd);
        assert((header_last_byte_low_bits & header_last_byte_hi_bits) == 0);
        uint64_t header_last_byte = header_last_byte_low_bits | header_last_byte_hi_bits;
        // assert(popcount128(new_header) == 50);
        // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        //redundent
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        memcpy(pd, &new_header, 12);
        memcpy(&((uint8_t *) pd)[12], &header_last_byte, 1);
        assert(old_header_meta_bits == get_hi_meta_bits(pd)); */
    }

    inline uint64_t pd_add_50_not_full_after(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(quot < 50);
        assert(!pd_full(pd));
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(popcount128(header) == 50);

        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);

        write_header(begin, end, header, pd);
        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        // new_header |= ((header >> end) << (end + 1));
        // assert(popcount128(new_header) == 50);
        // // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        // //redundent
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        // memcpy(pd, &new_header, kBytes2copy);

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

        //Todo Fix this:
        //
        //  uint64_t j = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));
        //
        // //std::cout << "IJ" << std::endl;
        // assert(i == j);
        // assert((i == end_fingerprint) || (rem <= ((const uint8_t *) pd)[kBytes2copy + i]));

        memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
                &((const uint8_t *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 2));// changed 1 to 2.
        ((uint8_t *) pd)[kBytes2copy + i] = rem;

        return 1ul << 15;
    }

    inline uint64_t pd_add_50_for_swap(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(quot < 50);
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(popcount128(header) == 50);

        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);

        write_header(begin, end, header, pd);

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


        memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
                &((const uint8_t *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((uint8_t *) pd)[kBytes2copy + i] = rem;

        return 1ul << 15;
    }


    inline uint64_t pd_add_50_full_after(int64_t quot, uint8_t rem, __m512i *pd) {
        //std::cout << "A_fa" << std::endl;
        assert(quot < 50);
        assert(!pd_full(pd));
        return pd_add_50_for_swap(quot, rem, pd);
        /* const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // assert(popcount128(header) == 50);

        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        const __m512i target = _mm512_set1_epi8(rem);

        write_header(begin, end, header, pd);
        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        // new_header |= ((header >> end) << (end + 1));
        // new_header &= kLeftoverMask;
        // assert(popcount128(new_header) == 50);
        // // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        // //redundent
        // // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 103ul : 0;
        // memcpy(pd, &new_header, kBytes2copy);

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

        // uint64_t j = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

        // assert((j == end_fingerprint) || (rem <= ((const uint8_t *) pd)[kBytes2copy + j]));
        // assert(j == i);

        memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
                &((const uint8_t *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((uint8_t *) pd)[kBytes2copy + i] = rem;

        return 1ul << 15; */
    }


    inline uint64_t pd_add_50_only_rem(uint8_t rem, size_t quot_capacity, __m512i *pd) {
        // assert(quot < 50);
        // assert(!pd_full(pd));
        // const unsigned __int128 *h = (const unsigned __int128 *) pd;
        // constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        // const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        // // assert(popcount128(header) == 50);

        // const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        // const uint64_t end = select128(header, quot);
        // assert(begin <= end);
        // assert(end <= 50 + 51);
        // const __m512i target = _mm512_set1_epi8(rem);

        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        // new_header |= ((header >> end) << (end + 1));
        // assert(popcount128(new_header) == 50);
        // // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        // //redundent
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 101ul : 0;
        // memcpy(pd, &new_header, kBytes2copy);

        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t begin_fingerprint = 51 - quot_capacity;
        const uint64_t end_fingerprint = 51;
        assert(begin_fingerprint < end_fingerprint);
        assert(end_fingerprint <= 51);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const uint8_t *) pd)[kBytes2copy + i]) break;
        }
        assert((i == end_fingerprint) ||
               (rem <= ((const uint8_t *) pd)[kBytes2copy + i]));

        // uint64_t i =
        //         _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

        // assert((i == end_fingerprint) || (rem <= ((const uint8_t *) pd)[kBytes2copy + i]));

        memmove(&((uint8_t *) pd)[kBytes2copy + i + 1],
                &((const uint8_t *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((uint8_t *) pd)[kBytes2copy + i] = rem;

        return 1ul << 15;
    }

    inline auto conditional_remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        /* This function has na error. */
        assert(quot < 50);
        // unsigned __int128 header = 0;
        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;

        if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        // assert(popcount128(header) == 50);
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

        // uint64_t hlb_old = get_header_last_byte(pd);
        // inline void write_header_naive_delete(uint64_t begin, uint64_t end, const unsigned __int128 header, __m512i *pd) {

        write_header_naive_delete(begin, end, header, pd);

        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        // new_header |= ((header >> end) << (end - 1));
        // new_header
        // assert(popcount128(new_header) == 50);
        // new_header |= (did_pd_overflowed(pd)) ? ((unsigned __int128) 1) << 103ul : 0;
        // memcpy(pd, &new_header, kBytes2copy);

        memmove(&((uint8_t *) pd)[kBytes2copy + i],
                &((const uint8_t *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));
        return true;
    }

    inline int my_cmp(int64_t quot1, char rem1, int64_t quot2, char rem2) {
        if (quot1 != quot2)
            return quot1 < quot2;
        return rem1 <= rem2;
    }

    inline uint64_t pd_swap_short_working_slow(int64_t quot, uint8_t rem, __m512i *pd) {
        set_overflow_bit(pd);
        uint64_t valid_after_swap_quot = get_last_quot_after_future_swap(quot, pd);
        uint64_t before_header_last_byte = get_header_last_byte(pd);

        const uint64_t last_quot = decode_last_quot_wrapper(pd);
        const uint64_t old_rem = get_last_byte(pd);

        uint64_t old_qr = (last_quot << 8) | old_rem;
        assert(last_quot < 50);
        const uint64_t new_qr = (quot << 8u) | ((uint64_t) rem);
        assert(old_rem < 256);
        // std::cout << "last_quot: " << last_quot << std::endl;
        // std::cout << "old_rem: " << old_rem << std::endl;
        // std::cout << "old_qr: " << old_qr << std::endl;


        if (old_qr <= new_qr) {
            // std::cout << "swap_1" << std::endl;
            return new_qr;
        }
        //Only remainder update (no quot decreasing).
        else if (last_quot == quot) {
            // std::cout << "swap_2" << std::endl;
            // if (!quot_capacity){
            //     std::cout << "quot: " << quot << std::endl;
            //     v_pd512_plus::print_headers(pd);
            // }
            size_t quot_capacity = get_last_quot_capacity(quot, pd);
            assert(get_specific_quot_capacity(quot, pd));
            pd_add_50_only_rem(rem, quot_capacity, pd);
            return old_qr;
        } else {
            // uint64_t hlb0 = get_header_last_byte(pd);
            // std::cout << "swap_3" << std::endl;
            conditional_remove(last_quot, get_last_byte(pd), pd);
            // uint64_t hlb1 = get_header_last_byte(pd);
            pd_add_50_for_swap(quot, rem, pd);
            // uint64_t hlb2 = get_header_last_byte(pd);

            // // This might be redundent in some cases.
            // decrease_pd_max_quot_after_swap_insertion(quot, last_quot, pd);
            // encode_last_quot_when_full_for_the_first_time(last_quot, quot, pd);
            encode_after_swap_3(valid_after_swap_quot, pd);
            // uint64_t hlb3 = get_header_last_byte(pd);
            assert(decode_last_quot_wrapper(pd) == valid_after_swap_quot);
            return old_qr;
        }
    }

    inline uint64_t pd_swap_short(int64_t quot, uint8_t rem, __m512i *pd) {
        set_overflow_bit(pd);
        // uint64_t valid_after_swap_quot = get_last_quot_after_future_swap(quot, pd);
        // uint64_t before_header_last_byte = get_header_last_byte(pd);

        const uint64_t last_quot = Lookup_Table[(_mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & 127)];
        if (last_quot < quot) {
            return (quot << 8u) | ((uint64_t) rem);
        } else if (last_quot == quot) {
            const uint64_t old_rem = get_last_byte(pd);
            if (old_rem < rem)
                return (quot << 8u) | ((uint64_t) rem);

            size_t quot_capacity = get_last_quot_capacity(quot, pd);
            assert(get_specific_quot_capacity(quot, pd));
            pd_add_50_only_rem(rem, quot_capacity, pd);
            return (last_quot << 8) | old_rem;
        } else {
            const uint64_t old_rem = get_last_byte(pd);
            uint64_t valid_after_swap_quot = get_last_quot_after_future_swap(quot, pd);
            conditional_remove(last_quot, get_last_byte(pd), pd);
            pd_add_50_for_swap(quot, rem, pd);

            encode_after_swap_3(valid_after_swap_quot, pd);
            assert(decode_last_quot_wrapper(pd) == valid_after_swap_quot);
            return (last_quot << 8) | old_rem;
        }
    }

    inline uint64_t pd_conditional_add_50(int64_t quot, uint8_t rem, __m512i *pd) {
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        // const uint64_t before_add_last_quot = decode_last_quot_wrapper(pd);
        // const uint64_t before_add_capacity = get_capacity(pd);
        if ((header_last_byte & 248) == 0) {
            // std::cout << "ca_1" << std::endl;
            uint64_t res = pd_add_50_not_full_after(quot, rem, pd);
            assert(res == (1 << 15));
            update_max_quot_when_pd_is_not_full(quot, pd);
            auto temp = pd_find_50(quot, rem, pd);
            assert(pd_find_50(quot, rem, pd) == Yes);
            return res;
        } else if ((header_last_byte & 248) == 8) {
            // std::cout << "ca_2" << std::endl;
            //std::cout << "before adding" << std::endl;
            // v_pd512_plus::print_h1(0, pd);
            // v_pd512_plus::print_headers_masked(pd);
            // //std::cout << std::endl;
            // const uint64_t last_quot_capacity = get_specific_quot_capacity(last_quot, pd);
            const uint64_t last_quot = get_last_byte(pd);
            assert(get_specific_quot_capacity(last_quot, pd));
            uint64_t res = pd_add_50_full_after(quot, rem, pd);
            assert(res == (1 << 15));
            encode_last_quot_when_full_for_the_first_time(last_quot, quot, pd);
            const uint64_t new_last_quot = decode_last_quot_wrapper(pd);
            assert(get_specific_quot_capacity(new_last_quot, pd));
            assert((new_last_quot == quot) || (new_last_quot == last_quot));
            assert(pd_find_50(quot, rem, pd) == Yes);
            assert(get_capacity(pd) >= 0);
            assert(get_capacity(pd) == 51);
            return res;
            // const uint64_t new_header_last_byte = get_header_last_byte(pd);

            //std::cout << "after adding" << std::endl;
            // v_pd512_plus::print_h1(0, pd);
            //std::cout << std::string(84, '-') << std::endl;

            // auto temp = pd_find_50(quot, rem, pd);
            // if (pd_find_50(quot, rem, pd) != Yes) {
            //     pd_find_50(quot, rem, pd);
            //     assert(0);
            // }
        } else {
            // std::cout << "ca_3" << std::endl;
            assert(pd_full(pd));
            return pd_swap_short(quot, rem, pd);
        }
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

    auto validate_number_of_quotient(const __m512i *pd) -> bool;

    auto get_capacity_old(const __m512i *x) -> size_t;

    auto get_capacity_naive(const __m512i *x) -> size_t;

    auto get_name() -> std::string;

}// namespace pd512_plus


#endif// FILTERS_PD512_PLUS_HPP


// inline uint64_t pd_swap(int64_t quot, uint8_t rem, __m512i *pd) {
//     assert(quot < 50);
//     const size_t capacity = pd_popcount(pd);
//     constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
//     const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;

//     assert(h1);
//     const uint64_t temp = h1 << (64 - 37);
//     assert((temp >> (64 - 37)) == h1);
//     const uint64_t index = _lzcnt_u64(temp);

//     const int64_t old_quot = get_last_quot_in_pd(pd);
//     if (old_quot < quot) {
//std::cout << "pd_swap 0" << std::endl;
//         set_overflow_bit(pd);
//         uint64_t res = (quot << 8u) | ((uint64_t) rem);
//         assert(res);
//         return res;
//     }
//     constexpr int imm1 = 3;
//     constexpr int imm2 = 15;
//     const uint8_t old_rem = _mm_extract_epi8(_mm512_extracti64x2_epi64(*pd, imm1), imm2);


//     if ((old_quot == quot) && (old_rem <= rem)) {
//std::cout << "pd_swap 1" << std::endl;
//         set_overflow_bit(pd);
//         uint64_t res = (quot << 8u) | ((uint64_t) rem);
//         if (!res) {
//             v_pd512::print_headers_masked(pd);
//         }
//         assert(res);
//         return res;
//         // return (quot << 8u) | ((uint8_t)rem);
//     }

//     const uint64_t old_qr = (old_quot << 8) | ((uint64_t) old_rem);
//     const uint64_t new_qr = (quot << 8) | ((uint64_t) rem);
//     if (old_qr <= new_qr) {
//         v_pd512::bin_print(old_qr);
//     }
//     assert(old_qr > new_qr);

//     const __m512i target = _mm512_set1_epi8(old_rem);
//     uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
//     assert(v & (1ULL << 63));

//     //todo: change to -> remove last.
//     conditional_remove(old_quot, old_rem, pd);
//     pd_add_50(quot, rem, pd);
//     set_overflow_bit(pd);
//     uint64_t res = (old_quot << 8u) | ((uint64_t) old_rem);
//     assert(res);
//std::cout << "pd_swap 2" << std::endl;
//     return res;
//     // return (old_quot << 8u) | old_rem;
// }


// inline uint8_t get_last_quot_in_pd_super_naive(const __m512i *pd) {
//     return count_ones_up_to_the_kth_zero(pd);
// }

// inline uint64_t get_last_quot_in_pd_naive_easy_case_helper(const __m512i *pd) {
//     assert(get_hi_meta_bits(pd) <= 32);
//     constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
//     const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
//     const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1) & h1_mask;

//     if (h1 == 0) {
//         uint64_t temp = (~h0) & (h0 >> 1);
//         assert(temp);
//         size_t first_one = _lzcnt_u64(h0);
//         size_t temps_first_one = _lzcnt_u64(temp);
//         assert(temps_first_one > first_one);
//         return temps_first_one - first_one;
//     }

//     uint64_t temp = (~h1) & (h1 >> 1);
//     if (temp == 0) {
//         size_t end = _tzcnt_u64(~h1);
//         size_t begin = _lzcnt_u64(~h0);
//         return end + begin;
//     }
//     assert(_lzcnt_u64(temp) > _lzcnt_u64(h1));
//     return _lzcnt_u64(temp) - _lzcnt_u64(h1);
// }
// inline uint64_t get_last_quot_in_pd_naive_easy_case(const __m512i *pd) {
//     // return get_last_quot_in_pd_naive_easy_case_helper(pd);
//     return QUOT_SIZE - get_last_quot_in_pd_naive_easy_case_helper(pd);
// }


// inline uint8_t get_last_quot_in_pd(const __m512i *pd) {
//     auto a = 50 - get_last_quot_in_pd_naive(pd);
//     auto b = get_last_quot_in_pd_super_naive(pd);
//     if (a != b) {
//         v_pd512::print_headers_masked(pd);
//     }
//     assert(a == b);
//     return 50 - get_last_quot_in_pd_naive(pd);
// }


// inline uint64_t decode_last_quot_in_pd_naive(const __m512i *pd) {
//     const uint64_t header_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
//     const uint64_t hi_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & (32 + 64);
//     if (hi_meta_bits == 0) {
//         uint64_t att = (header_meta_bits & 16) ? 49 : get_last_byte(pd);
//         if (header_meta_bits & 16) {
//             assert(!(header_meta_bits & 8));
//         }

//         // auto capacity = count_zeros_up_to_the_kth_one(pd, att + 1);
//         // assert(capacity == get_capacity(pd));
//std::cout << "h0" << std::endl;
//         // return count_ones_up_to_the_kth_zero(pd, capacity);
//     }
//     if (hi_meta_bits & 32) {
//         const uint64_t reduce_from_max_quot = header_meta_bits >> 6;
//         assert(reduce_from_max_quot < 4);
//std::cout << "h1" << std::endl;
//         return count_ones_up_to_the_kth_zero(pd);
//         // return count_ones_up_to_the_kth_zero(pd, QUOT_SIZE - reduce_from_max_quot);
//     }
//     if (hi_meta_bits == 64) {
//         const uint64_t reduce_from_max_quot = ((header_meta_bits >> 1) & 15) + 4;
//         assert(reduce_from_max_quot >= 4);
//         assert(reduce_from_max_quot <= 19);
//         // assert(QUOT_SIZE - reduce_from_max_quot > 0);
//std::cout << "h2" << std::endl;
//         return count_ones_up_to_the_kth_zero(pd);
//         // return count_ones_up_to_the_kth_zero(pd, QUOT_SIZE - reduce_from_max_quot);
//     }
//     if (hi_meta_bits == 128) {
//         const uint64_t reduce_from_max_quot = (header_meta_bits & 31) + 18;
//         assert(reduce_from_max_quot >= 18);
//         assert(reduce_from_max_quot <= 49);
//std::cout << "h3" << std::endl;
//         return count_ones_up_to_the_kth_zero(pd);
//         // return count_ones_up_to_the_kth_zero(pd, QUOT_SIZE - reduce_from_max_quot);
//     }
//     assert(false);

//     return count_ones_up_to_the_kth_zero(pd);
// }
    