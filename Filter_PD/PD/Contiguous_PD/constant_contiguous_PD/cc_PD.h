//
// Created by tomer on 11/15/19.
//

#ifndef CLION_CODE_CC_PD_H
#define CLION_CODE_CC_PD_H

#include "../../../bit_operations/bit_op.h"
#include "../../../Global_functions/basic_functions.h"

#define M 64
#define F 48
#define L 8
#define CC_TYPE uint_fast16_t

class cc_PD {
    uint8_t arr[64]{};

public:
    cc_PD() = default;

    bool lookup(CC_TYPE quotient, CC_TYPE remainder);

    void insert(CC_TYPE quotient, CC_TYPE remainder);

    void remove(CC_TYPE quotient, CC_TYPE remainder);

    bool conditional_remove(CC_TYPE quotient, CC_TYPE remainder);

    /**Header functions*/
    void header_get_interval(CC_TYPE quotient, CC_TYPE *start_index, CC_TYPE *end_index);

    bool header_lookup(CC_TYPE quotient, CC_TYPE *start_index, CC_TYPE *end_index);

    void header_insert(CC_TYPE quotient, CC_TYPE *start_index, CC_TYPE *end_index);

    void header_remove(CC_TYPE quotient, CC_TYPE *start_index, CC_TYPE *end_index);

    void header_pull(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    void header_push(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    /**Body functions*/
    void body_get_interval(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    bool body_lookup(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    void body_insert(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    void body_remove(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    bool body_conditional_remove(CC_TYPE quotient, CC_TYPE start_index, CC_TYPE end_index);

    size_t get_capacity();
};


#endif //CLION_CODE_CC_PD_H
