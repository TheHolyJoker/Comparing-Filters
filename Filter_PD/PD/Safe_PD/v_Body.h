//
// Created by tomer on 10/29/19.
//

#ifndef CLION_CODE_V_BODY_H
#define CLION_CODE_V_BODY_H

#include "../Non_Contiguous_PD/Body.h"
#include "../Constant_size_PD/const_Body.h"

class v_Body {
    Body body;
    const_Body const_body;
    vector<bool> vec;

public:
    v_Body(size_t m, size_t f, size_t l);

    /**
     *
     * @param abstract_body_start_index if quotient = x ,and quotient's run started in the i'th index in the header,
     * start_index should be i - x + 1.
     * @param abstract_body_end_index
     * @param remainder
     * @return
     */
    bool lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    bool vector_lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void vector_insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    void vector_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);

    int
    vector_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
                size_t *p_bit_index);

    void validate_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder);


    void vector_push(size_t vector_bit_counter);

    void print();
};



#endif //CLION_CODE_V_BODY_H
