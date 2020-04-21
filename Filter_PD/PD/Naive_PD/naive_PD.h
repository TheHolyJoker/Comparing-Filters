//
// Created by tomer on 11/5/19.
//

#ifndef CLION_CODE_NAIVE_PD_H
#define CLION_CODE_NAIVE_PD_H


#include "naive_Header.h"
#include "naive_Body.h"

class naive_PD {
public:
    naive_Header header;
    naive_Body body;
    size_t capacity, max_capacity;

    naive_PD(size_t m, size_t f, size_t l);

    bool lookup(size_t quotient, FP_TYPE remainder);

    void insert(size_t quotient, FP_TYPE remainder);

    void remove(size_t quotient, FP_TYPE remainder);

    bool conditional_remove(size_t quotient, FP_TYPE remainder);

    bool is_full();

    size_t get_capacity();
};


#endif //CLION_CODE_NAIVE_PD_H
