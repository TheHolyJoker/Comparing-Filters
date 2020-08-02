//
// Created by tomer on 11/9/19.
//

#ifndef CLION_CODE_CONST_HEADER_H
#define CLION_CODE_CONST_HEADER_H


#include "../../Global_functions/basic_functions.h"
#include "../../bit_operations/bit_op.h"

class const_Header {
    D_TYPE w1, w2;

public:
    const_Header();

    bool lookup(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

    void insert(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

    void remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

    inline void get_interval(size_t quotient, size_t *start_index, size_t *end_index) {
        if (quotient == 0) {
            *start_index = 0;
            *end_index = __builtin_clz(~w1);
            if (*end_index == D_TYPE_SIZE) *end_index += __builtin_clz(~w2);
        } else {
            uint64_t slot = ((ulong) (w1) << 32ul) | w2;
            *start_index = select_r(~slot, quotient) + 1;
            *end_index = select_r(~slot, quotient + 1);
//            *end_index = __builtin_clz(~(slot << (*start_index))) + *start_index;
        }
    }

    void push(size_t end);

    void pull(size_t end);

    inline size_t get_capacity() {
        return __builtin_popcount(w1) + __builtin_popcount(w2);
    }

    void get_w1w2(uint32_t *p1, uint32_t *p2);

    void print();

private:


};

static void get_interval(D_TYPE w1, D_TYPE w2, size_t quotient, size_t *start_index, size_t *end_index) {
    if (quotient == 0) {
        *start_index = 0;
        *end_index = __builtin_clz(~w1);
        if (*end_index == D_TYPE_SIZE) *end_index += __builtin_clz(~w2);
    } else {
        uint64_t slot = ((ulong) (w1) << 32ul) | w2;
        *start_index = select_r(~slot, quotient) + 1;
        *end_index = select_r(~slot, quotient + 1);
//        *end_index = __builtin_clz(~(slot << (*start_index - 1))) + *(start_index);

    }
}

static void get_interval2(D_TYPE w1, D_TYPE w2, size_t quotient, size_t *start_index, size_t *end_index) {
    if (quotient == 0) {
        *start_index = 0;
        *end_index = __builtin_clz(~w1);
        if (*end_index == D_TYPE_SIZE) *end_index += __builtin_clz(~w2);
    } else {
        uint64_t slot = ((ulong) (w1) << 32ul) | w2;
        *start_index = select_r(~slot, quotient);

//        *end_index = select_r(~slot, quotient + 1);
        *end_index = __builtin_clz(~(slot << (*start_index - 1))) + (*start_index)++;
//        (*start_index)++;

    }
}

void static_push(D_TYPE *w1, D_TYPE *w2, size_t end);

void static_pull(D_TYPE *w1, D_TYPE *w2, size_t end);

#endif //CLION_CODE_CONST_HEADER_H
