//
// Created by tomer on 10/29/19.
//

#include "v_Body.h"

v_Body::v_Body(size_t m, size_t f, size_t l) : body(m, f, l), const_body() {
    if (BODY_BLOCK_SIZE != (8 * sizeof(BODY_BLOCK_TYPE)))
        assert(false);
    this->vec.resize(f * l);

}

bool v_Body::lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    size_t B_index = 0, p_bit_index = 0;
    return vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index) == 1;
}

void v_Body::insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    vector_insert(abstract_body_start_index, abstract_body_end_index, remainder);
    body.insert(abstract_body_start_index, abstract_body_end_index, remainder);
    const_body.insert(abstract_body_start_index, abstract_body_end_index, remainder);
    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);

}

void v_Body::remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    vector_remove(abstract_body_start_index, abstract_body_end_index, remainder);
    body.remove(abstract_body_start_index, abstract_body_end_index, remainder);
    const_body.remove(abstract_body_start_index, abstract_body_end_index, remainder);
    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);


}

bool v_Body::vector_lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    return vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index) == 1;
}

void v_Body::vector_insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    vector_push(vector_bit_counter);
    write_FP_to_vector_by_index(&vec, vector_bit_counter, remainder, body.get_fp_size());

}

void v_Body::vector_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    if (res == 2) {
        cout << "vector - Trying to delete element with remainder that is not in the Body." << endl;
        return;
    }
    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    for (size_t i = vector_bit_counter; i < vec.size() - body.get_fp_size(); ++i) {
        vec[i] = vec[i + body.get_fp_size()];
    }
    for (size_t i = 0; i < body.get_fp_size(); ++i)
        vec[vec.size() - body.get_fp_size() + i] = false;

}

int v_Body::vector_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder,
                        size_t *p_B_index, size_t *p_bit_index) {

    size_t vector_start_index = abstract_body_start_index * body.get_fp_size();
    size_t vector_end_index = abstract_body_end_index * body.get_fp_size();
    size_t i = vector_start_index;
    for (; i < vector_end_index; i += body.get_fp_size()) {
        auto temp = read_FP_from_vector_by_index(&vec, i, body.get_fp_size());
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

void v_Body::validate_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
//    return;
    size_t s1 = -1, e1 = -1, s2 = -2, e2 = -2, s3 = -3, e3 = -3;
    auto r2 = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &s2, &e2);
    auto r1 = body.find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &s1, &e1);
    auto r3 = const_body.find(abstract_body_start_index, abstract_body_end_index, remainder, &s3);
    bool c1 = r1 == r2 and r1 == r3;
    bool c2 = c1 and (s1 == s2);// and s1 == s3);
    bool c3 = c2 and (e1 == e2);

    if (!c3) {
        cout << "Error start" << endl;

        printf("abst_start_index = %zu, abst_end_index = %zu\n", abstract_body_start_index, abstract_body_end_index);
        printf("actual result(r1) = %d2, expected result(r2) = %d2\n", r1, r2);
        printf("actual B_index(s1)= %zu, expected B_index(s2) = %zu\n", s1, s2);
        printf("actual bit_index(e1) = %zu, expected bit_index(e2) = %zu\n", e1, e2);

        cout << "vector print_with no spaces" << endl;
        print_bool_vector_no_spaces(&vec);
        cout << endl;
        print_array_as_consecutive_memory<BODY_BLOCK_TYPE>(body.get_b(), body.get_size(), cout);
        cout << "Above line is B as consecutive memory" << endl;

        cout << "print vector as words" << endl;
        print_bit_vector_as_words(&vec);
        cout << "naive_print B" << endl;
        body.naive_print();
        cout << "print vector by unpacking" << endl;
        print_vector_by_unpacking(&vec, body.get_fp_size());
        cout << "print B remainder consecutively" << endl;
        body.print_consecutive();

        cout << "\nPrints end" << endl;

        vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &s2, &e2);
        body.find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &s1, &e1);
        const_body.find(abstract_body_start_index, abstract_body_end_index, remainder, &s3);
        assert(false);
    }
}

void v_Body::vector_push(size_t vector_bit_counter) {
    for (size_t i = vec.size() - 1; i >= vector_bit_counter + body.get_fp_size(); --i)
        vec[i] = vec[i - body.get_fp_size()];
}

void v_Body::print() {
    body.print_consecutive();
    const_body.print();
    print_vector_by_unpacking(&vec, 8);

}

