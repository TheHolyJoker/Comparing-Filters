//
// Created by tomer on 3/14/20.
//

#ifndef CLION_CODE_NAIVE_COUNTER_ENDEC_HPP
#define CLION_CODE_NAIVE_COUNTER_ENDEC_HPP

#include <ostream>
#include "../Global_functions/basic_functions.h"
#include "../bit_operations/bit_op.h"
#include "../bit_operations/my_bit_op.hpp"

#define COUNTER_TYPE uint32_t
#define COUNTER_TYPE_SIZE (sizeof(COUNTER_TYPE) * (CHAR_BIT))
#define COUNTER_CELL_C8 (0x55)
#define COUNTER_CELL_C16 (0x5555)
#define COUNTER_CELL_C32 (0x55555555)
#define MAX_COUNTER (10)
#define MAX_DECODE_VAL (32)
#define COMMA_DECODE(x) ((~x) & (x << 1ul))
#define SPLITTED_COMMA_DECODER(w1, w2) (COMMA_DECODE(w1) | (w2 && (SL((COUNTER_TYPE_SIZE - 1)))))
#define DELIM_LENGTH (2)
using namespace std;

//static const uint32_t encode_table[10] = {5, 9, 13, 17, 25, 29, 33, 49, 57, 61};
static const uint32_t encode_table[10] = {1, 2, 3, 4, 6, 7, 8, 12, 14, 15};

static const int decode_table[33] = {-1, 1, 2, 3, 4, -1, 6, 7, 8, -1, -1, -1, 12, -1, 14, 15, 16, -1, -1, -1, -1,
                                     -1, -1, -1, 24, -1, -1, -1, 28, -1, 30, 31, 32};

/**
 * Number x to the x'th symbol in encode table.
 * x -> encode_table[x-1] -> (encode_table[x-1] << 2) | 1u
 * concatenating the delimiter to the end of the encoded symbol.
 * @tparam T
 * @tparam S
 * @param x
 * @return
 */
template<typename T, typename S>
static auto encode(S x) -> T;


/**
 *
 * @tparam T
 * @tparam S
 * @param x The encoded symbol. Includes the first delimiter, but does not include the second.
 * @return
 *         w = (x << 2) + 1             {x = x + '01'};
 *         return  SYMBOLS.index(w) + 1;
 */
template<typename T, typename S>
static auto decode(S x) -> T;


template<typename T, typename S>
static auto update(S x, int update_val) -> T;

/**
 *
 * @tparam T
 * @tparam S
 * @param x
 * @param update_val
 * @return
 * Returns 0 with there is overflow (updated counter value is bigger than MAX_COUNTER or equal to zero)
 */
template<typename T, typename S>
static auto update_with_overflow(S x, int update_val) -> T;

/**
 *
 * @param a array in which the counter are encoded.
 * @param a_size number of slots in the array.
 * @param start_lim Denotes the first bit in the bit stream.
 * @param end_lim Denotes the last bit in the bit stream.
 * @param index Denote the index of the counter that should be decoded.
 * @param start Store the address of the first bit (relative to start_lim) in the relevant counter.
 * @param end Store the address of the last bit +1 (relative to start_lim) in the relevant counter.
 */
template<typename T>
static void
find_counter_interval_naive(T *a, size_t a_size, size_t start_lim, size_t end_lim, size_t index, size_t *start,
                            size_t *end);


#endif //CLION_CODE_NAIVE_COUNTER_ENDEC_HPP
