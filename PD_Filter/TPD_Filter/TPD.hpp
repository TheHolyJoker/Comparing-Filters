//
// Created by tomer on 7/9/20.
//

#ifndef FILTERS_TPD_HPP
#define FILTERS_TPD_HPP


#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <ctgmath>
#include <climits>
#include <iomanip>
#include "macros.h"

template<typename slot_type>
static auto compute_size(size_t max_capacity, size_t bits_per_item) -> size_t {
    size_t slot_size = sizeof(slot_type) * CHAR_BIT;
    auto total_bit_size = (max_capacity * (2 + bits_per_item));
    return INTEGER_ROUND(total_bit_size, slot_size);
}

//bool is_aligned(void *p, int N) {
//    return (int) p % N == 0;
//}

template<class T>
bool
is_aligned(const void * ptr) noexcept {
    auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
    return !(iptr % alignof(T));
}

namespace TPD_name {

    static inline void
    body_find_helper(size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index) {
        *p_B_index = current_b_index;
        *p_bit_index = BODY_BLOCK_SIZE - bits_left;
    }


    template<typename slot_type, size_t bits_per_item, size_t max_capacity, size_t quotient_range = 64u>
    class TPD {
        const size_t size = compute_size<slot_type>(max_capacity, bits_per_item);
//                (max_capacity * (bits_per_item + 2)) / (sizeof(slot_type) * CHAR_BIT);
        slot_type a[max_capacity * bits_per_item + (quotient_range << 1u)];
        const bool deal_with_joined_slot{(quotient_range << 1u) % (sizeof(slot_type) * CHAR_BIT) != 0};

    public:
        TPD(size_t m, size_t f, size_t l) {
            memset(a, 0, max_capacity * bits_per_item + (quotient_range << 1u));


            assert(is_aligned<slot_type*>(a));
            for (int i = 0; i < max_capacity; ++i) {
                assert(a[i] == 0);
            }
        }

        auto lookup(slot_type q, slot_type r) -> bool {
            size_t start_index = -1, end_index = -1;
            if (not header_lookup(q, &start_index, &end_index))
                return false;

            assert(q <= start_index <= end_index);

            size_t unpacked_start_index = start_index - q;
            size_t unpacked_end_index = end_index - q;

            size_t p_array_index = -1, p_bit_index = -1;
            return body_find(r, unpacked_start_index, unpacked_end_index, &p_array_index, &p_bit_index);

        }

        void insert(slot_type q, slot_type r) {
            assert(get_capacity() < max_capacity);
            assert(q < quotient_range);
            size_t start_index = -1, end_index = -1;
            header_insert(q, &start_index, &end_index);

            assert((q <= start_index) and (start_index <= end_index));

            size_t unpacked_start_index = start_index - q;
            size_t unpacked_end_index = end_index - q;

            body_insert(r, unpacked_start_index, unpacked_end_index);
        }

        void remove(slot_type q, slot_type r) {
            size_t start_index = -1, end_index = -1;
            header_remove(q, &start_index, &end_index);

            assert(q <= start_index <= end_index);

            size_t unpacked_start_index = start_index - q;
            size_t unpacked_end_index = end_index - q;

//    size_t p_array_index = -1, p_bit_index = -1;
//    assert (body_find(r, unpacked_start_index, unpacked_end_index, &p_array_index, &p_bit_index));
            body_remove(r, unpacked_start_index, unpacked_end_index);
        }

        auto conditional_remove(slot_type q, slot_type r) -> bool {
//    return naive_conditional_remove(q, r);
            size_t start_index = -1, end_index = -1;
            if (not header_lookup(q, &start_index, &end_index))
                return false;

            assert(q <= start_index);
            assert(start_index <= end_index);

            size_t unpacked_start_index = start_index - q;
            size_t unpacked_end_index = end_index - q;
            if (body_conditional_remove(r, unpacked_start_index, unpacked_end_index)) {
                header_pull(end_index);
                return true;
            }
            return false;
        }

        auto is_full() -> bool {
            return get_capacity() == max_capacity;
        }

        auto get_capacity() -> size_t {
//    size_t total_bits = get_header_size_in_bits();
            size_t last_index = get_last_a_index_containing_the_header();
            size_t bit_index = get_header_bit_index();
            size_t res = 0;
            for (size_t i = 0; i < last_index; ++i) {
                res += __builtin_popcount(a[i]);
            }

            if (bit_index == 0)
                return res + __builtin_popcount(a[last_index]);
            auto temp = a[last_index] & (~MASK((sizeof(slot_type) * CHAR_BIT) - bit_index));
            return res + __builtin_popcount(temp);
        }

        auto get_name() -> std::string {
            string a = to_string(sizeof(slot_type) * CHAR_BIT);
            string b = to_string(bits_per_item);
            string c = to_string(max_capacity);
            return "TPD:<" + a + ", " + b + ", " + c + ">";
        }

    private:
        auto get_header_size_in_bits() -> size_t {
            return quotient_range << 1u;
        }

        /* auto should_deal_with_joined_slot(size_t m, size_t f, size_t l) -> bool {
             return bool((f * 2) % (sizeof(slot_type) * CHAR_BIT));
         }*/

/*
    auto naive_conditional_remove(slot_type q, slot_type r)->bool {
        bool res = lookup(q, r);
        if (res) { remove(q, r); }
        return res;
    }
*/


////Header functions
        auto header_lookup(slot_type q, size_t *start_index, size_t *end_index) -> bool {
            header_find(q, start_index, end_index);
            assert(*start_index <= *end_index);
            return (*start_index != *end_index);
        }

        void header_find(uint32_t q, size_t *start, size_t *end) {
            auto old_q = q;
            if (q == 0) {
                *start = 0;
                size_t j = 0;
                while (a[j] == MASK32) j++;
                *end = (j) * (sizeof(uint32_t) * CHAR_BIT) + __builtin_clz(~a[j]);
                return;
            }
            for (size_t i = 0; i <= get_last_a_index_containing_the_header(); ++i) {
                auto cz = __builtin_popcount(~a[i]);
                if (cz < q) q -= cz;
                else if (cz == q) {
                    uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
                    uint32_t bit_pos = select_r(~slot, q);
                    assert(bit_pos < (sizeof(uint32_t) * CHAR_BIT));
                    *start = (i + (bit_pos + 1 == (sizeof(uint32_t) * CHAR_BIT))) * (sizeof(uint32_t) * CHAR_BIT) +
                             (bit_pos + 1) % (sizeof(uint32_t) * CHAR_BIT);
                    size_t j = i + 1;
                    while (a[j] == MASK32) j++;
                    *end = (j) * (sizeof(uint32_t) * CHAR_BIT) + __builtin_clz(~a[j]);
                    return;
                } else {
                    assert(q < (sizeof(uint32_t) * CHAR_BIT));
                    uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
                    uint32_t bit_pos = select_r(~slot, q);
                    assert(bit_pos < (sizeof(uint32_t) * CHAR_BIT));
                    *start = i * (sizeof(uint32_t) * CHAR_BIT) + select_r(~slot, q) + 1;
                    *end = i * (sizeof(uint32_t) * CHAR_BIT) + select_r(~slot, q + 1);
                    return;

                }
            }
            std::size_t a = -1, b = -1;
            header_find(old_q, &a, &b);
            assert(false);
        }

        void header_find(uint64_t q, size_t *start, size_t *end) {
            if (q == 0) {
                *start = 0;
                size_t j = 0;
                while (a[j] == -1) j++;
                *end = (j) * (sizeof(uint64_t) * CHAR_BIT) + __builtin_clzl(~a[j]);
                return;
            }
            for (size_t i = 0; i <= get_last_a_index_containing_the_header(); ++i) {
                auto cz = __builtin_popcountl(~a[i]);
                if (cz < q) q -= cz;
                else if (cz == q) {
                    uint32_t bit_pos = select_r(~a[i], q);
                    assert(bit_pos < (sizeof(slot_type) * CHAR_BIT));
                    *start = (i + (bit_pos + 1 == (sizeof(slot_type) * CHAR_BIT))) * (sizeof(slot_type) * CHAR_BIT) +
                             (bit_pos + 1) % (sizeof(slot_type) * CHAR_BIT);
                    size_t j = i + 1;
                    while (a[j] == -1) j++;
                    *end = (j) * (sizeof(slot_type) * CHAR_BIT) + __builtin_clzl(~a[j]);
                    return;
                } else {
                    assert(q < (sizeof(slot_type) * CHAR_BIT));
                    uint32_t bit_pos = select_r(~a[i], q);
                    assert(bit_pos < (sizeof(slot_type) * CHAR_BIT));
                    *start = i * (sizeof(slot_type) * CHAR_BIT) + bit_pos + 1;
                    *end = i * (sizeof(slot_type) * CHAR_BIT) + select_r(~a[i], q + 1);
                    return;
                }
            }

        }

        void header_insert(slot_type q, size_t *start_index, size_t *end_index) {
            /*Body does not start in a new slot*/
//            assert(max_capacity * 2 % ((sizeof(slot_type) * CHAR_BIT)) == 0);
            size_t temp_index;
            slot_type temp_slot;
            if (deal_with_joined_slot) {
                temp_index = get_joined_slot_index();
                temp_slot = a[temp_index];
            }

            header_find(q, start_index, end_index);
            header_push(*end_index);

//            /*Restore body's part in the joined cell*/
            if (deal_with_joined_slot) {
                auto mask_bit = (sizeof(slot_type) * CHAR_BIT) - get_header_bit_index();
                a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
            }

        }

        void header_push(size_t end_index) {
            size_t index, last_header_a_slot = get_last_a_index_containing_the_header();
            index = end_index / (sizeof(slot_type) * CHAR_BIT);

            ulong to_shift = (sizeof(slot_type) * CHAR_BIT) - 1ul;
            for (uint_fast16_t i = last_header_a_slot; i > index; --i) {
                a[i] = ((ulong) (a[i]) >> 1ul) | ((a[i - 1] & 1ul) << to_shift);
            }


            uint_fast16_t bit_index = end_index % (sizeof(slot_type) * CHAR_BIT);
            uint_fast16_t shift = (sizeof(slot_type) * CHAR_BIT) - bit_index;
            ulong mask = MASK(shift);
            ulong upper = a[index] & (~mask);
            slot_type lower = (a[index] >> 1ul) & mask;
            a[index] = (upper | lower | SL(shift - 1ul));

        }

        void header_remove(slot_type q, size_t *start_index, size_t *end_index) {
            /*Body does not start in a new slot*/
//            assert(max_capacity * 2 % ((sizeof(slot_type) * CHAR_BIT)) == 0);

            size_t temp_index;
            slot_type temp_slot;
            if (deal_with_joined_slot) {
                temp_index = get_joined_slot_index();
                temp_slot = a[temp_index];
            }

            header_find(q, start_index, end_index);
            if (DB) {
                if (*start_index == *end_index) {
                    std::cout << "trying to remove element not in the PD. Specifically not in the header. "
                              << std::endl;
                    return;
                }
            }
            header_pull(*end_index);

            /*Restore body's part in the joined cell*/
            if (deal_with_joined_slot) {
                //Plus one to insure that body's first bit did not effect header last bit.
                //This bit (header last bit) should always be zero.
                auto mask_bit = (sizeof(slot_type) * CHAR_BIT) - get_header_bit_index() + 1;
                a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
            }

        }

        void header_pull(size_t end_index) {
            size_t index = (end_index - 1) / sizeof(slot_type) *
                           CHAR_BIT, last_header_a_slot = get_last_a_index_containing_the_header();

            if (index == last_header_a_slot) {
                uint_fast16_t bit_index = (end_index - 1) % (sizeof(slot_type) * CHAR_BIT);
                uint_fast16_t shift = (sizeof(slot_type) * CHAR_BIT) - bit_index;
                ulong mask = MASK(shift);
                ulong upper = a[index] & (~mask);
                slot_type mid = ((ulong) a[index] << 1ul) & mask;
                a[index] = upper | mid;
                return;
            }
            slot_type lower = ((ulong) a[index + 1]) >> ((ulong) ((sizeof(slot_type) * CHAR_BIT) - 1));

            for (size_t i = index + 1; i < last_header_a_slot; ++i) {
                a[i] = ((ulong) a[i] << 1ul) | ((ulong) a[i + 1] >> ((ulong) ((sizeof(slot_type) * CHAR_BIT) - 1)));
            }
            a[last_header_a_slot] <<= 1ul;

            uint_fast16_t bit_index = (end_index - 1) % (sizeof(slot_type) * CHAR_BIT);
            uint_fast16_t shift = (sizeof(slot_type) * CHAR_BIT) - bit_index;
            ulong mask = MASK(shift);
            ulong upper = a[index] & (~mask);
            slot_type mid = ((ulong) a[index] << 1ul) & mask;
            a[index] = upper | mid | lower;// | SL(shift);

        }

////Body functions
        auto body_find(slot_type r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                       size_t *p_bit_index) -> bool {
            size_t total_bit_counter = (unpacked_start_index * bits_per_item) + get_header_size_in_bits();
//    size_t total_bit_counter = abstract_body_start_index * bits_per_item;

            /*Empty case. */
            if (unpacked_start_index == unpacked_end_index) {
                *p_array_index = total_bit_counter / BODY_BLOCK_SIZE;
                *p_bit_index = total_bit_counter % BODY_BLOCK_SIZE;
//        *p_array_index = (unpacked_start_index * bits_per_item) / BODY_BLOCK_SIZE;
//        *p_bit_index = (unpacked_start_index * bits_per_item) % BODY_BLOCK_SIZE + get_header_size_in_bits();
                if (DB_PRINT) std::cout << "a1" << std::endl;
                return false;
            }

            size_t comparing_counter = 0, comparing_lim = unpacked_end_index - unpacked_start_index;

            for (; comparing_counter < comparing_lim; ++comparing_counter) {
                size_t B_index = total_bit_counter / BODY_BLOCK_SIZE;
                size_t bit_index_inside_slot = total_bit_counter % BODY_BLOCK_SIZE;
                size_t bits_left_to_read_inside_slot = BODY_BLOCK_SIZE - bit_index_inside_slot;

                BODY_BLOCK_TYPE current_cell = a[B_index];
                /*More than bits_per_item bits remain in B[B_index].*/
                if (bits_left_to_read_inside_slot > bits_per_item) {
                    ulong shift = bits_left_to_read_inside_slot - bits_per_item;
                    assert(shift < BODY_BLOCK_SIZE);
                    BODY_BLOCK_TYPE current_remainder = (current_cell >> (shift)) & MASK(bits_per_item);
                    if (r <= current_remainder) {
                        if (DB_PRINT) std::cout << "a2" << std::endl;
                        body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                        return r == current_remainder;
                    }
                    /*Exactly bits_per_item bits remain in B[B_index].*/
                } else if (bits_left_to_read_inside_slot == bits_per_item) {
                    BODY_BLOCK_TYPE current_remainder = current_cell & MASK(bits_per_item);
                    if (r <= current_remainder) {
                        if (DB_PRINT) std::cout << "a3" << std::endl;
                        body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                        return r == current_remainder;
                    }
                    /*Less than bits_per_item bits remain in B[B_index].*/
                } else {
                    size_t number_of_bits_to_read_from_next_slot = bits_per_item - bits_left_to_read_inside_slot;
                    ulong upper_shift = bits_per_item - bits_left_to_read_inside_slot;
                    assert(upper_shift >= 0 and upper_shift < BODY_BLOCK_SIZE);
                    ulong upper = (current_cell & MASK(bits_left_to_read_inside_slot)) << (upper_shift);
                    assert(size > B_index + 1);
                    ulong lower_shift = BODY_BLOCK_SIZE - number_of_bits_to_read_from_next_slot;
                    assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
                    ulong lower = (a[B_index + 1] >> lower_shift) & MASK(number_of_bits_to_read_from_next_slot);
                    BODY_BLOCK_TYPE current_remainder = upper | lower;
                    if (r <= current_remainder) {
                        if (DB_PRINT) std::cout << "a4" << std::endl;
                        body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                        return r == current_remainder;
                    }
                }
                total_bit_counter += bits_per_item;
            }
            *p_array_index = total_bit_counter / BODY_BLOCK_SIZE;
            *p_bit_index = total_bit_counter % BODY_BLOCK_SIZE;
            if (DB_PRINT) std::cout << "a5" << std::endl;
            return false;


        }

        void body_insert(slot_type r, size_t unpacked_start_index, size_t unpacked_end_index) {
            size_t temp_index;
            slot_type temp_slot;
            if (deal_with_joined_slot) {
                temp_index = get_joined_slot_index();
                temp_slot = a[temp_index];
            }

            size_t B_index = -1, bit_index = -1;
            auto res = body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
            body_push(r, B_index, bit_index);

            if (deal_with_joined_slot) {
                auto mask_bit = (sizeof(slot_type) * CHAR_BIT) - get_header_bit_index();
                a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
            }
        }

        void body_push(slot_type r, size_t B_index, size_t bit_index) {
//    size_t B_index = -1, bit_index = -1;
//    auto res = body_find(unpacked_start_index, unpacked_end_index, r, &B_index, &bit_index);
            size_t left_bit_index = BODY_BLOCK_SIZE - bit_index;
            for (size_t i = size - 1; i > B_index; --i) {
                a[i] = (a[i] >> bits_per_item) | ((a[i - 1]) << (BODY_BLOCK_SIZE - bits_per_item));
            }

            if (BODY_BLOCK_SIZE >= bits_per_item + bit_index) {
                ulong mask = MASK(left_bit_index);
                ulong upper = a[B_index] & (~mask);
                BODY_BLOCK_TYPE mid = r << (left_bit_index - bits_per_item);
                BODY_BLOCK_TYPE lower = (a[B_index] >> bits_per_item) & (MASK(left_bit_index - bits_per_item));
                a[B_index] = (upper | lower | mid);
                assert(left_bit_index - bits_per_item >= 0);
            } else { // Prevents negative shifting.
                size_t shift = left_bit_index;
                ulong mask = MASK(left_bit_index);
                ulong upper = a[B_index] & (~mask);
                BODY_BLOCK_TYPE lower = r >> (bits_per_item - left_bit_index);
                a[B_index] = (upper | lower);

                assert(bits_per_item - shift > 0);

                //same amount that r was shifted right by. (bits_per_item - shift)
                size_t bits_left = bits_per_item - shift; // bits_left =  bits_per_item + bit_index - BODY_BLOCK_SIZE
                upper = (r & MASK(bits_left)) << (BODY_BLOCK_SIZE - bits_left); // todo Check this.
                lower = (a[B_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits. todo try mask instead.
                a[B_index + 1] = (upper | lower);
            }
//    capacity++;
        }

        void body_remove(slot_type r, size_t unpacked_start_index, size_t unpacked_end_index) {
            /*Body does not start in a new slot*/
//            assert(max_capacity * 2 % ((sizeof(slot_type) * CHAR_BIT)) == 0);

            size_t temp_index;
            slot_type temp_slot;
            if (deal_with_joined_slot) {
                temp_index = get_joined_slot_index();
                temp_slot = a[temp_index];
            }

            size_t B_index = -1, bit_index = -1;
            auto res = body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
            if (not res) {
                body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
                std::cout << "trying to remove element not in the PD. "
                             "Specifically not in the Body (might be in the header). " << std::endl;
                return;
            }
            body_pull(B_index, bit_index);

            if (deal_with_joined_slot) {
                auto mask_bit = (sizeof(slot_type) * CHAR_BIT) - get_header_bit_index();
                a[temp_index] = (a[temp_index] & (MASK(mask_bit))) | (temp_slot & (~MASK(mask_bit)));
            }
        }

        auto body_conditional_remove(slot_type r, size_t unpacked_start_index, size_t unpacked_end_index) -> bool {
            /*Body does not start in a new slot*/
//            assert(max_capacity * 2 % ((sizeof(slot_type) * CHAR_BIT)) == 0);

            size_t temp_index;
            slot_type temp_slot;

            if (deal_with_joined_slot) {
                temp_index = get_joined_slot_index();
                temp_slot = a[temp_index];
            }

            size_t B_index = -1, bit_index = -1;
            auto res = body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
            if (not res) {
                return false;
            }
            body_pull(B_index, bit_index);

            if (deal_with_joined_slot) {
                auto mask_bit = (sizeof(slot_type) * CHAR_BIT) - get_header_bit_index();
                a[temp_index] = (a[temp_index] & (MASK(mask_bit))) | (temp_slot & (~MASK(mask_bit)));
            }
            return true;
        }

        void body_pull(size_t B_index, size_t bit_index) {
            size_t left_fp_start_index = BODY_BLOCK_SIZE - bit_index;

            if (B_index == size - 1) {
                assert(bit_index + bits_per_item <= BODY_BLOCK_SIZE);

//        ulong shift = left_fp_start_index;
                assert(left_fp_start_index >= 0);
                ulong mask = MASK(left_fp_start_index);
                ulong upper = a[B_index] & (~mask);
                BODY_BLOCK_TYPE mid = (a[B_index] & MASK(left_fp_start_index - bits_per_item)) << bits_per_item;
                assert(left_fp_start_index >= bits_per_item);
                a[B_index] = (upper | mid);
                return;
            }

            if (BODY_BLOCK_SIZE >= bits_per_item + bit_index) {
                ulong shift = left_fp_start_index;
                ulong mask = MASK(left_fp_start_index);
                ulong upper = a[B_index] & (~mask);
                BODY_BLOCK_TYPE mid = (a[B_index] & MASK(left_fp_start_index - bits_per_item)) << bits_per_item;
                BODY_BLOCK_TYPE lower = (a[B_index + 1]) >> (BODY_BLOCK_SIZE - bits_per_item);
                a[B_index] = (upper | lower | mid);

                assert(shift >= bits_per_item);
                assert(BODY_BLOCK_SIZE - bits_per_item >= 0);

                for (size_t i = B_index + 1; i < size - 1; ++i) {
                    a[i] = (a[i] << bits_per_item) | (a[i + 1] >> (BODY_BLOCK_SIZE - bits_per_item));
                }
                a[size - 1] <<= bits_per_item;

            } else {
                ulong shift = left_fp_start_index;
                ulong mask = MASK(left_fp_start_index);
                ulong upper = a[B_index] & (~mask);
                ulong lower_shift = BODY_BLOCK_SIZE - bits_per_item;
                BODY_BLOCK_TYPE lower = a[B_index + 1] >> lower_shift;
                lower &= MASK(left_fp_start_index);
                a[B_index] = upper | lower;

                for (size_t i = B_index + 1; i < size - 1; ++i) {
                    a[i] = (a[i] << bits_per_item) | (a[i + 1] >> (BODY_BLOCK_SIZE - bits_per_item));
                }
                a[size - 1] <<= bits_per_item;

                assert(0 <= shift and shift < bits_per_item);
                assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
            }

        }

        auto get_last_a_index_containing_the_header() -> size_t {
            return (quotient_range * 2 - 1) / ((sizeof(slot_type) * CHAR_BIT));
        }

        auto get_header_bit_index() -> size_t {
            auto temp = (quotient_range << 1u);
            return temp % ((sizeof(slot_type) * CHAR_BIT));
        }

        auto get_joined_slot_index() -> size_t {
            return get_last_a_index_containing_the_header();
        }

//size_t get_header_size_in_bits() {
//    return max_capacity << 1u;
//}

        auto get_a_size(size_t m, size_t f, size_t l) -> size_t {
            return INTEGER_ROUND(((l + 2) * f), ((sizeof(slot_type) * CHAR_BIT)));
        }

        size_t get_number_of_bits_in_a(size_t m, size_t f, size_t l) {
            return (l + 2) * f;
        }

        auto get_a() const -> uint32_t * {
            return a;
        }

        auto get_bits_per_item() const -> const uint_fast16_t {
            return bits_per_item;
        }

        auto get_max_capacity() const -> const uint_fast16_t {
            return max_capacity;
        }

        auto get_size() const -> const uint_fast16_t {
            return size;
        }

        auto get_deal_with_joined_slot() const -> const bool {
            return deal_with_joined_slot;
            /*Body does not start in a new slot*/
//            assert(max_capacity * 2 % ((sizeof(slot_type) * CHAR_BIT)) == 0);
//            return deal_with_joined_slot;
        }

/*
   void print_as_array() const {
    print_array_as_integers<uint32_t>(a, size);
   }

   void print_as_consecutive_memory() {
    print_array_as_consecutive_memory<slot_type>(a, size, std::cout);
   }

   auto operator<<(ostream &os, const PD &pd) -> ostream & {
    print_array_as_consecutive_memory<slot_type>(pd.a, pd.size, std::cout);
    return os;
   }
 */




//    ~PD() {
//        delete[] a;
//    }


        uint32_t select_r(uint64_t slot, uint32_t rank) {
//    uint64_t slot;          // Input value to find position with rank rank.
//    unsigned int rank;      // Input: bit's desired rank [1-64].
            unsigned int s;  // Output: Resulting position of bit with the desired rank.[1-64]
            uint64_t a, b, c, d; // Intermediate temporaries for bit count.
            unsigned int t;  // Bit count temporary.

            // Do a normal parallel bit count for a 64-bit integer,
            // but store all intermediate steps.
            // a = (slot & 0x5555...) + ((slot >> 1) & 0x5555...);
            a = slot - ((slot >> 1) & ~0UL / 3);
            // b = (a & 0x3333...) + ((a >> 2) & 0x3333...);
            b = (a & ~0UL / 5) + ((a >> 2) & ~0UL / 5);
            // c = (b & 0x0f0f...) + ((b >> 4) & 0x0f0f...);
            c = (b + (b >> 4)) & ~0UL / 0x11;
            // d = (c & 0x00ff...) + ((c >> 8) & 0x00ff...);
            d = (c + (c >> 8)) & ~0UL / 0x101;
            t = (d >> 32) + (d >> 48);
            // Now do branchless select!
            s = 64;
            // if (rank > t) {s -= 32; rank -= t;}
            s -= ((t - rank) & 256) >> 3;
            rank -= (t & ((t - rank) >> 8));
            t = (d >> (s - 16)) & 0xff;
            // if (rank > t) {s -= 16; rank -= t;}
            s -= ((t - rank) & 256) >> 4;
            rank -= (t & ((t - rank) >> 8));
            t = (c >> (s - 8)) & 0xf;
            // if (rank > t) {s -= 8; rank -= t;}
            s -= ((t - rank) & 256) >> 5;
            rank -= (t & ((t - rank) >> 8));
            t = (b >> (s - 4)) & 0x7;
            // if (rank > t) {s -= 4; rank -= t;}
            s -= ((t - rank) & 256) >> 6;
            rank -= (t & ((t - rank) >> 8));
            t = (a >> (s - 2)) & 0x3;
            // if (rank > t) {s -= 2; rank -= t;}
            s -= ((t - rank) & 256) >> 7;
            rank -= (t & ((t - rank) >> 8));
            t = (slot >> (s - 1)) & 0x1;
            // if (rank > t) s--;
            s -= ((t - rank) & 256) >> 8;
            //current res between [0,63]
            return 64 - s;
        }


    };
}
#endif //FILTERS_TPD_HPP
