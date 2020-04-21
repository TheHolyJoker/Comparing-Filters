//
// Created by tomer on 11/5/19.
//

#ifndef CLION_CODE_NAIVE_HEADER_H
#define CLION_CODE_NAIVE_HEADER_H

#include "../../Global_functions/basic_functions.h"
#include "../../Global_functions/macros.h"

class naive_Header {
public:
    vector<bool> vec;
    size_t max_capacity, capacity;

    naive_Header(size_t m, size_t f, size_t l);

    bool lookup(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

    void insert(size_t quotient, size_t *start_index, size_t *end_index);

    void insert_att(size_t quotient, size_t *start_index, size_t *end_index);

    void remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

    void remove_att(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

    void get_interval(size_t quotient, size_t *start_index, size_t *end_index);

    void pull(size_t start_index);

    vector<bool> *get_vec();


private:
//    naive_Header(naive_Header &naive_header);
    naive_Header(vector<bool> *vector);
//    bool conditional_remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index);

};


#endif //CLION_CODE_NAIVE_HEADER_H
