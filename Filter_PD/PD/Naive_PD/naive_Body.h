//
// Created by tomer on 11/5/19.
//

#ifndef CLION_CODE_NAIVE_BODY_H
#define CLION_CODE_NAIVE_BODY_H

#include "../../Global_functions/macros.h"
#include "../../Global_functions/basic_functions.h"
#include "../Non_Contiguous_PD/Body.h"


class naive_Body {
public:
    vector<bool> vec;
    const size_t fp_size, max_capacity;
    size_t capacity;
    naive_Body(size_t m, size_t f, size_t l);

    bool lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    bool conditional_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

//protected:
    int
    vector_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
                size_t *p_bit_index);

    void vector_push(size_t vector_bit_counter);


};


#endif //CLION_CODE_NAIVE_BODY_H
