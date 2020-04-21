//
// Created by tomer on 11/5/19.
//

#include "naive_PD.h"

naive_PD::naive_PD(size_t m, size_t f, size_t l) : header(m, f, l), body(m, f, l), capacity(0), max_capacity(f) {}


bool naive_PD::lookup(size_t quotient, FP_TYPE remainder) {
    size_t start_index = -1, end_index = -1;
    if (not header.lookup(quotient, &start_index, &end_index))
        return false;
    assert(quotient <= start_index);
    assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    return body.lookup(body_start, body_end, remainder);
}

void naive_PD::insert(size_t quotient, FP_TYPE remainder) {
    if (capacity == max_capacity) {
        cout << "Tyring to insert to full naive_PD!!!" << endl;
    }
    size_t start_index = -1, end_index = -1;

    header.insert_att(quotient, &start_index, &end_index);
    assert(quotient <= start_index);
    assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;

    body.insert(body_start, body_end, remainder);
    capacity++;
}

void naive_PD::remove(size_t quotient, FP_TYPE remainder) {
    size_t start_index = -1, end_index = -1;
    header.remove(quotient, &start_index, &end_index);

    assert(quotient <= start_index);
    assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    body.remove(body_start, body_end, remainder);
    capacity--;

}

size_t naive_PD::get_capacity() {
    return capacity;
}

bool naive_PD::conditional_remove(size_t quotient, FP_TYPE remainder) {
    size_t start_index = -1, end_index = -1;
    if (not this->header.lookup(quotient, &start_index, &end_index))
        return false;

    assert(quotient <= start_index);
    assert(start_index <= end_index);

    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    if (this->body.conditional_remove(body_start, body_end, remainder)) {
        this->header.pull(end_index - 1);
        capacity--;
        return true;
    }
    return false;
}

bool naive_PD::is_full() {
    return capacity == max_capacity;
}
