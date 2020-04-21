//
// Created by tomer on 10/29/19.
//
/**
 * This PD is used for debugging. It maintains two implementation for the header, and two for the body.
 * One is efficient implementation of the header and the body, packed in array,
 * the second one is an implementation of the header and body as bit vector.
 * After every operation on the safe_PD, it compares those two implementation,
 * return error if they don't.
 */
#ifndef CLION_CODE_SAFE_PD_H
#define CLION_CODE_SAFE_PD_H

#include "v_Header.h"
#include "v_Body.h"


class safe_PD {
    v_Header v_header;
    v_Body v_body;
    size_t capacity;
    size_t max_capacity;

public:
    safe_PD(size_t m, size_t f, size_t l);

    bool lookup(size_t quotient, FP_TYPE remainder);

    void insert(size_t quotient, FP_TYPE remainder);

    void remove(size_t quotient, FP_TYPE remainder);

    void print();
};


#endif //CLION_CODE_SAFE_PD_H
