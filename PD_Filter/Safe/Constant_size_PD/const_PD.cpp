//
// Created by tomer on 11/9/19.
//

#include "const_PD.h"


const_PD::const_PD(bool preventing_empty_constructor) : capacity(0), header(), body() {}

bool const_PD::lookup(size_t quotient, uint8_t remainder) {
    size_t start_index = -1, end_index = -1;
    if (not header.lookup(quotient, &start_index, &end_index))
        return false;
    if (DB) assert(quotient <= start_index);
    if (DB) assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    return body.lookup(body_start, body_end, remainder);
}

void const_PD::insert(size_t quotient, uint8_t remainder) {
    size_t start_index = -1, end_index = -1;
    this->header.insert(quotient, &start_index, &end_index);
//    end_index = max(start_index, end_index);
    if (DB) assert(quotient <= start_index);
    if (DB) assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;

    /*uint8_t body_start = start_index - quotient + 1;
    uint8_t body_end = end_index - quotient + 1;*/
    this->body.insert(body_start, body_end, remainder);
    capacity++;

}

void const_PD::remove(size_t quotient, uint8_t remainder) {
    size_t start_index = -1, end_index = -1;
    this->header.remove(quotient, &start_index, &end_index);
//    end_index = max(start_index, end_index);

    if (DB) assert(quotient <= start_index);
    if (DB) assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    this->body.remove(body_start, body_end, remainder);
    capacity--;
}

bool const_PD::conditional_remove(size_t quotient, uint8_t remainder) {
    size_t start_index = -1, end_index = -1;
    if (not this->header.lookup(quotient, &start_index, &end_index))
        return false;

    assert(quotient <= start_index);
    assert(start_index <= end_index);

    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    if (this->body.conditional_remove(body_start, body_end, remainder)) {
        this->header.pull(end_index);
        capacity--;
        return true;
    }
    return false;
}

void const_PD::print() {
    header.print();
    body.print();

}

bool const_PD::is_full() {
    return get_capacity() == 32;
}

size_t const_PD::get_capacity() const {
    return capacity;
}
