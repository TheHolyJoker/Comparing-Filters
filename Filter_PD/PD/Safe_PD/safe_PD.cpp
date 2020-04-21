//
// Created by tomer on 10/29/19.
//

#include "safe_PD.h"

safe_PD::safe_PD(size_t m, size_t f, size_t l) : v_header(m, f, l), v_body(m, f, l), capacity(0), max_capacity(f) {}

bool safe_PD::lookup(size_t quotient, FP_TYPE remainder) {
    size_t start_index = -1, end_index = -1;
    if (not v_header.lookup(quotient, &start_index, &end_index))
        return false;
    assert(quotient <= start_index);
    assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    return v_body.lookup(body_start, body_end, remainder);
}

void safe_PD::insert(size_t quotient, FP_TYPE remainder) {
    if (capacity == max_capacity) {
        cout << "Tyring to insert to full safe_PD!!!" << endl;
    }
    size_t start_index = -1, end_index = -1;
    v_header.insert(quotient, &start_index, &end_index);
    assert(quotient <= start_index);
    assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;

    /*uint8_t body_start = start_index - quotient + 1;
    uint8_t body_end = end_index - quotient + 1;*/
    v_body.insert(body_start, body_end, remainder);
    capacity++;
}

void safe_PD::remove(size_t quotient, FP_TYPE remainder) {
    size_t start_index = -1, end_index = -1;
    v_header.remove(quotient, &start_index, &end_index);

    assert(quotient <= start_index);
    assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    v_body.remove(body_start, body_end, remainder);
    capacity--;
}

void safe_PD::print() {
    v_header.print();
    v_body.print();
}