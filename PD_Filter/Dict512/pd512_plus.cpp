/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#include "pd512_plus.hpp"

namespace v_pd512_plus {
    void bin_print_header(uint64_t header) {
        // assert(_mm_popcnt_u64(header) == 32);
        uint64_t b = 1ULL << (64ul - 1u);
        while (b) {
            std::string temp = (b & header) ? "1" : "0";
            std::cout << temp;
            b >>= 1ul;
        }
    }
    auto bin_print_header_spaced(uint64_t header) -> std::string {
        // assert(_mm_popcnt_u64(header) == 32);
        uint64_t b = 1ULL << (64ul - 1u);
        std::string res = "";
        while (b) {
            for (size_t i = 0; i < 4; i++) {
                res += (b & header) ? "1" : "0";
                b >>= 1ul;
            }
            if (b)
                res += ".";
        }
        return res;
    }

    auto bin_print_header_spaced2(uint64_t header) -> std::string {
        // assert(_mm_popcnt_u64(header) == 32);
        uint64_t b = 1ULL << (64ul - 1u);
        std::string res = "";
        size_t counter = 0;
        while (b) {
            for (size_t i = 0; i < 4; i++) {
                res += (b & header) ? "1" : "0";
                b >>= 1ul;
            }
            counter++;
            if (b) {
                bool cond = (counter % 4 == 0);
                res += cond ? "|" : ".";
            }
        }
        return res;
    }

    auto bin_print_header_spaced128(unsigned __int128 header) -> std::string {
        // assert(_mm_popcnt_u64(header) == 32);
        unsigned __int128 b = ((unsigned __int128) 1) << 127ul;
        assert(b > 42);
        std::string res = "";
        while (b) {
            for (size_t i = 0; i < 4; i++) {
                res += (b & header) ? "1" : "0";
                b >>= 1ul;
            }
            if (b)
                res += ".";
        }
        return res;
    }

    void decode_by_table_validator() {
        // constexpr uint8_t table[112] = {45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48, 44, 44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

        constexpr uint8_t table[128] = {
                50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
                49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50, 50, 50, 50,
                45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 46, 46, 46,
                47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 48, 48, 48,
                44, 44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 39, 38, 38, 37, 37,
                36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
                31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
                15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

        for (size_t i = 0; i < 128; i++) {
            std::cout << "i: " << i << std::endl;
            if (table[i] == 50)
                continue;
            auto temp = pd512_plus::decode_last_quot_by_hlb(i);
            if (temp != table[i]) {
                std::cout << "temp: " << temp << std::endl;
                std::cout << "table[i]: " << table[i] << std::endl;
                std::cout << "i: " << i << std::endl;
                assert(false);
            }
        }
    }

}// namespace v_pd512_plus

namespace pd512_plus {

    auto validate_clz(int64_t quot, char rem, const __m512i *pd) -> bool {
        static int counter = 0;
        counter++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);

        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        const uint64_t h0 = ((uint64_t *) pd)[0];
        const uint64_t h1 = ((uint64_t *) pd)[1];

        if (quot == 0) {
            // std::cout << "h0" << std::endl;
            const uint64_t my_end = _tzcnt_u64(header);
            assert(begin == 0);
            assert(end == my_end);
            return true;
        } else if (quot < pop - 1) {
            // std::cout << "h1" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - quot;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else if (quot == pop - 1) {
            // std::cout << "h2" << std::endl;
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t temp = (63 - _lzcnt_u64(h0)) - quot;
            const uint64_t my_end = (63 - leading_one_index) - quot;
            const uint64_t my_begin = (63 - next_leading_one_index) - quot + 1;
            assert(my_begin <= my_end);
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else if (quot == pop) {
            // std::cout << "h3" << std::endl;
            const uint64_t my_begin = (63 - _lzcnt_u64(h0)) - quot + 1;
            // const uint64_t my_begin = (64 - _lzcnt_u64(h0)) - quot;
            const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else if (quot == pop + 1) {
            // std::cout << "h4" << std::endl;
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t my_begin = 64 - quot + temp;
            const uint64_t my_end = my_begin + _tzcnt_u64(h1 >> temp);
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else {
            // std::cout << "h5" << std::endl;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - pop - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - quot + 64;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);

            return true;
        }
    }
    auto validate_clz_old(int64_t quot, char rem, const __m512i *pd) -> bool {
        static int counter = 0;
        counter++;
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);

        const __m512i target = _mm512_set1_epi8(rem);
        const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;

        /*switch (quot )
        {
        case 0:
            assert(begin == 0);
            const uint64_t my_end = _tzcnt_u64(header);
            assert(end == my_end);
            break;
        case 1:
        
        default:
            break;
        } */

        if (quot == 0) {
            assert(begin == 0);
            const uint64_t my_end = _tzcnt_u64(header);
            assert(end == my_end);
            return true;
        } else if (quot < pop - 1) {
            const uint64_t small_header = ((uint64_t *) pd)[0];
            auto pre1 = _pdep_u64(UINT64_C(1) << (quot - 1), small_header);
            auto pre2 = _pdep_u64(UINT64_C(1) << (quot), small_header);

            auto mid1 = _tzcnt_u64(pre1);
            auto mid2 = _tzcnt_u64(pre2);

            const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
            const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));

            const uint64_t safe_begin2 = safe_begin - quot + 1;
            const uint64_t safe_end2 = safe_end - quot;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t mid_val = y >> temp;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - quot;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50(quot, rem, pd));
            return true;
        } else if (quot == pop - 1) {
            const uint64_t h0 = ((uint64_t *) pd)[0];
            // const uint64_t h1 = ((uint64_t *) pd)[1];
            const uint64_t leading_one_index = _lzcnt_u64(h0);
            const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
            const uint64_t temp = (63 - _lzcnt_u64(h0)) - quot;
            const uint64_t my_end = (63 - leading_one_index) - quot;
            const uint64_t my_begin = (63 - next_leading_one_index) - quot + 1;
            assert(my_begin <= my_end);
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else if (quot == pop) {
            const uint64_t h0 = ((uint64_t *) pd)[0];
            const uint64_t h1 = ((uint64_t *) pd)[1];
            const uint64_t my_begin = (64 - _lzcnt_u64(h0)) - quot;
            const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else if (quot == pop + 1) {
            // const uint64_t h0 = ((uint64_t *) pd)[0];
            const uint64_t h1 = ((uint64_t *) pd)[1];
            const uint64_t temp = _tzcnt_u64(h1) + 1;
            const uint64_t my_begin = 64 - quot + temp;
            const uint64_t my_end = my_begin + _tzcnt_u64(h1 >> temp);
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else {
            const int64_t h0_pop = _mm_popcnt_u64(((uint64_t *) pd)[0]);
            const int64_t small_header = ((uint64_t *) pd)[1];
            const uint64_t old_quot = quot;
            quot -= h0_pop;
            const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
            const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - old_quot + 64;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);

            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50(quot, rem, pd));
            return true;
        }

        // return validate_clz_helper(quot, rem, pd);
        /* } else if (quot - 1 < pop) {
            const uint64_t h0 = ((uint64_t *) pd)[0];
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);

            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50(quot, rem, pd));
            return true;
        } else if (pop == quot - 1) {
            const uint64_t h0 = ((uint64_t *) pd)[0];
            const uint64_t h1 = ((uint64_t *) pd)[1];
            const uint64_t my_begin = (64 - _lzcnt_u64(h0)) - quot + 1;
            const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
            assert(my_begin == begin);
            assert(my_end == end);
            return true;
        } else {
            const uint64_t h1 = ((uint64_t *) pd)[1];
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h1);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            assert(res == pd_find_50(quot, rem, pd));
            return true;
        } */
    }

    auto validate_clz_helper(int64_t quot, char rem, const __m512i *pd) -> bool {
        // const __m512i target = _mm512_set1_epi8(rem);
        // const uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        // // if (!v) return false;

        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;

        const int64_t pop = _mm_popcnt_u64(header);
        const int64_t h0_pop = _mm_popcnt_u64(((uint64_t *) pd)[0]);
        const int64_t h1_pop = _mm_popcnt_u64(((uint64_t *) pd)[1]);
        assert(h0_pop == pop);
        const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
        const uint64_t end = select128withPop64(header, quot, pop) - quot;
        const uint64_t h0 = ((uint64_t *) pd)[0];
        const uint64_t h1 = ((uint64_t *) pd)[1];
        const uint64_t small_header = (quot - 1 < pop) ? h0 : h1;
        if (quot - 1 < pop) {
            auto pre1 = _pdep_u64(UINT64_C(1) << (quot - 1), small_header);
            auto pre2 = _pdep_u64(UINT64_C(1) << (quot), small_header);

            auto mid1 = _tzcnt_u64(pre1);
            auto mid2 = _tzcnt_u64(pre2);

            const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
            const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));

            const uint64_t safe_begin2 = safe_begin - quot + 1;
            const uint64_t safe_end2 = safe_end - quot;
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t mid_val = y >> temp;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - quot;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);
            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50(quot, rem, pd));
            return true;
        } else {
            const uint64_t old_quot = quot;
            quot -= h0_pop;
            const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
            const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));
            const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
            const uint64_t temp = _tzcnt_u64(y) + 1;
            const uint64_t diff = _tzcnt_u64(y >> temp);
            const uint64_t my_begin = temp - old_quot + 64;
            const uint64_t my_end = my_begin + diff;
            assert(my_begin == begin);
            assert(my_end == end);

            // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
            // assert(res == pd_find_50(quot, rem, pd));
            return true;
        }
    }

    /* const uint64_t end2 = begin + lzcnt_u128(header >> (begin + quot));
        const uint64_t end4 = begin + lzcnt_u128(header >> (begin));
        const uint64_t end22 = begin + lzcnt_u128(header);
        const uint64_t end3 = begin + tzcnt_u128(header >> begin);
        const uint64_t end33 = begin + tzcnt_u128(header);
        const uint64_t end5 = end33 - 63;
        const uint64_t end6 = begin + lzcnt_u128(header >> (begin + quot)) + (quot - 128);
        if (end2 == end) return;
        if (end6 == end) return;

        std::cout << "quot: " << quot << std::endl;
        std::cout << "begin: " << begin << std::endl;
        std::cout << "end: " << end << std::endl;
        std::cout << "end2: " << end2 << std::endl;
        std::cout << "end22: " << end22 << std::endl;
        std::cout << "end3: " << end3 << std::endl;
        std::cout << "end33: " << end33 << std::endl;
        std::cout << "end4: " << end4 << std::endl;
        std::cout << "end5: " << end5 << std::endl;
        std::cout << "end6: " << end6 << std::endl;
        std::cout << std::string(64, '@') << std::endl;
        if (begin == 0) return;
        if (end5 == end) return;
        if (end6 == end) return;
        assert(0); */
    // }// namespace pd512
    void print512(const __m512i *var) {
        uint64_t val[8];
        memcpy(val, var, 64);
        printf("[%zu, %zu, %zu, %zu, %zu, %zu, %zu, %zu] \n",
               val[0], val[1], val[2], val[3], val[4], val[5],
               val[6], val[7]);
        //     long long unsigned int val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        //     assert(sizeof(val) == 64);
        //     memcpy(val, &var, sizeof(val));
        //     for (size_t i = 0; i < 8; i++) {
        //         std::cout << val[i] << ", ";
        //         /* code */
        //     }
        //     std::cout << std::endl;
        // }

        /* bool
    is_aligned(const __m512i *ptr) noexcept
    {
        auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
        return !(iptr % alignof(T));
    } */

        // cout <<
        // printf("Numerical: %i %i %i %i %i %i %i %i \n",
        //        val[0], val[1], val[2], val[3], val[4], val[5],
        //        val[6], val[7]);
    }

    auto validate_number_of_quotient(const __m512i *pd) -> bool {
        // std::cout << "h128: " << std::endl;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        static_assert(kBytes2copy == 13);
        uint64_t header_last_byte = get_header_last_byte(pd);
        assert((header_last_byte & (32 + 64)) == 0);
        // assert(!get_hi_meta_bits(pd));
        unsigned __int128 header = 0;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, pd, kBytes2copy);
        auto my_temp = popcount128(header);

        // std::cout << "my_temp: " << my_temp << std::endl;
        // Number of bits to keep. Requires little-endianness
        // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        size_t res = popcount128(header);
        if (res != 50) {
            std::cout << "popcount128: " << res << std::endl;
        }
        return popcount128(header) == 50;
        // return true;
    }

    auto get_capacity(const __m512i *pd) -> int {
        // return count_ones_up_to_the_kth_zero(pd);
        //todo: this case analysis does not have to be efficient, because this function is only used for debugging.
        constexpr uint64_t h1_mask = ((1ULL << (101 - 64)) - 1);
        const uint64_t header_last_byte = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12);
        const uint64_t mid_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & (32 + 64);
        if ((header_last_byte & (32 + 64)) == 0) {
            if (header_last_byte & 16) {
                // critical. (otherwise decoding is invalid)
                assert(!(header_last_byte & 8));
                return 51;
            }
            int last_quot = get_last_byte(pd);
            assert(last_quot < 50);
            return count_zeros_up_to_the_kth_one(pd, last_quot + 1);
        }
        auto last_quot = decode_last_quot_wrapper(pd);
        assert(last_quot < 50);
        return count_zeros_up_to_the_kth_one(pd, last_quot + 1);

        /* 
        // const uint64_t hi_meta_bits = _mm_extract_epi8(_mm512_castsi512_si128(*pd), 12) & (32 + 64);
        // switch (mid_meta_bits)
        // {
        // case 0:
        //     return get_capacity_easy_case(pd);
        //     break;
        // case 32:

        // default:
        //     break;
        // }
        if (mid_meta_bits == 0)
            return get_capacity_easy_case(pd);

        if (mid_meta_bits & 32) {
            const uint64_t reduce_from_max_quot = ((header_meta_bits >> 3) & 3) + 1;
            assert(reduce_from_max_quot <= 4);
            return count_zeros_up_to_the_kth_one(pd, QUOT_SIZE - reduce_from_max_quot);
        }
        if (mid_meta_bits == 64) {
            const uint64_t reduce_from_max_quot = ((header_meta_bits >> 1) & 15) + 4;
            assert(reduce_from_max_quot >= 4);
            assert(reduce_from_max_quot <= 19);
            // assert(QUOT_SIZE - reduce_from_max_quot > 0);
            return count_zeros_up_to_the_kth_one(pd, QUOT_SIZE - reduce_from_max_quot);
        }
        if (mid_meta_bits == 128) {
            const uint64_t reduce_from_max_quot = (header_meta_bits & 31) + 18;
            assert(reduce_from_max_quot >= 18);
            assert(reduce_from_max_quot <= 49);
            return count_zeros_up_to_the_kth_one(pd, QUOT_SIZE - reduce_from_max_quot);
        }
        assert(false);
        // switch (header_meta_bits & 224) {
        //     case 0:
        //         return get_capacity_easy_case(pd);
        //         break;
        //     case (32):
        //         return get_capacity_easy_case(pd);
        //         break;
        //     case (64):
        //         return get_capacity_easy_case(pd);
        //         break;
        //     case (128):
        //         break;
        //     case (160):
        //         break;
        //     case (192):
        //         break;
        //     case (224):
        //         break;
        //     default:
        //         assert(false);
        //         break;
        //     // case(64):
        //     // return get_capacity_easy_case(pd);
        //     // break;
        //     default:
        //         assert(false);
        //         break;
        // }
        // return -1; */
    }

    auto get_specific_quot_capacity(int64_t quot, const __m512i *pd) -> int {
        if (quot == 0) {
            return count_zeros_up_to_the_kth_one(pd, 1);
        }
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        const unsigned __int128 header = *h;
        auto index = count_zeros_up_to_the_kth_one(pd, quot) + quot;
        unsigned __int128 b = ((unsigned __int128) 1) << (index);
        // assert(header & b);
        // b <<= 1;
        auto res = 129;
        for (size_t i = index; i < 128; i++) {
            if (b & header) {
                res = i - (index);
                break;
            }
            b <<= 1;
        }
        b = ((unsigned __int128) 1) << (index);
        size_t counter = 0;
        while (b) {
            if (b & header) {
                assert(res == counter);
                return counter;
            }
            counter++;
            b <<= 1;
        }
        assert(false);
        return -42;
    }

    auto get_capacity_old(const __m512i *x) -> size_t {
        validate_number_of_quotient(x);
        // return get_capacity_naive();
        uint64_t header[2];
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        memcpy(header, x, 13);

        auto old_header = header[1];
        // header[1] = header[1] & ((UINT64_C(1) << (50 + 51 - 64)) - 1);

        auto h0_one_count = _mm_popcnt_u64(header[0]);
        if (h0_one_count == 50) {
            auto index = pd512_plus::select64(header[0], 49);
            assert(index < 64);
            auto res = index - 49;
            assert(res == get_capacity_naive(x));
            return res;
        } else {
            auto h0_zero_count = 64 - h0_one_count;
            auto ones_left = 49 - h0_one_count;
            // bool cond =
            assert(ones_left >= 0);
            auto index = pd512_plus::select64(header[1], 49 - h0_one_count);
            auto att_index = pd512_plus::select64(header[1], 49 - h0_one_count - 1);
            if (index == 64) {
                std::cout << header[0] << std::endl;
                std::cout << header[1] << std::endl;
                std::cout << old_header << std::endl;
                std::cout << h0_one_count << std::endl;
                std::cout << ones_left << std::endl;
                std::cout << get_capacity_naive(x) << std::endl;
                return -1;
                assert(false);
            }
            assert(index < 64);
            assert(index <= 37);
            assert(index >= ones_left);
            auto res = index - ones_left + h0_zero_count;
            bool cond = res == get_capacity_naive(x);
            if (!cond) {
                std::cout << "h0_zero_count " << h0_zero_count << std::endl;
                std::cout << "h0_one_count " << h0_one_count << std::endl;
                std::cout << res << std::endl;
                std::cout << get_capacity_naive(x) << std::endl;
                return -1;
                assert(false);
            }
            return (res == get_capacity_naive(x)) ? res : -1;
            // assert(res == get_capacity_naive(x));
            // return res;
            return index - ones_left;
        }
    }

    auto get_capacity_naive_with_OF_bit(const __m512i *x) -> size_t {
        uint64_t header[2];
        memcpy(header, x, 13);
        header[1] &= ((1ULL << (101 - 64)) - 1);
        size_t zero_count = 0, one_count = 0;
        for (size_t j = 0; j < 2; j++) {
            uint64_t temp = header[j];
            uint64_t b = 1ULL;
            for (size_t i = 0; i < 64; i++) {
                if (b & temp) {
                    one_count++;
                    if (one_count == 50)
                        return zero_count;
                } else {
                    zero_count++;
                }
                b <<= 1ul;
            }
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false);
    }

    auto get_capacity_naive(const __m512i *x) -> size_t {
        uint64_t header[2];
        memcpy(header, x, 13);
        size_t zero_count = 0, one_count = 0;
        for (size_t j = 0; j < 2; j++) {
            uint64_t temp = header[j];
            uint64_t b = 1ULL;
            for (size_t i = 0; i < 64; i++) {
                if (b & temp) {
                    one_count++;
                    if (one_count == 50)
                        return zero_count;
                } else {
                    zero_count++;
                }
                b <<= 1ul;
            }
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false);
    }

    auto count_ones_up_to_the_kth_zero(const __m512i *x, size_t k) -> size_t {
        assert(k > 0);
        assert(k <= 51);

        uint64_t header[2];
        memcpy(header, x, 13);
        // uint64_t masked_h1 = (header[1]) & ((1ULL << (101 - 64)) - 1);


        // size_t total_one_count_in_header = _mm_popcnt_u64(header[0]) +_mm_popcnt_u64(masked_h1);
        // assert(total_one_count_in_header == 50);

        size_t total_one_count = _mm_popcnt_u64(header[0]) + _mm_popcnt_u64(header[1]);
        size_t zero_count = 0, one_count = 0;
        for (size_t j = 0; j < 2; j++) {
            uint64_t temp = header[j];
            uint64_t b = 1ULL;
            for (size_t i = 0; i < 64; i++) {
                if (b & temp) {
                    one_count++;
                } else {
                    zero_count++;
                    if (zero_count == k)
                        return one_count;
                }
                b <<= 1ul;
            }
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false);
    }


    auto count_zeros_up_to_the_kth_one(const __m512i *x, size_t k) -> size_t {
        assert(k > 0);
        uint64_t header[2];
        memcpy(header, x, 13);
        size_t total_one_count = _mm_popcnt_u64(header[0]) + _mm_popcnt_u64(header[1]);
        assert(total_one_count >= k);
        size_t zero_count = 0, one_count = 0;
        for (size_t j = 0; j < 2; j++) {
            uint64_t temp = header[j];
            uint64_t b = 1ULL;
            for (size_t i = 0; i < 64; i++) {
                if (b & temp) {
                    one_count++;
                    if (one_count == k)
                        return zero_count;
                } else {
                    zero_count++;
                }
                b <<= 1ul;
            }
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false);
    }


    auto get_name() -> std::string {
        return "pd512 ";
    }

    auto remove_naive(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(false);
        return false;
    }
};// namespace pd512_plus
