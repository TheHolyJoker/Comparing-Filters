//
// Created by tomer on 11/24/19.
//
//todo:  (deal_with_joined_slot == false) Implies

#ifndef CLION_CODE_CG_PD_H
#define CLION_CODE_CG_PD_H


#include <ostream>
#include "../../Global_functions/basic_functions.h"
#include "../../bit_operations/bit_op.h"
#include "../../bit_operations/bit_word_converter.hpp"

#define CG_TYPE uint32_t
#define CG_TYPE_SIZE (sizeof(CG_TYPE) * (CHAR_BIT))
#define DEFAULT_PD_CAPACITY (64u)
#define DEFAULT_QUOTIENT_LENGTH (6u)
#define DEFAULT_QUOTIENT_RANGE (1u<<DEFAULT_QUOTIENT_LENGTH)


class cg_PD {
    CG_TYPE *a;
    const uint_fast16_t fp_size, max_capacity, size;
    const bool deal_with_joined_slot;

public:
    cg_PD(size_t m, size_t f, size_t l);

//    virtual ~cg_PD();

    auto lookup(CG_TYPE q, CG_TYPE r) -> bool;

    void insert(CG_TYPE q, CG_TYPE r);

    void remove(CG_TYPE q, CG_TYPE r);

    /**
     * Removes the element only if it is in the PD.
     * @param quotient
     * @param remainder
     * @return true if the element was removed, false otherwise.
     */
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

    inline auto get_header_size_in_bits() -> size_t {
        return max_capacity << 1u;
    }

    auto get_joined_slot_index() -> size_t;

    auto body_find(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                   size_t *p_bit_index) -> bool;

    void body_insert(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    void body_push(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    void body_remove(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index);

    auto body_conditional_remove(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) -> bool;


    /**
     * I think! Moves all elements after the element that starts at "bit_index" to their predecessor position.
     * @param B_index
     * @param bit_index
     */
    void body_pull(size_t B_index, size_t bit_index);

public:
    /*For testing*/
    auto get_a() const -> uint32_t *;

    auto get_fp_size() const -> const uint_fast16_t;

    auto get_max_capacity() const -> const uint_fast16_t;

    auto get_size() const -> const uint_fast16_t;

    auto get_deal_with_joined_slot() const -> const bool;

    void print_as_array() const;


    friend auto operator<<(ostream &os, const cg_PD &pd) -> ostream &;

};

static auto get_a_size(size_t m, size_t f, size_t l) -> size_t;


static inline void body_find_helper(size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index) {
    *p_B_index = current_b_index;
    *p_bit_index = BODY_BLOCK_SIZE - bits_left;
}

static auto should_deal_with_joined_slot(size_t m, size_t f, size_t l) -> bool {
    return bool((f * 2) % CG_TYPE_SIZE);
}


#endif //CLION_CODE_CG_PD_H
