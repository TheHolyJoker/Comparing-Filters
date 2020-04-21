//
// Created by tomer on 3/15/20.
//

//#include "Naive_Counter_Endec.hpp"
#include "my_bit_op.hpp"

template<typename T>
auto compute_diff_value_safe(T x, T y) -> int {
    if ((x == 0) or (y == 0)) {
        if (x == y) return 0;
        return (x) ? __builtin_clz(x) : -__builtin_clz(y);
    }
}

template<typename T>
auto get_numbers_bit_length_difference(const T x, const T y) -> int {
/*
//    if ((x == 0) or (y == 0)){
//        if (x == y) return 0;
//        return (x) ? __builtin_clz(x) : -__builtin_clz(y);
//    }

    auto res2 = 1u;
    if ((x == 0) or (y == 0)) {
        if (x == y) res2 = 0;
        else
            res2 = (x) ? __builtin_clz(x) : -__builtin_clz(y);
    }

    uint32_t old_res = (__builtin_clz(x)) - (__builtin_clz(y));
    uint32_t old_res2 = (__builtin_clz(x)) - (__builtin_clz(y));
    uint32_t old_res3 = (__builtin_clz(x)) - (__builtin_clz(y));
*/
    //For validation.
//    auto x_clz = __builtin_clz(x);
//    auto y_clz = __builtin_clz(y);
//    auto res = x_clz - y_clz;
    int x_temp = (x == 0) ? 0 : floor(log2(x));
    int y_temp = (y == 0) ? 0 : floor(log2(y));
    return y_temp - x_temp;
//    auto exp_res = y_temp - x_temp;
//    assert(exp_res == res);
//    return res;
//    assert (((x != 0) and (y != 0)) or (x == y)); pass
//    assert (((x != 0) and (y != 0))); pass

    /*Explanation: (32 - y) - (32 - x) == floor(l2(x)) - floor(l2(y)) */
//    return __builtin_clz(x) - __builtin_clz(y);
}

template<typename T>
auto get_x_bit_length(const T x) -> size_t {
    if (x == 0)
        return 0;
    auto T_size = sizeof(x) * CHAR_BIT;
    auto res = T_size - __builtin_clz(x);
    assert(x <= (MASK(res)));

    if (res > 1)
        assert(x > (MASK(res - 1u)));
    return res;
}


template<typename T>
void find_kth_interval_simple(T *a, size_t a_size, size_t k, size_t *start, size_t *end) {
    if (k == 0) {
        *start = 0;
        *end = find_first_set_bit<T>(a, a_size);
        if (DB) cout << "h1" << endl;
        return;
    }

    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    for (int i = 0; i < a_size; ++i) {
        if (k == 1) {
            find_first_and_second_set_bits(&a[i], a_size - i, start, end);
            *start += (i) * slot_size;
            *end += (i) * slot_size;
            cout << "h2" << endl;
            return;
        }

        auto temp_counter = __builtin_popcount(a[i]);
        if (temp_counter == k) {
            *start = ((i + 1) * slot_size) - __builtin_ctz(a[i]);
            *end = find_first_set_bit(&a[i + 1], a_size - (i + 1)) + slot_size * (i + 1);
            cout << "h3" << endl;
            return;
        }
        if (temp_counter > k) {
            uint shift = select_r(a[i], k) - slot_size + 1;
            *end = (i * slot_size) + shift + __builtin_clz(a[i] << (shift));
            *start = (i * slot_size) + shift;
            if (DB) cout << "h5" << endl;
            return;
/*
            return;
            T slot = a[i];
            T counter = 0;
            for (int j = 0; j < k; ++j) {
                auto shift = __builtin_clz(slot) + 1;
                assert(0 < shift < slot_size);
                slot <<= shift;
                assert(slot > 0);
                counter += (shift - 1);
            }
            *start = (i * slot_size) + (slot_size - (counter - 1) - 1);
            *end = (i * slot_size) + (slot_size - (counter + __builtin_ctz(slot)) - 1);
            if (DB) cout << "h5" << endl;
            return;
*/
        }
        k -= temp_counter;
        /*else if (temp_counter == k) {
            *start = (i * slot_size) + (slot_size - __builtin_ctz(a[i]) - 1);
            *end = find_first_set_bit<T>(&(a[i + 1]), a_size - (i + 1)) + slot_size * (i + 1);
            if (DB) cout << "h4: " << temp_counter << endl;
            return;*/
    }
    assert(false);
}

template<typename T>
auto find_kth_interval_simple(T *a, size_t a_size, size_t k) -> std::tuple<size_t, size_t> {
    if (k == 0) {
        return std::make_tuple(0, find_first_set_bit(a, a_size));
    }
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    for (int i = 0; i < a_size; ++i) {
        if (k == 1) {
            return find_first_and_second_set_bits(&a[i], a_size - i);
            /**start += (i) * slot_size;
            *end += (i) * slot_size;
            cout << "h2" << endl;
            return;*/
        }

        auto temp_counter = __builtin_popcount(a[i]);
        if (temp_counter == k) {
            size_t start, end;

            start = ((i + 1) * slot_size) - __builtin_ctz(a[i]);
            end = find_first_set_bit(&a[i + 1], a_size - (i + 1)) + slot_size * (i + 1);
            return std::make_tuple(start, end);

            /*cout << "h3" << endl;
            return;*/
        }
        if (temp_counter > k) {
            size_t start, end;

            uint shift = select_r(a[i], k) - slot_size + 1;

            end = (i * slot_size) + shift + __builtin_clz(a[i] << (shift));
            start = (i * slot_size) + shift;
            return std::make_tuple(start, end);
            /*if (DB) cout << "h5" << endl;
            return;*/
        }
        k -= temp_counter;
    }
    assert(false);
}

void word_k_select(uint32_t word, size_t k, size_t *start, size_t *end) {

}

template<typename T>
auto find_first_set_bit(T *a, size_t a_size) -> size_t {
    uint i = 0;
    while (a[i] == 0) i++;
    auto bit = __builtin_clz(a[i]);
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    return i * slot_size + bit;
}

template<typename T>
void find_first_and_second_set_bits(const T *a, size_t a_size, size_t *first, size_t *second) {
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    uint i = 0;
    while (a[i] == 0) i++;
    auto bit = __builtin_clz(a[i]);
    T slot = a[i] << (bit + 1);
    if (slot) {
        auto length = __builtin_clz(slot);
        *first = i * slot_size + bit + 1;
        *second = i * slot_size + bit + 1 + length;
//        if (DB) cout << "h1" << endl;
        return;
    }
    *first = i * slot_size + bit + 1;
    while (a[++i] == 0);
    bit = __builtin_clz(a[i]);
    *second = i * slot_size + bit;
//    if (DB) cout << "h2" << endl;
}

template<typename T>
auto find_first_and_second_set_bits(const T *a, size_t a_size) -> std::tuple<size_t, size_t> {
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    uint i = 0;
    while (a[i] == 0) i++;
    auto bit = __builtin_clz(a[i]);
    T slot = a[i] << (bit + 1);
    if (slot) {
        auto length = __builtin_clz(slot);
        return std::make_tuple(i * slot_size + bit + 1, i * slot_size + bit + 1 + length);
        /*
//        *first = i * slot_size + bit + 1;
//        *second = i * slot_size + bit + 1 + length;
//
//        if (DB) cout << "h1" << endl;
        return;
*/
    }
    size_t first = i * slot_size + bit + 1;
    size_t second;
//    *first = i * slot_size + bit + 1;
    while (a[++i] == 0);
    bit = __builtin_clz(a[i]);
    second = i * slot_size + bit;
    return std::make_tuple(first, second);
//    if (DB) cout << "h2" << endl;
}

template<typename T>
void update_element(T *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val,
                    size_t a_size) {
    /*if (prev_start != new_start) {
        cout << "update_element" << endl;
        cout << "weird: prev_start != new_start" << endl;
        assert(false);
    }*/
    assert(prev_start == new_start);
//    size_t prev_val_length = prev_end - prev_start;
//    size_t new_val_length = new_end - new_start;
    /*
    int new_length_minus_prev = new_val_length - prev_val_length;

    auto prev_val = read_word<T>(a, a_size, prev_start, prev_end);

    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    auto start_index = prev_start / slot_size;
    size_t shift = new_length_minus_prev;



    const size_t column_num = 3;
    const size_t var_count = 3;
    string columns[column_num] = {"total", "index", "bit_rem"};
    string var_names_c[var_count] = {"start", "prev_end", "new_end"};


    assert(start_index + 1 < a_size);
    size_t values_c[column_num * var_count] =
            {prev_start, prev_start / slot_size, prev_start % slot_size,
             prev_end, prev_end / slot_size, prev_end % slot_size,
             new_end, new_end / slot_size, new_end % slot_size};

    cout << string(32, '*') + "\tupdate_element_att call\t" + string(32, '*') << endl;
    table_print_columns(var_count, column_num, var_names_c, columns, values_c);

    const size_t num = 8;
    string var_names[num] = {"prev_val", "new_val",
                             "prev_val_length", "new_val_length",
                             "abs_length_diff", "shift",
                             "a[start_index]", "a[start_index + 1]"};

    size_t values[num] = {prev_val, new_val,
                          prev_val_length, new_val_length,
                          size_t(abs(new_length_minus_prev)), shift,
                          a[start_index], a[start_index + 1]};
    table_print(num, var_names, values);
    */


    if (prev_end == new_end) {
        update_element_with_fixed_size<T>(a, prev_start, prev_end, new_val, a_size);
//        cout << "H-1" << endl;
    } else if (prev_end < new_end) {
        update_element_push(a, prev_start, prev_end, new_start, new_end, new_val, a_size);
    } else if (prev_end > new_end)
        update_element_pull(a, prev_start, prev_end, new_start, new_end, new_val, a_size);
    else {
        assert(false);
    }
}

template<typename T>
void update_element_push(T *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val,
                         size_t a_size) {

    assert(prev_start == new_start);// I might be assuming this is true in the loop used for shifting.
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    auto shift = new_end - prev_end;
    assert((0 < shift) && (shift < slot_size));
//    size_t prev_val_length = prev_end - prev_start;
//    size_t new_val_length = new_end - new_start;
//    int new_length_minus_prev = new_val_length - prev_val_length;

    auto prev_val = read_word<T>(a, a_size, prev_start, prev_end);

    auto start_index = prev_start / slot_size;
    auto new_start_index = new_start / slot_size;
    auto new_end_index = new_end / slot_size;
    auto prev_end_index = prev_end / slot_size;

    bool case1 = (start_index < prev_end_index);
    bool case2 = (start_index == prev_end_index) and (prev_end_index < new_end_index);
    bool case3 = (start_index == prev_end_index) and (prev_end_index == new_end_index);
    assert(case1 or case2 or case3);


    //Todo: validate loop.
    for (auto i = a_size - 1; i > new_start_index; --i) {
        a[i] = (a[i] >> shift) | (a[i - 1] << (slot_size - shift));
        assert(a[i] <= MASK(slot_size));
    }
    if (case1 or case2) {
        assert(start_index < new_end_index);
        update_element_with_fixed_size<T>(a, new_start, new_end, new_val, a_size);
//        cout << "H0" << endl;
        return;

    } else if (case3) {
        assert(start_index == new_end_index);
        auto left_mask = slot_size - (prev_start % slot_size);
        auto right_mask = slot_size - (prev_end % slot_size);
        auto left = (left_mask) ? a[start_index] & (~MASK(left_mask)) : 0;
        auto right = (right_mask) ? a[start_index] & (MASK(right_mask)) : 0;
//        assert(a[start_index] == (left | right));
        a[start_index] = left | (right >> shift);

        update_element_with_fixed_size<T>(a, new_start, new_end, new_val, a_size);
//        cout << "H2" << endl;
        return;
    } else {
        assert(false);
    }

}

template<typename T>
void update_element_pull(T *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val,
                         size_t a_size) {

    T unchanged_a[a_size];
    for (int i = 0; i < a_size; ++i) { unchanged_a[i] = a[i]; }

    assert(prev_start == new_start);
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    size_t prev_val_length = prev_end - prev_start;
    size_t new_val_length = new_end - new_start;
    int new_length_minus_prev = prev_val_length - new_val_length;

    auto prev_val = read_word<T>(a, a_size, prev_start, prev_end);
    assert(prev_val_length > new_val_length);

    auto start_index = prev_start / slot_size;
    auto end_index = prev_end / slot_size;
    auto new_end_pos = prev_start + new_val_length;
    auto new_end_index = new_end_pos / slot_size;
    size_t shift = new_length_minus_prev;

    assert(prev_start <= prev_end);

    assert(start_index <= new_end_index);
    assert(new_end_index <= end_index);
    assert(end_index <= start_index + 1);

    bool case1 = (start_index < new_end_index) and (new_end_index == end_index);
    bool case2 = (start_index == new_end_index) and (new_end_index < end_index);
    bool case3 = (start_index == new_end_index) and (new_end_index == end_index);
    assert(case1 or case2 or case3);

    if (case1) {
        for (auto i = new_end_index; i < a_size - 1; ++i) {
            a[i] = (a[i] << shift) | (a[i + 1] >> (slot_size - shift));
        }
        a[a_size - 1] <<= shift;

        update_element_with_fixed_size<T>(a, new_start, new_end, new_val, a_size);
//        cout << "H3" << endl;
        return;
    } else if (case2) {
        assert(end_index <= start_index + 1);
        const auto temp = a[end_index];

        for (auto i = new_end_index + 1; i < a_size - 1; ++i) {
            a[i] = (a[i] << shift) | (a[i + 1] >> (slot_size - shift));
        }
        a[a_size - 1] <<= shift;

        auto num_of_bits_need_to_extract = slot_size - (new_end % slot_size);
        auto max_num_of_bits_that_can_be_extracted = slot_size - (prev_end % slot_size);
        bool is_mid_necessary = num_of_bits_need_to_extract > shift;
        assert(!is_mid_necessary);
        assert(num_of_bits_need_to_extract != 0);
        auto correct_right = read_word<T>(unchanged_a, a_size, prev_end, prev_end + num_of_bits_need_to_extract);

        auto prev_end_rem = prev_end % slot_size;
        auto left_mask = slot_size - (prev_start % slot_size);
        auto left = (left_mask) ? a[start_index] & (~MASK(left_mask)) : 0;
        auto right_shift = (slot_size - num_of_bits_need_to_extract);
        T right = (temp << prev_end_rem) >> right_shift;
//        assert(right == right & MASK(num_of_bits_need_to_extract));
        assert(right == correct_right);
        a[start_index] = left | right;

        update_element_with_fixed_size<T>(a, new_start, new_end, new_val, a_size);
//        cout << "H11" << endl;
        return;


    } else if (case3) {
        assert((start_index == new_end_index) and (new_end_index == end_index));
        auto num_of_bits_need_to_extract = slot_size - (new_end % slot_size);
        T res = 0;

        if (num_of_bits_need_to_extract + (prev_end % slot_size) > slot_size) {
//            cout << "cond is true" << endl;
            auto bits_to_take_from_slot_plus2 = num_of_bits_need_to_extract + (prev_end % slot_size) - slot_size;
            assert(start_index + 2 < a_size);
            res |= (a[start_index + 2]) >> (slot_size - bits_to_take_from_slot_plus2);
            assert(res == (res & MASK(num_of_bits_need_to_extract)));
        }

        const auto temp = a[start_index + 1];
        for (auto i = new_end_index + 1; i < a_size - 1; ++i) {
            a[i] = (a[i] << shift) | (a[i + 1] >> (slot_size - shift));
        }
        a[a_size - 1] <<= shift;

        assert(num_of_bits_need_to_extract != 0);
        auto correct_right = read_word<T>(unchanged_a, a_size, prev_end, prev_end + num_of_bits_need_to_extract);
        auto prev_end_rem = prev_end % slot_size;

        auto bit_start_rem = prev_start % slot_size;
        auto l_power = slot_size - (bit_start_rem);
        auto left = (l_power) ? a[start_index] & (~MASK(l_power)) : 0;

        auto right_shift = (slot_size - num_of_bits_need_to_extract);
        T right = ((temp << prev_end_rem) >> right_shift) | res;
        assert(right == (right & MASK(num_of_bits_need_to_extract)));
        assert(right == correct_right);

        T mid = (a[start_index] & (MASK(slot_size - (prev_end % slot_size)))) << shift;

        bool is_mid_necessary = num_of_bits_need_to_extract > shift;
        assert(is_mid_necessary);

        a[start_index] = left | mid | right;
        update_element_with_fixed_size<T>(a, new_start, new_end, new_val, a_size);
//        cout << "H6" << endl;
        return;

    } else
        assert(false);
}

template<typename T>
void update_element_with_fixed_size(T *a, size_t bit_start_pos, size_t bit_end_pos, T new_val, size_t a_size) {
    if (bit_start_pos == bit_end_pos) {
        cout << "bit_start_pos == bit_end_pos" << endl;
        cout << "J0" << endl;
        return;
    }
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    assert(bit_start_pos + slot_size >= bit_end_pos);


    auto start_index = bit_start_pos / slot_size;
    auto end_index = bit_end_pos / slot_size;

    if (bit_end_pos % slot_size == 0) {
        auto length = bit_end_pos - bit_start_pos;
        assert((length > 0) and (length <= slot_size));
        a[start_index] = (a[start_index] & (~MASK(length))) | new_val;
        cout << "J1" << endl;
        return;
    }

    if (start_index == end_index) {
        auto length = bit_end_pos - bit_start_pos;
        assert(length > 0);
        assert(length <= slot_size);
        assert(new_val <= MASK(length + 1u));
        auto start_rem = bit_start_pos % slot_size;
        auto end_rem = bit_end_pos % slot_size;

        //Sanity? Or not?
        assert((slot_size - start_rem) > (slot_size - end_rem));

        auto first_shift = (slot_size - start_rem);
        auto second_shift = (slot_size - end_rem);
        auto left = a[start_index] & MASK(second_shift);
        uint32_t right = (((a[start_index] >> first_shift) << length) | new_val) << second_shift;
        a[start_index] = right | left;
        cout << "J2" << endl;
        return;


        /*
        temp >>= first_shift;
        temp <<= length;
        temp |= new_val;
        temp <<= (second_shift);
        temp |= left;

          //        uint32_t  temp3 = (((a[start_index] >> first_shift) & (~MASK(length))) | new_val) << second_shift;
                    uint32_t temp2 = ((((a[start_index] >> first_shift) << length) | new_val) << second_shift) | left;

        //Maybe: shift_second  + 1 ;
        uint32_t temp_mask = ~(MASK(first_shift) & MASK(second_shift));
        uint32_t temp4 = a[start_index] & (temp_mask) | (new_val << second_shift);

//        bool c2 = temp3 == temp2;
//        bool c1 = true, c2 = true;
        bool c1 = temp2 == temp;
        bool c2 = temp4 == temp2;

        if (!(c1 && c2)) {
            bool c_array[2] = {c1, c2};
            for (int i = 0; i < 2; ++i) { if (!c_array[i]) { cout << "c" << i+1 << " failed." << endl; }}
//            printf("%u\n%u\n%u\n%u\n", temp, temp2, temp3, temp4);
            printf("%u\n%u\n%u\n", temp, temp2, temp4);

        }
        assert(c1 || c2);
        a[start_index] = temp;
        cout << "H2" << endl;
        return;*/

    } else if (start_index + 1 == end_index) {

        auto length = bit_end_pos - bit_start_pos;
        assert(length > 0);
        assert(new_val <= MASK(length + 1));

//        auto start_rem = bit_start_pos % slot_size;
        auto end_rem = bit_end_pos % slot_size;

        auto left_shift = length - end_rem;
        auto left_val = (a[start_index] & (~MASK(left_shift))) | (new_val >> end_rem);

        auto right_shift = slot_size - end_rem;
        auto right_val = (a[end_index]) & (MASK(right_shift)) | (new_val << right_shift);

        a[start_index] = left_val;
        a[end_index] = right_val;
        cout << "J3" << endl;
        return;

    } else {
        cout << "J4" << endl;
        assert(false);
    }
}

auto count_set_bits(uint32_t *a, size_t a_size) -> size_t {
    size_t counter = 0;
    for (int i = 0; i < a_size; ++i) {
        counter += __builtin_popcount(a[i]);
    }
    return counter;
}


template<typename T>
auto read_word(const T *A, size_t a_size, size_t bit_start_index, size_t bit_end_index) -> T {
    if (bit_start_index == bit_end_index) {
//        cout << "H0" << endl;
        return 0;
    }
    auto slot_size = sizeof(A[0]) * CHAR_BIT;
    auto bit_start_rem = bit_start_index % slot_size;
    auto bit_end_rem = bit_end_index % slot_size;
    size_t shift_power = slot_size - bit_start_rem;
    uint32_t mask = SL(shift_power) - 1u;
    uint32_t shift_left = slot_size - (bit_end_rem);

    if (bit_end_rem == 0) {
        size_t array_start_index = bit_start_index / slot_size;
        auto temp = A[array_start_index] & mask;
//        cout << "H1" << endl;
        return temp;
    }
    size_t array_start_index = bit_start_index / slot_size, array_end_index = bit_end_index / slot_size;

    if (array_start_index == array_end_index) {
        auto temp = A[array_start_index] & mask;
        assert(shift_left > 0);

//        cout << "H2" << endl;
        return temp >> shift_left;

    } else if (array_start_index + 1 == array_end_index) {
        auto i = array_start_index;
        assert(i + 1 == array_end_index);
        assert(i + 1 < a_size);

        // a[i] = (a[i] << fp_size) | (a[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
//        cout << "H3" << endl;
        return ((A[i] & mask) << bit_end_rem) | (A[i + 1] >> shift_left);
    } else
        assert(false);
}

/*
template<typename T>
void
read_k_words_fixed_length(const T *A, size_t a_size, size_t first_element_index, size_t element_length, T *res_array) {
    size_t k = sizeof(res_array) / sizeof(A[res_array]);
    uint32_t slot_size = sizeof(T) * CHAR_BIT;
    auto start = first_element_index * element_length;
    auto rem = start % slot_size;
    auto total_bit_length = element_length * k;
    auto A_length = INTEGER_ROUND(total_bit_length, slot_size);
    size_t A_start_index = INTEGER_ROUND(start, slot_size);
    auto bit_temp = start;
    auto A_index = A_start_index;
    auto bits_left = slot_size - rem;
    for (int i = 0; i < k; ++i) {
        if (bits_left > element_length) {
            uint32_t shift = bits_left - element_length;
            res_array[i] = A[A_index] >> shift;
            bits_left = shift;
        } else if (bits_left > element_length)
    }
//    assert(A_start_index)

}
*/

template<typename T>
void
read_k_words_fixed_length_att(const T *A, size_t a_size, size_t first_element_index, size_t element_length,
                              T *res_array, size_t k) {
//    size_t k = sizeof(res_array) / sizeof(res_array[0]);
    uint32_t slot_size = sizeof(T) * CHAR_BIT;

    size_t total_bit_counter = (first_element_index * element_length);

    size_t comparing_counter = 0;
    size_t comparing_lim = k;

    for (; comparing_counter < comparing_lim; ++comparing_counter) {
        size_t A_index = total_bit_counter / slot_size;
        size_t bit_index_inside_slot = total_bit_counter % slot_size;
        size_t bits_left_to_read_inside_slot = slot_size - bit_index_inside_slot;

        T current_cell = A[A_index];

        /*More than element_length bits remain in B[A_index].*/
        if (bits_left_to_read_inside_slot > element_length) {
            ulong shift = bits_left_to_read_inside_slot - element_length;
            if (DB) assert(shift < slot_size);
            T current_remainder = (current_cell >> (shift)) & MASK(element_length);
            res_array[comparing_counter] = current_remainder;
            /*Exactly element_length bits remain in B[A_index].*/
        } else if (bits_left_to_read_inside_slot == element_length) {
            T current_remainder = current_cell & MASK(element_length);
            res_array[comparing_counter] = current_remainder;
            /*Less than element_length bits remain in B[A_index].*/
        } else {
            size_t number_of_bits_to_read_from_next_slot = element_length - bits_left_to_read_inside_slot;
            ulong upper_shift = element_length - bits_left_to_read_inside_slot;
            if (DB) assert(upper_shift >= 0 and upper_shift < slot_size);
            ulong upper = (current_cell & MASK(bits_left_to_read_inside_slot)) << (upper_shift);
            if (DB) assert(a_size > A_index + 1);
            ulong lower_shift = slot_size - number_of_bits_to_read_from_next_slot;
            if (DB) assert(0 <= lower_shift and lower_shift < slot_size);
            ulong lower = (A[A_index + 1] >> lower_shift) & MASK(number_of_bits_to_read_from_next_slot);
            T current_remainder = upper | lower;
            res_array[comparing_counter] = current_remainder;
        }
        total_bit_counter += element_length;
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// vector functions.

auto vector_last_k_bits_are_zero(vector<bool> *vec, size_t k) -> bool {
    auto size = vec->size();
    for (auto i = size - k; i < size; ++i) {
        if (vec->at(i) != 0) {
            return false;
        }
    }
    return true;
}


template<typename T>
void from_array_to_vector(vector<bool> *vec, const T *a, size_t a_size) {
    size_t slot_size = (sizeof(a[0]) * CHAR_BIT);
    size_t vec_new_size = slot_size * a_size, vec_index = 0;
    vec->resize(vec_new_size);
    for (size_t i = 0; i < a_size; ++i) {
        size_t b = 1ULL << (slot_size - 1);
        for (size_t j = 0; j < slot_size; ++j) {
            vec->at(vec_index) = (b & a[i]);
            b >>= 1ULL;
            vec_index++;
        }
    }
}


template<typename T>
void from_vector_to_array(const vector<bool> *vec, T *a, size_t a_size) {
    size_t slot_size = (sizeof(a[0]) * CHAR_BIT);
    assert(a_size * slot_size >= vec->size());

    size_t expected_size = (vec->size() / slot_size) + (vec->size() % slot_size != 0);
    assert(a_size == expected_size);

    for (int i = 0; i < a_size; ++i) {
        size_t start = i * slot_size;
        size_t end = start + slot_size;
        a[i] = sub_vector_to_word<T>(vec, start, end);
    }
/*

    for (size_t i = 0; i < a_size - 1; ++i) {
        ulong b = (1ul << (slot_size - 1u));
        for (int j = 0; j < slot_size; ++j) {
            assert(b > 0);
            if (vec->at(slot_size * i + j)) {
                a[i] |= b;
            }
            b >>= 1ul;
        }
    }
    ulong b = (1ul << 31ul);
    for (size_t k = 0; k < vec->size() % slot_size; ++k) {
        if (vec->at(slot_size * (a_size - 1) + k)) {
            a[a_size - 1] |= b;
        }
        b >>= 1ul;
    }
*/
}

template<typename T>
void from_vector_to_array_by_bit_limits(const vector<bool> *vec, T *a, size_t abs_bit_start_index,
                                        size_t abs_bit_end_index) {
    assert(false);

}

template<typename T>
void split_bit_vector_to_words_array(const vector<bool> *vec, T *a, size_t a_size, size_t word_size) {
    assert (a_size == vec->size() / word_size);
    assert(vec->size() % word_size == 0);
    assert(sizeof(T) * CHAR_BIT <= word_size);
    for (int i = 0; i < a_size; ++i) {
        auto start = i * word_size;
        a[i] = sub_vector_to_word<T>(start, start + word_size);
    }
}

template<typename T>
auto sub_vector_to_word(const vector<bool> *v, size_t start, size_t end) -> T {
    T res = 0;
    for (int i = start; i < end; ++i) {
        res <<= 1u;
        res |= v->at(i);
    }
    return res;
}

void vector_find_kth_interval_simple(vector<bool> *vec, size_t k, size_t *start_index, size_t *end_index) {
    size_t set_bit_counter = 0, continue_from_index = 0;
    for (size_t i = 0; i < vec->size(); ++i) {
        if (set_bit_counter == k) {
            *start_index = i;
            continue_from_index = i;
            break;
        }
        if (vec->at(i))
            set_bit_counter++;
    }
    assert(*start_index < vec->size());

    for (size_t i = continue_from_index; i < vec->size(); ++i) {
        if (vec->at(i)) {
            *end_index = i;
            return;
        }
    }
    cout << "vector_find_kth_interval_simple: reached end of vector" << endl;
    assert(false);
}

auto vector_find_first_set_bit(vector<bool> *vec) -> size_t {
    for (int i = 0; i < vec->size(); ++i) { if (vec->at(i)) return i; }
    assert(false);
}


template<typename T>
void vector_update_element_with_fixed_size(vector<bool> *vec, size_t start, size_t end, T new_val) {
    assert(0 <= start);
    assert(start <= end);
    assert(end <= vec->size());
    auto length = end - start;
    size_t pos = end - 1;
    for (int j = 0; j < length; ++j) {
        vec->at(pos) = 1u & new_val;
        new_val >>= 1u;
        pos--;
    }
}

template<typename T>
void vector_update_element(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                           T new_val) {
    auto prev_length = prev_end - prev_start;
    auto new_length = new_end - new_start;

    if (prev_length == new_length) {
        assert(prev_start == new_start);
        vector_update_element_with_fixed_size<T>(vec, new_start, new_end, new_val);
//        cout << "S0" << endl;
        return;
    } else if (prev_length < new_length) {
//        cout << "S1" << endl;
        vector_update_push<T>(vec, prev_start, prev_end, new_start, new_end, new_val);
    } else {
//        cout << "S2" << endl;
        vector_update_pull<T>(vec, prev_start, prev_end, new_start, new_end, new_val);
    }


}

template<typename T>
void
vector_update_push(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val) {
    auto prev_length = prev_end - prev_start;
    auto new_length = new_end - new_start;
    assert(prev_length < new_length);
    auto shift = new_length - prev_length;
    bool BRC = (new_val == 252645135);

    assert(vector_last_k_bits_are_zero(vec, shift));
    assert(shift <= new_end);
    for (auto i = vec->size() - 1; i >= new_end; --i) { vec->at(i) = vec->at(i - shift); }

    assert(new_start == prev_start);
    vector_update_element_with_fixed_size<T>(vec, new_start, new_end, new_val);

}

template<typename T>
void
vector_update_pull(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end, T new_val) {
    auto prev_length = prev_end - prev_start;
    auto new_length = new_end - new_start;
    assert(prev_length > new_length);

    auto shift = prev_length - new_length;

    for (auto i = new_end; i < (vec->size() - shift); ++i) { vec->at(i) = vec->at(i + shift); }

    assert(new_start == prev_start);
    vector_update_element_with_fixed_size<T>(vec, new_start, new_end, new_val);

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Templates definitions.



template auto get_numbers_bit_length_difference<uint32_t>(const uint32_t x, const uint32_t y) -> int;

template auto get_x_bit_length<uint32_t>(const uint32_t x) -> size_t;

template size_t find_first_set_bit<uint32_t>(uint32_t *a, size_t a_size);

template void find_kth_interval_simple<uint32_t>(uint32_t *a, size_t a_size, size_t k, size_t *start, size_t *end);

template void from_array_to_vector<uint32_t>(vector<bool> *vec, const uint32_t *a, size_t a_size);

template void from_vector_to_array<uint32_t>(const vector<bool> *vec, uint32_t *a, size_t a_size);

template uint32_t sub_vector_to_word<uint32_t>(const vector<bool> *v, size_t start, size_t end);

template void
update_element_with_fixed_size<uint32_t>(uint32_t *a, size_t bit_start_pos, size_t bit_end_pos,
                                         uint32_t new_val, size_t a_size);


template void
update_element<uint32_t>(uint32_t *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                         uint32_t new_val, size_t a_size);

template void
update_element_pull<uint32_t>(uint32_t *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                              uint32_t new_val, size_t a_size);

template void
update_element_push<uint32_t>(uint32_t *a, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                              uint32_t new_val, size_t a_size);

template auto
read_word<uint32_t>(const uint32_t *A, size_t a_size, size_t bit_start_index, size_t bit_end_index) -> uint32_t;

//template auto read_T_word<uint32_t>(const vector<bool> *v, size_t start, size_t end) -> uint32_t;



template
void vector_update_element_with_fixed_size<uint32_t>(vector<bool> *vec, size_t bit_start_pos, size_t bit_end_pos,
                                                     uint32_t new_val);

template
void vector_update_element_with_fixed_size<size_t>(vector<bool> *vec, size_t bit_start_pos, size_t bit_end_pos,
                                                   size_t new_val);

template void
vector_update_element<uint32_t>(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                                uint32_t new_val);

template void
vector_update_element<size_t>(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                              size_t new_val);


template void
vector_update_push<uint32_t>(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                             uint32_t new_val);

template void
vector_update_push<size_t>(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                           size_t new_val);

template void
vector_update_pull<uint32_t>(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                             uint32_t new_val);


template void
vector_update_pull<size_t>(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                           size_t new_val);

template void
read_k_words_fixed_length_att<uint32_t>(const uint32_t *A, size_t a_size, size_t first_element_index,
                                        size_t element_length,
                                        uint32_t *res_array, size_t k);

template void
read_k_words_fixed_length_att<size_t>(const size_t *A, size_t a_size, size_t first_element_index, size_t element_length,
                                      size_t *res_array, size_t k);


template
auto find_first_and_second_set_bits<uint32_t>(const uint32_t *a, size_t a_size) -> std::tuple<size_t, size_t>;