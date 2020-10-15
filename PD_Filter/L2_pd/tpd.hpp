/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_TPD_HPP
#define FILTERS_TPD_HPP

#include "x86intrin.h"
#include <assert.h>
#include <immintrin.h>
#include <iostream>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <vector>

#define MASK(p) ((1ULL << (p)) - 1ULL)

typedef std::vector<bool> b_vec;
typedef unsigned __int128 u128;

template<size_t max_capacity, size_t batch_size, size_t quot_range, size_t bits_per_item>
constexpr size_t get_l2_bucket_bit_size() {
    constexpr size_t mask_part = max_capacity;
    constexpr size_t pd_index_part = batch_size + max_capacity;
//    constexpr size_t quot_part = 4 * max_capacity;
    constexpr size_t rem_part = bits_per_item * max_capacity;
    constexpr size_t min_size = mask_part + pd_index_part + rem_part;
//    constexpr size_t min_size = mask_part + pd_index_part + quot_part + rem_part;
    constexpr size_t rounded_size = ((min_size + (64 - 1)) / 64) * 64;
    return rounded_size;
}

template<size_t max_capacity, size_t batch_size, size_t quot_range, size_t bits_per_item>
constexpr auto get_l2_bucket_size() {
    return get_l2_bucket_bit_size() / (sizeof(uint64_t) * CHAR_BIT);
}

template<size_t max_capacity, size_t batch_size, size_t quot_range, size_t bits_per_item>
void init_vec(std::vector<bool> *v) {
    size_t total_size = get_l2_bucket_bit_size();
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

    //quot part
    offset = index;
    for (; index < offset + quot_range; index++) {
        v->at(index) = 1;
    }
    offset = index;
    for (; index < offset + max_capacity; index++) {
        v->at(index) = 0;
    }

    //Mask_part
    offset = index;
    for (; index < offset + mask; index++) {
        v->at(index) = 0;
    }

    //rem part
    for (; index < total_size; index++) {
        v->at(index) = 0;
    }
}

int compare_vector_and_tpd(b_vec *v, uint64_t *arr, size_t bits_to_compare);

// returns the position (starting from 0) of the jth set bit of x.
inline uint64_t select64(uint64_t x, int64_t j) {
    assert(j < 64);
    const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
    return _tzcnt_u64(y);
}


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
        return (header - 1) & (~header);
    }
    uint64_t hi = header >> 64;
    uint64_t lo = header;

    uint64_t pop = _mm_popcnt_u64(lo);
    if (index - 1 < pop) {
        uint64_t temp_mask = get_select_mask(lo, index - 1);
        uint64_t res = mask_between_bits(temp_mask);
        return res;
    } else if (index - 1 == pop) {
        uint64_t lo_temp_mask = get_select_mask(lo, index - 1);// keeping only the highest set bit.
        assert(_popcnt64(lo_temp_mask) == 1);
        uint64_t lo_mask = ~(lo_temp_mask - 1);
        uint64_t hi_mask = (hi - 1) & (~hi);
        u128 mask = lo_mask | (hi_mask << 64ULL);
        return mask;
    } else {
        uint64_t new_index = index - pop;
        assert(new_index >= 2);
        uint64_t temp_mask = get_select_mask(hi, index - 1);
        uint64_t hi_mask = mask_between_bits(temp_mask);
        u128 mask = hi_mask << 64;
        return mask;
    }
}

auto count_zeros_up_to_the_kth_one(uint64_t word, size_t k) -> size_t;

auto count_ones_up_to_the_kth_zero(uint64_t word, size_t k) -> size_t;

auto count_zeros_up_to_the_kth_one(const uint64_t *pd, size_t k) -> size_t;

auto count_ones_up_to_the_kth_zero(const uint64_t *arr, size_t k) -> size_t;

void right_shift_array(uint64_t *a, size_t a_size, uint64_t shift);

void read_bits(const uint64_t *src, uint64_t *dest, size_t start, size_t end);

bool test_bit(const uint64_t *a, size_t bit_index);

template<size_t max_capacity, size_t batch_size, size_t quot_range, size_t bits_per_item>
class tpd {

    constexpr size_t bucket_size = get_l2_bucket_size();
    uint64_t pd[get_l2_bucket_size()];

public:
    tpd() {
        std::vector<bool> v(get_l2_bucket_bit_size(), false);
        init_vec(&v);
        constexpr size_t zeros_left = max_capacity - (64 - batch_size);
        constexpr uint64_t empty_pd[get_l2_bucket_size()] = {MASK(batch_size), MASK(quot_range) << (zeros_left)};
        int cmp_res = compare_vector_and_tpd(&v, empty_pd, get_l2_bucket_bit_size());
        assert(cmp_res == -1);

        memcpy(pd, empty_pd, (get_l2_bucket_bit_size() + 7) / 8);
        int cmp_res2 = compare_vector_and_tpd(&v, pd, get_l2_bucket_bit_size());
        assert(cmp_res2 == -1);
    }

    auto get_pd_header_size() -> size_t {
        return max_capacity + batch_size;
    }
    auto get_quot_header_size() -> size_t {
        return max_capacity + quot_range;
    }
    auto get_bitmask_size() -> size_t {
        return max_capacity;
    }

    auto get_capacity() -> size_t {
        auto pd_index_res = count_zeros_up_to_the_kth_one(pd, batch_size);
        uint64_t pd_header[2] = {0};
        uint64_t quot_header[2] = {0};
        read_pd_header(pd_header);
        read_quot_header(quot_header);
        auto pd_header_res = count_zeros_up_to_the_kth_one(pd_header, max_capacity);
        auto quot_header_res = count_zeros_up_to_the_kth_one(quot_header, max_capacity);
        assert(pd_header == quot_header);
        assert(pd_header == pd_index_res);
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
        uint64_t a[2] = {0};
        read_pd_header(a);
        constexpr unsigned kBytes2copy = (max_capacity + batch_size + CHAR_BIT - 1) / CHAR_BIT;
        unsigned __int128 x;
        memcpy(&x, a, kBytes2copy);
        return x;
    }

    void read_quot_header(uint64_t *dest) {
        constexpr unsigned start = max_capacity + batch_size;
        constexpr unsigned size = max_capacity + quot_range;
        read_bits(pd, dest, start, start + size);
    }

    unsigned __int128 get_quot_header() {
        uint64_t a[2] = {0};
        read_quot_header(a);
        constexpr unsigned kBytes2copy = (max_capacity + quot_range + CHAR_BIT - 1) / CHAR_BIT;
        unsigned __int128 x;
        memcpy(&x, a, kBytes2copy);
        return x;
    }

    uint64_t get_location_bitmask_header() {
        constexpr unsigned start = get_pd_header_size() + quot_header_size() constexpr unsigned size = get_bitmask_size();
        uint64_t dest = 0;
        read_bits(pd, &dest, start, start + size);
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
        constexpr unsigned offset = get_pd_header_size() + quot_header_size();
        return test_bit(pd, index + offset);
    }

    bool find(uint64_t pd_index, int64_t quot, uint8_t rem, bool is_primary_location) {
        uint64_t v = body_find(rem);
        u128 pd_mask = get_pd_header_mask(pd_index);
        u128 quot_mask = get_quot_header_mask(quot);
        uint64_t location_mask = get_location_mask(is_primary_location);
        u128 v_pd = ((unsigned __int128) v) << pd_index;
        u128 v_quot = ((unsigned __int128) v) << quot;

        bool a = v_pd & pd_mask;
        bool b = v_quot & quot_mask;
        bool c = location_mask & v;
        auto res = a && b && c;
        return res;
    }

    uint64_t body_find(uint8_t rem) {
        constexpr unsigned bucket_size = get_l2_bucket_size();
        constexpr unsigned shift = 64 - max_capacity;
        assert(bucket_size >= 8);
        uint64_t *address = &pd + (bucket_size - 8);
        const __m512i body = _mm512_loadu_si512(address);
        const __m512i target = _mm512_set1_epi8(rem);
        return _mm512_cmpeq_epu8_mask(target, body) >> shift;
    }


    u128 get_pd_header_mask(uint64_t pd_index) {
        return get_mask_from_header(get_pd_header(), pd_index);
    }

    u128 get_quot_header_mask(int64_t quot) {
        return get_mask_from_header(get_quot_header(), quot);
    }

    uint64_t get_location_mask(bool is_primary_location) {
        uint64_t mask = get_location_bitmask_header();
        if (is_primary_location)
            return mask;
        else {
            return (~mask);// & MASK(max_capacity);
        }
    }


    bool insert(uint64_t pd_index, int64_t quot, uint8_t rem, bool is_primary_location) {
        if is_full ()
            return false;
    }
    /* void read_pd_header_complex(uint64_t *dest) {
        dest[0] = dest[1] = 0;
        constexpr size_t slot_size = 64;
        constexpr size_t total_bit_size = batch_size + max_capacity;
        constexpr size_t words_count = (total_bit_size + slot_size - 1) / slot_size;
        auto pop = _mm_popcnt_u64(pd[0]);
        if (pop >= batch_size){
            memcpy(dest, pd, slot_size / CHAR_BIT);
            auto index = select64(dest[0], batch_size);
            if (index == slot_size)
                return;
            
            assert(index >= batch_size);
            dest[0] &= MASK(index);
        }
    }
 */


}


// #define QUOT_SIZE22 (22)
// #define CAPACITY26 (26)

namespace v_tpd {
    template<typename T>
    void bin_print(T x) {
        if (x == 0) {
            std::cout << '0' << std::endl;
            return;
        }
        size_t slot_size = sizeof(T) * CHAR_BIT;
        uint64_t b = 1ULL << (slot_size - 1u);
        while ((!(b & x)) and (b)) {
            b >>= 1ul;
        }
        assert(b);
        while (b) {
            std::string temp = (b & x) ? "1" : "0";
            std::cout << temp;
            b >>= 1ul;
        }
        std::cout << std::endl;
    }
    void bin_print_header(uint64_t header);
    auto bin_print_header_spaced(uint64_t header)->std::string;

}// namespace v_tpd


namespace tpd {

    auto validate_number_of_quotient(const __m256i *pd) -> bool;
    auto validate_number_of_quotient(uint64_t clean_header) -> bool;
    auto get_capacity_att(const __m256i *x) -> size_t;


    void validate_clz(int64_t quot, char rem, const __m256i *pd);

    // returns the position (starting from 0) of the jth set bit of x.
    inline uint64_t select64(uint64_t x, int64_t j) {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    auto is_pd_full_naive(const __m256i *pd) -> bool;

    inline bool pd_full(const __m256i *pd) {
        assert(validate_number_of_quotient(pd));
        auto res = (((uint64_t *) pd)[0]) & (1ULL << 47);
        bool v_res = (get_capacity_att(pd) == CAPACITY26);
        assert((!!res) == (get_capacity_att(pd) == CAPACITY26));
        return res;
    }


    // inline int pd_popcount(const __m256i *pd) {
    //     return get_capacity_att(pd);
    // }

    inline auto get_clean_header(const __m256i *pd) -> uint64_t {
        auto res = (_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd))) & ((1ULL << 48) - 1);
        assert(validate_number_of_quotient(res));
        return (_mm_cvtsi128_si64(_mm256_castsi256_si128(*pd))) & ((1ULL << 48) - 1);
    }

    inline auto get_capacity(const __m256i *pd) -> int {
        const uint64_t header = get_clean_header(pd);
        auto res = 42 - _lzcnt_u64(header);
        auto v_res = get_capacity_att(pd);
        assert(v_res == res);
        assert(res == get_capacity_att(pd));
        return res;
    }


    //Another way to compte begin and end.
    inline void test_get_begin_and_end(int64_t quot, uint8_t rem, const __m256i *pd) {
        const uint64_t header = ((uint64_t *) pd)[0];
        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0) - quot;
        const uint64_t end_valid = select64(header, quot) - quot;
        uint64_t my_begin = 0;
        uint64_t my_end = 0;
        uint64_t shifted_header = header;

        int caser = 0;
        if (quot == 0) {
            caser = 1;
            my_begin = 0;
            my_end = _tzcnt_u64(header);
            assert(header);
            assert(my_end < 64);
            assert(my_begin <= my_end);
        } else {
            caser = 2;
            uint64_t temp = select64(header, quot - 1) + 1;
            my_begin = temp - quot;
            shifted_header = header >> temp;
            assert(shifted_header);
            uint64_t temp2 = _tzcnt_u64(shifted_header);
            assert(temp2 < 32);
            my_end = my_begin + temp2;
            assert(my_begin <= my_end);
        }

        assert(my_begin == begin);
        if (my_end != end_valid) {
            size_t line_length = 120;
            std::string line = std::string(line_length, '*');
            std::string line_mid = std::string(line_length, '_');
            std::cout << line << std::endl;
            std::cout << "caser: \t\t\t" << caser << std::endl;
            std::cout << "quot:  \t\t\t" << quot << std::endl;
            std::cout << "begin: \t\t\t " << begin << std::endl;
            std::cout << "end_valid: \t\t" << end_valid << std::endl;
            std::cout << "my_end: \t\t" << my_end << std::endl;
            // std::cout << "header_begin: " << header_begin << std::endl;
            // std::cout << "header_end: " << header_end << std::endl;
            // std::cout << "header_begin - quot: " << header_begin - quot << std::endl;
            // std::cout << "header_end - quot: " << header_end - quot << std::endl;

            std::cout << line_mid << std::endl;

            std::cout << "header: " << header << std::endl;
            std::cout << "bin(header): \t\t";
            v_tpd::bin_print_header(header);
            std::cout << std::endl;

            std::cout << "bin(shifted_header):\t";
            v_tpd::bin_print_header(shifted_header);
            std::cout << std::endl;

            std::cout << line_mid << std::endl;
            std::cout << "header:   " << v_tpd::bin_print_header_spaced(header) << std::endl;
            std::cout << "s_header: " << v_tpd::bin_print_header_spaced(shifted_header) << std::endl;


            std::cout << line << std::endl;


            // std::cout << "end2:\t " << end2 << std::endl;
            // std::cout << "end3:\t " << end3 << std::endl;
            // std::cout << "_tzcnt_u64(header >> begin): \t" << _tzcnt_u64(header >> begin) << std::endl;
            assert(0);
        }
        assert(my_end == end_valid);
    }


    inline uint32_t get_v(uint8_t rem, const __m256i *pd) {
        const __m256i target = _mm256_set1_epi8(rem);
        return _mm256_cmpeq_epu8_mask(target, *pd);
    }
    inline bool pd_find_26_ver9(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE22);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd) >> 6ul;
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

    inline uint64_t mask_between_bits_naive5(uint64_t x) {
        uint64_t hi_bit = (x - 1) & x;
        uint64_t clear_hi = hi_bit - 1;
        uint64_t lo_set = (x - 1);
        uint64_t res = clear_hi ^ lo_set;// & (~x0)
        return res;
    }


    inline bool pd_find_26_ver17(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 32));
        assert(quot < QUOT_SIZE22);

        const __m256i target = _mm256_set1_epi8(rem);
        const uint64_t v = _mm256_cmpeq_epu8_mask(target, *pd);
        if (!v)
            return false;
        const uint64_t h0 = get_clean_header(pd);
        const uint64_t new_v = ((v >> 6ul) << quot) & ~h0;
        const uint64_t v_mask = (quot) ? mask_between_bits_naive5(_pdep_u64(3ul << (quot - 1), h0)) : (_blsmsk_u64(h0) >> 1ul);
        return v_mask & new_v;
    }

    inline bool pd_find_with_v(int64_t quot, uint64_t v, const __m256i *pd) {
        const uint64_t h0 = get_clean_header(pd);
        const uint64_t new_v = ((v >> 6ul) << quot) & ~h0;
        const uint64_t v_mask = (quot) ? mask_between_bits_naive5(_pdep_u64(3ul << (quot - 1), h0)) : (_blsmsk_u64(h0) >> 1ul);
        return v_mask & new_v;
    }


    inline bool pd_find_26(int64_t quot, uint8_t rem, const __m256i *pd) {
        assert(pd_find_26_ver9(quot, rem, pd) == pd_find_26_ver17(quot, rem, pd));
        return pd_find_26_ver9(quot, rem, pd);
    }


    inline bool pd_add_26(int64_t quot, char rem, __m256i *pd) {
        assert(quot < QUOT_SIZE22);
        const uint64_t header = get_clean_header(pd);
        if (pd_full(pd))
            return false;
        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= 48);
        const __m256i target = _mm256_set1_epi8(rem);
        constexpr unsigned kBytes2copy = 6;
        uint64_t new_header = header & ((1ULL << begin) - 1);
        // unsigned __int128 new_header = header & ((((unsigned __int128) 1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(_mm_popcnt_u64(new_header) == QUOT_SIZE22);
        // assert(select64(new_header, 32 - 1) - (32 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 32);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem <= ((const char *) pd)[kBytes2copy + i])
                break;
        }
        assert((i == end_fingerprint) || (rem <= ((const char *) pd)[kBytes2copy + i]));
        memmove(&((char *) pd)[kBytes2copy + i + 1],
                &((const char *) pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((char *) pd)[kBytes2copy + i] = rem;

        assert(pd_find_26(quot, rem, pd));
        return true;
    }

    auto remove_naive(int64_t quot, char rem, __m256i *pd) -> bool;

    inline auto remove(int64_t quot, char rem, __m256i *pd) -> bool {
        assert(false);
        // assert(quot < QUOT_SIZE22);
        // assert(pd_find_26(quot, rem, pd));
        // const uint64_t header = ((uint64_t *) pd)[0];
        // assert(_mm_popcnt_u64(header) == 24);
        // constexpr unsigned kBytes2copy = 6;

        // const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        // const uint64_t end = select64(header, quot);
        // assert(begin <= end);
        // assert(end <= 63);

        // uint64_t new_header = header & ((1ULL << begin) - 1);
        // new_header |= ((header >> end) << (end - 1));
        // assert(_mm_popcnt_u64(new_header) == 24);
        // //
        // memcpy(pd, &new_header, kBytes2copy);
        // const uint64_t begin_fingerprint = begin - quot;
        // const uint64_t end_fingerprint = end - quot;
        // assert(begin_fingerprint <= end_fingerprint);
        // assert(end_fingerprint <= 32);
        // uint64_t i = begin_fingerprint;
        // for (; i < end_fingerprint; ++i) {
        //     if (rem == ((const char *) pd)[kBytes2copy + i])
        //         break;
        // }
        // assert(rem == ((const char *) pd)[kBytes2copy + i]);

        // memmove(&((char *) pd)[kBytes2copy + i],
        //         &((const char *) pd)[kBytes2copy + i + 1],
        //         sizeof(*pd) - (kBytes2copy + i + 1));
        // // ((char *) pd)[kBytes2copy + i] = rem;

        // assert(pd_find_32(quot, rem, pd));
        return true;
    }

    inline auto conditional_remove(int64_t quot, char rem, __m256i *pd) -> bool {
        assert(false);

        assert(quot < 32);
        // assert(pd_find_32(quot, rem, pd));
        const uint64_t header = ((uint64_t *) pd)[0];
        assert(_mm_popcnt_u64(header) == 24);
        constexpr unsigned kBytes2copy = 6;

        const uint64_t begin = (quot ? (select64(header, quot - 1) + 1) : 0);
        const uint64_t end = select64(header, quot);
        assert(begin <= end);
        assert(end <= 63);

        uint64_t new_header = header & ((1ULL << begin) - 1);
        new_header |= ((header >> end) << (end - 1));

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 32);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const char *) pd)[kBytes2copy + i])
                break;
        }
        if ((i == end_fingerprint) || (rem != ((const char *) pd)[kBytes2copy + i])) {
            return false;
        }

        assert(_mm_popcnt_u64(new_header) == 24);
        // memcpy(pd, &new_header, kBytes2copy);
        memmove(&((char *) pd)[kBytes2copy + i],
                &((const char *) pd)[kBytes2copy + i + 1],
                sizeof(*pd) - (kBytes2copy + i + 1));

        return true;
    }

    void print512(const __m256i *var);

    inline auto pd_popcount_att_helper_start(const __m256i *pd) -> int {
        uint64_t header;
        memcpy(&header, reinterpret_cast<const uint64_t *>(pd), 8);
        return 14ul - _lzcnt_u64(header);
    }

    auto validate_number_of_quotient(const __m256i *pd) -> bool;

    auto get_capacity_naive(const __m256i *x) -> size_t;

    auto get_name() -> std::string;
    ////New functions
    inline auto is_full(const __m256i *x) -> bool {
        assert(get_capacity_naive(x) == get_capacity(x));
        return get_capacity(x) == 32;
    }
}// namespace tpd


#endif// FILTERS_TPD_HPP

// auto my_equal(__m256i x, __m256i y) -> bool;
