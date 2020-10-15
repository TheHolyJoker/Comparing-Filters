/**
 * @file L2Bucket_mq.hpp Level 2 Bucket minus the quots.
 * @author Tomer Even (you@domain.com)
 * @brief This class is an attempt to implement a part of the second level bucket.
 * Every query of an element x, in the second level, contains several fields:
 * 
 * 1) remainder : same as x's remainder in the first level.
 * 2) quotient  : same as x's remainder in the first level.
 * 3) pd_index  : relative pd_index inside the batch. 
 * 
 * Every set of 'batch_size' contiguous PDs from the first level are mapped into 'd' (currently d = 2) buckets in the second level.
 * Elements are mapped solely by their  pd_index, in L1. That way, one can locate all the elements that overflowed from specific PD easily,
 * i.e by checking in 'd' buckets.
 * 
 * The first hash function 'h1' is defined as follows: h1(x) := x / batch_size              (integer division).
 * This way, we can store only the lowest log2(batch_size) bits of x's 'pd_index', instead of the entire value. 
 * This method saves roughly 16 bits per element in the second level.
 * Moreover, since the pd_indexes that reaches the second level are random, we can use a non-random hash function.
 * The second hash function is random. 
 * 
 * The insertion Policy is "Power Of Two Choices". 
 * If an element 'x' was mapped to the buckets b1, b2,... it will be inserted into the emptiest bucket from that set. 
 * Ties are broken by the bucket index (any other consistent function will work).
 * 
 * location bitmask - location bitmask is a bitmask of size max_capacity. It is used to determine if the element inside a bucket
 * is in its primary location or not. 
 * This distinction is important for two reasons.
 * 1) To reduce the false-positive probability.
 * 2) For deletions. 
 * 
 * 
 * This part of the bucket does not contain the quotients. 
 * 
 * @version 0.1
 * @date 2020-10-12
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef FILTERS_L2_BUCKET_HPP
#define FILTERS_L2_BUCKET_HPP

#include "x86intrin.h"
#include <assert.h>
#include <immintrin.h>
#include <iostream>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <tuple>
#include <unordered_map>
#include <vector>

#define MASK(p) ((1ULL << (p)) - 1ULL)
#define BITWISE_DISJOINT(a, b) (!((a) & (b)))
#define MAX(a, b) (((a) <= (b)) ? (b) : (a))
#define GAP (4)

typedef std::vector<bool> b_vec;
typedef std::tuple<uint64_t, uint8_t, bool> key;
// typedef std::unordered_map<key, size_t> keymap;

typedef unsigned __int128 u128;

namespace my_map {

    // typedef std::tuple<int, char, char> key;

    struct key_hash : public std::unary_function<key, std::size_t> {
        std::size_t operator()(const key &k) const {
            auto res = (std::get<0>(k) << 8ul) | std::get<1>(k);
            res = (res << 1ul) | std::get<2>(k);
            return res;
        }
    };

    struct key_equal : public std::binary_function<key, key, bool> {
        bool operator()(const key &v0, const key &v1) const {
            return (
                    std::get<0>(v0) == std::get<0>(v1) &&
                    std::get<1>(v0) == std::get<1>(v1) &&
                    std::get<2>(v0) == std::get<2>(v1));
        }
    };


    typedef std::unordered_map<const key, size_t, key_hash, key_equal> map_t;

}// namespace my_map

template<size_t m>
bool is_divisible(size_t n) {
    return (n % m) == 0;
}

template<typename T>
bool is_divisible(T n, T m) {
    return (n % m) == 0;
}


namespace print_memory {

    /**
     * @brief 
     * LE is for little endian.
     * @tparam T 
     * @param word 
     * @param gap 
     * @return std::string 
     */
    template<typename T>
    auto str_word_LE(T word, size_t gap) -> std::string {
        constexpr unsigned slot_size = sizeof(word) * CHAR_BIT;
        T b = 1ULL;
        std::string res = "";
        for (size_t i = 0; i < slot_size; i++) {
            if ((i > 0) && (is_divisible(i, gap * gap))) {
                res += "|";
            } else if ((i > 0) && (is_divisible(i, gap))) {
                res += ".";
            }

            res += (word & b) ? "1" : "0";
            b <<= 1ul;
        }
        return res;
    }


    template<typename T>
    void print_word_LE(T word, size_t gap) {
        std::cout << str_word_LE(word, gap) << std::endl;
    }

    // template<>
    // auto str_word_LE<u128>(u128 word, size_t gap) -> std::string {
    //     constexpr unsigned slot_size = 128;
    //     u128 b = 1ULL;
    //     std::string res = "";
    //     for (size_t i = 0; i < slot_size; i++) {
    //         if ((i > 0) && (is_divisible(i, gap * gap))) {
    //             res += "|";
    //         } else if ((i > 0) && (is_divisible(i, gap))) {
    //             res += ".";
    //         }

    //         res += (word & b) ? "1" : "0";
    //         b <<= 1ul;
    //     }
    //     return res;
    // }
    // template<>
    // void print_word_LE<u128>(u128 word, size_t gap) {
    //     std::cout << str_word_LE<u128>(word, gap) << std::endl;
    // }

    template<typename T>
    void print_array(T *a, size_t a_size) {
        std::cout << "[" << a[0];
        for (size_t i = 1; i < a_size; i++) {
            std::cout << ", " << a[i];
        }
        std::cout << "] " << std::endl;
    }

    template<typename T>
    void print_array_with_indexes(T *a, size_t a_size) {
        std::cout << "[0:" << a[0];
        for (size_t i = 1; i < a_size; i++) {
            std::cout << ", " << i << ":" << a[i];
        }
        std::cout << "] " << std::endl;
    }
    template<typename T>
    void print_array_with_nonzero_indexes(T *a, size_t a_size) {
        std::cout << "[";

        size_t i = 0;
        while (!a[i]) { i++; }

        std::cout << i << ":" << a[i];
        i++;
        for (; i < a_size; i++) {
            if (a[i]) {
                std::cout << ", " << i << ":" << a[i];
            }
        }
        std::cout << "]" << std::endl;
    }


    template<typename T>
    auto str_array_LE(T *a, size_t a_size, size_t gap) -> std::string {
        std::string res = str_word_LE(a[0], gap);
        for (size_t i = 1; i < a_size; i++) {
            res += "|";
            res += str_word_LE(a[i], gap);
        }
        return res;
    }

    template<typename T>
    void print_array_LE(T *a, size_t a_size, size_t gap) {
        std::cout << str_array_LE(a, a_size, gap) << std::endl;
    }


}// namespace print_memory


template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
constexpr size_t get_l2_bucket_bit_size() {
    constexpr size_t mask_part = max_capacity;
    constexpr size_t pd_index_part = batch_size + max_capacity;
    // constexpr size_t quot_part = 4 * max_capacity;
    constexpr size_t rem_part = bits_per_item * max_capacity;
    constexpr size_t min_size = mask_part + pd_index_part + rem_part;
    constexpr size_t rounded_size = ((min_size + (64 - 1)) / 64) * 64;
    return rounded_size;
}

template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
constexpr size_t get_l2_bucket_size() {
    return get_l2_bucket_bit_size<max_capacity, batch_size, bits_per_item>() / (sizeof(uint64_t) * CHAR_BIT);
}

template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
void init_vec(std::vector<bool> *v) {
    size_t total_size = get_l2_bucket_bit_size<max_capacity, batch_size, bits_per_item>();
    size_t index = 0;
    size_t offset = 0;


    //PD_index part
    for (; index < offset + batch_size; index++) {
        v->at(index) = 1;
    }
    offset = index;
    for (; index < offset + max_capacity; index++) {
        v->at(index) = 0;
    }

    //Mask_part
    offset = index;
    for (; index < offset + max_capacity; index++) {
        v->at(index) = 0;
    }

    //rem part
    for (; index < total_size; index++) {
        v->at(index) = 0;
    }
}

int compare_vector_and_tpd(b_vec *v, const uint64_t *arr, size_t bits_to_compare);

// returns the position (starting from 0) of the jth set bit of x.
inline uint64_t select64(uint64_t x, int64_t j) {
    assert(j < 64);
    const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
    return _tzcnt_u64(y);
}

inline uint64_t select128withPop64(unsigned __int128 x, int64_t j, int64_t pop) {
    if (j < pop) return select64(x, j);
    return 64 + select64(x >> 64, j - pop);
}

inline int popcount128(unsigned __int128 x) {
    const uint64_t hi = x >> 64;
    const uint64_t lo = x;
    return _popcnt64(lo) + _popcnt64(hi);
}

inline uint64_t select128(unsigned __int128 x, int64_t j) {
    const int64_t pop = _mm_popcnt_u64(x);
    if (j < pop)
        return select64(x, j);
    return 64 + select64(x >> 64, j - pop);
}

// uint64_t get_select_mask_naive(uint64_t x, int64_t j);
/**
 * @brief Get the select mask object
 * 
 * keeps the (j)'th 1 and the (j + 1)'th 1 in x turn on, and turn off the other bits.  
 * @param x 
 * @param j >= 0
 * @return uint64_t 
 */
inline uint64_t get_select_mask(uint64_t x, int64_t j) {
    assert(_mm_popcnt_u64(x) > j);
    return _pdep_u64(3ul << (j), x);
}


/**
 * @brief 
 * 
 * @param x a number with only two set bits. (62 zeros, and 2 ones).
 * @return uint64_t turn on the bits between the currently set bits. 
 *         Also turn off the previously turned on bits. 
 */
inline uint64_t mask_between_bits(uint64_t x) {

    assert(_mm_popcnt_u64(x) == 2);
    uint64_t hi_bit = (x - 1) & x;
    uint64_t clear_hi = hi_bit - 1;
    uint64_t lo_set = (x - 1);
    uint64_t res = (clear_hi ^ lo_set) & (~x);
    return res;
}

inline u128 get_mask_from_header(u128 header, uint64_t index) {
    if (index == 0) {
        // std::cout << "get_mask 1" << std::endl;
        return (header - 1) & (~header);
    }
    uint64_t hi = header >> 64;
    uint64_t lo = header;

    uint64_t pop = _mm_popcnt_u64(lo);
    if (index < pop) {
        uint64_t temp_mask = get_select_mask(lo, index - 1);
        uint64_t temp_mask2 = get_select_mask(lo, index - 2);
        /* if (_mm_popcnt_u64(temp_mask) != 2) {

            std::cout << "pop:     \t" << pop << std::endl;
            std::cout << "index:   \t" << index << std::endl;
            // std::cout << "index:   \t" << index << std::endl;


            std::cout << "header:       \t";
            print_memory::print_word_LE(header, GAP);
            // std::cout << std::endl;
            // std::cout << "pop:          \t";
            // print_memory::print_word_LE(pop, GAP);
            // std::cout << std::endl;
            // std::cout << "index:        \t";
            // print_memory::print_word_LE(index, GAP);
            // std::cout << std::endl;
            std::cout << "lo:           \t";
            print_memory::print_word_LE(lo, GAP);
            // std::cout << std::endl;
            std::cout << "temp_mask:    \t";
            print_memory::print_word_LE(temp_mask, GAP);
            // std::cout << std::endl;
            // auto temp_mask2 = get_select_mask(lo, index - 2);
            std::cout << "temp_mask2:    \t";
            print_memory::print_word_LE(temp_mask2, GAP);
            get_select_mask(lo, index - 1);
            assert(0);
        }
 */
        assert(_mm_popcnt_u64(temp_mask) == 2);
        uint64_t res = mask_between_bits(temp_mask);
        // std::cout << "get_mask 2" << std::endl;
        return res;
    } else if (index == pop) {
        uint64_t lo_temp_mask = get_select_mask(lo, index - 1);// keeping only the highest set bit.
        assert(_popcnt64(lo_temp_mask) == 1);
        uint64_t lo_mask = ~(lo_temp_mask | (lo_temp_mask - 1));
        uint64_t hi_mask = (hi - 1) & (~hi);
        // std::cout << "lo:       " << print_memory::str_word_LE(lo, GAP) << std::endl;
        // std::cout << "hi:       " << print_memory::str_word_LE(hi, GAP) << std::endl;
        // std::cout << std::endl;
        // std::cout << "lo_tmask: " << print_memory::str_word_LE(lo_temp_mask, GAP) << std::endl;
        // std::cout << "lo_mask:  " << print_memory::str_word_LE(lo_mask, GAP) << std::endl;
        // std::cout << "hi_mask:  " << print_memory::str_word_LE(hi_mask, GAP) << std::endl;
        u128 mask = lo_mask | (((u128) hi_mask) << 64ULL);
        // std::cout << "get_mask 3" << std::endl;
        return mask;
    } else {
        uint64_t new_index = index - pop;
        assert(new_index >= 1);
        uint64_t temp_mask = get_select_mask(hi, new_index - 1);
        uint64_t hi_mask = mask_between_bits(temp_mask);
        u128 mask = ((u128) hi_mask) << 64ULL;
        // std::cout << "get_mask 4" << std::endl;
        return mask;
    }
}

auto count_zeros_up_to_the_kth_one(uint64_t word, size_t k) -> size_t;

auto count_zeros_up_to_the_kth_one(u128 word, size_t k) -> size_t;

auto count_ones_up_to_the_kth_zero(uint64_t word, size_t k) -> size_t;

auto count_zeros_up_to_the_kth_one(const uint64_t *pd, size_t k) -> size_t;

auto count_ones_up_to_the_kth_zero(const uint64_t *pd, size_t k) -> size_t;


namespace bits_memcpy {

    /**
     * @brief Copy 'length' bits from src, according to the given limits.
     * Assumptions: 
     * 1) dest_start is byte aligned.
     * 2) length < CHAR_BIT; (maybe length <= CHAR_BIT is enough)
     * 
     * @tparam T 
     * @tparam S 
     * @param src 
     * @param dest 
     * @param src_start 
     * @param dest_start 
     * @param length 
     */
    template<typename T, typename S>
    void fix_first_word_start(T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        assert(length < CHAR_BIT);
        assert(is_divisible<CHAR_BIT>(dest_start));
        uint8_t *src_start_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);
        uint8_t *dest_start_pointer = ((uint8_t *) dest) + (dest_start / CHAR_BIT);

        uint16_t temp_dest = 0;
        memcpy(&temp_dest, src_start_pointer, 2);
        // uint16_t old_temp_dest = temp_dest;
        temp_dest = (temp_dest >> (src_start % CHAR_BIT)) & MASK(length);
        assert(BITWISE_DISJOINT(temp_dest, dest_start_pointer[0]));
        dest_start_pointer[0] |= temp_dest;
    }

    /**
     * @brief Copy 'length' bits from src, according to the given limits.
     * Assumptions: 
     * 1) dest_start + length is byte aligned.
     * 2) length < CHAR_BIT; (maybe length <= CHAR_BIT is enough)
     * 
     * @tparam T 
     * @tparam S 
     * @param src 
     * @param dest 
     * @param src_start 
     * @param dest_start 
     * @param length 
     */
    template<typename T, typename S>
    void fix_first_word_end(T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        assert(length < CHAR_BIT);
        assert(is_divisible<CHAR_BIT>(dest_start + length));

        uint8_t *src_start_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);
        uint8_t *dest_start_pointer = ((uint8_t *) dest) + (dest_start / CHAR_BIT);
        uint64_t mask = MASK(length);

        // uint8_t temp_dest = 0;
        uint16_t temp_dest = 0;
        memcpy(&temp_dest, src_start_pointer, 2);
        temp_dest = (temp_dest >> (src_start % CHAR_BIT)) & mask;
        uint8_t hi = temp_dest << (CHAR_BIT - length);
        assert(_popcnt64(hi) == _popcnt64(temp_dest));
        // assert(BITWISE_DISJOINT(hi, dest_start_pointer[0]));
        // dest_is_aligned(src, &temp_dest, src_start, 0, length);

        uint8_t lo = dest_start_pointer[0] & mask;

        assert(BITWISE_DISJOINT(hi, lo));
        dest_start_pointer[0] = hi | lo;
    }


    template<typename T, typename S>
    void copy_inside_word_boundaries(T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        constexpr unsigned slot_size = sizeof(dest[0]) * CHAR_BIT;
        uint64_t shift = (dest_start) % slot_size;
        assert(length + shift <= slot_size);

        uint64_t mask = MASK(length);
        uint8_t *src_start_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);

        uint64_t temp_dest = 0;
        size_t number_of_bytes_to_copy = (length + CHAR_BIT - 1) / CHAR_BIT;
        if ((src_start % CHAR_BIT) != 0) {
            number_of_bytes_to_copy += 1;
        }

        memcpy(&temp_dest, src_start_pointer, number_of_bytes_to_copy);

        temp_dest = (temp_dest >> (src_start % CHAR_BIT)) & mask;// clearing lower bits that where copied.

        uint64_t index = dest_start / slot_size;
        uint64_t s_mask = mask << shift;
        uint64_t old_part = (dest[index]) & (~s_mask);
        uint64_t new_part = temp_dest << shift;

        assert(BITWISE_DISJOINT(old_part, new_part));
        dest[index] = new_part | old_part;
    }


    template<typename T, typename S>
    void copy_recursive(const T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        constexpr unsigned slot_size = sizeof(dest[0]) * CHAR_BIT;
        uint64_t shift = (dest_start) & (slot_size - 1);

        //base_case
        if (shift + length <= slot_size) {
            copy_inside_word_boundaries(src, dest, src_start, dest_start, length);
            return;
        }

        size_t first_length = slot_size - shift;
        size_t second_length = length - first_length;

        copy_inside_word_boundaries(src, dest, src_start, dest_start, first_length);
        copy_recursive(src, dest, src_start + first_length, dest_start + first_length, second_length);
    }


    template<typename T>
    void shift_array_towards_the_start(T *a, size_t start, size_t end, size_t shift) {
        constexpr unsigned slot_size = sizeof(a[0]) * CHAR_BIT;
        assert(is_divisible<slot_size>(start));
        assert(shift < slot_size);

        size_t i = start / slot_size;
        size_t last_index = end / slot_size;
        for (; i < last_index; i++) {
            a[i] >>= shift;
            uint64_t temp = (a[i + 1] << (slot_size - shift));
            a[i] |= temp;
        }

        size_t rel_index = end % slot_size;
        uint64_t mask = MASK(rel_index);
        T lo = (a[last_index] & mask) >> shift;
        T hi = a[last_index] & (~mask);
        assert(BITWISE_DISJOINT(hi, lo));
        a[last_index] = hi | lo;
    }

    /**
     * @brief 
     * 
     * @tparam T 
     * @param a 
     * @param start 
     * @param end the last bit that will be changed. meaning we write 'end - start' + 1 bits. 
     * @param shift 
     */
    template<typename T>
    void shift_array_towards_the_end_word_aligned(T *a, size_t start, size_t end, size_t shift) {
        constexpr unsigned slot_size = sizeof(a[0]) * CHAR_BIT;
        assert(is_divisible<slot_size>(start));
        assert(shift < slot_size);


        size_t last_index = end / slot_size;
        size_t first_index = start / slot_size;
        T last_slot = a[last_index];

        if (first_index == last_index) {
            size_t rel_index = end % slot_size;
            uint64_t mask = MASK(rel_index);
            T lo = (a[last_index] << shift) & mask;
            T hi = a[last_index] & (~mask);
            assert(BITWISE_DISJOINT(hi, lo));
            a[last_index] = hi | lo;
            return;
        }

        size_t i = last_index;

        for (; i > first_index; i--) {
            a[i] <<= shift;
            uint64_t temp = (a[i - 1] >> (slot_size - shift));
            a[i] |= temp;
        }

        assert(i == first_index);
        a[i] <<= shift;

        size_t rel_index = end % slot_size;
        uint64_t mask = MASK(rel_index);
        T lo = a[last_index] & mask;
        T hi = last_slot & (~mask);
        assert(BITWISE_DISJOINT(hi, lo));
        a[last_index] = hi | lo;
    }

    /**
     * @brief 
     * start must be byte aligned.
     * @tparam T 
     * @param a 
     * @param start 
     * @param end 
     * @param shift 
     */
    template<typename T>
    void shift_array_towards_the_end(T *a, size_t start, size_t end, size_t shift) {
        assert(is_divisible<CHAR_BIT>(start));

        constexpr unsigned slot_size = sizeof(a[0]) * CHAR_BIT;
        if (is_divisible<slot_size>(start)) {
            shift_array_towards_the_end_word_aligned(a, start, end, shift);
        } else if (is_divisible<CHAR_BIT * 2>(start)) {
            uint16_t *pointer = ((uint16_t *) a);
            shift_array_towards_the_end_word_aligned<uint16_t>(pointer, start, end, shift);
        } else {
            uint8_t *pointer = ((uint8_t *) a);
            shift_array_towards_the_end_word_aligned<uint8_t>(pointer, start, end, shift);
        }
    }

    template<typename T, typename S>
    void beginnings_are_aligned(T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        assert((src_start & 7) == 0);
        assert((dest_start & 7) == 0);

        uint8_t *src_start_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);
        uint8_t *dest_start_pointer = ((uint8_t *) dest) + (dest_start / CHAR_BIT);
        const unsigned Bytes2Copy = length / CHAR_BIT;
        memcpy(dest_start_pointer, src_start_pointer, Bytes2Copy);

        if (is_divisible<CHAR_BIT>(length))
            return;


        //fix last word
        uint8_t old_val = (dest_start_pointer + Bytes2Copy)[0];
        uint8_t new_val = (src_start_pointer + Bytes2Copy)[0];
        uint64_t index = length % CHAR_BIT;
        uint64_t mask = MASK(index);
        uint8_t hi = old_val & (~mask);
        uint8_t lo = new_val & mask;
        assert(BITWISE_DISJOINT(hi, lo));
        uint8_t val = hi | lo;
        (dest_start_pointer + Bytes2Copy)[0] = val;
    }

    template<typename T, typename S>
    void dest_is_aligned(T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        assert(is_divisible<CHAR_BIT>(dest_start));

        if (is_divisible<CHAR_BIT>(src_start)) {
            beginnings_are_aligned(src, dest, src_start, dest_start, length);
            return;
        }


        // 1) Copy bits as if the arrays are both aligned.
        // 2) Shift dest to the correct location.
        // 3) fix first word start.

        // 1)
        auto bit_count_to_next_byte = CHAR_BIT - (src_start % CHAR_BIT);
        auto shift = bit_count_to_next_byte;
        auto new_src_start = src_start + bit_count_to_next_byte;
        assert(is_divisible<CHAR_BIT>(new_src_start));

        beginnings_are_aligned(src, dest, new_src_start, dest_start, length - bit_count_to_next_byte);

        // 2)
        auto last_bit_to_change = dest_start + length;
        shift_array_towards_the_end(dest, dest_start, last_bit_to_change, shift);

        // 3)
        fix_first_word_start(src, dest, src_start, dest_start, shift);
    }

    template<typename T, typename S>
    void copy(T *src, S *dest, size_t src_start, size_t dest_start, size_t length) {
        if (length < 32) {
            copy_recursive(src, dest, src_start, dest_start, length);
            return;
        }
        auto dest_alignment_offset = CHAR_BIT - (dest_start % CHAR_BIT);
        auto short_name = dest_alignment_offset;

        //reduction to dest is aligned, src is not:
        dest_is_aligned(src, dest, src_start + short_name, dest_start + short_name, length - short_name);
        // and fixing the beginning.
        copy_inside_word_boundaries(src, dest, src_start, dest_start, short_name);
    }


}// namespace bits_memcpy


template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
class L2Bucket_mq {
    static_assert(max_capacity <= 64, "max_capacity is too big");
    static_assert(max_capacity + batch_size <= 128, "pd_index header does not fit inside 128 bit");

    static constexpr size_t bucket_size = get_l2_bucket_size<max_capacity, batch_size, bits_per_item>();

public:
    uint64_t pd[bucket_size] __attribute__((aligned(64)));

    L2Bucket_mq() {
        constexpr unsigned bit_size = bucket_size * sizeof(pd[0]) * CHAR_BIT;
        std::vector<bool> v(bit_size, false);
        init_vec<max_capacity, batch_size, bits_per_item>(&v);
        // constexpr size_t zeros_left = max_capacity - (64 - batch_size);
        constexpr uint64_t empty_pd[get_l2_bucket_size<max_capacity, batch_size, bits_per_item>()] = {MASK(batch_size)};
        int cmp_res = compare_vector_and_tpd(&v, empty_pd, bit_size);
        assert(cmp_res == -1);
        memcpy(pd, empty_pd, (bit_size + 7) / 8);
        int cmp_res2 = compare_vector_and_tpd(&v, pd, bit_size);
        assert(cmp_res2 == -1);
    }

    L2Bucket_mq(const L2Bucket_mq &temp_bucket) {
        for (size_t i = 0; i < bucket_size; i++) {
            pd[i] = temp_bucket.pd[i];
        }
    }


    static constexpr auto get_pd_header_size() -> size_t {
        return max_capacity + batch_size;
    }

    static constexpr auto get_location_bitmask_size() -> size_t {
        return max_capacity;
    }

    auto get_capacity() -> size_t {
        auto pd_index_res = count_zeros_up_to_the_kth_one(pd, batch_size - 1);
        return pd_index_res;
    }

    bool is_full() {
        return get_capacity() == max_capacity;
    }

    void read_pd_header(uint64_t *dest) {
        constexpr unsigned end = max_capacity + batch_size;
        read_bits(pd, dest, 0, end);
        // constexpr unsigned kBytes2copy = (max_capacity + batch_size + CHAR_BIT - 1) / CHAR_BIT;
        // constexpr unsigned last_index = (max_capacity + batch_size + 63) / 64;
        // constexpr unsigned mask = (max_capacity + batch_size) & 63;
        // memcpy(dest, pd, kBytes2copy);
        // bool cond = ((dest[last_index] & MASK(mask)) == dest[last_index]);
        // assert(cond);
    }

    unsigned __int128 get_pd_header() {
        unsigned __int128 x = 0;
        // uint64_t y[2] = {0};
        // constexpr size_t temp = max_capacity;
        constexpr unsigned kBytes2copy = (max_capacity + batch_size + CHAR_BIT - 1) / CHAR_BIT;
        //        this->pd
        //        print_memory::print_array(pd, 8);
        // memcpy(y, this->pd, 1);
        // memcpy(y, this->pd, kBytes2copy);
        memcpy(&x, pd, kBytes2copy);
        assert(popcount128(x) == batch_size);
        return x;
    }

    // void read_quot_header(uint64_t *dest) {
    //     constexpr unsigned start = max_capacity + batch_size;
    //     constexpr unsigned size = max_capacity + quot_range;
    //     read_bits(pd, dest, start, start + size);
    // }
    // unsigned __int128 get_quot_header() {
    //     uint64_t a[2] = {0};
    //     read_quot_header(a);
    //     constexpr unsigned kBytes2copy = (max_capacity + quot_range + CHAR_BIT - 1) / CHAR_BIT;
    //     unsigned __int128 x;
    //     memcpy(&x, a, kBytes2copy);
    //     return x;
    // }

    uint64_t get_location_bitmask_header() {
        constexpr unsigned start = get_pd_header_size();
        constexpr unsigned size = get_location_bitmask_size();
        uint64_t dest = 0;
        bits_memcpy::copy(pd, &dest, start, 0, size);
        return dest;
    }

    /**
     * @brief location bitmask is a bitmask of size max_capacity. It is used to determine if the element there
     * is in its primary location or not. This distinction is important for two reasons.
     * The first is to reduce the false-positive probability, and the second is for deletions.  
     * 
     * @param indedx 
     * @return true 
     * @return false 
     */
    bool test_location_on_bitmask(uint64_t index) {
        assert(index < 64);
        uint64_t dest = get_location_bitmask_header();
        return dest & (1ULL << index);
    }

    void write_new_location_bitmask(uint64_t new_bitmask) {
        constexpr auto dest_start = get_pd_header_size();
        constexpr auto length = get_location_bitmask_size();
        bits_memcpy::copy(&new_bitmask, pd, 0, dest_start, length);
    }

    void insert_bit_to_location_bitmask(uint64_t index, bool value) {
        uint64_t mask = MASK(index);
        uint64_t dest = get_location_bitmask_header();
        uint64_t lo = dest & mask;
        uint64_t mid = (value) ? (1ULL << index) : 0;
        uint64_t hi = (dest & (~mask)) << 1ul;
        uint64_t new_dest = lo | mid | hi;
        write_new_location_bitmask(new_dest);
    }

    void remove_bit_from_location_bitmask(uint64_t index) {
        uint64_t dest = get_location_bitmask_header();
        const uint64_t mask = MASK(index);
        uint64_t lo_dest = dest & mask;
        uint64_t hi_dest = (dest >> 1ul) & (~mask);
        uint64_t new_dest = hi_dest | lo_dest;
        /* std::cout << std::string(80, '~') << std::endl;
        std::cout << "index:      \t";
        print_memory::print_word_LE(index, GAP);
        std::cout << "dest:       \t";
        print_memory::print_word_LE(dest, GAP);
        std::cout << "new_dest:   \t";
        print_memory::print_word_LE(new_dest, GAP);
        std::cout << std::string(80, '~') << std::endl; */
        write_new_location_bitmask(new_dest);
    }

    uint64_t find_naive(uint64_t pd_index, uint8_t rem, bool is_primary_location) {
        assert(pd_index < batch_size);
        //        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        //        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = body_find(rem);

        if (!v) return 0;

        u128 header = get_pd_header();
        assert(popcount128(header) == batch_size);
        const int64_t pop = _mm_popcnt_u64(header);
        // const int64_t pop = popcount128(header);

        const uint64_t begin = (pd_index ? (select128withPop64(header, pd_index - 1, pop) + 1) : 0) - pd_index;
        const uint64_t end = select128withPop64(header, pd_index, pop) - pd_index;
        if (begin == end) return false;
        if (begin > end) {
            print_find_arg(pd_index, rem, is_primary_location);
            print_pd();
            std::cout << "pd_index: " << pd_index << std::endl;
            std::cout << "begin: " << begin << std::endl;
            std::cout << "end: " << end << std::endl;
            assert(0);
        }
        assert(begin <= end);
        assert(end <= max_capacity);
        // u128 mask = ((u128(1) << end) - 1) ^ ((u128(1) << end) - 1)
        uint64_t mask = (MASK(end) ^ MASK(begin)) | (1ULL << begin);
        // if (begin)
        // mask ^= MASK(begin - 1);
        // uint64_t temp_res = (v & ((UINT64_C(1) << end) - 1)) >> begin;
        uint64_t temp_res = v & mask;
        if (!temp_res) return 0;
        return temp_res & get_location_bitmask(is_primary_location);
    }


    uint64_t find(uint64_t pd_index, uint8_t rem, bool is_primary_location) {
        uint64_t v = body_find(rem);
        u128 pd_mask = get_pd_header_mask(pd_index) >> pd_index;// todo.
        uint64_t location_mask = get_location_bitmask(is_primary_location);
        auto res = v & pd_mask & location_mask;
        auto naive_res = find_naive(pd_index, rem, is_primary_location);
        if (naive_res != res) {
            std::cout << "naive_res:     \t";
            print_memory::print_word_LE(naive_res, GAP);
            std::cout << "res            \t";
            print_memory::print_word_LE(((uint64_t) res), GAP);
            print_location_mask();
            std::cout << std::string(80, '~') << std::endl;
            std::cout << "pd_mask:     \t";
            print_memory::print_word_LE((uint64_t) pd_mask, GAP);
            std::cout << "v:           \t";
            print_memory::print_word_LE(v, GAP);
            std::cout << std::string(80, '~') << std::endl;

            print_find_arg(pd_index, rem, is_primary_location);
            print_pd();
            find_naive(pd_index, rem, is_primary_location);
            find_naive(pd_index, rem, is_primary_location);
            assert(0);
        }
        assert(naive_res == res);
        return res;
    }

    uint64_t find_print_db(uint64_t pd_index, uint8_t rem, bool is_primary_location) {
        uint64_t v = body_find(rem);
        u128 pd_mask = get_pd_header_mask(pd_index) >> pd_index;// todo.
        uint64_t location_mask = get_location_bitmask(is_primary_location);
        // std::cout << "naive_res:     \t";
        // print_memory::print_word_LE(naive_res, GAP);
        // std::cout << "res            \t";
        // print_memory::print_word_LE(((uint64_t) res), GAP);
        std::cout << std::string(80, '~') << std::endl;
        std::cout << std::string(80, '~') << std::endl;
        print_find_arg(pd_index, rem, is_primary_location);
        std::cout << "locBitMask:  \t";
        print_memory::print_word_LE(get_location_bitmask(is_primary_location), GAP);
        std::cout << "header:      \t";
        print_memory::print_word_LE((uint64_t) get_pd_header(), GAP);
        std::cout << "nsh_pdMask:  \t";
        print_memory::print_word_LE((uint64_t) get_pd_header_mask(pd_index), GAP);
        std::cout << "sh_header:   \t";
        print_memory::print_word_LE((uint64_t) (get_pd_header() >> pd_index), GAP);
        std::cout << "pd_mask:     \t";
        print_memory::print_word_LE((uint64_t) pd_mask, GAP);
        std::cout << "v:           \t";
        print_memory::print_word_LE(v, GAP);
        std::cout << "shifted_ v:  \t";
        print_memory::print_word_LE(v << pd_index, GAP);
        std::cout << std::string(80, '~') << std::endl;
        std::cout << std::string(80, '~') << std::endl;
        print_pd();


        auto res = v & pd_mask & location_mask;
        auto naive_res = find_naive(pd_index, rem, is_primary_location);
        if (naive_res != res) {
            std::cout << "naive_res:     \t";
            print_memory::print_word_LE(naive_res, GAP);
            std::cout << "res            \t";
            print_memory::print_word_LE(((uint64_t) res), GAP);
            print_location_mask();
            std::cout << std::string(80, '~') << std::endl;
            std::cout << "pd_mask:     \t";
            print_memory::print_word_LE((uint64_t) pd_mask, GAP);
            std::cout << "v:           \t";
            print_memory::print_word_LE(v, GAP);
            std::cout << std::string(80, '~') << std::endl;

            print_find_arg(pd_index, rem, is_primary_location);
            print_pd();
            find_naive(pd_index, rem, is_primary_location);
            find_naive(pd_index, rem, is_primary_location);
            assert(0);
        }
        assert(naive_res == res);
        return res;
    }

    uint64_t find_true_db(uint64_t pd_index, uint8_t rem, bool is_primary_location) {
        assert(pd_index < batch_size);
        uint64_t v = body_find(rem);
        u128 pd_mask_not_shifted = get_pd_header_mask(pd_index);
        u128 pd_mask = get_pd_header_mask(pd_index) >> pd_index;
        uint64_t location_mask = get_location_bitmask(is_primary_location);
        //todo: error of specification of the template print function of type u128.
        if (!v) {
            assert(0);
        }
        if (!pd_mask) {
            print_find_arg(pd_index, rem, is_primary_location);
            u128 complete_pd_index_header = get_pd_header();
            std::cout << "completePDindexHeader:  ";
            print_memory::print_word_LE(complete_pd_index_header, GAP);
            std::cout << "pd_maskNS:\t\t";
            print_memory::print_word_LE(pd_mask_not_shifted, GAP);
            std::cout << "pd_mask:  \t\t";
            print_memory::print_word_LE(pd_mask, GAP);
            assert(0);
        }
        if (!location_mask) {
            assert(0);
        }
        auto res = v & pd_mask & location_mask;
        if (!res) {
            std::cout << "pd_maskNS:\t\t";
            print_memory::print_word_LE(pd_mask_not_shifted, GAP);
            std::cout << "v:        \t\t";
            print_memory::print_word_LE(v, GAP);
            std::cout << "pd_mask:  \t\t";
            print_memory::print_word_LE(pd_mask, GAP);
            std::cout << "loc_mask: \t\t";
            print_memory::print_word_LE(location_mask, GAP);
        }
        assert(res);

        return res;
        return v & pd_mask & location_mask;
    }

    uint64_t body_find(uint8_t rem) {
        constexpr bool does_bucket_size_is_512_bit =
                get_l2_bucket_bit_size<max_capacity, batch_size, bits_per_item>() == 512;
        if (does_bucket_size_is_512_bit) {
            constexpr unsigned shift = 64 - max_capacity;
            // const __m512i body = _mm512_load_si512(pd);
            const __m512i target = _mm512_set1_epi8(rem);
            return _mm512_cmpeq_epu8_mask(target, *((__m512i *) pd)) >> shift;
        }
        constexpr unsigned bucket_size = get_l2_bucket_size<max_capacity, batch_size, bits_per_item>();
        constexpr unsigned shift = 64 - max_capacity;
        assert(bucket_size >= 8);
        uint64_t *address = &(pd[0]) + (bucket_size - 8);
        const __m512i body = _mm512_loadu_si512(address);
        const __m512i target = _mm512_set1_epi8(rem);
        return _mm512_cmpeq_epu8_mask(target, body) >> shift;
    }


    u128 get_pd_header_mask(uint64_t pd_index) {
        auto res = get_mask_from_header(get_pd_header(), pd_index);
        auto pop_res = popcount128(res);
        auto v_res = get_pd_index_capacity(pd_index);
        if (pop_res != v_res) {
            print_pd();
        }
        assert(pop_res == v_res);
        // assert(popcount128(res) == get_pd_index_capacity(pd_index));
        return res;
    }

    uint64_t get_location_bitmask(bool is_primary_location) {
        uint64_t mask = get_location_bitmask_header();
        if (is_primary_location)
            return mask;
        else {
            return (~mask);// & MASK(max_capacity);
        }
    }


    bool insert(uint64_t pd_index, uint8_t rem, bool is_primary_location) {
        assert(!find(pd_index, rem, is_primary_location));
        if (is_full())
            return false;

        assert(pd_index < batch_size);

        unsigned __int128 header = get_pd_header();
        constexpr unsigned kBytes2copy = (get_pd_header_size() + CHAR_BIT - 1) / CHAR_BIT;
        assert(popcount128(header) == batch_size);
        //        return true;
        // const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        // assert((fill <= 14) || (fill == pd_popcount(pd)));
        // assert((fill == 51) == pd_full(pd));
        // if (fill == 51)
        //     return false;


        const uint64_t begin = pd_index ? (select128(header, pd_index - 1) + 1) : 0;
        const uint64_t end = select128(header, pd_index);
        assert(begin <= end);
        assert(end < get_pd_header_size());
        const __m512i target = _mm512_set1_epi8(rem);

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == batch_size);
        // assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);

        const uint64_t begin_fingerprint = begin - pd_index;
        const uint64_t end_fingerprint = end - pd_index;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= max_capacity);


        constexpr unsigned firstBodyByte = (max_capacity * 2 + batch_size + CHAR_BIT - 1) / CHAR_BIT;
        constexpr unsigned size0fPd =
                (get_l2_bucket_bit_size<max_capacity, batch_size, bits_per_item>() + CHAR_BIT - 1) / CHAR_BIT;

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const uint8_t *) pd)[firstBodyByte + i])
                break;
        }

        assert((i == end_fingerprint) || (rem <= ((const uint8_t *) pd)[firstBodyByte + i]));

        memmove(&((uint8_t *) pd)[firstBodyByte + i + 1],
                &((const uint8_t *) pd)[firstBodyByte + i],
                size0fPd - (firstBodyByte + i + 1));
        ((uint8_t *) pd)[firstBodyByte + i] = rem;

        insert_bit_to_location_bitmask(i, is_primary_location);

        // std::cout << std::string(80, '.') << std::endl;
        // std::cout << "index is: \t" << i << std::endl;
        // std::cout << "val is:   \t" << is_primary_location << std::endl;
        // std::cout << std::string(80, '-') << std::endl;
        // print_location_mask();
        // insert_bit_to_location_bitmask(i, is_primary_location);
        // print_location_mask();
        // std::cout << std::string(80, '.') << std::endl;


        assert(find_true_db(pd_index, rem, is_primary_location));
        return true;
    }

    auto conditional_remove_db(int64_t pd_index, uint8_t rem, bool is_primary_location) -> bool {
        //todo::the error is in identifying the index of the remainder we delete. We identify it solely on the body, and don't check the location_bitmask.  
        // bool db_cond = (pd_index == 29) && (rem == 58) && is_primary_location;
        // if (db_cond){
        //     find_print_db(pd_index, rem, is_primary_location);
        //     std::cout << std::string(80, 'H') << std::endl;
        // }
        uint64_t old_find_res = find(pd_index, rem, is_primary_location);
        assert(find(pd_index, rem, is_primary_location));
        assert(pd_index < batch_size);

        auto old_header = get_pd_header();
        auto old_location_bit_mask = get_location_bitmask(is_primary_location);
        auto old_capacity = get_capacity();
        uint64_t old_capacity_list[batch_size] = {0};// todo
        get_pd_index_capacity_list(old_capacity_list);
        uint64_t old_v = body_find(rem);

        unsigned __int128 header = get_pd_header();
        constexpr unsigned kBytes2copy = (get_pd_header_size() + CHAR_BIT - 1) / CHAR_BIT;

        assert(popcount128(header) == batch_size);
        const uint64_t begin = pd_index ? (select128(header, pd_index - 1) + 1) : 0;
        const uint64_t end = select128(header, pd_index);
        assert(begin <= end);
        assert(end < get_pd_header_size());

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        if (popcount128(new_header) != batch_size) {
            print_find_arg(pd_index, rem, is_primary_location);
            std::cout << "header:     \t";
            print_memory::print_word_LE(header, GAP);
            std::cout << "new_header: \t";
            print_memory::print_word_LE(new_header, GAP);
            std::cout << std::string(80, '=') << std::endl;
            assert(0);
        }
        assert(popcount128(new_header) == batch_size);

        const uint64_t begin_fingerprint = begin - pd_index;
        const uint64_t end_fingerprint = end - pd_index;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= max_capacity);

        constexpr unsigned firstBodyByte = (max_capacity * 2 + batch_size + CHAR_BIT - 1) / CHAR_BIT;
        constexpr unsigned size0fPd =
                (get_l2_bucket_bit_size<max_capacity, batch_size, bits_per_item>() + CHAR_BIT - 1) / CHAR_BIT;

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if ((rem <= ((const uint8_t *) pd)[firstBodyByte + i])){
                //add test here.
                break;
            }
                
        }
        if ((i == end_fingerprint) || (rem != ((const uint8_t *) pd)[firstBodyByte + i])) {
            assert(0);
            return false;
        }
        uint64_t find_res = find(pd_index, rem, is_primary_location);
        auto find_res_i = _tzcnt_u64(find_res);
        i = find_res_i;

        assert(popcount128(new_header) == batch_size);
        memcpy(pd, &new_header, kBytes2copy);

        memmove(&((uint8_t *) pd)[firstBodyByte + i],
                &((const uint8_t *) pd)[firstBodyByte + i + 1],
                size0fPd - (firstBodyByte + i + 1));

        remove_bit_from_location_bitmask(i);
        /* bool res = find(pd_index, rem, is_primary_location);
        if (res) {
            assert(find_res_i == i);

            std::cout << std::string(80, '.') << std::endl;
            print_find_arg(pd_index, rem, is_primary_location);
            std::cout << std::string(80, '.') << std::endl;
            print_memory::print_array_with_nonzero_indexes(old_capacity_list, batch_size);
            std::cout << std::string(80, '*') << std::endl;
            // std::cout << "old_header:            " << old_header << std::endl;
            // std::cout << "old_location_bit_mask: " << old_location_bit_mask << std::endl;
            std::cout << "old_capacity:          " << old_capacity << std::endl;
            std::cout << std::string(80, '^') << std::endl;
            std::cout << "old_header:\t";
            print_memory::print_word_LE(old_header, GAP);
            std::cout << "new_header:\t";
            print_memory::print_word_LE(new_header, GAP);
            std::cout << "old locBit:\t";
            print_memory::print_word_LE(old_location_bit_mask, GAP);
            std::cout << "new locBit:\t";
            print_memory::print_word_LE(get_location_bitmask(is_primary_location), GAP);
            std::cout << "old_v:     \t";
            print_memory::print_word_LE(old_v, GAP);
            std::cout << "new_v:     \t";
            print_memory::print_word_LE(body_find(rem), GAP);
            std::cout << "o_findRes: \t";
            print_memory::print_word_LE(old_find_res, GAP);
            std::cout << "n_findRes: \t";
            print_memory::print_word_LE(find(pd_index, rem, is_primary_location), GAP);
            std::cout << std::string(80, '*') << std::endl;
            print_pd();

            find_print_db(pd_index, rem, is_primary_location);
        }
         */
        assert(!find(pd_index, rem, is_primary_location));


        return true;
    }


    auto conditional_remove(int64_t pd_index, uint8_t rem, bool is_primary_location) -> bool {
        assert(0);
        assert(pd_index < batch_size);

        unsigned __int128 header = get_pd_header();
        constexpr unsigned kBytes2copy = (get_pd_header_size() + CHAR_BIT - 1) / CHAR_BIT;

        assert(popcount128(header) == batch_size);
        const uint64_t begin = pd_index ? (select128(header, pd_index - 1) + 1) : 0;
        const uint64_t end = select128(header, pd_index);
        assert(begin <= end);
        assert(end < get_pd_header_size());

        unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end - 1));
        // if (popcount128(new_header) != batch_size){
        //     print_find_arg(pd_index, rem, is_primary_location);
        //     std::cout << "header:     \t";
        //     print_memory::print_word_LE(header, GAP);
        //     std::cout << "new_header: \t";
        //     print_memory::print_word_LE(new_header, GAP);
        //     std::cout << std::string(80, '=') << std::endl;
        //     assert(0);
        // }
        // assert(popcount128(new_header) == batch_size);

        const uint64_t begin_fingerprint = begin - pd_index;
        const uint64_t end_fingerprint = end - pd_index;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= max_capacity);

        constexpr unsigned firstBodyByte = (max_capacity * 2 + batch_size + CHAR_BIT - 1) / CHAR_BIT;
        constexpr unsigned size0fPd =
                (get_l2_bucket_bit_size<max_capacity, batch_size, bits_per_item>() + CHAR_BIT - 1) / CHAR_BIT;

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const uint8_t *) pd)[firstBodyByte + i])
                break;
        }
        if ((i == end_fingerprint) || (rem != ((const uint8_t *) pd)[firstBodyByte + i])) {
            return false;
        }

        assert(popcount128(new_header) == batch_size);
        memcpy(pd, &new_header, kBytes2copy);

        memmove(&((uint8_t *) pd)[firstBodyByte + i],
                &((const uint8_t *) pd)[firstBodyByte + i + 1],
                size0fPd - (firstBodyByte + i + 1));

        remove_bit_from_location_bitmask(i);
        return true;
    }

    auto get_pd_index_capacity(uint64_t pd_index) {
        u128 header = get_pd_header();
        auto temp = count_zeros_up_to_the_kth_one(header, pd_index);
        if (pd_index) {
            temp -= count_zeros_up_to_the_kth_one(header, pd_index - 1);
        }
        return temp;
    }

    void print_location_mask() {
        uint64_t location_mask = get_location_bitmask(1);
        std::cout << "location mask:     \t";
        print_memory::print_word_LE(location_mask, GAP);
    }

    void print_pd_header() {
        auto header = get_pd_header();
        std::cout << "header:     \t";
        print_memory::print_word_LE(header, GAP);
    }

    void print_pd_header(uint64_t pd_index) {
        auto header = get_pd_header();
        header = get_mask_from_header(header, pd_index);
        std::cout << "rel_pd_index:  \t\t";
        print_memory::print_word_LE(header, GAP);
    }

    void print_find_arg(uint64_t pd_index, uint8_t rem, bool is_primary_location) {
        std::cout << std::string(80, '=') << std::endl;
        // std::cout << "Start" << std::endl;

        std::cout << "pd_index:    \t" << pd_index;
        // std::cout << "\t\t" << (pd_index / CHAR_BIT);
        // std::cout << "\t\t" << (pd_index & 7ul);
        std::cout << std::endl;

        std::cout << "rem:         \t" << ((uint64_t) rem);
        // std::cout << "\t\t" << (dest_start / CHAR_BIT);
        // std::cout << "\t\t" << (dest_start & 7ul);
        std::cout << std::endl;

        std::cout << "is_prime_loc:\t" << is_primary_location;
        // std::cout << "\t\t" << (length / CHAR_BIT);
        // std::cout << "\t\t" << (length & 7ul);
        std::cout << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }


    void print_pd() {
        std::cout << std::string(80, '=') << std::endl;
        std::cout << "capacity:   \t" << get_capacity() << std::endl;
        uint64_t temp[batch_size] = {0};// todo
        get_pd_index_capacity_list(temp);
        std::cout << "pd_index_list: ";
        print_memory::print_array_with_nonzero_indexes(temp, batch_size);
        //to do: sum temp and validate against the capacity.
        std::cout << std::string(80, '-') << std::endl;
        print_pd_header();
        print_location_mask();
        std::cout << std::string(80, '=') << std::endl;
    }

    void get_pd_index_capacity_list(uint64_t *a) {
        //        for (int i = 0; i < max_capacity; ++i) {
        //            a[i] = 0;
        //        }
        u128 header = get_pd_header();

        // uint64_t hi = header >> 64ull;
        // uint64_t temp_hi = hi;
        // uint64_t lo = header;
        // uint64_t temp_lo = lo;

        int counter = 0;
        // auto lo_pop = _mm_popcnt_u64(lo);
        // auto hi_pop = _mm_popcnt_u64(hi);
        // // auto lo_capacity = 64 - _mm_popcnt_u64(lo);
        // // auto hi_capacity = 64 - _mm_popcnt_u64(hi);

        while (counter < batch_size) {
            assert(_mm_popcnt_u64(header));
            auto temp_cap = _tzcnt_u64(header);
            a[counter] = temp_cap;
            counter++;
            header >>= (temp_cap);
            assert(header & 1ul);
            header >>= 1ul;
        }
        assert(header == 0);
    }
};


namespace l2_bucket_tests {
    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    bool rt0() {
        auto bucket = L2Bucket_mq<max_capacity, batch_size, bits_per_item>();
        const uint64_t pd_index = 0;
        for (size_t i = 0; i < max_capacity; i++) {
            bucket.insert(pd_index, i, i & 1ul);
            bool temp = bucket.find_true_db(pd_index, i, i & 1ul);
            if (!temp) {
                std::cout << "i: " << i << std::endl;
                return false;
            }
        }
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto insert_find_single(uint64_t pd_index, uint8_t rem, bool is_prime_location,
                            L2Bucket_mq<max_capacity, batch_size, bits_per_item> *pd) -> bool {
        assert(!pd->is_full());
        L2Bucket_mq<max_capacity, batch_size, bits_per_item> temp_pd(*pd);

        // auto before_last_quot = pd512_plus::decode_last_quot_wrapper(pd);
        auto res = temp_pd.insert(pd_index, rem, is_prime_location);
        assert(res);
        auto lookup_res = temp_pd.find(pd_index, rem, is_prime_location);
        assert(lookup_res);
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto insert_find_all(L2Bucket_mq<max_capacity, batch_size, bits_per_item> *pd) -> bool {
        for (size_t pd_index = 0; pd_index < batch_size; pd_index++) {
            for (size_t rem = 0; rem < 256; rem++) {
                insert_find_single(pd_index, rem, 1, pd);
            }
        }
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto insert_find_all() -> bool {
        L2Bucket_mq<max_capacity, batch_size, bits_per_item> temp_pd;
        return insert_find_all(&temp_pd);
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto rand_test1() -> bool {
        // size_t max_capacity = 51;
        // uint64_t valid_max_quot = 0;
        L2Bucket_mq<max_capacity, batch_size, bits_per_item> temp_pd;
        for (size_t i = 0; i < max_capacity; i++) {
            uint64_t pd_index = rand() % batch_size;
            uint64_t r = rand() & 255;
            bool location_mask = rand() & 1;
            auto res = temp_pd.insert(pd_index, r, location_mask);
            assert(res);
            assert(temp_pd.find(pd_index, r, location_mask));
        }
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto rand_test2() -> bool {
        L2Bucket_mq<max_capacity, batch_size, bits_per_item> temp_pd;
        for (size_t i = 0; i < max_capacity; i++) {
            uint64_t pd_index = rand() % batch_size;
            uint64_t r = rand() & 255;
            bool location_mask = rand() & 1;
            auto res = temp_pd.insert(pd_index, r, location_mask);
            assert(res);
            assert(temp_pd.find(pd_index, r, location_mask));
        }
        return true;
    }


    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto recursive_add_delete_core(L2Bucket_mq<max_capacity, batch_size, bits_per_item> *pd, size_t *reps,
                                   size_t depth) -> bool {
        static int counter = 0;
        if (depth == max_capacity)
            return true;

        assert(depth < max_capacity);

        while (rand() & 1) {
            if (*reps == 0) return true;
            *reps -= 1;
            uint64_t pd_index = rand() % batch_size;
            uint64_t rem = rand() & 255;
            bool location_mask = rand() & 1;

            //prevents insertion of an element that is already in the pd again.
            if (!pd->find(pd_index, rem, location_mask)) {
                auto res = pd->insert(pd_index, rem, location_mask);
                assert(res);
                assert(pd->find(pd_index, rem, location_mask));
            }
            bool rec_res = recursive_add_delete_core(pd, reps, depth + 1);

            if (pd->find(pd_index, rem, location_mask)) {
                bool del_res = pd->conditional_remove_db(pd_index, rem, location_mask);
                assert(del_res);
                bool refind_res = pd->find(pd_index, rem, location_mask);
                assert(!refind_res);
            }
            counter++;
        }
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto recursive_add_delete(size_t reps) -> bool {
        auto pd = L2Bucket_mq<max_capacity, batch_size, bits_per_item>();
        size_t counter = reps;
        while (counter) {
            if (counter-- == 0) return true;

            uint64_t pd_index = rand() % batch_size;
            uint64_t rem = rand() & 255;
            bool location_mask = rand() & 1;

            if (!pd.find(pd_index, rem, location_mask)) {
                auto res = pd.insert(pd_index, rem, location_mask);
                assert(res);
                assert(pd.find(pd_index, rem, location_mask));
            }
            bool rec_res = recursive_add_delete_core(&pd, &counter, 1);
            if (pd.find(pd_index, rem, location_mask)) {
                bool del_res = pd.conditional_remove_db(pd_index, rem, location_mask);
                assert(del_res);
                bool refind_res = pd.find(pd_index, rem, location_mask);
                assert(!refind_res);
            }
        }

        return true;
    }
    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    void rec_cmp_single(L2Bucket_mq<max_capacity, batch_size, bits_per_item> *pd, my_map::map_t *my_map, key temp_key) {
        uint64_t pd_index = std::get<0>(temp_key);
        uint8_t rem = std::get<1>(temp_key);
        bool location_mask = std::get<2>(temp_key);

        auto map_count = my_map->count(temp_key);
        assert(map_count <= 1);
        bool map_find_res = !!(map_count);
        bool find_res = pd->find(pd_index, rem, location_mask);
        if (find_res != map_find_res) {
            pd->print_find_arg(pd_index, rem, location_mask);
            std::cout << "find_res:     " << find_res << std::endl;
            std::cout << "map_find_res: " << map_find_res << std::endl;
            std::cout << "map_count: " << map_count << std::endl;
            pd->print_pd();
            pd->find(pd_index, rem, location_mask);
        }
        assert(find_res == map_find_res);
        return;


        // //prevents insertion of an element that is already in the pd again.
        // if (!pd->find(pd_index, rem, location_mask)) {
        //     my_map->insert(std::pair<key, size_t>(temp_key, 42));
        //     auto res = pd->insert(pd_index, rem, location_mask);
        //     assert(res);
        //     assert(pd->find(pd_index, rem, location_mask));
        // }

        // if (post_find_res != post_map_find_res) {
        //     pd->print_find_arg(pd_index, rem, location_mask);
        //     std::cout << "post_find_res:     " << post_find_res << std::endl;
        //     std::cout << "post_map_find_res: " << post_map_find_res << std::endl;
        //     std::cout << "post_map_count: " << post_map_count << std::endl;
        //     pd->print_pd();
        //     pd->find(pd_index, rem, location_mask);
        // }
    }


    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto recursive_add_delete_core_with_map(L2Bucket_mq<max_capacity, batch_size, bits_per_item> *pd, size_t *reps,
                                            size_t depth, my_map::map_t *my_map) -> bool {
        static int counter = 0;
        if (depth == max_capacity)
            return true;

        assert(depth < max_capacity);

        while (rand() & 1) {
            if (*reps == 0) return true;
            *reps -= 1;

            uint64_t pd_index = rand() % batch_size;
            uint64_t rem = rand() & 255;
            bool location_mask = rand() & 1;
            const key temp_key = {pd_index, rem, location_mask};

            rec_cmp_single(pd, my_map, temp_key);

            //prevents insertion of an element that is already in the pd again.
            if (!pd->find(pd_index, rem, location_mask)) {
                my_map->insert(std::pair<key, size_t>(temp_key, 42));
                auto res = pd->insert(pd_index, rem, location_mask);
                assert(res);
                assert(pd->find(pd_index, rem, location_mask));
                rec_cmp_single(pd, my_map, temp_key);
            }

            recursive_add_delete_core_with_map(pd, reps, depth + 1, my_map);

            rec_cmp_single(pd, my_map, temp_key);

            if (pd->find(pd_index, rem, location_mask)) {
                bool del_res = pd->conditional_remove_db(pd_index, rem, location_mask);
                assert(del_res);
                bool refind_res = pd->find(pd_index, rem, location_mask);
                assert(!refind_res);
                my_map->erase(temp_key);
                rec_cmp_single(pd, my_map, temp_key);
            }
            counter++;
        }
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto recursive_add_delete_with_map(size_t reps) -> bool {
        size_t counter = reps;
        auto pd = L2Bucket_mq<max_capacity, batch_size, bits_per_item>();
        my_map::map_t my_map;

        while (counter) {
            recursive_add_delete_core_with_map(&pd, &counter, 1, &my_map);
        }
        /*     uint64_t pd_index = rand() % batch_size;
            uint64_t rem = rand() & 255;
            bool location_mask = rand() & 1;

            const key temp_key = {pd_index, rem, location_mask};
            // key temp_key = std::make_tuple(pd_index, rem, location_mask);

            auto map_count = my_map.count(temp_key);
            assert(map_count <= 1);
            bool map_find_res = !!(map_count);

            bool find_res = pd.find(pd_index, rem, location_mask);
            assert(find_res == map_find_res);

            if (!pd.find(pd_index, rem, location_mask)) {
                my_map.insert(std::pair<key, size_t>(temp_key, 42));
                auto res = pd.insert(pd_index, rem, location_mask);
                assert(res);
                assert(pd.find(pd_index, rem, location_mask));
            }

            auto before_capacity = pd.get_capacity();
            bool rec_res = recursive_add_delete_core_with_map(&pd, &counter, 1, &my_map);
            auto after_capacity = pd.get_capacity();
            assert(before_capacity == after_capacity);

            auto post_map_count = my_map.count(temp_key);
            assert(post_map_count <= 1);
            bool post_map_find_res = !!(map_count);

            bool post_find_res = pd.find(pd_index, rem, location_mask);
            if (post_find_res != post_map_find_res) {
                std::cout << "post_find_res:     " << post_find_res << std::endl;
                std::cout << "post_map_find_res: " << post_map_find_res << std::endl;
                pd.print_pd();
                pd.find(pd_index, rem, location_mask);
            }
            assert(post_find_res == post_map_find_res);
            if (pd.find(pd_index, rem, location_mask)) {
                bool del_res = pd.conditional_remove_db(pd_index, rem, location_mask);
                assert(del_res);
                bool refind_res = pd.find(pd_index, rem, location_mask);
                assert(!refind_res);
                my_map.erase(temp_key);
            }
        }
 */
        return true;
    }


    /*
    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto test_swap_smaller(const __m512i *cpd) -> bool {
        __m512i x = *cpd;
        __m512i *pd = &x;
        assert(pd512_plus::pd_full(pd));
        uint64_t last_quot = pd512_plus::decode_last_quot_wrapper(pd);
        uint64_t last_r = pd512_plus::get_last_byte(pd);
        uint64_t old_qr = (last_quot << 8) | ((uint64_t) last_r);
        // std::cout << "last_quot: " << last_quot << std::endl;
        // std::cout << "last_r: " << last_r << std::endl;
        // std::cout << "old_qr: " << old_qr << std::endl;
        if ((last_quot == 0) || (last_r == 0))
            return true;
        uint64_t temp_q = rand() % last_quot;
        uint64_t temp_r = rand() & 255;
        uint64_t new_qr = (temp_q << 8) | ((uint64_t) temp_r);
        // if las
        auto res = pd512_plus::pd_conditional_add_50(temp_q, temp_r, pd);
        assert(new_qr < old_qr);
        assert(res == old_qr);

        x = *cpd;
        pd = &x;

        temp_q = last_quot;
        assert(last_r);
        temp_r = rand() % last_r;
        res = pd512_plus::pd_conditional_add_50(temp_q, temp_r, pd);
        assert(res == old_qr);
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto test_swap_same(const __m512i *cpd) -> bool {
        __m512i x = *cpd;
        __m512i *pd = &x;
        assert(pd512_plus::pd_full(pd));
        uint64_t last_quot = pd512_plus::decode_last_quot_wrapper(pd);
        uint64_t last_r = pd512_plus::get_last_byte(pd);
        uint64_t old_qr = (last_quot << 8) | ((uint64_t) last_r);

        auto res = pd512_plus::pd_conditional_add_50(last_quot, last_r, pd);
        assert(res == old_qr);
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto test_swap_bigger(const __m512i *cpd) -> bool {
        __m512i x = *cpd;
        __m512i *pd = &x;
        assert(pd512_plus::pd_full(pd));
        uint64_t last_quot = pd512_plus::decode_last_quot_wrapper(pd);
        uint64_t last_r = pd512_plus::get_last_byte(pd);
        uint64_t old_qr = (last_quot << 8) | ((uint64_t) last_r);

        uint64_t diff = 49 - last_quot;
        uint64_t temp_q = last_quot;
        uint64_t temp_r = rand() & 255;
        uint64_t new_qr;
        if (diff) {
            temp_q += (rand() % diff);
            new_qr = (temp_q << 8) | ((uint64_t) temp_r);
            if (old_qr <= new_qr) {
                auto res = pd512_plus::pd_conditional_add_50(temp_q, temp_r, pd);
                assert(res == new_qr);
                x = *cpd;
                pd = &x;
            }
        }

        diff = 255 - last_r;
        temp_q = last_quot;
        temp_r = last_r;
        if (diff) {
            temp_r += (rand() % diff);
            new_qr = (temp_q << 8) | ((uint64_t) temp_r);
            if (old_qr <= new_qr) {
                auto res = pd512_plus::pd_conditional_add_50(temp_q, temp_r, pd);
                assert(res == new_qr);
            }
        }

        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto test_swap(size_t reps, const __m512i *cpd) -> bool {
        for (size_t i = 0; i < reps; i++) {
            test_swap_bigger(cpd);
            test_swap_smaller(cpd);
            test_swap_same(cpd);
        }
        return true;
    }
    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto test_swap_wrapper() -> bool {
        for (size_t i = 0; i < 1024; i++) {
            std::cout << "i: " << i << std::endl;
            __m512i x;
            pd_init(&x);
            rand_fill_and_test1(&x);
            test_swap(256, &x);
        }
        return true;
    }

    template<size_t max_capacity, size_t batch_size, size_t bits_per_item>
    auto rand_test_OF() -> bool {
        size_t max_capacity = 51;
        uint64_t valid_max_quot = 0;
        __m512i x;
        pd_init(&x);
        for (size_t i = 0; i < max_capacity; i++) {
            uint64_t q = rand() % QUOT_SIZE;
            uint64_t r = rand() & 255;
            auto res = pd512_plus::pd_conditional_add_50(q, r, &x);
            assert(res == (1 << 15));
            if (valid_max_quot < q)
                valid_max_quot = q;

            assert(pd512_plus::pd_find_50(q, r, &x) == pd512_plus::Yes);
            auto att_last_quot = pd512_plus::decode_last_quot_wrapper(&x);
            assert(att_last_quot == valid_max_quot);
        }
        return true;
    }
*/

}// namespace l2_bucket_tests


namespace bits_memcpy {
    namespace tests {
        template<typename T>
        void xor_array(T *a, T *b, T *res, size_t a_size);

        template<typename T>
        void print_results(T original_dest, T dest_att, T dest_res, size_t gap = 4) {
            std::cout << "dest:     \t\t";
            print_memory::print_word_LE(original_dest, gap);
            // std::cout << std::endl;
            std::cout << "dest_att: \t\t";
            print_memory::print_word_LE(dest_att, gap);
            // std::cout << std::endl;
            std::cout << "dest_res: \t\t";
            print_memory::print_word_LE(dest_res, gap);
            // std::cout << std::endl;
        }

        template<typename T>
        void print_results(T *original_dest, T *dest_att, T *dest_res, size_t arrays_size, size_t gap = 4) {
            std::cout << "dest:     \t\t";
            print_memory::print_array_LE(original_dest, arrays_size, gap);
            // std::cout << std::endl;
            std::cout << "dest_att: \t\t";
            print_memory::print_array_LE(dest_att, arrays_size, gap);
            // std::cout << std::endl;
            std::cout << "dest_res: \t\t";
            print_memory::print_array_LE(dest_res, arrays_size, gap);
            // std::cout << std::endl;
        }

        void print_arg(size_t src_start, size_t dest_start, size_t length);

        template<typename T>
        void
        test_single_failed_data(const T *src, T *dest, T *dest_att1, T *dest_att2, size_t src_start, size_t dest_start,
                                size_t length, size_t temp_size) {
            T res_for_xor_array[temp_size] = {0};
            std::cout << std::string(80, '.') << std::endl;
            std::cout << "Start" << std::endl;

            std::cout << "src_start:    \t" << src_start;
            std::cout << "\t\t" << (src_start / CHAR_BIT);
            std::cout << "\t\t" << (src_start & 7ul);
            std::cout << std::endl;

            std::cout << "dest_start:   \t" << dest_start;
            std::cout << "\t\t" << (dest_start / CHAR_BIT);
            std::cout << "\t\t" << (dest_start & 7ul);
            std::cout << std::endl;

            std::cout << "length:       \t" << length;
            std::cout << "\t\t" << (length / CHAR_BIT);
            std::cout << "\t\t" << (length & 7ul);
            std::cout << std::endl;
            std::cout << std::string(80, '=') << std::endl;
            std::cout << "src:      \t" << print_memory::str_array_LE(src, temp_size, 4) << std::endl;
            std::cout << "dest:      \t" << print_memory::str_array_LE(dest, temp_size, 4) << std::endl;
            std::cout << "dest_att:  \t" << print_memory::str_array_LE(dest_att1, temp_size, 4) << std::endl;
            std::cout << "dest_att2: \t" << print_memory::str_array_LE(dest_att2, temp_size, 4) << std::endl;
            xor_array(dest_att1, dest_att2, res_for_xor_array, temp_size);
            std::cout << "xor_array: \t" << print_memory::str_array_LE(res_for_xor_array, temp_size, 4) << std::endl;

            std::cout << std::string(80, '-') << std::endl;
            std::cout << "dest:       \t";
            print_memory::print_array(dest, temp_size);
            std::cout << "dest_att:   \t";
            print_memory::print_array(dest_att1, temp_size);
            std::cout << "dest_att2:  \t";
            print_memory::print_array(dest_att2, temp_size);
            std::cout << "xor_array:  \t";
            print_memory::print_array(res_for_xor_array, temp_size);
            std::cout << "src:  \t";
            print_memory::print_array(src, temp_size);
            std::cout << std::string(80, '=') << std::endl;
        }


        template<typename T>
        bool cmp_array(T *a, T *b, size_t words_to_compare, bool to_print = true) {
            for (size_t i = 0; i < words_to_compare; i++) {
                bool temp = (a[i] == b[i]);
                if (!temp) {
                    if (to_print) {
                        std::cout << std::string(80, '!') << std::endl;
                        std::cout << "cmp failed: " << i << std::endl;
                        std::cout << "a[i]: " << a[i] << std::endl;
                        std::cout << "b[i]: " << b[i] << std::endl;
                        std::cout << std::string(80, '-') << std::endl;

                        std::cout << "a: \t\t\t" << print_memory::str_array_LE(a, words_to_compare, 4) << std::endl;
                        std::cout << "b: \t\t\t" << print_memory::str_array_LE(b, words_to_compare, 4) << std::endl;
                        std::cout << "a: \t\t\t";
                        print_memory::print_array(a, words_to_compare);
                        std::cout << "b: \t\t\t";
                        print_memory::print_array(b, words_to_compare);
                    }
                    return false;
                }
            }
            return true;
        }

        template<typename T>
        bool cmp_array(T *a, T *b, T *c, size_t words_to_compare, bool to_print = true) {
            for (size_t i = 0; i < words_to_compare; i++) {
                bool temp = (a[i] == b[i]) && (c[i] == b[i]);
                if (!temp) {
                    if (to_print) {
                        std::cout << std::string(80, '!') << std::endl;
                        std::cout << "cmp failed: " << i << std::endl;
                        std::cout << "a[i]: " << a[i] << std::endl;
                        std::cout << "b[i]: " << b[i] << std::endl;
                        std::cout << "c[i]: " << c[i] << std::endl;
                        std::cout << std::string(80, '-') << std::endl;

                        std::cout << "a: \t\t\t" << print_memory::str_array_LE(a, words_to_compare, 4) << std::endl;
                        std::cout << "b: \t\t\t" << print_memory::str_array_LE(b, words_to_compare, 4) << std::endl;
                        std::cout << "c: \t\t\t" << print_memory::str_array_LE(c, words_to_compare, 4) << std::endl;
                        std::cout << "a: \t\t\t";
                        print_memory::print_array(a, words_to_compare);
                        std::cout << "b: \t\t\t";
                        print_memory::print_array(b, words_to_compare);
                        std::cout << "c: \t\t\t";
                        print_memory::print_array(c, words_to_compare);
                    }
                    return false;
                }
            }
            return true;
        }

        template<typename T>
        void copy_array(T *src, T *dest, size_t a_size) {
            constexpr unsigned slot_byte_size = sizeof(src[0]);
            memcpy(dest, src, slot_byte_size * a_size);
        }


        template<typename T>
        void xor_array(T *a, T *b, T *res, size_t a_size) {
            for (size_t i = 0; i < a_size; i++) {
                res[i] = a[i] ^ b[i];
            }
        }

        template<typename T>
        void rand_fill_array(T *a, size_t a_size) {
            for (size_t i = 0; i < a_size; i++) {
                a[i] = rand();
            }
        }

        template<typename T>
        bool test_recursive_copy_single(const T *src, T *dest, size_t src_start, size_t dest_start, size_t length,
                                        size_t dest_size) {
            //            constexpr unsigned slot_size = sizeof(src[0]) * CHAR_BIT;
            //            size_t temp_size = (dest_start + length - 1) / slot_size;
            T dest_att[dest_size] = {0};
            T dest_att2[dest_size] = {0};
            T dest_att3[dest_size] = {0};

            copy_array(dest, dest_att, dest_size);
            copy_array(dest, dest_att2, dest_size);
            copy_array(dest, dest_att3, dest_size);
            assert(cmp_array(dest_att, dest_att2, dest_size));
            assert(cmp_array(dest_att, dest_att2, dest_att3, dest_size));

            copy_recursive(src, dest_att, src_start, dest_start, length);
            copy_recursive(src, dest_att2, src_start, dest_start, length);

            assert(cmp_array(dest_att, dest_att, dest_size));
            assert(cmp_array(dest_att2, dest_att2, dest_size));
            auto cmp_res = cmp_array(dest_att, dest_att2, dest_size);
            if (!cmp_res) {
                test_single_failed_data(src, dest, dest_att, dest_att2, src_start, dest_start, length, dest_size);
                assert(0);
            }

            assert(cmp_array(dest_att, dest_att2, dest_size));

            copy_recursive(dest, dest_att, dest_start, dest_start, length);
            copy_recursive(dest, dest_att2, dest_start, dest_start, length);
            cmp_res = cmp_array(dest_att, dest_att2, dest_size);

            if (!cmp_res) {
                test_single_failed_data(src, dest, dest_att, dest_att2, src_start, dest_start, length, dest_size);
                assert(0);
            }

            assert(cmp_array(dest_att, dest_att2, dest_size));
            return true;
        }


        template<typename T>
        bool test_single(const T *src, T *dest, size_t src_start, size_t dest_start, size_t length, size_t dest_size) {
            assert(test_recursive_copy_single(src, dest, src_start, dest_start, length, dest_size));

            // constexpr unsigned slot_size = sizeof(src[0]) * CHAR_BIT;
            //            size_t temp_size = (dest_start + length - 1) / slot_size;
            size_t temp_size = dest_size;
            T dest_att[temp_size] = {0};
            T dest_att2[temp_size] = {0};

            copy_array(dest, dest_att, temp_size);
            copy_array(dest, dest_att2, temp_size);
            assert(cmp_array(dest_att, dest_att2, temp_size));

            copy(src, dest_att, src_start, dest_start, length);
            copy_recursive(src, dest_att2, src_start, dest_start, length);
            if (!cmp_array(dest_att, dest_att2, temp_size)) {
                T dest_att3[temp_size] = {0};
                copy_array(dest, dest_att3, temp_size);
                copy(src, dest_att3, src_start, dest_start, length);
                test_single_failed_data(src, dest, dest_att, dest_att2, src_start, dest_start, length, temp_size);
                assert(0);
            }
            assert(cmp_array(dest_att, dest_att2, temp_size));

            copy(dest, dest_att, dest_start, dest_start, length);
            copy_recursive(dest, dest_att2, dest_start, dest_start, length);

            assert(cmp_array(dest_att, dest_att2, temp_size));
            return true;
        }

        template<typename T>
        bool test_single_old(const T *src, T *dest, size_t src_start, size_t dest_start, size_t length) {
            std::cout << std::string(80, '.') << std::endl;
            std::cout << "Start" << std::endl;

            std::cout << "src_start:    \t" << src_start;
            std::cout << "\t\t" << (src_start / CHAR_BIT);
            std::cout << "\t\t" << (src_start & 7ul);
            std::cout << std::endl;

            std::cout << "dest_start:   \t" << dest_start;
            std::cout << "\t\t" << (dest_start / CHAR_BIT);
            std::cout << "\t\t" << (dest_start & 7ul);
            std::cout << std::endl;

            std::cout << "length:       \t" << length;
            std::cout << "\t\t" << (length / CHAR_BIT);
            std::cout << "\t\t" << (length & 7ul);
            std::cout << std::endl;

            assert(test_recursive_copy_single(src, dest, src_start, dest_start, length));

            constexpr unsigned slot_size = sizeof(src[0]) * CHAR_BIT;
            size_t temp_size = (dest_start + length - 1) / slot_size;
            T dest_att[temp_size] = {0};
            T dest_att2[temp_size] = {0};
            //            T dest_att3[temp_size] = {0};
            //            T dest_att4[temp_size] = {0};
            //            T res_for_xor_array[temp_size] = {0};
            copy_array(dest, dest_att, temp_size);
            copy_array(dest, dest_att2, temp_size);
            assert(cmp_array(dest_att, dest_att2, temp_size));

            copy(src, dest_att, src_start, dest_start, length);
            copy_recursive(src, dest_att2, src_start, dest_start, length);
            assert(cmp_array(dest_att, dest_att2, temp_size));

            copy(dest, dest_att, dest_start, dest_start, length);
            copy_recursive(dest, dest_att2, dest_start, dest_start, length);

            assert(cmp_array(dest_att, dest_att2, temp_size));
            return true;
            //            copy_array(dest, dest_att3, temp_size);

            //            copy_array(dest, dest_att4, temp_size);

            //            assert(cmp_array(dest_att, dest_att3, temp_size));
            //            assert(cmp_array(dest_att3, dest_att2, temp_size));
            //            assert(cmp_array(dest_att4, dest_att3, temp_size));
            /*//            int temp = memcmp(dest_att, dest_att2, temp_size * slot_size);
//            if (temp != 0){
//                std::cout << std::string(80, 'x') << std::endl;
//                std::cout << "dest:      \t" << print_memory::str_array_LE(dest, temp_size, 4) << std::endl;
//                std::cout << "dest_att:  \t" << print_memory::str_array_LE(dest_att, temp_size, 4) << std::endl;
//                std::cout << "dest_att2: \t" << print_memory::str_array_LE(dest_att2, temp_size, 4) << std::endl;
//                xor_array(dest_att, dest_att2, res_for_xor_array, temp_size);
//                std::cout << "xor_array: \t" << print_memory::str_array_LE(res_for_xor_array, temp_size, 4) << std::endl;
//
//                std::cout << std::string(80, '-') << std::endl;
//                std::cout << "dest:       \t";
//                print_memory::print_array(dest, temp_size);
//                std::cout << "dest_att:   \t";
//                print_memory::print_array(dest_att, temp_size);
//                std::cout << "dest_att2:  \t";
//                print_memory::print_array(dest_att2, temp_size);
//                std::cout << "xor_array:  \t";
//                print_memory::print_array(res_for_xor_array, temp_size);
//                std::cout << std::string(80, 'x') << std::endl;
//
//            }
//            assert(temp == 0);
//            copy(src, dest_att, src_start, dest_start, length);*/

            //            assert(cmp_array(dest_att2, dest_att3, dest_att4, temp_size));
            /*assert(cmp_array(dest_att4, dest_att3, temp_size));
            assert(cmp_array(dest_att4, dest_att2, temp_size));*/

            //            copy(src, dest_att, src_start, dest_start, length);
            //            copy_recursive(src, dest_att2, src_start, dest_start, length);
            //            copy_recursive(src, dest_att3, src_start, dest_start, length);
            //            copy_recursive(src, dest_att4, src_start, dest_start, length);
            //
            //            assert(cmp_array(dest_att2, dest_att3, dest_att4, temp_size));

            //            assert(cmp_array(dest_att2, dest_att2, temp_size));
            //            assert(cmp_array(dest_att3, dest_att3, temp_size));
            //            assert(cmp_array(dest_att3, dest_att2, temp_size));

            //            bool naive_cmp = memcmp(dest_att, dest_att2, temp_size * slot_size);
            //            std::cout << "naive_cmp_res: " << naive_cmp << std::endl;

            /*bool res = cmp_array(dest_att, dest_att2, temp_size);
            std::cout << "Part1" << std::endl;
            if (!res) {
                */
            /* std::cout << std::string(80, '=') << std::endl;

                std::cout << "src_start:    \t" << src_start;
                std::cout << "\t\t" << (src_start / CHAR_BIT);
                std::cout << "\t\t" << (src_start & 7);
                std::cout << std::endl;

                std::cout << "dest_start:   \t" << dest_start;
                std::cout << "\t\t" << (dest_start / CHAR_BIT);
                std::cout << "\t\t" << (dest_start & 7);
                std::cout << std::endl;

                std::cout << "length:       \t" << length;
                std::cout << "\t\t" << (length / CHAR_BIT);
                std::cout << "\t\t" << (length & 7);
                std::cout << std::endl;
 */
            /*
                std::cout << std::string(80, '=') << std::endl;
                std::cout << "src:       \t" << print_memory::str_array_LE(src, temp_size, 4) << std::endl;
                std::cout << "dest:      \t" << print_memory::str_array_LE(dest, temp_size, 4) << std::endl;
                std::cout << "dest_att:  \t" << print_memory::str_array_LE(dest_att, temp_size, 4) << std::endl;
                std::cout << "dest_att2: \t" << print_memory::str_array_LE(dest_att2, temp_size, 4) << std::endl;
                std::cout << "dest_att3: \t" << print_memory::str_array_LE(dest_att3, temp_size, 4) << std::endl;
                xor_array(dest_att, dest_att2, res_for_xor_array, temp_size);
                std::cout << "xor_array: \t" << print_memory::str_array_LE(res_for_xor_array, temp_size, 4) << std::endl;

                std::cout << std::string(80, '-') << std::endl;
                std::cout << "dest:       \t";
                print_memory::print_array(dest, temp_size);
                std::cout << "dest_att:   \t";
                print_memory::print_array(dest_att, temp_size);
                std::cout << "dest_att2:  \t";
                print_memory::print_array(dest_att2, temp_size);
                std::cout << "dest_att3:  \t";
                print_memory::print_array(dest_att3, temp_size);
                std::cout << "xor_array:  \t";
                print_memory::print_array(res_for_xor_array, temp_size);

                // std::cout << "copy and copy_recursive res are different. " << std::endl;
                return false;
            }

            std::cout << "Part2!" << std::endl;
*/
            //            assert(cmp_array(dest_att2, dest_att3, dest_att4, temp_size));
            //
            //            copy(dest, dest_att, dest_start, dest_start, length);
            //            copy_recursive(dest, dest_att2, dest_start, dest_start, length);
            //            copy_recursive(dest, dest_att3, dest_start, dest_start, length);
            //            copy_recursive(dest, dest_att4, dest_start, dest_start, length);
            //
            //            assert(cmp_array(dest_att2, dest_att3, dest_att4, temp_size));
            //            return true;
            /*assert(cmp_array(dest_att3, dest_att3, temp_size));
            assert(cmp_array(dest_att2, dest_att2, temp_size));

            bool c1 = cmp_array(dest_att, dest_att2, temp_size);
//            bool c2 = cmp_array(dest_att2, dest_att3, temp_size);
            //            bool c3 = c1 && c2;

            if (c1) {
                assert(cmp_array(dest_att3, dest_att2, temp_size));
                return true;
            }

            else {
                std::cout << std::string(80, 'H') << std::endl;

                std::cout << "src_start:    \t" << src_start;
                std::cout << "\t\t" << (src_start / CHAR_BIT);
                std::cout << "\t\t" << (src_start & 7);
                std::cout << std::endl;

                std::cout << "dest_start:   \t" << dest_start;
                std::cout << "\t\t" << (dest_start / CHAR_BIT);
                std::cout << "\t\t" << (dest_start & 7);
                std::cout << std::endl;

                std::cout << "length:       \t" << length;
                std::cout << "\t\t" << (length / CHAR_BIT);
                std::cout << "\t\t" << (length & 7);
                std::cout << std::endl;

                std::cout << std::string(80, '=') << std::endl;
                std::cout << "src:       \t" << print_memory::str_array_LE(src, temp_size, 4) << std::endl;
                std::cout << "dest:      \t" << print_memory::str_array_LE(dest, temp_size, 4) << std::endl;
                std::cout << "dest_att:  \t" << print_memory::str_array_LE(dest_att, temp_size, 4) << std::endl;
                std::cout << "dest_att2: \t" << print_memory::str_array_LE(dest_att2, temp_size, 4) << std::endl;
                std::cout << "dest_att3: \t" << print_memory::str_array_LE(dest_att3, temp_size, 4) << std::endl;
                xor_array(dest_att, dest_att2, res_for_xor_array, temp_size);
                std::cout << "xor_array: \t" << print_memory::str_array_LE(res_for_xor_array, temp_size, 4) << std::endl;

                std::cout << std::string(80, '-') << std::endl;
                std::cout << "dest:       \t";
                print_memory::print_array(dest, temp_size);
                std::cout << "dest_att:   \t";
                print_memory::print_array(dest_att, temp_size);
                std::cout << "dest_att2:  \t";
                print_memory::print_array(dest_att2, temp_size);
                std::cout << "dest_att3:  \t";
                print_memory::print_array(dest_att3, temp_size);
                std::cout << "xor_array:  \t";
                print_memory::print_array(res_for_xor_array, temp_size);

                assert(cmp_array(dest_att3, dest_att2, temp_size));

                //                std::cout << "dest_att == dest_att2: \t" << cmp_array(dest_att, dest_att2, temp_size) << std::endl;
                //                std::cout << "dest_att == dest:      \t" << cmp_array(dest_att, dest, temp_size) << std::endl;
                //                std::cout << "dest_att2 == dest:     \t" << cmp_array(dest_att2, dest, temp_size) << std::endl;
                return false;
            }*/
        }

        template<typename T>
        bool t0() {
            constexpr unsigned size = 4;
            T src[size];
            T dest[size];
            constexpr unsigned slot_size = sizeof(src[0]) * CHAR_BIT;
            constexpr unsigned total_bit_size = slot_size * size;

            rand_fill_array(src, size);
            rand_fill_array(dest, size);

            size_t src_start = 0, dest_start = 0, length = 0;

            for (size_t i = 0; i < 256; i++) {
                //                std::cout << std::string(120, '*') << std::endl;
                // std::cout << "t0 i: " << i << std::endl;
                while (length == 0) {
                    src_start = rand() % total_bit_size;
                    dest_start = rand() % total_bit_size;
                    length = rand() % (total_bit_size - MAX(src_start, dest_start));
                }
                auto temp = test_single(src, dest, src_start, dest_start, length, size);
                if (!temp) {
                    std::cout << "t0 failed.";
                    std::cout << "\ti: " << i << std::endl;
                    return false;
                }
            }
            return true;
        }

        template<typename T>
        bool t0_rec() {
            constexpr unsigned size = 4;
            T src[size];
            T dest[size];
            constexpr unsigned slot_size = sizeof(src[0]) * CHAR_BIT;
            constexpr unsigned total_bit_size = slot_size * size;

            rand_fill_array(src, size);
            rand_fill_array(dest, size);

            size_t src_start = 0, dest_start = 0, length = 0;

            for (size_t i = 0; i < 256; i++) {
                //                std::cout << std::string(120, '*') << std::endl;
                //                std::cout << "t0_rec i: " << i << std::endl;

                while (length == 0) {
                    src_start = rand() % total_bit_size;
                    dest_start = rand() % total_bit_size;
                    length = rand() % (total_bit_size - MAX(src_start, dest_start));
                }
                assert(src_start + length < total_bit_size);
                assert(dest_start + length < total_bit_size);
                auto temp = test_recursive_copy_single(src, dest, src_start, dest_start, length, size);
                if (!temp) {
                    std::cout << "t0_recursive failed.";
                    std::cout << "\ti: " << i << std::endl;
                    return false;
                }
            }
            return true;
        }


    }// namespace tests
}// namespace bits_memcpy
#endif// FILTERS_L2_BUCKET_HPP


// void right_shift_array(uint64_t *a, size_t a_size, uint64_t shift);

// void memcpy_bits(const uint64_t *src, uint64_t *dest, size_t src_start, size_t src_end);

// void memcpy_bits(const uint64_t *src, uint64_t *dest, size_t src_start, size_t src_end, size_t dest_start, size_t dest_end);

// bool test_bit(const uint64_t *a, size_t bit_index);
