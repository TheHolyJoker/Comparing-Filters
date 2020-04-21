//
// Created by tomer on 11/25/19.
//

#include "cg_naive_PD.h"

cg_naive_PD::cg_naive_PD(size_t m, size_t f, size_t l) : d(m, f, l) {}

bool cg_naive_PD::lookup(size_t quotient, FP_TYPE remainder) {
    return d.lookup(quotient, remainder);
}

void cg_naive_PD::insert(size_t quotient, FP_TYPE remainder) {
    d.insert(quotient, remainder);
}

void cg_naive_PD::remove(size_t quotient, FP_TYPE remainder) {
    d.remove(quotient, remainder);
}

bool cg_naive_PD::conditional_remove(size_t quotient, FP_TYPE remainder) {
    return d.conditional_remove(quotient, remainder);
}

bool cg_naive_PD::is_full() {
    return d.is_full();
}

size_t cg_naive_PD::get_capacity() {
    return d.get_capacity();
}
