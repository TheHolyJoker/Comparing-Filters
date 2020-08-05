/*
#ifndef FILTERS_CCPD_HPP
#define FILTERS_CCPD_HPP

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <ctgmath>
#include <climits>
#include <iomanip>
#include "immintrin.h"

template <size_t bits_per_item = 8, size_t capacity = 51, size_t quot_range = 50>
class cc_pd
{
    // uint64_t a[(bits_per_item + 2) * capacity];
    uint64_t a[8];
    // const uint_fast16_t fp_size, max_capacity, size;
    // const bool deal_with_joined_slot;

public:
    cc_pd(size_t m, size_t f, size_t l)
    {
        assert(bits_per_item == 8);
        assert(capacity == 51);
        assert(quot_range == 50);
        assert(capacity <= 64);
        a[0] = (1<<50) - 1;
        std::fill(&a[1],pd_array + number_of_pd, 0);
    }

    virtual ~cc_pd();

    auto lookup(uint64_t q, char r) -> bool
    {
        size_t start = 0, end = 0;
        if (q == 0) {
            end = __builtin_clz(a[0]);
            assert(end < 64);
            bool end_is_zero_validation = !(end == 0) or (a[0] & ((1ULL) << 63u))
            assert(end_is_zero_validation);
        } else {
            assert(__builtin_popcount(a[0]) == q); // Did not handle this case.
            
            auto temp =  select64(a[0],q);
            if (temp < 64){
                start = temp;
                end_temp = select64(a[0],q + 1);
                end = (end_temp < 64) ? end_temp : __builtin_clz(a[1]);
            }
            else{
                auto count __builtin_popcount(a[0]);
                assert (count < q);
                start = select64(a[1], q - count);
                assert(start < 64);
                end = select64(a[1], q - count + 1)
            }
            uint64_t slot = ((ulong) (w1) << 32ul) | w2;
            *start_index = select_r(~slot, quotient) + 1;
            *end_index = select_r(~slot, quotient + 1);
//            *end_index = __builtin_clz(~(slot << (*start_index))) + *start_index;
        }
        auto temp =  select64(a[0],q);
        if (temp == 64){
            auto count __builtin_popcount(a[0])
        }
    }

    void insert(CG_TYPE q, CG_TYPE r);

    void remove(CG_TYPE q, CG_TYPE r);

    */
/**
     * Removes the element only if it is in the PD.
     * @param quotient
     * @param remainder
     * @return true if the element was removed, false otherwise.
     *//*

    auto conditional_remove(CG_TYPE q, CG_TYPE r) -> bool;

    auto naive_conditional_remove(CG_TYPE q, CG_TYPE r) -> bool;

    auto get_last_a_index_containing_the_header() -> size_t;

    void print_as_consecutive_memory();

    auto get_capacity() -> size_t;

    auto is_full() -> bool;

private:
    auto header_lookup(CG_TYPE q, size_t *start_index, size_t *end_index) -> bool;

    void header_insert(CG_TYPE q, size_t *start_index, size_t *end_index);

    void header_remove(CG_TYPE q, size_t *start_index, size_t *end_index);

    void header_find(CG_TYPE q, size_t *start, size_t *end);

    void header_push(size_t end_index);

    void header_pull(size_t end_index);

    static auto get_number_of_bits_in_a(size_t m, size_t f, size_t l) -> size_t;

    auto get_header_bit_index() -> size_t;

    inline auto get_header_size_in_bits() -> size_t
    {
        return max_capacity << 1u;
    }

    auto get_joined_slot_index() -> size_t;

    auto body_find(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                   size_t *p_bit_index) -> bool;

    void body_insert(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    void body_push(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    void body_remove(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    auto body_conditional_remove(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) -> bool;

    */
/**
     * I think! Moves all elements after the element that starts at "bit_index" to their predecessor position.
     * @param B_index
     * @param bit_index
     *//*

    void body_pull(size_t B_index, size_t bit_index);

public:
    */
/*For testing*//*

    auto get_a() const -> uint32_t *;

    auto get_fp_size() const -> const uint_fast16_t;

    auto get_max_capacity() const -> const uint_fast16_t;

    auto get_size() const -> const uint_fast16_t;

    auto get_deal_with_joined_slot() const -> const bool;

    void print_as_array() const;

    friend auto operator<<(std::ostream &os, const PD &pd) -> std::ostream &;
};

static auto select64(uint64_t x, int64_t j) -> uint64_t
{
    assert(j < 64);
    const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
    return _tzcnt_u64(y);
}

static auto get_a_size(size_t m, size_t f, size_t l) -> size_t;

static inline void body_find_helper(size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index)
{
    *p_B_index = current_b_index;
    *p_bit_index = BODY_BLOCK_SIZE - bits_left;
}

static auto should_deal_with_joined_slot(size_t m, size_t f, size_t l) -> bool
{
    return bool((f * 2) % CG_TYPE_SIZE);
}

uint32_t select_r(uint64_t slot, uint32_t rank);
}

#endif //FILTERS_CCPD_HPP*/
