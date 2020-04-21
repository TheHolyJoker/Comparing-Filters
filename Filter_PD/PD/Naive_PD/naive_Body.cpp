//
// Created by tomer on 11/5/19.
//

#include "naive_Body.h"

naive_Body::naive_Body(size_t m, size_t f, size_t l) : fp_size(l), max_capacity(f), capacity(0) {
    this->vec.resize(f * l);

}

bool naive_Body::lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    return vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index) == 1;
}

void naive_Body::insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    if (capacity == max_capacity) {
        cout << "Tyring to insert to full vector!!!" << endl;
    }
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    vector_push(vector_bit_counter);
    write_FP_to_vector_by_index(&vec, vector_bit_counter, remainder, fp_size);
    capacity++;
}

void naive_Body::remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    if (res == 2) {
        cout << "vector - Trying to delete element with remainder that is not in the Body." << endl;
        return;
    }
    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    for (size_t i = vector_bit_counter; i < vec.size() - fp_size; ++i) {
        vec[i] = vec[i + fp_size];
    }
    for (size_t i = 0; i < fp_size; ++i)
        vec[vec.size() - fp_size + i] = false;

    capacity--;
}

bool
naive_Body::conditional_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    if (res == 2) return false;

    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    for (size_t i = vector_bit_counter; i < vec.size() - fp_size; ++i) {
        vec[i] = vec[i + fp_size];
    }
    for (size_t i = 0; i < fp_size; ++i)
        vec[vec.size() - fp_size + i] = false;

    capacity--;
    return true;
    /*//todo: lookup and delete simultaneously.
    if (lookup(abstract_body_start_index, abstract_body_end_index, remainder)) {
        remove(abstract_body_start_index, abstract_body_end_index, remainder);
        return true;
    }
    return false;*/
}

int naive_Body::vector_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder,
                            size_t *p_B_index, size_t *p_bit_index) {

    size_t vector_start_index = abstract_body_start_index * fp_size;
    size_t vector_end_index = abstract_body_end_index * fp_size;
    size_t i = vector_start_index;
    for (; i < vector_end_index; i += fp_size) {
        auto temp = read_FP_from_vector_by_index(&vec, i, fp_size);
        if (temp < remainder)
            continue;
        else if (temp == remainder) {
            *p_B_index = i / BODY_BLOCK_SIZE;
            *p_bit_index = i % BODY_BLOCK_SIZE;
            return 1;
        } else {
            *p_B_index = i / BODY_BLOCK_SIZE;
            *p_bit_index = i % BODY_BLOCK_SIZE;
            return 2;
        }
    }
    assert(i == vector_end_index);
    *p_B_index = vector_end_index / BODY_BLOCK_SIZE;
    *p_bit_index = vector_end_index % BODY_BLOCK_SIZE;
    return 2;

}

void naive_Body::vector_push(size_t vector_bit_counter) {
    for (size_t i = vec.size() - 1; i >= vector_bit_counter + fp_size; --i)
        vec[i] = vec[i - fp_size];
}

