//
// Created by tomer on 11/9/19.
//

#include "const_Header.h"

const_Header::const_Header() : w1(0), w2(0) {}

bool const_Header::lookup(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    get_interval(quotient, start_index, end_index);
    return *start_index < *end_index;
}

void const_Header::insert(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    if (DB) assert(get_capacity() < D_TYPE_SIZE);
    get_interval(quotient, start_index, end_index);
    push(*end_index);
}

void const_Header::remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    if (DB) assert(get_capacity() < D_TYPE_SIZE);
    get_interval(quotient, start_index, end_index);
    pull(*end_index);
}
/*

void const_Header::get_interval(size_t quotient, size_t *start_index, size_t *end_index) {
    if (quotient == 0) {
        *start_index = 0;
//        *end_index = bit_rank(~slot, 1);
        *end_index = __builtin_clz(~w1);
        if (*end_index == D_TYPE_SIZE) *end_index += __builtin_clz(~w2);
        return;
    }
    uint64_t slot = ((ulong) (w1) << 32ul) | w2;
    *start_index = bit_rank(~slot, quotient) + 1;
    *end_index = select_r(~slot, quotient + 1);
//    size_t i = *start_index / HEADER_BLOCK_SIZE, mask_bit = HEADER_BLOCK_SIZE - (*start_index % HEADER_BLOCK_SIZE);
//    *end_index = __builtin_clz(~a[i] & MASK(mask_bit));
}
*/

void const_Header::push(size_t end) {
    if (end < D_TYPE_SIZE) {
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong mask = MASK(left_bit_index);
        ulong upper = w1 & (~mask);
        ulong lower = ((w1 & (mask)) >> 1ul) | SL(left_bit_index - 1ul);
        ulong next = w1 & 1ul;

        w1 = upper | lower;
        w2 = (w2 >> 1ul) | (next << (D_TYPE_SIZE - 1ul));
    } else if (end == D_TYPE_SIZE) {
        w2 = (w2 >> 1ul) | (SL(D_TYPE_SIZE - 1ul));
    } else {
        end ^= 32ul;
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong mask = MASK(left_bit_index);
        ulong upper = w2 & (~mask);
        ulong lower = ((w2 & (mask)) >> 1ul) | SL(left_bit_index - 1ul);
        w2 = upper | lower;
    }
    /*
if (end < D_TYPE_SIZE) {
ulong left_bit_index = D_TYPE_SIZE - end;
ulong mask = MASK(left_bit_index);
ulong upper = w1 & (~mask);
ulong lower = ((w1 & (mask)) >> 1ul) | SL(left_bit_index);
ulong next = w1 & 1ul;

w1 = upper | lower;
w2 = (w2 >> 1ul) | (next << (D_TYPE_SIZE - 1ul));
} else if (end == D_TYPE_SIZE) {
w2 = (w2 >> 1ul) | (SL(D_TYPE_SIZE - 1ul));
} else {
ulong left_bit_index = D_TYPE_SIZE - end;
ulong mask = MASK(left_bit_index);
ulong upper = w2 & (~mask);
ulong lower = ((w2 & (mask)) >> 1ul) | SL(left_bit_index);
w2 = upper | lower;
}
*/
}

void const_Header::pull(size_t end) {
    if (end < D_TYPE_SIZE) {
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong upper = w1 & (~MASK(left_bit_index + 1));
        ulong mid = ((w1 & MASK(left_bit_index)) << 1ul);
        ulong lower = (w2 & SL(D_TYPE_SIZE - 1ul)) >> (D_TYPE_SIZE - 1ul);
        w1 = upper | mid | lower;
        w2 <<= 1ul;
    } else if (end == D_TYPE_SIZE) {
        w1 = (w1 | 1ul) ^ (1ul);
        w2 <<= 1ul;
    } else {
        end ^= 32ul;
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong upper = w2 & (~MASK(left_bit_index + 1));
        ulong mid = ((w2 & MASK(left_bit_index)) << 1ul);
        w2 = upper | mid;
    }
    /*if (end < D_TYPE_SIZE) {
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong mask = MASK(left_bit_index + 1);
        ulong upper = w1 & (~mask);
        ulong mid = ((w1 & (mask)) << 1ul);
        ulong lower = (w1 & SL(D_TYPE_SIZE - 1ul)) >> (D_TYPE_SIZE - 1ul);
        w1 = upper | mid | lower;
        w2 <<= 1ul;
    } else if (end == D_TYPE_SIZE) {
        w1 = (w1 | 1ul) ^ (1ul);
        w2 <<= 1ul;
    } else {
        end ^= 32ul;
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong mask = MASK(left_bit_index + 1);
        ulong upper = w2 & (~mask);
        ulong mid = ((w2 & (mask)) << 1ul);
        w2 = upper | mid;
    }

*/
}
/*
size_t const_Header::get_capacity() {
    return __builtin_popcount(w1) + __builtin_popcount(w2);
}*/

void const_Header::get_w1w2(uint32_t *p1, uint32_t *p2) {
    *p1 = w1;
    *p2 = w2;
}

void const_Header::print() {
    cout << "[" << w1 << ", " << w2 << "]" << endl;
}

void static_push(D_TYPE *w1, D_TYPE *w2, size_t end) {
    if (end < D_TYPE_SIZE) {
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong mask = MASK(left_bit_index);
        ulong upper = *w1 & (~mask);
        ulong lower = ((*w1 & (mask)) >> 1ul) | SL(left_bit_index - 1ul);
        ulong next = *w1 & 1ul;

        *w1 = upper | lower;
        *w2 = (*w2 >> 1ul) | (next << (D_TYPE_SIZE - 1ul));
//        cout << "h1" << endl;
    } else if (end == D_TYPE_SIZE) {
        *w2 = (*w2 >> 1ul) | (SL(D_TYPE_SIZE - 1ul));
//        cout << "h2" << endl;
    } else {
        end -= D_TYPE_SIZE;
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong mask = MASK(left_bit_index);
        ulong upper = *w2 & (~mask);
        ulong lower = ((*w2 & (mask)) >> 1ul) | SL(left_bit_index - 1ul);
        *w2 = upper | lower;
//        cout << "h3" << endl;
    }
}

void static_pull(D_TYPE *w1, D_TYPE *w2, size_t end) {
    if (end < D_TYPE_SIZE) {
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong upper = *w1 & (~MASK(left_bit_index + 1));
        ulong mid = ((*w1 & MASK(left_bit_index)) << 1ul);
        ulong lower = (*w2 & SL(D_TYPE_SIZE - 1ul)) >> (D_TYPE_SIZE - 1ul);
        *w1 = upper | mid | lower;
        *w2 <<= 1ul;
        cout << "h1" << endl;
    } else if (end == D_TYPE_SIZE) {
        *w1 = (*w1 | 1ul) ^ (1ul);
        *w2 <<= 1ul;
        cout << "h2" << endl;
    } else {
        end ^= 32ul;
        ulong left_bit_index = D_TYPE_SIZE - end;
        ulong upper = *w2 & (~MASK(left_bit_index + 1));
        ulong mid = ((*w2 & MASK(left_bit_index)) << 1ul);
        *w2 = upper | mid;
        cout << "h3" << endl;
    }
}