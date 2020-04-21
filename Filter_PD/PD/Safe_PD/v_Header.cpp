//
// Created by tomer on 10/29/19.
//

#include "v_Header.h"

v_Header::v_Header(size_t m, size_t f, size_t l) : header(m, f, l), const_header() {
    size_t number_of_bits = ((m + f) << 1ULL) + 1;
    if (HEADER_BLOCK_SIZE != (8 * sizeof(HEADER_BLOCK_TYPE))) {
        assert(false);
    }
    this->vec.resize(number_of_bits);

}

bool v_Header::lookup(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    validate_get_interval(quotient);
    header.get_quotient_start_and_end_index(quotient, start_index, end_index);
    return *start_index != *end_index;
}

void v_Header::insert(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    validate_get_interval(quotient);
    vector_insert(quotient);
    header.insert(quotient, start_index, end_index);
    const_header.insert(quotient, start_index, end_index);
    validate_get_interval(quotient);
}

void v_Header::remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    validate_get_interval(quotient);
    vector_remove(quotient);
    header.remove(quotient, start_index, end_index);
    const_header.remove(quotient, start_index, end_index);
    validate_get_interval(quotient);
}

void v_Header::vector_get_interval(size_t quotient, size_t *start_index, size_t *end_index) {
    size_t zero_counter = -1, continue_from_index = 0;

    for (size_t i = 0; i < vec.size(); ++i) {
        if (zero_counter == quotient - 1) {
            *start_index = i;
            continue_from_index = i;
            break;
        }
        if (vec[i] == 0)
            zero_counter++;
    }

    for (size_t i = continue_from_index; i < vec.size(); ++i) {
        if (vec[i] == 0) {
            *end_index = i;
            return;
        }
    }
    assert(false);
}

void v_Header::vector_insert(size_t quotient) {
    size_t a = -1, b = -1;
    this->vector_get_interval(quotient, &a, &b);
    vec.insert(vec.begin() + a, true);
    vec.pop_back();
}

void v_Header::vector_remove(uint_fast16_t quotient) {
    size_t a = -1, b = -1;
    this->vector_get_interval(quotient, &a, &b);
    assert(a < b);
    vec.erase(vec.begin() + a);
    vec.push_back(false);
}


void v_Header::validate_get_interval(size_t quotient) {
    size_t s1 = -1, e1 = -1, s2 = -2, e2 = -2, s3 = -3, e3 = -3;
    vector_get_interval(quotient, &s1, &e1);
    get_interval_attempt(header.get_h(), header.get_size(), quotient, &s2, &e2);
    const_header.get_interval(quotient, &s3, &e3);
    bool cond = (s1 == s2 and s1 == s3) && (e1 == e2 and e1 == e3);
    if (not cond) {
        cout << s1 << ", " << e1 << endl;
        cout << s2 << ", " << e2 << endl;
        cout << s3 << ", " << e3 << endl;
        cout << "header as word is: ";
        header.print_as_word();
        cout << "vector as word is: ";
        print_bit_vector_as_words(&vec);
        cout << "const_header is: ";
        const_header.print();
    }
    assert(s3 == s1 and s2 == s1);
    assert(e3 == e1 and e2 == e1);
}

void v_Header::print() {
    header.print();
    const_header.print();
    print_bit_vector_as_words(&vec);
}
