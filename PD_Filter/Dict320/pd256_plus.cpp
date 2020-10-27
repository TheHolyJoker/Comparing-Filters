/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#include "pd256_plus.hpp"

namespace v_pd256_plus {

    void p_format_header(const __m256i *pd) {
        uint64_t header = pd256_plus::get_header(pd);
        p_format_header(header);
        /*        std::string s = to_bin(header, 64 - 8);
        assert(s.substr(0, s.size()) == s.substr(0, -1));
        std::string new_s = s.substr(0, 3) + "," + s.substr(3, 3) + "|" + space_string(s.substr(6, s.size()));
        std::cout << new_s << std::endl;*/
    }

    void p_format_header(uint64_t header) {
        std::string s = to_bin(header, 64 - 8);
        assert(s.substr(0, s.size()) == s.substr(0, -1));
        std::string new_s = s.substr(0, 3) + "," + s.substr(3, 3) + "|" + space_string(s.substr(6, s.size()));
        std::cout << new_s << std::endl;
    }

    void p_format_word(uint64_t x) {
        std::string res = to_bin(x, 64);
        std::cout << space_string(res) << std::endl;
        //        uint64_t b = 1ULL;
        //        std::string res = "";
        //        for (size_t i = 0; i < 64; i++) {
        //            res += (b & x) ? "1" : "0";
        //            b <<= 1ul;
        //            if ((i % 4 == 0) && b > x) {
        //                break;
        //            }
        //        }
    }

    auto to_bin(uint64_t x, size_t length) -> std::string {
        assert(length <= 64);
        uint64_t b = 1ULL;
        std::string res = "";
        for (size_t i = 0; i < length; i++) {
            res += (b & x) ? "1" : "0";
            b <<= 1ul;
        }
        return res;
    }

    // From CuckooFilter repository, where this following message was written:
    //
    // inspired from
    // http://www-graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
    uint64_t upperpower2(uint64_t x) {
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        x++;
        return x;
    }

    auto space_string(std::string s) -> std::string {
        std::string new_s = "";
        for (size_t i = 0; i < s.size(); i += 4) {
            if (i) {
                if (i % 16 == 0) {
                    new_s += "|";
                } else if (i % 4 == 0) {
                    new_s += ".";
                }
            }
            new_s += s.substr(i, 4);
        }
        return new_s;
    }


    auto to_bin(uint64_t x) -> std::string {
        assert(x);
        size_t p = upperpower2(x);
        assert(p);
        uint64_t b = p >> 1;
        assert(b);
        std::string res = "";
        while (b) {
            res += (b & x) ? "1" : "0";
            b >>= 1ul;
        }
        return res;
    }

    auto to_bin_reversed(uint64_t x) -> std::string {
        size_t p = upperpower2(x);
        assert(p);
        uint64_t b = 1ULL;
        // size_t index = 0;
        std::string res = "";
        while (b < p) {
            res += (b & x) ? "1" : "0";
            b <<= 1ul;
        }
        // assert((b << 1) == (1ULL << p));
        return res;
    }

    auto to_bin_reversed2(uint64_t x) -> std::string {
        // size_t p = upperpower2(x);
        // if (!p)
        // p = 64;
        size_t index = 0;
        uint64_t b = 1ULL;
        // size_t index = 0;
        std::string res = "";
        while (b) {
            res += (b & x) ? "1" : "0";
            b <<= 1ul;
        }
        // assert((b << 1) == (1ULL << p));
        return res;
    }


    auto to_bin_reversed(const unsigned __int128 header) -> std::string {
        uint64_t hi = header >> 64;
        uint64_t lo = header & 0xffff'ffff'ffff'ffff;
        auto s1 = to_bin_reversed2(hi);
        s1 = space_string(s1);
        auto s2 = to_bin_reversed2(lo);
        s2 = space_string(s2);
        return s1 + "-" + s2;
    }


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


    auto test_bit(size_t index, const __m256i *pd) -> bool {
        size_t sixteen_index = index / 16;
        size_t rel_index = index & 15;

        uint16_t *h_array = ((uint16_t *) pd);

        return h_array[sixteen_index] & (1 << rel_index);
        // uint64_t res = h_array[7] >> (64 - 8);
    }

    void print_hlb(const __m256i *pd) {
        uint64_t hlb = _mm_extract_epi8(_mm256_castsi256_si128(*pd), 12);
        std::string s = to_bin_reversed(hlb);
        std::string final_s = s.substr(0, 4) + "." + s.substr(4, 1) + "," + s.substr(5, 3);
        std::cout << "lhb:\t" << final_s << std::endl;
        // std::cout << "hlb: " << (hlb & 15) << "." << (hlb & 16) << (hlb & 16) << std::endl;
    }

    void print_headers(const __m256i *pd) {
        // constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm256_castsi256_si128(*pd), 1);
        std::cout << "h0: " << bin_print_header_spaced2(h0) << std::endl;
        std::cout << "h1: " << bin_print_header_spaced2(h1) << std::endl;
    }
}// namespace v_pd256_plus

namespace pd256_plus {

    // void validate_clz(int64_t quot, char rem, const __m256i *pd);

    // auto validate_clz_old(int64_t quot, char rem, const __m256i *pd) -> bool {
    //     static int counter = 0;
    //     counter++;
    //     assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
    //     assert(quot < 50);

    //     const __m256i target = _mm256_set1_epi8(rem);
    //     const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 13ul;
    //     // if (!v) return false;

    //     const unsigned __int128 *h = (const unsigned __int128 *) pd;
    //     constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
    //     const unsigned __int128 header = (*h) & kLeftoverMask;

    //     const int64_t pop = _mm_popcnt_u64(header);
    //     const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
    //     const uint64_t end = select128withPop64(header, quot, pop) - quot;

    //     /*switch (quot )
    //     {
    //     case 0:
    //         assert(begin == 0);
    //         const uint64_t my_end = _tzcnt_u64(header);
    //         assert(end == my_end);
    //         break;
    //     case 1:

    //     default:
    //         break;
    //     } */

    //     if (quot == 0) {
    //         assert(begin == 0);
    //         const uint64_t my_end = _tzcnt_u64(header);
    //         assert(end == my_end);
    //         return true;
    //     } else if (quot < pop - 1) {
    //         const uint64_t small_header = ((uint64_t *) pd)[0];
    //         auto pre1 = _pdep_u64(UINT64_C(1) << (quot - 1), small_header);
    //         auto pre2 = _pdep_u64(UINT64_C(1) << (quot), small_header);

    //         auto mid1 = _tzcnt_u64(pre1);
    //         auto mid2 = _tzcnt_u64(pre2);

    //         const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
    //         const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));

    //         const uint64_t safe_begin2 = safe_begin - quot + 1;
    //         const uint64_t safe_end2 = safe_end - quot;
    //         const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
    //         const uint64_t temp = _tzcnt_u64(y) + 1;
    //         const uint64_t mid_val = y >> temp;
    //         const uint64_t diff = _tzcnt_u64(y >> temp);
    //         const uint64_t my_begin = temp - quot;
    //         const uint64_t my_end = my_begin + diff;
    //         assert(my_begin == begin);
    //         assert(my_end == end);
    //         // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
    //         // assert(res == pd_find_50(quot, rem, pd));
    //         return true;
    //     } else if (quot == pop - 1) {
    //         const uint64_t h0 = ((uint64_t *) pd)[0];
    //         // const uint64_t h1 = ((uint64_t *) pd)[1];
    //         const uint64_t leading_one_index = _lzcnt_u64(h0);
    //         const uint64_t next_leading_one_index = _lzcnt_u64(h0 ^ (1ull << (63ul - leading_one_index)));
    //         const uint64_t temp = (63 - _lzcnt_u64(h0)) - quot;
    //         const uint64_t my_end = (63 - leading_one_index) - quot;
    //         const uint64_t my_begin = (63 - next_leading_one_index) - quot + 1;
    //         assert(my_begin <= my_end);
    //         assert(my_begin == begin);
    //         assert(my_end == end);
    //         return true;
    //     } else if (quot == pop) {
    //         const uint64_t h0 = ((uint64_t *) pd)[0];
    //         const uint64_t h1 = ((uint64_t *) pd)[1];
    //         const uint64_t my_begin = (64 - _lzcnt_u64(h0)) - quot;
    //         const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
    //         assert(my_begin == begin);
    //         assert(my_end == end);
    //         return true;
    //     } else if (quot == pop + 1) {
    //         // const uint64_t h0 = ((uint64_t *) pd)[0];
    //         const uint64_t h1 = ((uint64_t *) pd)[1];
    //         const uint64_t temp = _tzcnt_u64(h1) + 1;
    //         const uint64_t my_begin = 64 - quot + temp;
    //         const uint64_t my_end = my_begin + _tzcnt_u64(h1 >> temp);
    //         assert(my_begin == begin);
    //         assert(my_end == end);
    //         return true;
    //     } else {
    //         const int64_t h0_pop = _mm_popcnt_u64(((uint64_t *) pd)[0]);
    //         const int64_t small_header = ((uint64_t *) pd)[1];
    //         const uint64_t old_quot = quot;
    //         quot -= h0_pop;
    //         const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
    //         const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));
    //         const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
    //         const uint64_t temp = _tzcnt_u64(y) + 1;
    //         const uint64_t diff = _tzcnt_u64(y >> temp);
    //         const uint64_t my_begin = temp - old_quot + 64;
    //         const uint64_t my_end = my_begin + diff;
    //         assert(my_begin == begin);
    //         assert(my_end == end);

    //         // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
    //         // assert(res == pd_find_50(quot, rem, pd));
    //         return true;
    //     }

    //     // return validate_clz_helper(quot, rem, pd);
    //     /* } else if (quot - 1 < pop) {
    //         const uint64_t h0 = ((uint64_t *) pd)[0];
    //         const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h0);

    //         const uint64_t temp = _tzcnt_u64(y) + 1;
    //         const uint64_t diff = _tzcnt_u64(y >> temp);
    //         bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
    //         assert(res == pd_find_50(quot, rem, pd));
    //         return true;
    //     } else if (pop == quot - 1) {
    //         const uint64_t h0 = ((uint64_t *) pd)[0];
    //         const uint64_t h1 = ((uint64_t *) pd)[1];
    //         const uint64_t my_begin = (64 - _lzcnt_u64(h0)) - quot + 1;
    //         const uint64_t my_end = 64 + _tzcnt_u64(h1) - quot;
    //         assert(my_begin == begin);
    //         assert(my_end == end);
    //         return true;
    //     } else {
    //         const uint64_t h1 = ((uint64_t *) pd)[1];
    //         const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), h1);
    //         const uint64_t temp = _tzcnt_u64(y) + 1;
    //         const uint64_t diff = _tzcnt_u64(y >> temp);
    //         bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
    //         assert(res == pd_find_50(quot, rem, pd));
    //         return true;
    //     } */
    // }

    // auto validate_clz_helper(int64_t quot, char rem, const __m256i *pd) -> bool {
    //     // const __m256i target = _mm256_set1_epi8(rem);
    //     // const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 13ul;
    //     // // if (!v) return false;

    //     const unsigned __int128 *h = (const unsigned __int128 *) pd;
    //     constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
    //     const unsigned __int128 header = (*h) & kLeftoverMask;

    //     const int64_t pop = _mm_popcnt_u64(header);
    //     const int64_t h0_pop = _mm_popcnt_u64(((uint64_t *) pd)[0]);
    //     const int64_t h1_pop = _mm_popcnt_u64(((uint64_t *) pd)[1]);
    //     assert(h0_pop == pop);
    //     const uint64_t begin = (quot ? (select128withPop64(header, quot - 1, pop) + 1) : 0) - quot;
    //     const uint64_t end = select128withPop64(header, quot, pop) - quot;
    //     const uint64_t h0 = ((uint64_t *) pd)[0];
    //     const uint64_t h1 = ((uint64_t *) pd)[1];
    //     const uint64_t small_header = (quot - 1 < pop) ? h0 : h1;
    //     if (quot - 1 < pop) {
    //         auto pre1 = _pdep_u64(UINT64_C(1) << (quot - 1), small_header);
    //         auto pre2 = _pdep_u64(UINT64_C(1) << (quot), small_header);

    //         auto mid1 = _tzcnt_u64(pre1);
    //         auto mid2 = _tzcnt_u64(pre2);

    //         const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
    //         const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));

    //         const uint64_t safe_begin2 = safe_begin - quot + 1;
    //         const uint64_t safe_end2 = safe_end - quot;
    //         const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
    //         const uint64_t temp = _tzcnt_u64(y) + 1;
    //         const uint64_t mid_val = y >> temp;
    //         const uint64_t diff = _tzcnt_u64(y >> temp);
    //         const uint64_t my_begin = temp - quot;
    //         const uint64_t my_end = my_begin + diff;
    //         assert(my_begin == begin);
    //         assert(my_end == end);
    //         // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
    //         // assert(res == pd_find_50(quot, rem, pd));
    //         return true;
    //     } else {
    //         const uint64_t old_quot = quot;
    //         quot -= h0_pop;
    //         const uint64_t safe_begin = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot - 1), small_header));
    //         const uint64_t safe_end = _tzcnt_u64(_pdep_u64(UINT64_C(1) << (quot), small_header));
    //         const uint64_t y = _pdep_u64(UINT64_C(3) << (quot - 1), small_header);
    //         const uint64_t temp = _tzcnt_u64(y) + 1;
    //         const uint64_t diff = _tzcnt_u64(y >> temp);
    //         const uint64_t my_begin = temp - old_quot + 64;
    //         const uint64_t my_end = my_begin + diff;
    //         assert(my_begin == begin);
    //         assert(my_end == end);

    //         // bool res = diff && ((v >> (temp - quot)) & ((UINT64_C(1) << diff) - 1));
    //         // assert(res == pd_find_50(quot, rem, pd));
    //         return true;
    //     }
    // }

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
    // }// namespace pd256
    void print256(const __m256i *var) {
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
    is_aligned(const __m256i *ptr) noexcept
    {
        auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
        return !(iptr % alignof(T));
    } */

        // cout <<
        // printf("Numerical: %i %i %i %i %i %i %i %i \n",
        //        val[0], val[1], val[2], val[3], val[4], val[5],
        //        val[6], val[7]);
    }

    // auto validate_number_of_quotient(const __m256i *pd) -> bool {
    //     // std::cout << "h128: " << std::endl;
    //     constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
    //     static_assert(kBytes2copy == 13);
    //     uint64_t header_last_byte = get_header_last_byte(pd);
    //     assert((header_last_byte & (32 + 64)) == 0);
    //     // assert(!get_hi_meta_bits(pd));
    //     unsigned __int128 header = 0;
    //     assert(kBytes2copy < sizeof(header));
    //     memcpy(&header, pd, kBytes2copy);
    //     auto my_temp = popcount128(header);

    //     // std::cout << "my_temp: " << my_temp << std::endl;
    //     // Number of bits to keep. Requires little-endianness
    //     // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
    //     const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
    //     header = header & kLeftoverMask;
    //     size_t res = popcount128(header);
    //     if (res != 50) {
    //         std::cout << "popcount128: " << res << std::endl;
    //     }
    //     return popcount128(header) == 50;
    //     // return true;
    // }

    auto get_capacity_naive(const __m256i *x) -> size_t {
        uint64_t header = get_clean_header(x);
        return count_zeros_up_to_the_kth_one(header, 25 - 1);
    }

    auto get_capacity(const __m256i *pd) -> int {
        // static int counter = 0;
        // counter++;
        // return count_ones_up_to_the_kth_zero(pd);
        //todo: this case analysis does not have to be efficient, because this function is only used for debugging.
        const uint64_t header = get_clean_header(pd);
        auto temp = _lzcnt_u64(header);
        assert(temp >= 14);
        assert(temp <= 39);
        uint64_t res = CAPACITY25 - (temp - 14);
        // auto v_res = get_capacity_naive(pd);
        // if (res != get_capacity_naive(pd)) {
        //     std::cout << std::string(80, '^') << std::endl;
        //     std::cout << "res: " << res << std::endl;
        //     std::cout << "v_cap: " << get_capacity_naive(pd) << std::endl;
        //     v_pd256_plus::p_format_header(pd);
        //     std::cout << std::string(80, '^') << std::endl;

        //     assert(0);
        // }
        assert(res == get_capacity_naive(pd));
        return res;
    }
    auto get_specific_quot_capacity_naive2(int64_t quot, const __m256i *pd) -> int {
        assert(quot < QUOT_SIZE25);
        assert(quot <= _mm_popcnt_u64(get_clean_header(pd)));
        if (quot == 0) {
            return count_zeros_up_to_the_kth_one(pd, 0);
        }
        return count_zeros_up_to_the_kth_one(pd, quot) - count_zeros_up_to_the_kth_one(pd, quot - 1);
    }

    auto get_specific_quot_capacity_naive(int64_t quot, const __m256i *pd) -> int {
        assert(quot < 25);
        if (quot == 0) {
            return count_zeros_up_to_the_kth_one(pd, 0);
        }
        const uint64_t header = get_clean_header(pd);
        assert(quot < _mm_popcnt_u64(header));

        size_t zero_count = 0, one_count = 0;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & header) {
                one_count++;
                if (one_count == quot - 1)
                    break;
            }
            b <<= 1ul;
        }
        if (one_count != quot - 1) {
            std::cout << zero_count << std::endl;
            std::cout << one_count << std::endl;
            return -1;
            assert(false);
        }

        size_t counter = 0;
        b <<= 1ul;
        while (!(b & header)) {
            counter++;
            b <<= 1ul;
        }
        return counter;
    }
    // auto get_specific_quot_capacity(int64_t quot, const __m256i *pd) -> int {
    //     auto res = select64(header, quot) - (select64(header, quot - 1) + 1);

    // }

    auto get_specific_quot_capacity_wrapper(int64_t quot, const __m256i *pd) -> int {
        static int counter = 0;
        assert(quot < 25);
        if (quot == 0) {
            return count_zeros_up_to_the_kth_one(pd, 0);
        } else if (quot == 1) {
            return count_zeros_up_to_the_kth_one(pd, 1) - count_zeros_up_to_the_kth_one(pd, 0);
        }
        counter++;
        const uint64_t header = get_clean_header(pd);
        const uint64_t pop = _mm_popcnt_u64(header);
        assert(quot <= pop);
        uint64_t a = select64(header, quot);
        uint64_t b = select64(header, quot - 1);
        uint64_t c = select64(header, quot - 2);
        auto res = select64(header, quot) - (select64(header, quot - 1) + 1);
        // auto v_res = get_specific_quot_capacity_naive(quot, pd);
        auto v_res2 = get_specific_quot_capacity_naive2(quot, pd);
        bool cond = (res == v_res2);
        // bool cond = (res == v_res) && (res == v_res2);
        if (!cond) {
            std::cout << std::string(80, '*') << std::endl;
            v_pd256_plus::p_format_header(pd);
            if (counter == 1) {
                std::cout << std::string(80, '!') << std::endl;
                std::cout << std::string(80, '!') << std::endl;
                std::cout << std::string(80, '!') << std::endl;
            }
            std::cout << "counter: " << counter << std::endl;
            std::cout << "quot: " << quot << std::endl;
            std::cout << "res: " << res << std::endl;
            // std::cout << "v_res: " << v_res << std::endl;
            std::cout << "v_res2: " << v_res2 << std::endl;
            std::cout << std::string(80, '*') << std::endl;
            assert(0);
        }
        // assert(res == v_res);
        return res;
    }

    auto get_capacity_old(const __m256i *x) -> size_t {
        // assert(validate_number_of_quotient(x));
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
            auto index = pd256_plus::select64(header[0], 49);
            assert(index < 64);
            auto res = index - 49;
            assert(res == get_capacity_naive(x));
            return res;
        } else {
            auto h0_zero_count = 64 - h0_one_count;
            auto ones_left = 49 - h0_one_count;
            // bool cond =
            assert(ones_left >= 0);
            auto index = pd256_plus::select64(header[1], 49 - h0_one_count);
            auto att_index = pd256_plus::select64(header[1], 49 - h0_one_count - 1);
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

    auto get_capacity_naive_with_OF_bit(const __m256i *x) -> size_t {
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


    auto count_ones_up_to_the_kth_zero(uint64_t word, size_t k) -> size_t {
        int zero_count = -1;
        int one_count = 0;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & word) {
                one_count++;
            } else {
                zero_count++;
                assert((int) (i - zero_count) == one_count);
                if (zero_count == ((int) k)) {
                    // auto res = i - k;
                    // assert(res == zero_count);
                    return i - k;
                }
            }
            b <<= 1ul;
        }
        v_pd256_plus::p_format_word(word);
        std::cout << one_count << std::endl;
        std::cout << zero_count << std::endl;
        assert(false);
        return -1;
        /* if (k == 0) {
            assert(word != -1);
            return _tzcnt_u64(~word);// If word == 1 then _tzcnt_u64(word) == 0;
        }
        assert(k <= QUOT_SIZE25);

        size_t zero_count = 0, one_count = 0;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & word) {
                one_count++;
            } else {
                zero_count++;
                if (zero_count == k)
                    return one_count;
            }
            b <<= 1ul;
        }

        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false); */
    }


    auto count_zeros_up_to_the_kth_one(uint64_t word, size_t k) -> size_t {
        assert(k <= CAPACITY25);
        size_t pop = _mm_popcnt_u64(word);
        assert(k < pop);
        // assert(__mpop);
        int one_count = -1;
        int zero_count = 0;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & word) {
                one_count++;
                assert(((int) i - one_count) == zero_count);
                if (one_count == (int) k) {
                    // auto res = i - k;
                    // assert(res == zero_count);
                    return i - k;
                }
            } else {
                zero_count++;
            }
            b <<= 1ul;
        }
        std::cout << one_count << std::endl;
        std::cout << zero_count << std::endl;
        assert(false);
        return -1;
        /* // #ifdef NDEBUG
        //         std::cout << "called count_zeros_up_to_the_kth_one with -O3" << std::endl;
        // #endif// !NDEBUG
        // assert(k > 0);
        if (k == 0) {
            assert(word);
            return _tzcnt_u64(word);// If word == 1 then _tzcnt_u64(word) == 0;
        }
        // assert(_mm_popcnt_u64(word) >= k);
        assert(_mm_popcnt_u64(word) >= k);
        size_t zero_count = 0, one_count = 0;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & word) {
                one_count++;
                if (one_count == k)
                    return zero_count;
            } else {
                zero_count++;
            }
            b <<= 1ul;
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false); */
    }

    auto count_ones_up_to_the_kth_zero(const __m256i *x, size_t k) -> size_t {
        return count_ones_up_to_the_kth_zero(get_clean_header(x), k);
    }


    auto count_zeros_up_to_the_kth_one(const __m256i *x, size_t k) -> size_t {
        return count_zeros_up_to_the_kth_one(get_clean_header(x), k);
    }

    auto get_name() -> std::string {
        return "pd256 ";
    }

    auto validate_number_of_quotient(const __m256i *pd) -> bool {
        return validate_number_of_quotient(get_header(pd));
        // uint64_t res = get_clean_header(pd);
        // if (_mm_popcnt_u64(res) != QUOT_SIZE25) {
        //     std::cout << "number of ones: " << _mm_popcnt_u64(res) << std::endl;
        //     v_pd256_plus::p_format_header(pd);
        //     assert(0);
        // }
        // return true;
    }
    auto validate_number_of_quotient_from_clean_header(uint64_t c_header) -> bool {
        assert(_mm_popcnt_u64(c_header) == QUOT_SIZE25);
        return true;
    }

    auto validate_number_of_quotient(uint64_t header) -> bool {
        uint64_t c_header = clean_header(header);
        if (_mm_popcnt_u64(c_header) != QUOT_SIZE25) {
            std::cout << "number of ones: " << _mm_popcnt_u64(c_header) << std::endl;
            v_pd256_plus::p_format_header(header);
            assert(0);
        }
        return true;
    }

    auto remove_naive(int64_t quot, char rem, __m256i *pd) -> bool {
        assert(false);
        return false;
    }
};// namespace pd256_plus
