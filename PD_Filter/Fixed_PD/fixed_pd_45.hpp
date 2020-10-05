//
// Created by tomer on 05/10/2020.
//

#ifndef COMPARING_FILTERS_SIMPLER_FIXED_PD_45_HPP
#define COMPARING_FILTERS_SIMPLER_FIXED_PD_45_HPP


#include <cassert>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <immintrin.h>
#include <x86intrin.h>


#ifndef MASK
#define MASK(p) ((1ULL << (p)) - 1)
#endif// !MASK
namespace Fixed_pd45 {

    static constexpr uint8_t Table3_9[512] =
            {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 7, 1,
             2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3,
             4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5,
             6, 7, 8, 9, 10, 11, 12, 6, 0, 1, 2, 3, 4, 5, 6, 0,
             1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 2,
             3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4,
             5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6,
             7, 8, 9, 10, 11, 12, 13, 7, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9,
             3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13,
             7, 8, 9, 10, 11, 12, 13, 14, 8, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5,
             6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8,
             9, 10, 11, 12, 13, 14, 15, 9, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6,
             7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8, 9, 10, 11, 12, 13, 14, 15,
             9, 10, 11, 12, 13, 14, 15, 16, 10, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11,
             12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8, 9, 10, 11, 12, 13, 14, 15, 9, 10, 11, 12,
             13, 14, 15, 16, 10, 11, 12, 13, 14, 15, 16, 17, 11, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7,
             8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8, 9, 10, 11, 12, 13, 14, 15, 9, 10, 11, 12, 13, 14, 15,
             16, 10, 11, 12, 13, 14, 15, 16, 17, 11, 12, 13, 14, 15, 16, 17, 18, 12, 6, 7, 8, 9, 10, 11, 12, 6, 0, 1, 2,
             3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9,
             10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 0, 1, 2, 3, 4, 5, 6, 0
            };


    typedef uint64_t header_type;
    static constexpr size_t counter_size = 3;
    static constexpr size_t counter_overflowed_val = (1ULL << counter_size) - 1;
    static constexpr size_t max_valid_counter_value = counter_overflowed_val - 1;
    static constexpr size_t slot_size = sizeof(header_type) * CHAR_BIT;
    static constexpr size_t word_capacity = slot_size / counter_size;
    static constexpr size_t QUOT_RANGE = 50;
    static constexpr size_t CAPACITY = 45;
    constexpr uint64_t bodyStartIndex = 16 + 3;
    static_assert(QUOT_RANGE * 3 + 2 + CAPACITY * 8 == 512, "Sizes are wrong");
    static_assert(bodyStartIndex * 8 + CAPACITY * 8 == 512, "bodyStartIndex is wrong.");
    constexpr size_t words_count = QUOT_RANGE / word_capacity;

    auto to_bin(uint64_t x, size_t length = 64) -> std::string;

    void spaced_bin(uint64_t word, size_t gap = counter_size, uint64_t length = 64);

    void p_format_word(uint64_t word, size_t gap = counter_size, uint64_t length = 64);

    void p_format_word_by_int_and_sum(uint64_t word, size_t gap = counter_size, uint64_t length = 64);

    auto space_string(std::string s, size_t gap = counter_size) -> std::string;

    namespace Header {

        namespace v_fixed_pd {
            void header_to_array(const header_type *header, uint64_t *a);

            void array_to_header(header_type *header, const uint64_t *a);

            void array_to_header(header_type *header, const uint64_t *a, size_t a_size);

            auto compare_headers(const header_type *h1, const header_type *h2) -> bool;


        }// namespace v_fixed_pd

        namespace tests {
            auto array_to_header_by_insertions(header_type *header, const uint64_t *a) -> bool;

            bool t0(size_t reps);
        }// namespace tests


        auto get_capacity(const __m512i *pd) -> size_t;

        auto validate_capacity(const __m512i *pd) -> bool;

        auto get_max_quot(header_type *header) -> size_t;

        void set_quot_as_overflow(uint64_t quot, uint64_t curr_val, header_type *header);

        void set_quot_as_overflow(uint64_t quot, header_type *header);

        inline auto read_counter(uint64_t quot, const __m512i *pd) -> size_t {
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;
            return (((uint64_t *) pd)[w_index] >> (rel_index * counter_size)) & MASK(counter_size);
        }

        auto sum_words_naive(size_t w_end_index, const header_type *header) -> size_t;

        auto validate_sum_words_naive(size_t w_end_index, const header_type *header) -> size_t;

        inline auto did_quot_overflowed(uint64_t quot, const __m512i *pd) -> bool {
            return read_counter(quot, pd) == counter_overflowed_val;
        }


        /**
     * @brief adding to the header. might fail.
     * Failing is hitted overflowing quot.
     *
     * @param quot
     * @param header
     * @return int -1 - successful insertions.
     *         int -2 - hitted overflowed quot.
     *         int -3 - the hitted quot just overflowed, and its elements should be moved to the next level.
     *         int -4 - the pd overflowed, and the fullest quot should be promoted (moved to the next level).
     */
        inline auto add(uint64_t quot, __m512i *pd) -> int {

            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            const size_t res = read_counter(quot, pd);
            if (res < max_valid_counter_value) {
                if (get_capacity(pd) == CAPACITY) {
                    return -4;
                }
                ((uint64_t *) pd)[w_index] += 1ULL << (rel_index * counter_size);
                return -1;
            } else if (res == max_valid_counter_value) {
                ((uint64_t *) pd)[w_index] += 1ULL << (rel_index * counter_size);
                return -3; // or quot.
            } else
                return -2;
        }

        inline auto add_db(uint64_t quot, __m512i *pd) -> int {
            static int counter = 0;
            counter += 1;
            auto temp = get_capacity(pd);
            assert(temp <= CAPACITY);
            auto res = add(quot, pd);

            auto temp2 = get_capacity(pd);
            assert(temp <= CAPACITY);
            if (res == -1) {
                assert(temp + 1 == temp2);
            }
            return res;
        }


        inline auto find(uint64_t quot, const __m512i *pd) -> int {
            return read_counter(quot, pd);
        }


        inline auto sum_word_v0(uint64_t word) -> size_t {
            constexpr uint64_t mask = MASK(counter_size);
            size_t res = 0;
            for (size_t i = 0; i < word_capacity; i++) {
                if ((word & mask) != counter_overflowed_val) {
                    res += (word & mask);
                }
                word >>= counter_size;
            }
            return res;
        }

        /*inline auto sum_word_v11(uint64_t word) -> size_t {
            constexpr size_t pack_const = 3;
            assert(pack_const == 3);
            constexpr uint64_t mask = MASK(counter_size * pack_const);
            uint16_t a = Table3_9[word & mask];
            uint16_t b = Table3_9[(word >> 9) & mask];
            uint16_t c = Table3_9[(word >> 18) & mask];
            uint16_t d = Table3_9[(word >> 27) & mask];
            uint16_t e = Table3_9[(word >> 36) & mask];
            uint16_t f = Table3_9[(word >> 45) & mask];
            uint16_t g = Table3_9[(word >> 54) & mask];
            size_t res = a + b + c + d + e + f + g;
            return res;
        }*/

        inline auto sum_word_v1(uint64_t word) -> size_t {
            constexpr size_t pack_const = 3;
            assert(pack_const == 3);
            constexpr uint64_t mask = MASK(counter_size * pack_const);
            uint16_t a = Table3_9[word & mask];
            uint16_t b = Table3_9[(word >> 9ul) & mask];
            uint16_t c = Table3_9[(word >> 18ul) & mask];
            uint16_t d = Table3_9[(word >> 27ul) & mask];
            uint16_t e = Table3_9[(word >> 36ul) & mask];
            uint16_t f = Table3_9[(word >> 45ul) & mask];
            uint16_t g = Table3_9[(word >> 54ul) & mask];
            size_t res = a + b + c + d + e + f + g;
//            size_t v_res = sum_word_v11(word);
//            assert(res == v_res);
            return res;
        }

        inline auto sum_last_word_naive(uint64_t word) -> size_t {
            uint64_t masked_word = word & MASK(8 * counter_size);
            return sum_word_v1(masked_word);
        }


        inline auto sum_last_word(uint64_t word) -> size_t {
            constexpr size_t pack_const = 3;
            assert(pack_const == 3);
            constexpr uint64_t mask = MASK(counter_size * pack_const);
            constexpr uint64_t last_mask = MASK(counter_size * 2);
            uint16_t a = Table3_9[word & mask];
            uint16_t b = Table3_9[(word >> 9ul) & mask];
            uint16_t c = Table3_9[(word >> 18ul) & last_mask];
//            auto res = a + b + c;
//            auto v_res = sum_last_word_naive(word);
//            assert(res == v_res);
            return a + b + c;
        }

//        inline auto sum_words(size_t w_end_index, const header_type *header) -> size_t {
//            size_t res = 0;
//            for (size_t i = 0; i < w_end_index; i++) {
//                res += sum_word_v1(header[i]);
//            }
//            return res;
//        }

        inline auto sum_words(const __m512i *pd) -> size_t {
            return sum_word_v1(((uint64_t *) pd)[0]) + sum_word_v1(((uint64_t *) pd)[1]) +
                   sum_last_word(((uint64_t *) pd)[2]);

        }

/*
        inline auto sum_word_lim_v0(size_t rel_index, header_type word) -> size_t {
            constexpr uint64_t mask = MASK(counter_size);
            assert(rel_index <= (slot_size / counter_size));
            size_t res = 0;
            for (size_t i = 0; i < rel_index; i++) {
                res += (word & mask) * ((word & mask) != counter_overflowed_val);
                word >>= counter_size;
            }
            return res;
        }
*/


        inline auto sum_word_lim_v2(size_t rel_index, header_type word) -> size_t {
            uint64_t masked_word = word & MASK(rel_index * counter_size);
            return sum_word_v1(masked_word);
            /*// auto res = sum_word_v1(masked_word);
            // auto v_res = sum_word_lim_v0(rel_index, word);
            // if (res != v_res) {
            //     spaced_bin(word);
            //     spaced_bin(masked_word);
            // }
            // assert(res == v_res);
            // return res;
            // return sum_word_v1(word) - sum_word_v1(masked_word);*/
        }

        inline auto sum_last_word_lim_v2(size_t rel_index, header_type word) -> size_t {
            uint64_t masked_word = word & MASK(rel_index * counter_size);
            return sum_last_word(masked_word);
        }

        inline auto get_capacity(const __m512i *pd) -> size_t {
            return sum_words(pd);
        }

        inline auto get_start(uint64_t quot, const __m512i *pd) -> size_t {
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            switch (w_index) {
                case 0:
                    return sum_word_lim_v2(rel_index, ((uint64_t *) pd)[0]);
                case 1:
                    return sum_word_v1(((uint64_t *) pd)[0]) +
                           sum_word_lim_v2(rel_index, ((uint64_t *) pd)[1]);
                case 2:
                    return sum_word_v1(((uint64_t *) pd)[0]) + sum_word_v1(((uint64_t *) pd)[1]) +
                           sum_last_word_lim_v2(rel_index, ((uint64_t *) pd)[2]);
                default:
                    assert(false);
                    return -1;
            }
        }

        /*inline auto get_start_old(uint64_t quot, const header_type *header) -> size_t {
            const size_t rel_index = quot % word_capacity;
            const size_t w_index = quot / word_capacity;
            // size_t res = 0;

            // size_t temp1 = sum_words(header, w_index);
            auto res = sum_words(w_index, header) + sum_word_lim_v2(rel_index, header[w_index]);
            assert(res <= CAPACITY);
            return sum_words(w_index, header) + sum_word_lim_v2(rel_index, header[w_index]);
        }
*/
        inline void get_interval(uint64_t quot, size_t *start, size_t *end, const __m512i *pd) {
            // constexpr size_t slot_size = sizeof(header_type) * CHAR_BIT;
            assert(0);// not used.
            const size_t temp = get_start(quot, pd);
            *start = temp;
            *end = temp + read_counter(quot, pd);
        }


        /**
     * @brief Get the max quot object
     *
     * @param header
     * @return size_t maximal non overflowing quot.
     */
        inline auto get_max_quot(const __m512i *pd) -> size_t {
            //todo: make this faster.
            size_t min_val = 0;
            size_t min_index = 0;
            constexpr uint64_t mask = MASK(counter_size);
            for (size_t i = 0; i < words_count; i++) {
                uint64_t temp = ((uint64_t *) pd)[i];
                for (size_t j = 0; j < word_capacity; j++) {
                    size_t temp_counter = temp & mask;
                    if (temp_counter != counter_overflowed_val) {
                        if (temp_counter >= min_val) {
                            min_val = temp_counter;
                            min_index = i * word_capacity + j;
                        }
                    }
                    temp >>= counter_size;
                }
            }
            assert(min_val);
            assert(min_val < counter_overflowed_val);
            assert(min_index <= QUOT_RANGE);
            // set_quot_as_overflow(min_index, min_val, header);
            return min_index;
        }

        inline void set_quot_as_overflow_db(uint64_t quot, uint64_t curr_val, header_type *header) {
            constexpr uint64_t mask = MASK(counter_size);
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            assert(curr_val < counter_overflowed_val);
            uint64_t old_val = header[w_index];
            header[w_index] += (counter_overflowed_val - curr_val) << (rel_index * counter_size);

            uint64_t temp_mask = mask << (rel_index * counter_size);
            uint64_t new_val = old_val | temp_mask;
            assert(header[w_index] == new_val);
            bool cond = ((header[w_index] & temp_mask) == temp_mask);
            assert(cond);
        }


        /*inline void set_quot_as_overflow(uint64_t quot, uint64_t curr_val, header_type *header) {
            constexpr uint64_t mask = MASK(counter_size);
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            assert(curr_val < counter_overflowed_val);
            uint64_t old_val = header[w_index];
            header[w_index] += (counter_overflowed_val - curr_val) << (rel_index * counter_size);

            uint64_t temp_mask = mask << (rel_index * counter_size);
            uint64_t new_val = old_val | temp_mask;
            assert(header[w_index] == new_val);
            bool cond = ((header[w_index] & temp_mask) == temp_mask);
            assert(cond);
        }
*/

        inline void set_quot_as_overflow(uint64_t quot, __m512i *pd) {
            constexpr uint64_t mask = MASK(counter_size);
            const size_t rel_index = quot % word_capacity;
            const size_t w_index = quot / word_capacity;
            const uint64_t temp_mask = mask << (rel_index * counter_size);
            ((uint64_t *) pd)[w_index] |= temp_mask;
        }


    }// namespace Header

    namespace Body {

        inline void add(uint64_t begin_fingerprint, uint64_t end_fingerprint, uint8_t rem, __m512i *pd) {
            assert(begin_fingerprint <= end_fingerprint);
            assert(end_fingerprint <= CAPACITY);

            uint64_t i = begin_fingerprint;
            for (; i < end_fingerprint; ++i) {
                if (rem <= ((const uint8_t *) pd)[bodyStartIndex + i]) break;
            }
            assert((i == end_fingerprint) ||
                   (rem <= ((const uint8_t *) pd)[bodyStartIndex + i]));

            //Todo Fix this:
            //
            //  uint64_t j = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

            memmove(&((uint8_t *) pd)[bodyStartIndex + i + 1],
                    &((const uint8_t *) pd)[bodyStartIndex + i],
                    sizeof(*pd) - (bodyStartIndex + i + 1));
            ((uint8_t *) pd)[bodyStartIndex + i] = rem;
        }

        inline void remove(uint64_t body_index, uint8_t rem, __m512i *pd) {
            const __m512i target = _mm512_set1_epi8(rem);
            uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
            bool cond = v & (1ULL << body_index);
            assert(cond);// testing the deleted element is actually in the pd element.
            memmove(&((uint8_t *) pd)[bodyStartIndex + body_index],
                    &((const uint8_t *) pd)[bodyStartIndex + body_index + 1],
                    sizeof(*pd) - (bodyStartIndex + body_index + 1));
        }

        inline void remove_quot(uint64_t begin_fingerprint, uint64_t end_fingerprint, __m512i *pd) {
            assert(begin_fingerprint < end_fingerprint);
            memmove(&((uint8_t *) pd)[bodyStartIndex + begin_fingerprint],
                    &((const uint8_t *) pd)[bodyStartIndex + end_fingerprint],
                    sizeof(*pd) - (bodyStartIndex + end_fingerprint));
        }


        inline uint64_t get_v(uint8_t rem, const __m512i *pd) {
            const __m512i target = _mm512_set1_epi8(rem);
            return _mm512_cmpeq_epu8_mask(target, *pd) >> bodyStartIndex;
        }

    }// namespace Body

    inline auto add(uint64_t quot, uint8_t rem, __m512i *pd) -> int {
        assert(quot < QUOT_RANGE);
        int header_add_res = Header::add(quot, pd);
        if (header_add_res == -1) { ;
            uint64_t start = Header::get_start(quot, pd);
            assert(start <= CAPACITY);
            uint64_t end = Header::read_counter(quot, pd);
            Body::add(start, start + end - 1, rem, pd);
        }
        return header_add_res;
    }


}// namespace Fixed_pd


#endif //COMPARING_FILTERS_SIMPLER_FIXED_PD_45_HPP
