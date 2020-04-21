//
// Created by tomer on 10/24/19.
//


#include "PD.h"

PD::PD(size_t m, size_t f, size_t l) : header(m, f, l), body(m, f, l), capacity(0), max_capacity(f),
                                       max_quotient(m) { if (DB) assert(m >= f); }

PD::~PD() = default;

//PD::~PD() {
//    delete(&header);
//    delete &body;
//}

bool PD::lookup(size_t quotient, FP_TYPE remainder) {
    if (DB) assert(quotient < max_quotient);
    size_t start_index = -1, end_index = -1;
    if (not header.lookup(quotient, &start_index, &end_index))
        return false;
    if (DB) assert(quotient <= start_index);
    if (DB) assert(start_index <= end_index);
//    end_index = max(start_index, end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    return body.wrap_lookup(body_start, body_end, remainder);
}


void PD::insert(size_t quotient, FP_TYPE remainder) {
    if (DB) assert(quotient < max_quotient);
    if (DB) assert(capacity < max_capacity);
    size_t start_index = -1, end_index = -1;
    this->header.insert(quotient, &start_index, &end_index);
//    end_index = max(start_index, end_index);
    if (DB) assert(quotient <= start_index);
    if (DB) assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;

    /*uint8_t body_start = start_index - quotient + 1;
    uint8_t body_end = end_index - quotient + 1;*/
    this->body.wrap_insert(body_start, body_end, remainder);
    capacity++;
}


void PD::remove(size_t quotient, FP_TYPE remainder) {
    if (DB) assert(quotient < max_quotient);
    size_t start_index = -1, end_index = -1;
    this->header.remove(quotient, &start_index, &end_index);
//    end_index = max(start_index, end_index);

    if (DB) assert(quotient <= start_index);
    if (DB) assert(start_index <= end_index);
    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    this->body.wrap_remove(body_start, body_end, remainder);
    capacity--;
}


bool PD::conditional_remove(size_t quotient, FP_TYPE remainder) {
    size_t start_index = -1, end_index = -1;
    if (not this->header.lookup(quotient, &start_index, &end_index))
        return false;

    assert(quotient <= start_index);
    assert(start_index <= end_index);

    size_t body_start = start_index - quotient;
    size_t body_end = end_index - quotient;
    if (this->body.conditional_remove(body_start, body_end, remainder)) {
        this->header.pull(quotient, start_index, end_index);
        capacity--;
        return true;
    }
    return false;
}


void PD::header_pp() {
    header.pretty_print();
}

void PD::print() {
    header.pretty_print();
//    body.print_consecutive_with_vector();
//    body.naive_print_with_vector();

}

uint8_t PD::get_body_abstract_start_index(size_t header_interval_start_index, size_t header_interval_end_index,
                                          size_t quotient) {
//    if (quotient == 0){
//        assert(header_interval_end_index == 0);
//        return 0;
//    }
    return header_interval_start_index - quotient;
}

uint8_t
PD::get_body_abstract_end_index(size_t header_interval_start_index, size_t header_interval_end_index, size_t quotient) {
    return header_interval_end_index - quotient;

}

size_t PD::get_capacity() const {
    return capacity;
}

bool PD::is_full() {
    return capacity == max_capacity;
}

ostream &operator<<(ostream &os, const PD &pd) {
    os << "header: " << pd.header;
    os << "body: " << pd.body;
    return os;
}



