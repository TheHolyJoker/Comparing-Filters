//
// Created by tomer on 4/9/20.
//

#include "CPD_validator.hpp"

CPD_validator::CPD_validator(size_t quotient_range, size_t max_distinct_capacity, size_t remainder_length,
                             size_t counter_size)
        : d(quotient_range, max_distinct_capacity, remainder_length, counter_size),
          max_distinct_capacity(max_distinct_capacity), remainder_length(remainder_length), counter_size(counter_size),
          quotient_range(quotient_range),
          H_vec(max_distinct_capacity, 0), B_vec(), C_vec() {
}


CPD_validator::CPD_validator(size_t max_distinct_capacity, size_t remainder_length, size_t counter_size)
        : CPD_validator(42, max_distinct_capacity, remainder_length, counter_size) {
    cout << "CPD_validator 2nd constructor" << endl;
}


auto CPD_validator::lookup(CPD_TYPE q, CPD_TYPE r) -> bool {
//    assert(r > 0);
    assert(compare_all());

    bool att_res = d.lookup(q, r);
    bool valid_res = v_lookup(q, r);
    assert(compare_all());
    assert (valid_res == att_res);
    return valid_res;
}

auto CPD_validator::v_lookup(CPD_TYPE q, CPD_TYPE r) -> bool {
    size_t start, end;
    H_find(q, &start, &end);
    assert (start <= end);
    if (start == end)
        return false;

    size_t r_index = -1;
    return B_find(r, start, end, &r_index);

    /*size_t A_size, rel_bit_index;
    return B_find(r, start, end, &A_size, &rel_bit_index, &r_index);*/
}

auto CPD_validator::lookup_multi(CPD_TYPE q, CPD_TYPE r) -> size_t {
//    assert(r > 0);
    assert(compare_all());
    auto att_res = d.lookup_multi(q, r);
    auto valid_res = v_lookup_multi(q, r);
    assert(compare_all());
    assert (valid_res == att_res);
    return valid_res;
}

auto CPD_validator::v_lookup_multi(CPD_TYPE q, CPD_TYPE r) -> size_t {
    size_t start, end;
    H_find(q, &start, &end);
    assert (start <= end);
    if (start == end)
        return 0;

    size_t r_index = -1;
    if (B_find(r, start, end, &r_index)) {
        return C_vec[r_index];
    }
    return 0;

    /*size_t A_size, rel_bit_index;
    if (B_find(r, start, end, &A_size, &rel_bit_index, &r_index)) {*/
}

auto CPD_validator::insert(CPD_TYPE q, CPD_TYPE r) -> counter_status {
//    assert(r > 0);

    assert(compare_all());
    auto att_res = d.insert(q, r);
    auto valid_res = v_insert(q, r);
    assert(compare_all());
    assert (valid_res == att_res);
    return valid_res;
}

auto CPD_validator::v_insert(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    size_t start, end;
    H_find(q, &start, &end);
    assert (start <= end);

    /**New element easy case.*/
    if (start == end) {
        insert_new_element(q, r);
        return not_a_member;
    }

    size_t r_index = -1;
    /**Only counter update.*/
    if (B_find(r, start, end, &r_index)) {
//        print_vector(&C_vec);
        if (increase_counter(r_index) == inc_overflow) {
            assert(false);
        }
        return OK;
    }
    /**new element with existing quotient*/
    ++(H_vec[q]);
    B_vec.insert(B_vec.begin() + r_index, r);
    C_vec.insert(C_vec.begin() + r_index, 1);
    return not_a_member;
}

auto CPD_validator::insert_inc_attempt(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    assert(compare_all());
    auto att_res = d.insert_inc_attempt(q, r);
    auto valid_res = v_insert_inc_attempt(q, r);
    assert(compare_all());
    assert (valid_res == att_res);
    return valid_res;

}

auto CPD_validator::v_insert_inc_attempt(uint32_t q, uint32_t r) -> counter_status {
    size_t start, end;
    H_find(q, &start, &end);
    assert (start <= end);

    /**New element easy case.*/
    if (start == end) {
        return not_a_member;
    }

    size_t r_index = -1;
    /**Only counter update.*/
    if (B_find(r, start, end, &r_index)) {
//        print_vector(&C_vec);
        if (increase_counter(r_index) == inc_overflow) {
            //todo
            assert(false);
        }
        return OK;
    }
    return not_a_member;

}


void CPD_validator::insert_new_element(CPD_TYPE q, CPD_TYPE r) {
    size_t start, end;
    size_t prev_h_val = H_vec[q];
    H_find(q, &start, &end);
    ++(H_vec[q]);
    size_t new_h_val = H_vec[q];
    assert(prev_h_val + 1 == new_h_val);
    assert (start <= end);

    size_t r_index = -1;
    B_insert_new_element(r, start, end, &r_index);

    C_vec.insert(C_vec.begin() + r_index, 1);
}

void CPD_validator::insert_new_element_with_counter(CPD_TYPE q, CPD_TYPE r, CPD_TYPE counter) {
    size_t start, end;
    size_t prev_h_val = H_vec[q];
    H_find(q, &start, &end);
    ++(H_vec[q]);
    size_t new_h_val = H_vec[q];
    assert(prev_h_val + 1 == new_h_val);
    assert (start <= end);

    size_t r_index = -1;
    B_insert_new_element(r, start, end, &r_index);

    C_vec.insert(C_vec.begin() + r_index, counter);
}

void CPD_validator::remove(CPD_TYPE q, CPD_TYPE r) {
//    assert(r > 0);

    assert(compare_all());
    d.remove(q, r);
    v_remove(q, r);
    assert(compare_all());
}

auto CPD_validator::conditional_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    assert(compare_all());
    auto att_res = d.conditional_remove(q, r);
    auto valid_res = v_conditional_remove(q, r);
    assert(compare_all());
    assert (valid_res == att_res);
    return valid_res;
}

auto CPD_validator::v_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    size_t start, end;
    H_find(q, &start, &end);
    assert (start <= end);
    assert(start < end); // removing element not in the header.

    size_t r_index = -1;
    bool res = B_find(r, start, end, &r_index);
    assert(res);

    if (get_counter(r_index) > 1) {
        C_vec[r_index]--;
        return OK;
    } else {
        remove_element_completely(q, r_index);
        return dec_underflow;
    }

}

auto CPD_validator::v_conditional_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    if (v_lookup(q, r)) {
        return v_remove(q, r);
    }
    return not_a_member;
}

void CPD_validator::remove_element_completely(CPD_TYPE q, size_t unpacked_index) {
    H_vec[q]--;
    B_vec.erase(B_vec.begin() + unpacked_index);
    C_vec.erase(C_vec.begin() + unpacked_index);
}


auto CPD_validator::naive_comparison() -> bool {
    return false;
}

auto CPD_validator::compare_all() -> bool {
    bool res = compare_header();
    res &= compare_body();
    res &= compare_counters();

    return res;
}

auto CPD_validator::compare_header() -> bool {

    vector<bool> att_vec;
    vector<bool> valid_vec;
    size_t abs_bit_start = 0;
    size_t abs_bit_end = get_CPD_header_size_in_bits();
    auto value_vec = &H_vec;

    from_array_to_vector_by_bit_limits(&att_vec, d.get_a(), abs_bit_start, abs_bit_end);
    from_val_vector_to_bit_vector_representing_PD_header<CPD_TYPE>(value_vec, &valid_vec);

    auto current_valid_vec_size = sum_header_values();
    assert(current_valid_vec_size + max_distinct_capacity == valid_vec.size());

    size_t num_of_zeros_to_add = get_CPD_header_size_in_bits() - valid_vec.size();
    assert(num_of_zeros_to_add >= 0);
    valid_vec.insert(valid_vec.end(), num_of_zeros_to_add, false);

    bool res = (att_vec == valid_vec);
    if (res) return true;

    cout << "Headers" << endl;
    vector_differences_printer(&valid_vec, &att_vec);

    auto att_vec_size = att_vec.size();
    auto valid_vec_size = valid_vec.size();
    assert(att_vec_size == valid_vec_size);
    return false;
}

auto CPD_validator::compare_body() -> bool {
    vector<bool> att_vec;
    vector<bool> valid_vec;
    size_t abs_bit_start = get_CPD_header_size_in_bits();
    size_t abs_bit_end = abs_bit_start + get_CPD_body_size_in_bits();
    size_t word_size = remainder_length;

    vector<CPD_TYPE> value_vec(B_vec);
    size_t num_of_zeros_to_add = max_distinct_capacity - B_vec.size();
    assert(num_of_zeros_to_add >= 0);
    value_vec.insert(value_vec.end(), num_of_zeros_to_add, 0);

    from_array_to_vector_by_bit_limits(&att_vec, d.get_a(), abs_bit_start, abs_bit_end);
    from_val_vector_to_bit_vector_with_given_word_size(&value_vec, &valid_vec, word_size);


    bool res = att_vec == valid_vec;
    if (res) return true;

    cout << "Body1" << endl;
    vector_differences_printer(&valid_vec, &att_vec, remainder_length);
    auto att_vec_size = att_vec.size();
    auto valid_vec_size = valid_vec.size();
    assert(att_vec_size == valid_vec_size);

    return false;
}

auto CPD_validator::compare_counters() -> bool {
    vector<bool> att_vec;
    vector<bool> valid_vec;
    size_t abs_bit_start = get_CPD_header_size_in_bits() + get_CPD_body_size_in_bits();
    size_t abs_bit_end = abs_bit_start + get_CPD_counters_size_in_bits();
    size_t word_size = counter_size;
//    auto value_vec = &C_vec;

    vector<CPD_TYPE> value_vec(C_vec);
    size_t num_of_zeros_to_add = max_distinct_capacity - value_vec.size();
    assert(num_of_zeros_to_add >= 0);
    value_vec.insert(value_vec.end(), num_of_zeros_to_add, 0);


    from_array_to_vector_by_bit_limits(&att_vec, d.get_a(), abs_bit_start, abs_bit_end);
    from_val_vector_to_bit_vector_with_given_word_size(&value_vec, &valid_vec, word_size);


    bool res = att_vec == valid_vec;
    if (res) return true;


    cout << "Counters " << endl;
    vector_differences_printer(&valid_vec, &att_vec, counter_size);

    auto att_vec_size = att_vec.size();
    auto valid_vec_size = valid_vec.size();
    assert(att_vec_size == valid_vec_size);

    return false;
}

auto CPD_validator::H_insert(CPD_TYPE q, size_t *start, size_t *end) {
    return nullptr;
}


void CPD_validator::H_find(CPD_TYPE q, size_t *start, size_t *end) {
    size_t sum = 0;
    for (int i = 0; i < q; ++i) {
        sum += H_vec[i];
    }
    *start = sum;
    *end = sum + H_vec[q];
}

//auto CPD_validator::B_find(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *A_index,
//                           size_t *rel_bit_index, size_t *r_unpack_index) -> bool {
//    assert (false);
//    return true;
//}

auto CPD_validator::B_find(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index,
                           size_t *r_unpack_index) -> bool {
    for (int i = unpacked_start_index; i < unpacked_end_index; ++i) {
        assert (B_vec[i] <= MASK(remainder_length));
        if (B_vec[i] == r) {
            *r_unpack_index = i;
            return true;
        }
        if (B_vec[i] > r) {
            *r_unpack_index = i;
            return false;
        }
    }
    *r_unpack_index = unpacked_end_index;
    return false;
}

void
CPD_validator::B_insert_new_element(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index,
                                    size_t *r_unpack_index) {
    auto res = B_find(r, unpacked_start_index, unpacked_end_index, r_unpack_index);
    assert(!res);
    B_vec.insert(B_vec.begin() + *r_unpack_index, r);
}

auto CPD_validator::get_counter(size_t counter_index) -> CPD_TYPE {
    return C_vec.at(counter_index);
}

void CPD_validator::set_counter(size_t counter_index, size_t new_val) {
    C_vec.at(counter_index) = new_val;
}


auto CPD_validator::increase_counter(size_t counter_index) -> counter_status {
    assert(C_vec.at(counter_index) > 0);
    if (C_vec.at(counter_index) == MASK(counter_size)) {
        assert(false);
        return inc_overflow;
    }
    ++C_vec.at(counter_index);
    return OK;
}

auto CPD_validator::decrease_counter(size_t counter_index) -> counter_status {
    auto current_val = C_vec.at(counter_index);
    assert(current_val > 0);
    if (C_vec.at(counter_index) == 1) {
        return dec_underflow;
    }
    --C_vec.at(counter_index);
    return OK;
}


auto CPD_validator::get_CPD_header_size_in_bits() -> size_t {
    return max_distinct_capacity << 1u;
}

auto CPD_validator::get_CPD_body_size_in_bits() -> size_t {
    return max_distinct_capacity * remainder_length;
}

auto CPD_validator::get_CPD_counters_size_in_bits() -> size_t {
    return max_distinct_capacity * counter_size;
}

auto CPD_validator::is_empty() -> bool {
    return get_capacity() == 0;
}

auto CPD_validator::is_full() -> bool {
    return get_capacity() == max_distinct_capacity;
}

auto CPD_validator::get_capacity() -> size_t {
    return sum_header_values();
}

auto CPD_validator::sum_header_values() -> size_t {
    auto res = 0;
    for (unsigned int i : H_vec) {
        res += i;
    }
    return res;
}

void CPD_validator::get_elements(vector<vector<q_r>> *el_vec) {
    el_vec->resize(SL(counter_size) - 1);
    size_t remainder_counter = 0;
    for (int i = 0; i < H_vec.size(); ++i) {
        CPD_TYPE temp_el = B_vec[remainder_counter];
        auto temp_qr = std::make_tuple(temp_el, i);

        CPD_TYPE temp_counter = C_vec[remainder_counter];
        assert(temp_counter > 0);
        vector<q_r> temp_vec = el_vec->at(temp_counter);

        temp_vec.insert(temp_vec.end(), temp_qr);
    }
    /*for (int i = 0; i < max_distinct_capacity; ++i) {
        CPD_TYPE temp_el = B_vec[i];
        CPD_TYPE temp_counter = B_vec[i];
        auto temp_vec = el_vec->at(temp_counter);
        temp_vec.insert(temp_vec.end(), temp_el);
    }*/
}



//void CPD_validator::prepare_B_vec_for_comparison(vector<CPD_TYPE> *v) {
//    auto body_unpack_size =
//
//}
