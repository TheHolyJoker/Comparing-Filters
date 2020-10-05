#ifndef FILTERS_FPD_HPP
#define FILTERS_FPD_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <iostream>

#include <immintrin.h>
#include <x86intrin.h>


#ifndef MASK
#define MASK(p) ((1ULL << (p)) - 1)
#endif// !MASK

static constexpr uint8_t Table3_9[512] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4,
                                          5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8,
                                          9, 10, 11, 12, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5,
                                          6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9,
                                          10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 1, 2, 3, 4,
                                          5, 6, 7, 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9,
                                          10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11,
                                          12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7,
                                          8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10,
                                          11, 12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8, 9, 10, 11,
                                          12, 13, 14, 15, 9, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7,
                                          8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9,
                                          10, 11, 12, 13, 14, 8, 9, 10, 11, 12, 13, 14, 15, 9, 10, 11, 12, 13, 14, 15,
                                          16, 10, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11,
                                          12, 6, 7, 8, 9, 10, 11, 12, 13, 7, 8, 9, 10, 11, 12, 13, 14, 8, 9, 10, 11, 12,
                                          13, 14, 15, 9, 10, 11, 12, 13, 14, 15, 16, 10, 11, 12, 13, 14, 15, 16, 17, 11,
                                          5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 7, 8, 9, 10, 11, 12, 13,
                                          7, 8, 9, 10, 11, 12, 13, 14, 8, 9, 10, 11, 12, 13, 14, 15, 9, 10, 11, 12, 13,
                                          14, 15, 16, 10, 11, 12, 13, 14, 15, 16, 17, 11, 12, 13, 14, 15, 16, 17, 18,
                                          12, 6, 7, 8, 9, 10, 11, 12, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 7, 1,
                                          2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 9, 3, 4, 5, 6, 7, 8, 9, 10, 4, 5, 6,
                                          7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10, 11, 12, 6, 0, 1, 2, 3, 4, 5, 6, 0};


namespace Fixed_pd {
    typedef uint64_t header_type;
    static constexpr size_t counter_size = 3;
    // static constexpr size_t last_val = (1ULL << counter_size) - 1;
    static constexpr size_t counter_overflowed_val = (1ULL << counter_size) - 1;
    static constexpr size_t max_valide_counter_value = counter_overflowed_val - 1;
    // static constexpr size_t last_val = (1ULL << counter_size) - 1;
    static constexpr size_t slot_size = sizeof(header_type) * CHAR_BIT;
    static constexpr size_t word_capacity = slot_size / counter_size;

    static constexpr size_t QUOT_RANGE = 63;
    static constexpr size_t CAPACITY = 63;

    constexpr size_t words_count = QUOT_RANGE / word_capacity;

    auto to_bin(uint64_t x, size_t length = 64) -> std::string;

    void spaced_bin(uint64_t word, size_t gap = counter_size, uint64_t length = 64);

    void p_format_word(uint64_t word, size_t gap = counter_size, uint64_t length = 64);

    void p_format_word_by_int_and_sum(uint64_t word, size_t gap = counter_size, uint64_t length = 64);

    auto space_string(std::string s, size_t gap = counter_size) -> std::string;

    namespace Body {
        uint64_t get_v(uint8_t rem, const __m512i *body);

        void add(uint64_t begin_fingerprint, uint64_t end_fingerprint, uint8_t rem, __m512i *body);

        void remove(uint64_t body_index, uint8_t rem, __m512i *body);

        void remove_quot(uint64_t begin_fingerprint, uint64_t end_fingerprint, __m512i *body);
    }// namespace Body

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


        auto get_capacity(const header_type *header) -> size_t;

        auto get_max_quot(header_type *header) -> size_t;

        void set_quot_as_overflow(uint64_t quot, uint64_t curr_val, header_type *header);

        void set_quot_as_overflow(uint64_t quot, header_type *header);

        inline auto read_counter(uint64_t quot, const header_type *header) -> size_t {
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;
            return (header[w_index] >> (rel_index * counter_size)) & MASK(counter_size);
        }


        auto did_quot_overflowed(uint64_t quot, header_type *header) -> bool;


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
        inline auto add(uint64_t quot, header_type *header) -> int {
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            // auto old_val = read_counter(quot, header);
            const size_t res = read_counter(quot, header);
            if (res < max_valide_counter_value) {
                if (get_capacity(header) == CAPACITY) {
                    return -4;
                }
                header[w_index] += 1ULL << (rel_index * counter_size);
                return -1;
            } else if (res == max_valide_counter_value) {
                // auto old_temp_counter = read_counter(quot, header);
                header[w_index] += 1ULL << (rel_index * counter_size);
                // auto temp_counter = read_counter(quot, header);
                // assert(temp_counter == counter_overflowed_val);
                return -3;// or quot.
            } else
                return -2;
        }


        inline auto find(uint64_t quot, const header_type *header) -> int {
            return read_counter(quot, header);
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

        inline auto sum_word_v1(uint64_t word) -> size_t {
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
            // auto v_res = sum_word_v0(word);
            // if (res != sum_word_v0(word)) {
            //     std::cout << "res:   \t" << res << std::endl;
            //     std::cout << "v_res: \t" << v_res << std::endl;
            //     spaced_bin(word);
            // }
            // assert(res == sum_word_v0(word));
            return res;
        }

        inline auto sum_words(size_t w_end_index, const header_type *header) -> size_t {
            size_t res = 0;
            for (size_t i = 0; i < w_end_index; i++) {
                res += sum_word_v1(header[i]);
            }
            return res;
        }

        inline auto sum_words(const header_type *header) -> size_t {
            size_t res = 0;
            for (size_t i = 0; i < words_count; i++) {
                res += sum_word_v1(header[i]);
            }
            return res;
        }

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


        // auto sum_word_lim_v1(size_t rel_index, header_type word) -> size_t {
        //     // const size_t w_index = quot / word_capacity;
        //     // const size_t rel_index = quot % word_capacity;
        //     size_t all_sum = sum_word_v1(word);
        //     constexpr uint64_t mask = MASK(counter_size);
        //     // if (index % 9 == 0) {
        //     // }

        //     assert(index <= (slot_size / counter_size));
        //     size_t res = 0;
        //     for (size_t i = 0; i <= index; i++) {

        //         res += (word & mask) * ((word & mask) != last_val);
        //         word >>= 1ul;
        //     }
        //     return res;
        // }

        inline auto sum_word_lim_v2(size_t rel_index, header_type word) -> size_t {
            uint64_t masked_word = word & MASK(rel_index * counter_size);
            return sum_word_v1(masked_word);
            // auto res = sum_word_v1(masked_word);
            // auto v_res = sum_word_lim_v0(rel_index, word);
            // if (res != v_res) {
            //     spaced_bin(word);
            //     spaced_bin(masked_word);
            // }
            // assert(res == v_res);
            // return res;
            // return sum_word_v1(word) - sum_word_v1(masked_word);
        }

        inline auto get_capacity(const header_type *header) -> size_t {
            return sum_words(header);
        }

        inline auto get_start(uint64_t quot, const header_type *header) -> size_t {
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            switch (w_index) {
                case 0:
                    return sum_word_lim_v2(rel_index, header[w_index]);
                case 1:
                    return sum_word_v1(header[0]) +
                           sum_word_lim_v2(rel_index, header[w_index]);
                case 2:
                    return sum_word_v1(header[0]) + sum_word_v1(header[1]) +
                           sum_word_lim_v2(rel_index, header[w_index]);
                case 3:
                    assert(quot + 1 == QUOT_RANGE);
                    return sum_word_v1(header[0]) + sum_word_v1(header[1]) + sum_word_v1(header[2]);
                default:
                    assert(false);
                    return -1;
            }
            // size_t temp1 = sum_words(header, w_index);
            // auto res = sum_words(w_index, header) + sum_word_lim_v2(rel_index, header[w_index]);
            // assert(res <= CAPACITY);
            // return sum_words(w_index, header) + sum_word_lim_v2(rel_index, header[w_index]);
        }

        inline auto get_start_old(uint64_t quot, const header_type *header) -> size_t {
            const size_t rel_index = quot % word_capacity;
            const size_t w_index = quot / word_capacity;
            // size_t res = 0;

            // size_t temp1 = sum_words(header, w_index);
            auto res = sum_words(w_index, header) + sum_word_lim_v2(rel_index, header[w_index]);
            assert(res <= CAPACITY);
            return sum_words(w_index, header) + sum_word_lim_v2(rel_index, header[w_index]);
        }

        inline void get_interval(uint64_t quot, size_t *start, size_t *end, const header_type *header) {
            // constexpr size_t slot_size = sizeof(header_type) * CHAR_BIT;
            assert(0);// not used.
            const size_t temp = get_start(quot, header);
            *start = temp;
            *end = temp + read_counter(quot, header);
        }


        /**
     * @brief Get the max quot object
     * 
     * @param header 
     * @return size_t maximal non overflowing quot.
     */
        inline auto get_max_quot(header_type *header) -> size_t {
            //todo: make this faster.
            size_t min_val = 0;
            size_t min_index = 0;
            constexpr uint64_t mask = MASK(counter_size);
            for (size_t i = 0; i < words_count; i++) {
                uint64_t temp = header[i];
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


        inline void set_quot_as_overflow(uint64_t quot, uint64_t curr_val, header_type *header) {
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

        inline void set_quot_as_overflow(uint64_t quot, header_type *header) {
            constexpr uint64_t mask = MASK(counter_size);
            const size_t rel_index = quot % word_capacity;
            const size_t w_index = quot / word_capacity;
            const uint64_t temp_mask = mask << (rel_index * counter_size);
            header[w_index] |= temp_mask;
        }

    }// namespace Header

    namespace Body {
        inline void add(uint64_t begin_fingerprint, uint64_t end_fingerprint, uint8_t rem, __m512i *body) {
            assert(begin_fingerprint <= end_fingerprint);
            assert(end_fingerprint <= CAPACITY);
            // assert(curr_val < last_val);
            // header[w_index] += (last_val - curr_val) << (rel_index * counter_size);


            uint64_t i = begin_fingerprint;
            for (; i < end_fingerprint; ++i) {
                if (rem <= ((const uint8_t *) body)[i]) break;
            }
            assert((i == end_fingerprint) ||
                   (rem <= ((const uint8_t *) body)[i]));

            //Todo Fix this:
            //
            //  uint64_t j = (begin == end) ? end_fingerprint : _tzcnt_u64(((_mm512_cmp_epi8_mask(target, *pd, 2) >> (begin_fingerprint + 13)) << (begin_fingerprint)) | (1ull << end_fingerprint));

            memmove(&((uint8_t *) body)[i + 1],
                    &((const uint8_t *) body)[i],
                    sizeof(*body) - (i + 2));// 2 because the capacity is 63.
            ((uint8_t *) body)[i] = rem;
        }

        inline void remove(uint64_t body_index, uint8_t rem, __m512i *body) {
            const __m512i target = _mm512_set1_epi8(rem);
            uint64_t v = _mm512_cmpeq_epu8_mask(target, *body);
            bool cond = v & (1ULL << body_index);
            assert(cond);// testing the deleted element is actually in the pd element.
            memmove(&((uint8_t *) body)[body_index],
                    &((const uint8_t *) body)[body_index + 1],
                    sizeof(*body) - (body_index + 1) - 1);// -1 because the capacity is 63.
        }

        inline void remove_quot(uint64_t begin_fingerprint, uint64_t end_fingerprint, __m512i *body) {
            assert(begin_fingerprint < end_fingerprint);
            memmove(&((uint8_t *) body)[begin_fingerprint],
                    &((const uint8_t *) body)[end_fingerprint],
                    sizeof(*body) - (end_fingerprint) - 1);// -1 because the capacity is 63.
        }


        inline uint64_t get_v(uint8_t rem, const __m512i *body) {
            const __m512i target = _mm512_set1_epi8(rem);
            return _mm512_cmpeq_epu8_mask(target, *body);
        }
    }// namespace Body

}// namespace Fixed_pd


#endif//FILTERS_FPD_HPP
