//
// Created by tomer on 4/3/20.
//

#ifndef CLION_CODE_HASH_TABLE_VAR_HPP
#define CLION_CODE_HASH_TABLE_VAR_HPP

#include <iostream>
#include <zconf.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <cmath>
#include "../Global_functions/macros.h"
#include "../Hash/static_hashing.h"
#include "../bit_operations/my_bit_op.hpp"

template<typename T>
class hash_table_var {
    T *table;
    const size_t table_size, max_capacity, element_size, bucket_size;
    size_t capacity;
    const double max_load_factor;


public:
    hash_table_var(size_t max_capacity, size_t element_size, size_t bucket_size, double max_load_factor);

    void insert(T x);

    void pop_attempt(T x);

    void remove(T x);

    auto find(T x);

    auto find_helper(T x, size_t bucket_index)-> bool ;

    virtual ~hash_table_var();

private:
    auto get_bucket_capacity(size_t bucket_index);

    auto is_equal(T x, T y) -> bool;
};


auto compute_size(const size_t max_capacity, const size_t element_size, const double max_load_factor,
                  const size_t slot_size) -> size_t;

#endif //CLION_CODE_HASH_TABLE_VAR_HPP
