

#ifndef CLION_CODE_BIT_OP_H
#define CLION_CODE_BIT_OP_H

#include <iostream>
#include <zconf.h>
#include <vector>
#include <cstdint>
#include "../Global_functions/macros.h"
//#include "bit_word_converter.hpp"

using namespace std;
typedef unsigned int u32;


/**
 * @techreport{magicalgorithms,
 * author={Henry Gordon Dietz},
 * title={{The Aggregate Magic Algorithms}},
 * institution={University of Kentucky},
 * howpublished={Aggregate.Org online technical report},
 * URL={http://aggregate.org/MAGIC/}
 * Date={10/24/19}
 * }
**/

unsigned int msb32(register unsigned int x);

unsigned char msb8(register __uint8_t x);

unsigned int reverse(register unsigned int x);

unsigned int floor_log2(register unsigned int x);

unsigned int ones32(register unsigned int x);

/**Bit Twiddling Hacks: https://graphics.stanford.edu/~seander/bithacks.html*/

/**
 * Select the bit position (from the most-significant bit) with the given count (rank)
 * The following 64-bit code selects the position of the rth 1 bit when counting from the left.
 * In other words if we start at the most significant bit and proceed to the right, counting the number of bits set to
 * 1 until we reach the desired rank, r, then the position where we stop is returned. If the rank requested exceeds the
 * count of bits set, then 63 is returned. The code may be modified for 32-bit or counting from the right.
 * @param slot Value to find position with rank r.
 * @param rank Bit's desired rank [1-64].
 * @return Resulting position of bit with the desired rank.[1-64]
 */
uint32_t select_r(uint64_t slot, uint32_t rank);

/*
uint32_t select_rank_th_bit(uint64_t slot, uint32_t rank) {
    unsigned int s;      // Output: Resulting position of bit with the desired rank.[1-64]
    uint64_t a, b, c, d; // Intermediate temporaries for bit count.
    unsigned int t;      // Bit count temporary.

    a = slot - ((slot >> 1) & ~0UL / 3);
    b = (a & ~0UL / 5) + ((a >> 2) & ~0UL / 5);
    c = (b + (b >> 4)) & ~0UL / 0x11;
    d = (c + (c >> 8)) & ~0UL / 0x101;
    t = (d >> 32) + (d >> 48);
    s = 64;
    if (rank > t) {s -= 32; rank -= t;}
    if (rank > t) {s -= 16; rank -= t;}
    if (rank > t) {s -= 8; rank -= t;}
    if (rank > t) {s -= 4; rank -= t;}
    if (rank > t) {s -= 2; rank -= t;}
    if (rank > t) s--;
    return 64 - s;
}
*/


/**
 *The following finds the the rank of a bit, meaning it returns the sum of bits that are set to 1 from the
 * most-signficant bit downto the bit at the given position.
 * @param slot Compute the rank (bits set) in slot from the MSB to bit_lim.
 * @param bit_lim Bit position to count bits upto.
 * @return Resulting rank of bit at bit_lim goes here.
 */
uint64_t count(uint64_t slot, unsigned int bit_lim);

/**
 * Count number of set bits in v.
 * @param v
 * @return
 */
//uint32_t bit_count(uint32_t v);
static inline uint32_t bit_count(uint32_t v) {
    v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
    return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count

}

uint32_t my_count(uint32_t slot);

uint64_t convert_32_to_64(uint32_t slot);

inline bool is_bit_rank_valid(uint64_t slot, uint32_t rank, uint32_t res);

uint32_t my_bit_rank(uint64_t slot, uint32_t rank);

size_t array_zero_count(const uint32_t *a, size_t a_size);


static inline uint32_t popcnt32(uint32_t x) {
    __asm__ ("popcnt %1, %0" : "=r" (x) : "0" (x));
    return x;
}

static inline uint64_t popcnt64(uint64_t x) {
    __asm__ ("popcnt %1, %0" : "=r" (x) : "0" (x));
    return x;
}

static inline uint32_t lzcnt32(uint32_t x) {
    __asm__ ("lzcnt %1, %0" : "=r" (x) : "0" (x));
    return x;
}

static inline uint64_t lzcnt64(uint64_t x) {
    __asm__ ("lzcnt %1, %0" : "=r" (x) : "0" (x));
    return x;
}


#endif //CLION_CODE_BIT_OP_H
