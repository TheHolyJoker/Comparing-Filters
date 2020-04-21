//
// Created by tomer on 3/13/20.
//

#include "CPD.hpp"

CPD::CPD(size_t m, size_t f, size_t l, size_t counter_size) :
        max_distinct_capacity(f), fp_size(l), counter_size(counter_size),
        size(get_a_size(m, f, l, counter_size)),
        header_and_body_has_joined_slot(do_header_and_body_has_joined_slot(m, f, l)),
        body_and_counters_has_joined_slot(do_body_and_counters_has_joined_slot(m, f, l, counter_size)) {
//    static_assert(CPD_TYPE_SIZE == 32, "");
    assert(counter_size <= CPD_TYPE_SIZE);
    a = new CPD_TYPE[size]();
    for (size_t i = 0; i < size; ++i) {
        a[i] = 0;
    }

}

auto CPD::lookup(CPD_TYPE q, CPD_TYPE r) -> bool {
    size_t start_index = -1, end_index = -1;
    if (not header_lookup(q, &start_index, &end_index))
        return false;

    if (CPD_DB_MODE0) assert(q <= start_index <= end_index);

    size_t p_array_index = -1, p_bit_index = -1;
    return body_find_wrapper(r, start_index - q, end_index - q, &p_array_index, &p_bit_index);

}

auto CPD::lookup_multi(CPD_TYPE q, CPD_TYPE r) -> size_t {
    size_t start_index = -1, end_index = -1;
    if (not header_lookup(q, &start_index, &end_index))
        return 0;

    if (CPD_DB_MODE0) assert(q <= start_index <= end_index);

    size_t A_index = -1, rel_bit_index = -1;
    if (!body_find_wrapper(r, start_index - q, end_index - q, &A_index, &rel_bit_index))
        return 0;

    auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
    if (CPD_DB_MODE1) {
        auto res = read_counter(counter_index);
        assert(res <= MASK(counter_size));
    }
    return read_counter(counter_index);
}

//void CPD::insert(CPD_TYPE q, CPD_TYPE r) {
//    if (DB) assert(get_capacity() <= max_distinct_capacity);
//    size_t start_index = -1, end_index = -1;
//    header_insert(q, &start_index, &end_index);
//
//    if (DB) assert((q <= start_index) and (start_index <= end_index));
//
//    size_t unpacked_start_index = start_index - q;
//    size_t unpacked_end_index = end_index - q;
//
//    body_insert(r, unpacked_start_index, unpacked_end_index);
//}

auto CPD::insert(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    size_t start_index, end_index;
    header_find(q, &start_index, &end_index);

    size_t A_index = -1, rel_bit_index = -1;
    bool in_body = body_find_wrapper(r, start_index - q, end_index - q, &A_index, &rel_bit_index);

    if (!in_body)
        return insert_new_helper(r, end_index, A_index, rel_bit_index);
    else
        return insert_inc_helper(r, end_index, A_index, rel_bit_index);

    /*
    //new element (easy case).
    if (start_index == end_index) {
//        return insert_new_helper(r, end_index, A_index, rel_bit_index);

        header_push_wrapper(end_index);

        if (DB) assert((q <= start_index) and (start_index <= end_index));

        size_t A_index = -1, rel_bit_index = -1;
        body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &rel_bit_index);
        body_push_wrapper(r, A_index, rel_bit_index);
        auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
        counter_push(1, translate_counter_index_to_abs_bit_index(counter_index));
        assert(read_counter(counter_index));
//        cout << "I1" << endl;
        return not_a_member;
    }
    size_t A_index = -1, rel_bit_index = -1;
    //Only increment is needed.
    if (body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &rel_bit_index)) {
        auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
        if (increase_counter(counter_index) == inc_overflow) {
            //todo delete the element, and move it to the next memory level.
//            cout << "I2" << endl;
            assert(false);
            return inc_overflow;
        }
        assert(read_counter(counter_index));
//        cout << "I3" << endl;
        return OK;
    }
        //Insert new element
    else {
        return insert_new_helper(r, end_index, A_index, rel_bit_index);
        *//**header_push_wrapper(end_index);
        body_push_wrapper(r, A_index, rel_bit_index);
        auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
        counter_push(1, translate_counter_index_to_abs_bit_index(counter_index));
        assert(read_counter(counter_index));
//        cout << "I4" << endl;
        return not_a_member;**//*
    }*/
}

auto CPD::insert_inc_helper(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index) -> counter_status {
    auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
    if (increase_counter(counter_index) == inc_overflow) {
        insert_overflow_handler(r, end_index, A_index, rel_bit_index);
        assert(false);
        return inc_overflow;
    }
    if (CPD_DB_MODE1) {
        assert(read_counter(counter_index));
    }

    return OK;
}

auto CPD::insert_new_helper(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index) -> counter_status {
    header_push_wrapper(end_index);
    body_push_wrapper(r, A_index, rel_bit_index);
    auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
    counter_push(1, translate_counter_index_to_abs_bit_index(counter_index));
    assert(!CPD_DB_MODE1 or read_counter(counter_index));
    return not_a_member;
}

auto CPD::insert_new_helper(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index,
                            CPD_TYPE counter) -> counter_status {
    header_push_wrapper(end_index);
    body_push_wrapper(r, A_index, rel_bit_index);
    auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
    counter_push(counter, translate_counter_index_to_abs_bit_index(counter_index));
    assert(!CPD_DB_MODE1 or read_counter(counter_index));
    return not_a_member;
}

void CPD::insert_new_element_with_counter(CPD_TYPE q, CPD_TYPE r, CPD_TYPE counter) {
    assert(!CPD_DB_MODE1 or (counter <= MASK(counter_size)));
    size_t start_index, end_index;
    header_find(q, &start_index, &end_index);

    size_t A_index = -1, rel_bit_index = -1;
    bool in_body = body_find_wrapper(r, start_index - q, end_index - q, &A_index, &rel_bit_index);
    assert(!CPD_DB_MODE1 or !in_body);
//    if (!in_body)
    insert_new_helper(r, end_index, A_index, rel_bit_index, counter);
}

auto CPD::insert_overflow_handler(CPD_TYPE r, size_t end_index, size_t A_index, size_t rel_bit_index) -> void {
    //todo delete the element, and move it to the next memory level.
    // cout << "I2" << endl;
    assert (false);
}

/*
void CPD::insert_new(CPD_TYPE r, size_t end_index) {

}
*/


auto CPD::insert_inc_attempt(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    size_t start_index, end_index;
    header_find(q, &start_index, &end_index);

    size_t A_index = -1, rel_bit_index = -1;
    bool in_body = body_find_wrapper(r, start_index - q, end_index - q, &A_index, &rel_bit_index);

    if (!in_body)
        return not_a_member;

    return insert_inc_helper(r, end_index, A_index, rel_bit_index);
}


void CPD::remove_old(CPD_TYPE q, CPD_TYPE r) {
    size_t start_index = -1, end_index = -1;
    header_remove(q, &start_index, &end_index);

    if (CPD_DB_MODE0) assert(q <= start_index <= end_index);

    body_remove(r, start_index - q, end_index - q);
}

void CPD::remove(CPD_TYPE q, CPD_TYPE r) {
    size_t start_index = -1, end_index = -1;
    header_find(q, &start_index, &end_index);

    assert(start_index < end_index);

    size_t unpacked_start_index = start_index - q;
    size_t unpacked_end_index = end_index - q;

    size_t A_index = -1, rel_bit_index = -1;
    auto res = body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &rel_bit_index);
    if (not res) {
        body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &rel_bit_index);
        cout << "trying to remove element not in the PD. "
                "Specifically not in the Body (might be in the header). " << endl;
        assert(false);
    }
    auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
    if (decrease_counter(counter_index) == dec_underflow) {
        header_pull_wrapper(end_index);
        body_pull_wrapper(A_index, rel_bit_index);
        counter_pull(translate_counter_index_to_abs_bit_index(counter_index));
        /*cout << "before header pull:" << endl;
        print_counters_as_consecutive_memory();
        header_pull_wrapper(end_index);

        cout << "after header pull:" << endl;
        print_counters_as_consecutive_memory();
        body_pull_wrapper(A_index, rel_bit_index);

        cout << "after body pull:" << endl;
        print_counters_as_consecutive_memory();
        counter_pull(translate_counter_index_to_abs_bit_index(counter_index));

         cout << "after counter pull" << endl;
        print_counters_as_consecutive_memory();*/
    }

}

auto CPD::conditional_remove(CPD_TYPE q, CPD_TYPE r) -> counter_status {
    size_t start_index = -1, end_index = -1;
    if (not header_lookup(q, &start_index, &end_index))
        return not_a_member;

    size_t A_index = -1, rel_bit_index = -1;
    auto res = body_find_wrapper(r, start_index - q, end_index - q, &A_index, &rel_bit_index);
    if (res) {
        auto counter_index = translate_to_unpacked_index(A_index, rel_bit_index);
        if (decrease_counter(counter_index) == dec_underflow) {
            header_pull_wrapper(end_index);
            body_pull_wrapper(A_index, rel_bit_index);
            counter_pull(translate_counter_index_to_abs_bit_index(counter_index));
            return dec_underflow;
        }
        return OK;
    }
    return not_a_member;


}

auto CPD::conditional_remove_old(CPD_TYPE q, CPD_TYPE r) -> bool {
//    return naive_conditional_remove(q, r);
    size_t start_index = -1, end_index = -1;
    if (not header_lookup(q, &start_index, &end_index))
        return false;

    if (CPD_DB_MODE1) {
        assert(q <= start_index);
        assert(start_index <= end_index);
    }

    if (body_conditional_remove(r, start_index - q, end_index - q)) {
        header_pull(end_index);
        return true;
    }
    return false;
}

auto CPD::naive_conditional_remove(CPD_TYPE q, CPD_TYPE r) -> bool {
    bool res = lookup(q, r);
    if (res) { remove(q, r); }
    return res;
}


////Header functions
auto CPD::header_lookup(CPD_TYPE q, size_t *start_index, size_t *end_index) -> bool {
    header_find(q, start_index, end_index);
    if (CPD_DB_MODE0) assert(*start_index <= *end_index);
    return (*start_index != *end_index);
}

void CPD::header_find(CPD_TYPE q, size_t *start, size_t *end) {
    if (q == 0) {
        *start = 0;
        size_t j = 0;
        while (a[j] == MASK32) j++;
        *end = (j) * CPD_TYPE_SIZE + __builtin_clz(~a[j]);
        return;
    }
    for (size_t i = 0; i <= get_last_a_index_containing_the_header(); ++i) {
        auto cz = __builtin_popcount(~a[i]);
        if (cz < q) q -= cz;
        else if (cz == q) {
            uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, q);
            if (DB) assert(bit_pos < CPD_TYPE_SIZE);
            *start = (i + (bit_pos + 1 == CPD_TYPE_SIZE)) * CPD_TYPE_SIZE + (bit_pos + 1) % CPD_TYPE_SIZE;
            size_t j = i + 1;
            while (a[j] == MASK32) j++;
            *end = (j) * CPD_TYPE_SIZE + __builtin_clz(~a[j]);
            return;
        } else {
            if (DB) assert(q < CPD_TYPE_SIZE);
            uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, q);
            if (DB) assert(bit_pos < CPD_TYPE_SIZE);
            *start = i * CPD_TYPE_SIZE + select_r(~slot, q) + 1;
            *end = i * CPD_TYPE_SIZE + select_r(~slot, q + 1);
            return;

        }
    }
    assert(false);
}

void CPD::header_insert(CPD_TYPE q, size_t *start_index, size_t *end_index) {
    header_find(q, start_index, end_index);
    header_push_wrapper(*end_index);
}

void CPD::header_push_wrapper(size_t end) {

    size_t temp_index;
    CPD_TYPE temp_slot;
    if (header_and_body_has_joined_slot) {
        temp_index = get_last_a_index_containing_the_header();
        temp_slot = a[temp_index];
    }

    header_push(end);

    /*Restore body's part in the joined cell*/
    if (header_and_body_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_header_rel_bit_index_in_last_header_slot();
        a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
    }
}

void CPD::header_push(size_t end_index) {
    size_t index, last_header_a_slot = get_last_a_index_containing_the_header();
    index = end_index / CPD_TYPE_SIZE;

    ulong to_shift = CPD_TYPE_SIZE - 1ul;
    for (uint_fast16_t i = last_header_a_slot; i > index; --i) {
        a[i] = ((ulong) (a[i]) >> 1ul) | ((a[i - 1] & 1ul) << to_shift);
    }


    uint_fast16_t bit_index = end_index % CPD_TYPE_SIZE;
    uint_fast16_t shift = CPD_TYPE_SIZE - bit_index;
    ulong mask = MASK(shift);
    ulong upper = a[index] & (~mask);
    CPD_TYPE lower = (a[index] >> 1ul) & mask;
    a[index] = (upper | lower | SL(shift - 1ul));

}

void CPD::header_remove(CPD_TYPE q, size_t *start_index, size_t *end_index) {
    header_find(q, start_index, end_index);
    if (DB) {
        if (*start_index == *end_index) {
            cout << "trying to remove element not in the PD. Specifically not in the header. " << endl;
            return;
        }
    }
    header_pull_wrapper(*end_index);
}

void CPD::header_pull_wrapper(size_t end) {
    size_t temp_index;
    CPD_TYPE temp_slot;
    if (header_and_body_has_joined_slot) {
        temp_index = get_last_a_index_containing_the_header();
        temp_slot = a[temp_index];
    }

    header_pull(end);

    /*Restore body's part in the joined cell*/
    if (header_and_body_has_joined_slot) {
        //Plus one to insure that body's first bit did not effect header last bit.
        //This bit (header last bit) should always be zero.
        auto mask_bit = CPD_TYPE_SIZE - get_header_rel_bit_index_in_last_header_slot() + 1;
        a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
    }

}

void CPD::header_pull(size_t end_index) {
    size_t index = (end_index - 1) / CPD_TYPE_SIZE, last_header_a_slot = get_last_a_index_containing_the_header();

    if (index == last_header_a_slot) {
        uint_fast16_t bit_index = (end_index - 1) % CPD_TYPE_SIZE;
        uint_fast16_t shift = CPD_TYPE_SIZE - bit_index;
        ulong mask = MASK(shift);
        ulong upper = a[index] & (~mask);
        CPD_TYPE mid = ((ulong) a[index] << 1ul) & mask;
        a[index] = upper | mid;
        return;
    }
    CPD_TYPE lower = ((ulong) a[index + 1]) >> ((ulong) (CPD_TYPE_SIZE - 1));

    for (size_t i = index + 1; i < last_header_a_slot; ++i) {
        a[i] = ((ulong) a[i] << 1ul) | ((ulong) a[i + 1] >> ((ulong) (CPD_TYPE_SIZE - 1)));
    }
    a[last_header_a_slot] <<= 1ul;

    uint_fast16_t bit_index = (end_index - 1) % CPD_TYPE_SIZE;
    uint_fast16_t shift = CPD_TYPE_SIZE - bit_index;
    ulong mask = MASK(shift);
    ulong upper = a[index] & (~mask);
    CPD_TYPE mid = ((ulong) a[index] << 1ul) & mask;
    a[index] = upper | mid | lower;// | SL(shift);

}

////Body functions

auto CPD::body_find_wrapper(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                            size_t *p_bit_index) -> bool {
    auto res = body_find(r, unpacked_start_index, unpacked_end_index, p_array_index, p_bit_index);
    if (res) {
        auto counter_index = translate_to_unpacked_index(*p_array_index, *p_bit_index);
        assert(!CPD_DB_MODE1 or read_counter(counter_index));
    }
    return res;
}

auto CPD::body_find(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                    size_t *p_bit_index) -> bool {
    size_t total_bit_counter = (unpacked_start_index * fp_size) + get_header_size_in_bits();

    /*Empty case. */
    if (unpacked_start_index == unpacked_end_index) {
        *p_array_index = total_bit_counter / CPD_TYPE_SIZE;
        *p_bit_index = total_bit_counter % CPD_TYPE_SIZE;
        if (DB_PRINT) cout << "a1" << endl;
        return false;
    }

    size_t comparing_counter = 0, comparing_lim = unpacked_end_index - unpacked_start_index;

    for (; comparing_counter < comparing_lim; ++comparing_counter) {
        size_t B_index = total_bit_counter / CPD_TYPE_SIZE;
        size_t bit_index_inside_slot = total_bit_counter % CPD_TYPE_SIZE;
        size_t bits_left_to_read_inside_slot = CPD_TYPE_SIZE - bit_index_inside_slot;

        CPD_TYPE current_cell = a[B_index];
        /*More than fp_size bits remain in B[B_index].*/
        if (bits_left_to_read_inside_slot > fp_size) {
            ulong shift = bits_left_to_read_inside_slot - fp_size;
            if (DB) assert(shift < CPD_TYPE_SIZE);
            CPD_TYPE current_remainder = (current_cell >> (shift)) & MASK(fp_size);
            if (r <= current_remainder) {
                if (DB_PRINT) cout << "a2" << endl;
                body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                return r == current_remainder;
            }
            /*Exactly fp_size bits remain in B[B_index].*/
        } else if (bits_left_to_read_inside_slot == fp_size) {
            CPD_TYPE current_remainder = current_cell & MASK(fp_size);
            if (r <= current_remainder) {
                if (DB_PRINT) cout << "a3" << endl;
                body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                return r == current_remainder;
            }
            /*Less than fp_size bits remain in B[B_index].*/
        } else {
            size_t number_of_bits_to_read_from_next_slot = fp_size - bits_left_to_read_inside_slot;
            ulong upper_shift = fp_size - bits_left_to_read_inside_slot;
            if (CPD_DB_MODE0) assert(upper_shift >= 0 and upper_shift < CPD_TYPE_SIZE);
            ulong upper = (current_cell & MASK(bits_left_to_read_inside_slot)) << (upper_shift);
            if (CPD_DB_MODE0) assert(get_last_a_index_containing_the_body() >= B_index + 1);
            ulong lower_shift = CPD_TYPE_SIZE - number_of_bits_to_read_from_next_slot;
            if (CPD_DB_MODE0) assert(0 <= lower_shift and lower_shift < CPD_TYPE_SIZE);
            ulong lower = (a[B_index + 1] >> lower_shift) & MASK(number_of_bits_to_read_from_next_slot);
            CPD_TYPE current_remainder = upper | lower;
            if (r <= current_remainder) {
                if (DB_PRINT) cout << "a4" << endl;
                body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                return r == current_remainder;
            }
        }
        total_bit_counter += fp_size;
    }
    *p_array_index = total_bit_counter / CPD_TYPE_SIZE;
    *p_bit_index = total_bit_counter % CPD_TYPE_SIZE;
    if (DB_PRINT) cout << "a5" << endl;
    return false;

}

void CPD::body_insert(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) {
    assert (false);
    size_t A_index = -1, bit_index = -1;
    body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &bit_index);
    body_push(r, A_index, bit_index);

}

void CPD::body_push_wrapper(CPD_TYPE r, size_t A_index, size_t rel_bit_index) {
    size_t temp_index;
    CPD_TYPE temp_slot;
    if (body_and_counters_has_joined_slot) {
        temp_index = get_last_a_index_containing_the_body();
        temp_slot = a[temp_index];
    }

    body_push(r, A_index, rel_bit_index);

    /*Restore body's part in the joined cell*/
    if (body_and_counters_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_body_rel_bit_index_in_last_body_slot();
        a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
    }
}

void CPD::body_push(CPD_TYPE r, size_t A_index, size_t rel_bit_index) {
    size_t left_bit_index = CPD_TYPE_SIZE - rel_bit_index;
    for (size_t i = get_last_a_index_containing_the_body(); i > A_index; --i) {
        a[i] = (a[i] >> fp_size) | ((a[i - 1]) << (CPD_TYPE_SIZE - fp_size));
    }

    if (CPD_TYPE_SIZE >= fp_size + rel_bit_index) {
        ulong mask = MASK(left_bit_index);
        ulong upper = a[A_index] & (~mask);
        CPD_TYPE mid = r << (left_bit_index - fp_size);
        CPD_TYPE lower = (a[A_index] >> fp_size) & (MASK(left_bit_index - fp_size));
        a[A_index] = (upper | lower | mid);
        if (CPD_DB_MODE0) assert(left_bit_index - fp_size >= 0);
    } else { // Prevents negative shifting.
        size_t shift = left_bit_index;
        ulong mask = MASK(left_bit_index);
        ulong upper = a[A_index] & (~mask);
        CPD_TYPE lower = r >> (fp_size - left_bit_index);
        a[A_index] = (upper | lower);

        if (CPD_DB_MODE0) assert(fp_size - shift > 0);

        //same amount that r was shifted right by. (fp_size - shift)
        size_t bits_left = fp_size - shift; // bits_left =  fp_size + bit_index - CPD_TYPE_SIZE
        upper = (r & MASK(bits_left)) << (CPD_TYPE_SIZE - bits_left); // todo Check this.
        lower = (a[A_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits. todo try mask instead.
        a[A_index + 1] = (upper | lower);
    }
}

void CPD::body_push_old(CPD_TYPE r, size_t B_index, size_t bit_index) {
    assert(false);// old function
    size_t left_bit_index = CPD_TYPE_SIZE - bit_index;
    for (size_t i = get_body_last_index(); i > B_index; --i) {
        a[i] = (a[i] >> fp_size) | ((a[i - 1]) << (CPD_TYPE_SIZE - fp_size));
    }

    if (CPD_TYPE_SIZE >= fp_size + bit_index) {
        ulong mask = MASK(left_bit_index);
        ulong upper = a[B_index] & (~mask);
        CPD_TYPE mid = r << (left_bit_index - fp_size);
        CPD_TYPE lower = (a[B_index] >> fp_size) & (MASK(left_bit_index - fp_size));
        a[B_index] = (upper | lower | mid);
        if (DB) assert(left_bit_index - fp_size >= 0);
    } else { // Prevents negative shifting.
        size_t shift = left_bit_index;
        ulong mask = MASK(left_bit_index);
        ulong upper = a[B_index] & (~mask);
        CPD_TYPE lower = r >> (fp_size - left_bit_index);
        a[B_index] = (upper | lower);

        if (DB) assert(fp_size - shift > 0);

        //same amount that r was shifted right by. (fp_size - shift)
        size_t bits_left = fp_size - shift; // bits_left =  fp_size + bit_index - CPD_TYPE_SIZE
        upper = (r & MASK(bits_left)) << (CPD_TYPE_SIZE - bits_left); // todo Check this.
        lower = (a[B_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits. todo try mask instead.
        a[B_index + 1] = (upper | lower);
    }
}


void CPD::body_remove(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) {
    size_t A_index = -1, rel_bit_index = -1;
    auto res = body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &rel_bit_index);
    if (not res) {
        body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &A_index, &rel_bit_index);
        cout << "trying to remove element not in the PD. "
                "Specifically not in the Body (might be in the header). " << endl;
        return;
    }
    body_pull_wrapper(A_index, rel_bit_index);

}

void CPD::body_pull_wrapper(size_t A_index, size_t rel_bit_index) {
    size_t body_first_A_index;
    size_t body_last_A_index;
    CPD_TYPE first_slot;
    CPD_TYPE last_slot;
    /*if (header_and_body_has_joined_slot) {
        body_first_A_index = get_last_a_index_containing_the_header();
        first_slot = a[body_first_A_index];
    }*/
    if (body_and_counters_has_joined_slot) {
        body_last_A_index = get_last_a_index_containing_the_body();
        last_slot = a[body_last_A_index];
    }

    body_pull(A_index, rel_bit_index);

    /*if (header_and_body_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_header_rel_bit_index_in_last_header_slot();
        auto mask = MASK(mask_bit);
//        auto old_val = a[body_first_A_index];
//        auto new_val = (a[body_first_A_index] & (~mask)) | (first_slot & (mask));
//        assert(old_val == new_val);
        a[body_first_A_index] = (a[body_first_A_index] & (~mask)) | (first_slot & (mask));
    }*/
    if (body_and_counters_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_body_rel_bit_index_in_last_body_slot();
        a[body_last_A_index] = (a[body_last_A_index] & (~MASK(mask_bit))) | (last_slot & (MASK(mask_bit)));
    }
}

/*
void CPD::body_remove_helper(size_t A_index, size_t rel_bit_index) {

    //TODO need to fix possibly 2 slots (start for header, and end for counters).
    size_t body_first_A_index;
    size_t body_last_A_index;
    CPD_TYPE first_slot;
    CPD_TYPE last_slot;
    if (header_and_body_has_joined_slot) {
        body_first_A_index = get_header_and_body_joined_slot_index();
        first_slot = a[body_first_A_index];
    }
    if (body_and_counters_has_joined_slot) {
        body_last_A_index = get_body_and_counters_joined_slot_index();
        last_slot = a[body_last_A_index];
    }

    body_pull(A_index, rel_bit_index);

    if (header_and_body_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_header_rel_bit_index_in_last_header_slot();
        a[body_first_A_index] = (a[body_first_A_index] & (MASK(mask_bit))) | (first_slot & (~MASK(mask_bit)));
    }
    if (body_and_counters_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_body_rel_bit_index_in_last_body_slot();
        a[body_last_A_index] = (a[body_last_A_index] & (MASK(mask_bit))) | (last_slot & (~MASK(mask_bit)));
    }
}
*/

auto CPD::body_conditional_remove(CPD_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) -> bool {
    assert(false); // old function

    size_t temp_index;
    CPD_TYPE temp_slot;
    if (header_and_body_has_joined_slot) {
        temp_index = get_last_a_index_containing_the_header();
        temp_slot = a[temp_index];
    }

    size_t B_index = -1, bit_index = -1;
    auto res = body_find_wrapper(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
    if (not res) {
        return false;
    }
    body_pull(B_index, bit_index);

    if (header_and_body_has_joined_slot) {
        auto mask_bit = CPD_TYPE_SIZE - get_header_rel_bit_index_in_last_header_slot();
        a[temp_index] = (a[temp_index] & (MASK(mask_bit))) | (temp_slot & (~MASK(mask_bit)));
    }
    return true;
}

void CPD::body_pull(size_t B_index, size_t bit_index) {
    size_t left_fp_start_index = CPD_TYPE_SIZE - bit_index;
    auto body_last_index = get_last_a_index_containing_the_body();
    if (B_index == body_last_index) {
        assert(bit_index + fp_size <= CPD_TYPE_SIZE);

        if (CPD_DB_MODE0) assert(left_fp_start_index >= 0);
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[B_index] & (~mask);
        CPD_TYPE mid = (a[B_index] & MASK(left_fp_start_index - fp_size)) << fp_size;
        if (CPD_DB_MODE0) assert(left_fp_start_index >= fp_size);
        a[B_index] = (upper | mid);
        return;
    }

    if (CPD_TYPE_SIZE >= fp_size + bit_index) {
        ulong shift = left_fp_start_index;
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[B_index] & (~mask);
        CPD_TYPE mid = (a[B_index] & MASK(left_fp_start_index - fp_size)) << fp_size;
        CPD_TYPE lower = (a[B_index + 1]) >> (CPD_TYPE_SIZE - fp_size);
        a[B_index] = (upper | lower | mid);

        if (CPD_DB_MODE0) assert(shift >= fp_size);
        if (CPD_DB_MODE0) assert(CPD_TYPE_SIZE - fp_size >= 0);

        for (size_t i = B_index + 1; i < body_last_index; ++i) {
            a[i] = (a[i] << fp_size) | (a[i + 1] >> (CPD_TYPE_SIZE - fp_size));
        }
        a[body_last_index] <<= fp_size;

    } else {
        ulong shift = left_fp_start_index;
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[B_index] & (~mask);
        ulong lower_shift = CPD_TYPE_SIZE - fp_size;
        CPD_TYPE lower = a[B_index + 1] >> lower_shift;
        lower &= MASK(left_fp_start_index);
        a[B_index] = upper | lower;

        for (size_t i = B_index + 1; i < body_last_index; ++i) {
            a[i] = (a[i] << fp_size) | (a[i + 1] >> (CPD_TYPE_SIZE - fp_size));
        }
        a[body_last_index] <<= fp_size;

        if (CPD_DB_MODE0) assert(0 <= shift and shift < fp_size);
        if (CPD_DB_MODE0) assert(0 <= lower_shift and lower_shift < CPD_TYPE_SIZE);
    }

}

void CPD::counter_push(CPD_TYPE val, size_t bit_count_from_start) {
    size_t A_index = bit_count_from_start / CPD_TYPE_SIZE;
    size_t bit_index = bit_count_from_start % CPD_TYPE_SIZE;
    size_t left_bit_index = CPD_TYPE_SIZE - bit_index;
    for (size_t i = size - 1; i > A_index; --i) {
        a[i] = (a[i] >> counter_size) | ((a[i - 1]) << (CPD_TYPE_SIZE - counter_size));
    }

    if (CPD_TYPE_SIZE >= counter_size + bit_index) {
        ulong mask = MASK(left_bit_index);
        ulong upper = a[A_index] & (~mask);
        CPD_TYPE mid = val << (left_bit_index - counter_size);
        CPD_TYPE lower = (a[A_index] >> counter_size) & (MASK(left_bit_index - counter_size));
        a[A_index] = (upper | lower | mid);
        if (CPD_DB_MODE0) assert(left_bit_index - counter_size >= 0);
    } else { // Prevents negative shifting.
        size_t shift = left_bit_index;
        ulong mask = MASK(left_bit_index);
        ulong upper = a[A_index] & (~mask);
        CPD_TYPE lower = val >> (counter_size - left_bit_index);
        a[A_index] = (upper | lower);

        if (CPD_DB_MODE0) assert(counter_size - shift > 0);

        //same amount that val was shifted right by. (fp_size - shift)
        size_t bits_left = counter_size - shift; // bits_left =  fp_size + bit_index - CPD_TYPE_SIZE
        upper = (val & MASK(bits_left)) << (CPD_TYPE_SIZE - bits_left); // todo Check this.
        lower = (a[A_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits. todo try mask instead.
        a[A_index + 1] = (upper | lower);
    }
}


//void CPD::counter_pull_wrapper(size_t bit_count_from_start) {
//
//
//}

void CPD::counter_pull(size_t bit_count_from_start) {
    size_t A_index = bit_count_from_start / CPD_TYPE_SIZE;
    size_t bit_index = bit_count_from_start % CPD_TYPE_SIZE;
    size_t left_fp_start_index = CPD_TYPE_SIZE - bit_index;
//    cout << "counter_pull:: a[A_index]: " << a[A_index] << endl;

    if (A_index == size - 1) {
        assert(bit_index + counter_size <= CPD_TYPE_SIZE);

        if (CPD_DB_MODE0) assert(left_fp_start_index >= 0);
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[A_index] & (~mask);
        CPD_TYPE mid = (a[A_index] & MASK(left_fp_start_index - counter_size)) << counter_size;
        if (CPD_DB_MODE0) assert(left_fp_start_index >= counter_size);
        a[A_index] = (upper | mid);
        return;
    }

    if (CPD_TYPE_SIZE >= counter_size + bit_index) {
        ulong shift = left_fp_start_index;
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[A_index] & (~mask);
//        CPD_TYPE pre_mid = a[A_index] & MASK(left_fp_start_index - counter_size);
//        CPD_TYPE mid_val = pre_mid << counter_size;
        CPD_TYPE mid = (a[A_index] & MASK(left_fp_start_index - counter_size)) << counter_size;
        CPD_TYPE lower = (a[A_index + 1]) >> (CPD_TYPE_SIZE - counter_size);
        a[A_index] = (upper | lower | mid);

        if (CPD_DB_MODE0) assert(shift >= counter_size);
        if (CPD_DB_MODE0) assert(CPD_TYPE_SIZE - counter_size >= 0);

        for (size_t i = A_index + 1; i < size - 1; ++i) {
            a[i] = (a[i] << counter_size) | (a[i + 1] >> (CPD_TYPE_SIZE - counter_size));
        }
        a[size - 1] <<= counter_size;

    } else {
        ulong shift = left_fp_start_index;
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[A_index] & (~mask);
        ulong lower_shift = CPD_TYPE_SIZE - counter_size;
        CPD_TYPE lower = a[A_index + 1] >> lower_shift;
        lower &= MASK(left_fp_start_index);
        a[A_index] = upper | lower;

        for (size_t i = A_index + 1; i < size - 1; ++i) {
            a[i] = (a[i] << counter_size) | (a[i + 1] >> (CPD_TYPE_SIZE - counter_size));
        }
        a[size - 1] <<= counter_size;

        if (CPD_DB_MODE0) assert(0 <= shift and shift < counter_size);
        if (CPD_DB_MODE0) assert(0 <= lower_shift and lower_shift < CPD_TYPE_SIZE);
    }

}


auto CPD::get_last_a_index_containing_the_header() const -> size_t {
    return (max_distinct_capacity << 1u) / CPD_TYPE_SIZE - int(!header_and_body_has_joined_slot);
//    return INTEGER_ROUND(temp, CPD_TYPE_SIZE);
}

auto CPD::get_last_a_index_containing_the_body() const -> size_t {
    return (total_bit_before_counter_begin() / CPD_TYPE_SIZE) - int(!body_and_counters_has_joined_slot);
}

auto CPD::get_first_index_containing_the_body() const -> size_t {
    if (header_and_body_has_joined_slot)
        return get_last_a_index_containing_the_header();
    return get_last_a_index_containing_the_header() + 1;
}

auto CPD::get_first_index_containing_the_counters() const -> size_t {
    if (body_and_counters_has_joined_slot)
        return get_last_a_index_containing_the_body();
    return get_last_a_index_containing_the_body() + 1;
}


auto CPD::get_header_rel_bit_index_in_last_header_slot() -> size_t {
    auto temp = (max_distinct_capacity << 1u);
    return temp % CPD_TYPE_SIZE;
}

auto CPD::get_body_rel_bit_index_in_last_body_slot() -> size_t {
    auto temp = total_bit_before_counter_begin();
    return temp % CPD_TYPE_SIZE;
}


//auto CPD::get_header_and_body_joined_slot_index() -> size_t {
//    return get_last_a_index_containing_the_header();
//}

//auto CPD::get_body_and_counters_joined_slot_index() -> size_t {
////    return (total_bit_before_counter_begin() / CPD_TYPE_SIZE) - int(!body_and_counters_has_joined_slot);
////    size_t
////    return (max_distinct_capacity << 1u) / CPD_TYPE_SIZE - int(!header_and_body_has_joined_slot);
//}


auto get_a_size(size_t q_range, size_t max_distinct_element, size_t remainder_size, size_t counter_size) -> size_t {
    /*
     * each distinct_element gets:
     * 1) 2 bits in the header.
     * 2) "remainder_size" bits in the body.
     * 3) "counter_size" bits in the body.
     */
    return INTEGER_ROUND(((remainder_size + 2 + counter_size) * max_distinct_element), (CPD_TYPE_SIZE));
}

auto CPD::get_number_of_bits_in_a(size_t m, size_t f, size_t l) -> size_t {
    return (l + 2) * f;
}

auto CPD::get_a() const -> CPD_TYPE * {
    return a;
}



auto CPD::get_fp_size() const -> const uint_fast16_t {
    return fp_size;
}

auto CPD::get_max_capacity() const -> const uint_fast16_t {
    return max_distinct_capacity;
}

auto CPD::get_size() const -> const uint_fast16_t {
    return size;
}

auto CPD::get_deal_with_joined_slot() const -> const bool {
    return header_and_body_has_joined_slot;
}

void CPD::print_as_array() const {
    print_array_as_integers(a, size);
}

void CPD::print_as_consecutive_memory() {
    print_array_as_consecutive_memory<CPD_TYPE>(a, size, cout);
}

//auto operator<<(ostream &os, const CPD &pd) -> ostream & {
//    print_array_as_consecutive_memory<CPD_TYPE>(pd.a, pd.size, cout);
//    return os;
//}

auto CPD::get_capacity() -> size_t {
//    size_t total_bits = get_header_size_in_bits();
    size_t last_index = get_last_a_index_containing_the_header();
    size_t bit_index = get_header_rel_bit_index_in_last_header_slot();
    size_t res = 0;
    for (size_t i = 0; i < last_index; ++i) {
        res += __builtin_popcount(a[i]);
    }

    if (bit_index == 0)
        return res + __builtin_popcount(a[last_index]);
    auto temp = a[last_index] & (~MASK(CPD_TYPE_SIZE - bit_index));
    return res + __builtin_popcount(temp);
}

auto CPD::is_full() -> bool {
    return get_capacity() == max_distinct_capacity;
}

auto CPD::is_empty() -> bool {
    return get_capacity() == 0;
}

auto CPD::translate_to_unpacked_index(size_t A_index, size_t rel_bit_start_index) -> size_t {
    if (!CPD_DB_MODE1)
        return ((A_index * CPD_TYPE_SIZE) - (get_header_size_in_bits() - rel_bit_start_index)) / fp_size;

    auto res = ((A_index * CPD_TYPE_SIZE) - (get_header_size_in_bits() - rel_bit_start_index)) / fp_size;

    auto header_size = get_header_size_in_bits();
    auto bit_count_up_to_remainder = ((A_index * CPD_TYPE_SIZE) + rel_bit_start_index);
    auto bit_length = bit_count_up_to_remainder - header_size;
    assert(!(bit_length % fp_size));
    assert(res == (bit_length / fp_size));
    return bit_length / fp_size;
}

auto CPD::translate_counter_index_to_abs_bit_index(size_t counter_index) -> size_t {
    return total_bit_before_counter_begin() + (counter_index * counter_size);
}

auto CPD::increase_counter(size_t counter_index) -> counter_status {
    //todo not naive implementation.
    auto temp = read_counter(counter_index);
    if (!CPD_DB_MODE1)
        assert(temp > 0);
    if (temp == MASK(counter_size)) {
        return inc_overflow;
    }
    write_counter(counter_index, ++temp);
    return OK;
}

auto CPD::decrease_counter(size_t counter_index) -> counter_status {
    auto temp = read_counter(counter_index);
    if (!CPD_DB_MODE1)
        assert(temp > 0);
    if (temp == 1) {
        return dec_underflow;
    }
    write_counter(counter_index, --temp);
    return OK;
}

auto CPD::read_counter(size_t counter_index) -> CPD_TYPE {
    auto H_plus_B_size = total_bit_before_counter_begin();
    auto bit_start = H_plus_B_size + (counter_index * counter_size);
    auto A_index = bit_start / CPD_TYPE_SIZE;
    auto bit_rem = bit_start % CPD_TYPE_SIZE;
    auto bits_left_in_slot = CPD_TYPE_SIZE - bit_rem;
    if (bits_left_in_slot > counter_size) {
        return (a[A_index] & MASK(bits_left_in_slot)) >> (bits_left_in_slot - counter_size);
    } else if (bits_left_in_slot == counter_size) {
        return a[A_index] & MASK(counter_size);
    } else {
        if (!CPD_DB_MODE1)
            assert(A_index < size);
        auto bits_to_take = counter_size - bits_left_in_slot;
        auto upper = (a[A_index] & MASK(bits_left_in_slot)) << bits_to_take;
        auto lower = a[A_index + 1] >> (CPD_TYPE_SIZE - bits_to_take);
        return upper | lower;
    }
}


void CPD::write_counter(size_t counter_index, CPD_TYPE value) {
    auto H_plus_B_size = total_bit_before_counter_begin();
    auto bit_start = H_plus_B_size + (counter_index * counter_size);
    auto A_index = bit_start / CPD_TYPE_SIZE;
    auto bit_rem = bit_start % CPD_TYPE_SIZE;
    auto bits_left_in_slot = CPD_TYPE_SIZE - bit_rem;
    if (bits_left_in_slot > counter_size) {
        auto to_shift = bits_left_in_slot - counter_size;
        CPD_TYPE mid_mask = (~MASK(bits_left_in_slot)) | MASK(to_shift);
        assert(!(mid_mask) | (value << to_shift));
        a[A_index] = (a[A_index] & mid_mask) | (value << to_shift);
    } else if (bits_left_in_slot == counter_size) {
        a[A_index] = (a[A_index] & (~MASK(counter_size))) | value;
    } else {

        size_t shift = bits_left_in_slot;
        ulong mask = MASK(bits_left_in_slot);
        ulong upper = a[A_index] & (~mask);
        CPD_TYPE lower = value >> (counter_size - bits_left_in_slot);
        a[A_index] = (upper | lower);

        if (CPD_DB_MODE0) assert(counter_size - shift > 0);

        //same amount that r was shifted right by. (fp_size - shift)
        size_t bits_left = counter_size - shift; // bits_left =  fp_size + bit_index - CPD_TYPE_SIZE
        upper = (value & MASK(bits_left)) << (CPD_TYPE_SIZE - bits_left); // todo Check this.
//        lower = (a[A_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits. todo try mask instead.
//        CPD_TYPE lower_att = a[A_index + 1] & MASK(bits_left) ; DOES NOT WORK.
        lower = a[A_index + 1] & MASK(CPD_TYPE_SIZE - bits_left);
//        assert(lower_att2 == lower);
//        assert(lower_att == lower);
        a[A_index + 1] = (upper | lower);


        /*assert(A_index < size);
        auto bits_to_take = counter_size - bits_left_in_slot;
        assert(bits_to_take > 0);
        CPD_TYPE mask = MASK(bits_left_in_slot);
        a[A_index] = (a[A_index] & ~mask) | ((value >> bits_to_take)  & mask);
        auto shift = CPD_TYPE_SIZE - bits_to_take;
        mask = MASK(shift);
        auto prev_val = a[A_index + 1];
        auto upper = value << shift;
        auto lower = a[A_index + 1] & mask;
        auto after_val = lower | upper;
        a[A_index + 1] = (a[A_index + 1] & mask) | (value << shift);
        assert((value << shift) < MASK32);*/
    }
}

/*
CPD::~CPD() {
    delete[] a;
}
*/

void CPD::print_body_as_array() const {
    auto start = get_first_index_containing_the_body();
    auto end = get_last_a_index_containing_the_body();
    print_array_as_integers(&(a[start]), end - start + 1);
}

void CPD::print_body_as_consecutive_memory() const {
    auto start = get_first_index_containing_the_body();
    auto end = get_last_a_index_containing_the_body();
    print_array_as_integers(&(a[start]), end - start + 1);
}


void CPD::print_counters_as_array() const {
    auto temp_index = get_last_a_index_containing_the_body();
    print_array_as_integers(&(a[temp_index]), size - temp_index);
}

void CPD::print_counters_as_consecutive_memory() const {
    auto temp_index = get_last_a_index_containing_the_body();
    print_array_as_consecutive_memory(&(a[temp_index]), size - temp_index, cout);
}

auto operator<<(ostream &os, const CPD &cpd) -> ostream & {
    os << "counters:" << endl;
    cpd.print_counters_as_array();
    cpd.print_counters_as_consecutive_memory();
    os << "\nbody:" << endl;
    cpd.print_body_as_array();
    cpd.print_body_as_consecutive_memory();
    return os;
/*
    os << "a: " << cpd.a << " fp_size: " << cpd.fp_size << " max_distinct_capacity: " << cpd.max_distinct_capacity
       << " size: " << cpd.size << " counter_size: " << cpd.counter_size << " header_and_body_has_joined_slot: "
       << cpd.header_and_body_has_joined_slot << " body_and_counters_has_joined_slot: "
       << cpd.body_and_counters_has_joined_slot;
       return os;
       */
}

auto CPD::has_unique_remainders() -> bool {
    return false;
}
