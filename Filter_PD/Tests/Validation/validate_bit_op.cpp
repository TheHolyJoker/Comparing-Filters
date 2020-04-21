//
// Created by tomer on 3/15/20.
//

#include "validate_bit_op.hpp"

auto validate_find_kth_interval_iter(size_t reps, size_t a_size) -> bool {
    uint32_t a[a_size];
    for (size_t j = 0; j < reps; ++j) {
        for (size_t i = 0; i < a_size; ++i) {
            a[i] = j;
        }
        auto set_bit_count = count_set_bits(a, a_size);
        for (int q = 0; q < set_bit_count; ++q) {
            bool res = validate_find_kth_interval_single(a, a_size, q);
            if (!res) {
                cout << "rep number " << j << endl;
                cout << "quot is " << q << endl;
                return false;
            }

        }
    }
    return true;
}


auto validate_find_kth_interval_random(size_t reps, size_t a_size) -> bool {
    uint32_t a[a_size];
    for (size_t j = 0; j < reps; ++j) {
        for (size_t i = 0; i < a_size; ++i) {
            a[i] = random();
        }
        auto set_bit_count = count_set_bits(a, a_size);
        uint32_t q = random() % set_bit_count;
//        cout << j << ":\t";
        if (!validate_find_kth_interval_single(a, a_size, q)) {
            cout << "Failed in the " << j << "'th rep." << endl;
            return false;
        }
    }
    return true;

}

auto validate_find_kth_interval_single(uint32_t *a, size_t a_size, size_t k) -> bool {
    size_t start = -1, end = -1, vec_start = -2, vec_end = -2;
    vector<bool> vec(a_size * sizeof(uint32_t) * CHAR_BIT);
    from_array_of_words_to_bit_vector(&vec, a, a_size);
    vector_find_kth_interval_simple(&vec, k, &vec_start, &vec_end);

    find_kth_interval_simple(a, a_size, k, &start, &end);

    bool cond1 = (start == vec_start);
    bool cond2 = (end == vec_end);
    bool res = cond1 & cond2;

    if (res) return true;


    if (!cond1) { printf("start %zu, vec_start %zu\n", start, vec_start); }
    if (!cond2) { printf("end %zu, vec_end %zu\n", end, vec_end); }

    return false;
}

template<typename T>
auto validate_find_first_and_second_set_bits_iter(size_t reps, size_t a_size) -> bool {
    uint32_t a[a_size];
    for (size_t j = 0; j < reps; ++j) {
        for (size_t i = 0; i < a_size; ++i) { a[i] = j + 1; }

        bool res = validate_find_first_and_second_set_bits_single<T>(a, a_size);
        if (!res) {
            cout << "rep number " << j << endl;
            return false;
        }
    }
    return true;
}

template<typename T>
auto validate_find_first_and_second_set_bits_rand(size_t reps, size_t a_size) -> bool {
    uint32_t a[a_size];
    for (size_t j = 0; j < reps; ++j) {
        for (size_t i = 0; i < a_size; ++i) { a[i] = random(); }
//        auto set_bit_count = count_set_bits(a, a_size);
//        uint32_t q = random() % set_bit_count;
//        cout << j << ":\t";
        if (!validate_find_first_and_second_set_bits_single(a, a_size)) {
            cout << "rep number " << j << endl;
            size_t k = 0;
            while (a[k] == 0) k++;
            cout << a[k] << endl;
            return false;
        }
    }
    return true;

}

template<typename T>
auto validate_find_first_and_second_set_bits_single(T *a, size_t a_size) -> bool {
    size_t start = -1, end = -1, vec_start = -2, vec_end = -2;
    vector<bool> vec(a_size * sizeof(uint32_t) * CHAR_BIT);
    from_array_of_words_to_bit_vector(&vec, a, a_size);
    vector_find_kth_interval_simple(&vec, 1, &vec_start, &vec_end);

    tie(start, end) = find_first_and_second_set_bits<T>(a, a_size);
//    find_first_and_second_set_bits(a, a_size, &start, &end);

    bool cond1 = (start == vec_start);
    bool cond2 = (end == vec_end);
    bool res = cond1 & cond2;

    if (res) return true;


    if (!cond1) { printf("start %zu, vec_start %zu\n", start, vec_start); }
    if (!cond2) { printf("end %zu, vec_end %zu\n", end, vec_end); }

    return false;
}

template<typename T>
auto v_vec_and_array_transformation(size_t reps, size_t total_bits) -> bool {
    const size_t slot_size = sizeof(T) * CHAR_BIT;
    const size_t a_size = INTEGER_ROUND(total_bits, slot_size);
    T a[a_size];
    vector<bool> vec(total_bits);

    T b[a_size];
    vector<bool> vec2(total_bits);

    for (int i = 0; i < reps; ++i) {
        for (int j = 0; j < a_size; ++j) { a[j] = random(); }
//        print_array_as_integers(a, a_size);
        from_array_of_words_to_bit_vector(&vec, a, a_size);
        from_array_of_words_to_bit_vector(&vec, b, a_size);
        for (int k = 0; k < a_size; ++k) {
            if (a[k] != b[k]) {
                cout << "h1:" << endl;
                cout << "i is:" << i << endl;
                cout << "k is:" << i << endl;
                cout << a[k] << ", " << b[k] << endl;
                return false;
            }
        }
    }
//    cout << "mid" << endl;
    for (int i = 0; i < reps; ++i) {
        for (int j = 0; j < total_bits; ++j) { vec[j] = (random() % 2); }
//        print_vector(&vec);
        from_array_of_words_to_bit_vector(&vec, a, a_size);
        from_array_of_words_to_bit_vector(&vec2, a, a_size);
        for (int k = 0; k < total_bits; ++k) {
            if (vec[k] != vec2[k]) {
                cout << "h2:" << i << endl;
                return false;
            }
        }
    }
//    cout << "Pass" << endl;
    return true;
}

/*
template<typename T>
auto v_update_element_with_fixed_size_single(T *a, size_t a_size, size_t bit_start_pos, size_t bit_end_pos,
                                             T new_val) -> bool {
    return 0;
}*/

template<typename T>
auto v_update_element_with_fixed_size_rand(size_t reps, const size_t sub_reps) -> bool {
    T a[sub_reps], changed_array[sub_reps], unchanged_array[sub_reps];
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    size_t total_bits = slot_size * sub_reps;
    for (int i = 0; i < reps; ++i) {
        for (int k = 0; k < sub_reps; ++k) { unchanged_array[k] = changed_array[k] = a[k] = random(); }
        for (int j = 0; j < sub_reps; ++j) {


            size_t start = random() % (total_bits - slot_size);
            size_t length = random() % slot_size;
            size_t end = start + length;
            T new_val = random() & MASK(length);

            ////Testing updating and extracting are invertible. (extract first)
//            for (int m = 0; m < sub_reps; ++m) {assert(a[m] == changed_array[m]);}
            auto prev_val = read_word<T>(a, sub_reps, start, end);
            update_element_with_fixed_size<T>(changed_array, start, end, prev_val, sub_reps);
//            for (int m = 0; m < sub_reps; ++m) {assert(a[m] == changed_array[m]);}
            assert(read_word<T>(changed_array, sub_reps, start, end) == prev_val);
//            for (int m = 0; m < sub_reps; ++m) {assert(a[m] == changed_array[m]);}

            /*cout << "prev slot is: " << a[start / slot_size] << endl;
            cout << "prev val is: " << prev_val << endl;
            */

            update_element_with_fixed_size<T>(a, start, end, new_val, sub_reps);
            update_element_with_fixed_size<T>(changed_array, start, end, new_val, sub_reps);
            auto res = read_word<T>(a, sub_reps, start, end);

            /*cout << "res is: " << res << endl;
            cout << "new_val is: " << new_val << endl;*/

            if (new_val != res) {
                cout << "Failed.\n" << endl;

                cout << "start is: " << start << "\t start index is :" << start / slot_size << "\t start_bit_pos is :"
                     << start % slot_size << endl;
                cout << "end is: " << start << "\t end index is :" << end / slot_size << "\t end_bit_pos is :"
                     << end % slot_size << endl;

                cout << "a[i] is: " << a[start / slot_size] << endl;
                if ((start / slot_size) < (end / slot_size))
                    cout << "a[i + 1] is: " << a[end / slot_size] << endl;

                cout << "prev_val is:" << prev_val << endl;
                cout << "new_val is:" << new_val << endl;
                cout << "res is:" << res << endl;

                cout << "(" << i << ", " << j << ")" << endl;
                return false;
            }

            update_element_with_fixed_size<T>(a, start, end, prev_val, sub_reps);
//            bool unchanged = true;
            for (int k = 0; k < sub_reps; ++k) {
                if (a[k] != unchanged_array[k]) {
                    print_array_as_integers<T>(a, sub_reps);
//                    print_array_as_integers<T>(a, sub_reps);
                    print_array_as_integers<T>(unchanged_array, sub_reps);

                    cout << "Failed.\n" << endl;

                    cout << "start is: " << start << "\t start index is :" << start / slot_size
                         << "\t start_bit_pos is :" << start % slot_size << endl;
                    cout << "end is: " << start << "\t end index is :" << end / slot_size << "\t end_bit_pos is :"
                         << end % slot_size << endl;

                    cout << "a[i] is: " << a[start / slot_size] << endl;
                    if ((start / slot_size) < (end / slot_size))
                        cout << "a[i + 1] is: " << a[end / slot_size] << endl;

                    cout << "prev_val is:" << prev_val << endl;
                    cout << "new_val is:" << new_val << endl;


                    cout << "First different index : " << k << endl;
                    cout << "a[k]: " << a[k] << endl;
                    cout << "changed_array[k]: " << changed_array[k] << endl;
                    cout << "unchanged_array[k]: " << unchanged_array[k] << endl;

                    if (k + 1 < sub_reps) {
                        cout << "a[k + 1]: " << a[k + 1] << endl;
                        cout << "changed_array[k + 1]: " << changed_array[k + 1] << endl;
                        cout << "unchanged_array[k + 1]: " << unchanged_array[k + 1] << endl;
                    }
                    return false;
                    /*for (int l = k; l < sub_reps; ++l) {
                        if (a[l] == changed_array[l])
                            break;
                        cout << "a[k]: " << a[k] << endl
                        cout << "a[k]: " << a[k] << endl
                    }*/

                }

            }
        }
    }
    return true;
}


/*
template<typename T>
auto v_update_element_single() -> bool{

}*/


template<typename T>
auto v_update_element_single(T *a, size_t a_size, size_t start, size_t end, size_t new_val) -> bool {
    T changed_array[a_size], unchanged_array[a_size];
    for (auto i = 0; i < a_size; ++i) { changed_array[i] = unchanged_array[i] = a[i]; }

    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    size_t total_bits = slot_size * a_size;
    assert(new_val <= MASK(slot_size));
    auto prev_val = read_word<T>(a, a_size, start, end);
    auto new_end = start + get_x_bit_length<T>(new_val);
    auto new_start = start;
    update_element<T>(changed_array, start, end, new_start, new_end, new_val, a_size);
//    assert(v_update_push_helper<T>(changed_array, start, end, new_start, new_end, prev_val, new_val, a_size));

    update_element<T>(a, start, end, new_start, new_end, new_val, a_size);
    auto res = read_word<T>(a, a_size, new_start, new_end);

    if (new_val != res) {
        /*//    auto res2 = extract_symbol<T>(a, a_size, start, new_end + 1);
        //    auto r3 = extract_symbol<T>(a, a_size, start, new_end + 2);
        //    auto r4 = read_word<T>(a, a_size, start, new_end + 3);
        //    auto r5 = read_word<T>(a, a_size, start + 1, new_end + 3);
*/
        string line = string(32, '-') + "Failed 1" + string(32, '-');
        cout << line << endl;
        const auto var_num = 15;
        string var_names[var_num] = {"start", "start_array_index", "start_bit_pos",
                                     "end", "end_array_index", "end_bit_pos",
                                     "new_end", "new_end_array_index", "new_end_bit_pos",
                                     "prev_val", "new_val", "res", "a[0]", "changed_array[0]", "unchanged_array[0]"};
        //, "res2", "r3", "r4", "r5"};

        size_t values[var_num] = {start, start / slot_size, start % slot_size,
                                  end, end / slot_size, end % slot_size,
                                  new_end, new_end / slot_size, new_end % slot_size,
                                  prev_val, new_val, res, a[0], changed_array[0], unchanged_array[0]};
        //, res2, r3, r4, r5};

        table_print(var_num, var_names, values);
        cout << endl;
        /*cout << "start is: " << start << "\t start index is :" << start / slot_size << "\t start_bit_pos is :"
             << start % slot_size << endl;
        cout << "end is: " << end << "\t end index is :" << end / slot_size << "\t end_bit_pos is :"
             << end % slot_size << endl;
        cout << "new_end is: " << new_end << "\t end index is :" << new_end / slot_size
             << "\t new_end_bit_pos is :" << new_end % slot_size << endl;

        cout << "a[i] is: " << a[start / slot_size] << endl;
        if ((start / slot_size) < (end / slot_size))
            cout << "a[i + 1] is: " << a[end / slot_size] << endl;
*/
        /*
//        cout << "prev_val is:" << prev_val << endl;
//        cout << "new_val is:" << new_val << endl;
//        cout << "res is:" << res << endl;

//        cout << "(" << i << ", " << j << ")" << endl;
*/
        return false;
    }

    /*for (int j = 0; j < a_size; ++j) { assert(a[j] == changed_array[j]); }*/

    update_element<T>(a, new_start, new_end, start, end, prev_val, a_size);
    /*auto temp_cond = false;
    for (int j = 0; j < a_size; ++j) {
        if (a[j] != changed_array[j]) {
            temp_cond = true;
            break;
        }
    }
    assert (temp_cond);*/

    for (size_t k = 0; k < a_size - 1; ++k) {
        if (a[k] != unchanged_array[k]) {
            string line2 = string(32, '-') + "Failed 2" + string(32, '-');
            cout << line2 << endl;
            const auto var_num = 16;
            string var_names[var_num] = {"start", "start_array_index", "start_bit_pos",
                                         "end", "end_array_index", "end_bit_pos",
                                         "new_end", "new_end_array_index", "new_end_bit_pos",
                                         "prev_val", "new_val", "res", "k", "a[k]", "changed_array[k]",
                                         "unchanged_array[k]"};
            //, "res2", "r3", "r4", "r5"};

            size_t values[var_num] = {start, start / slot_size, start % slot_size,
                                      end, end / slot_size, end % slot_size,
                                      new_end, new_end / slot_size, new_end % slot_size,
                                      prev_val, new_val, res, k, a[k], changed_array[k], unchanged_array[k]};
            //, res2, r3, r4, r5};

            table_print(var_num, var_names, values);
            cout << endl;

            int bit_length_difference = get_numbers_bit_length_difference<T>(prev_val, new_val);
            cout << bit_length_difference << endl;
            auto c = k + 1;
            bool cond = true;
            while (a[c] != unchanged_array[c]) { c++; }
            print_array_as_integers<T>(&a[k], c - k);
            print_array_as_integers<T>(&unchanged_array[k], c - k);

            print_array_as_consecutive_memory<T>(&a[k], c - k, cout);
            print_array_as_consecutive_memory<T>(&unchanged_array[k], c - k, cout);

            cout << "\nchanged_array:" << endl;
            print_array_as_integers<T>(&changed_array[k], c - k);
            print_array_as_consecutive_memory<T>(&changed_array[k], c - k, cout);
            if (c < a_size - 1) {
                cout << "\nchanged_array plus 1:" << endl;
                print_array_as_integers<T>(&changed_array[k], c - k + 1);
                print_array_as_consecutive_memory<T>(&changed_array[k], c - k + 1, cout);
            }
            cout << "\n\n\n" << endl;

            print_array_as_integers<T>(a, a_size);
            print_array_as_integers<T>(changed_array, a_size);
            print_array_as_integers<T>(unchanged_array, a_size);

            cout << "start is: " << start << "\t start index is :" << start / slot_size
                 << "\t start_bit_pos is :" << start % slot_size << endl;
            cout << "end is: " << end << "\t end index is :" << end / slot_size << "\t end_bit_pos is :"
                 << end % slot_size << endl;
            cout << "new_end is: " << new_end << "\t end index is :" << new_end / slot_size
                 << "\t new_end_bit_pos is :" << new_end % slot_size << endl;


            cout << "a[i] is: " << a[start / slot_size] << endl;
            if ((start / slot_size) < (end / slot_size))
                cout << "a[i + 1] is: " << a[end / slot_size] << endl;

            cout << "prev_val is:" << prev_val << endl;
            cout << "new_val is:" << new_val << endl;


            cout << "First different index : " << k << endl;
            cout << "a[k]: " << a[k] << endl;
            cout << "changed_array[k]: " << changed_array[k] << endl;
            cout << "unchanged_array[k]: " << unchanged_array[k] << endl;

            if (k + 1 < a_size) {
                cout << "a[k + 1]: " << a[k + 1] << endl;
                cout << "changed_array[k + 1]: " << changed_array[k + 1] << endl;
                cout << "unchanged_array[k + 1]: " << unchanged_array[k + 1] << endl;
            }
            return false;

        }
    }

    return true;
}

template<typename T>
auto v_update_element_iter(size_t reps, size_t sub_reps) -> bool {
    T a[sub_reps];
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    auto counter = 0;
    for (auto i = 0; i < reps; ++i) {
        for (auto j = 0; j < sub_reps; ++j) { a[j] = i; }
        //start in [0,64)
        for (size_t start = 0; start < 64; ++start) {
            size_t new_val = 15u + (15u << 8u);
            auto new_val_length = get_x_bit_length<T>(new_val);
            for (int l = 0; l < 8; ++l) {
                for (int prev_length = 1; prev_length < 8; ++prev_length) {
                    size_t end = start + prev_length;
                    bool cond = v_update_element_single<T>(a, sub_reps, start, end, new_val);
                    if (!cond) {
                        cout << "failed" << endl;
                        cout << i << ", " << start << ", " << end << ", " << l << ", " << new_val << endl;
                        v_update_element_single<T>(a, sub_reps, start, end, new_val);
                        return false;
                    }
                    cout << "Counter is: " << counter++ << "\n" + string(32, '$') << endl;
                }
                new_val <<= 1u;
                if (new_val_length > slot_size)
                    break;
            }
            /*//length who determines end loop
            for (size_t length = 1; length <= (slot_size - new_val_length); ++length) {
                size_t end = start + length;
                for (int l = 0; l < 8; ++l) {
                    bool cond = v_update_element_single<T>(a, sub_reps, start, end, new_val);
                    if (!cond) {
                        cout << "failed" << endl;
                        cout << i << ", " << start << ", " << end << ", " << l << ", " << new_val << endl;
                        v_update_element_single<T>(a, sub_reps, start, end, new_val);
                        return false;
                    }
                    new_val <<= 1u;
                }
            }*/
        }
    }

    /*    for (uint32_t shift = 0; shift <= (slot_size - new_val_bit_length); ++shift) {
            for (size_t new_val = 1; new_val < (SL(new_val_bit_length)); ++new_val) {
                new_val <<= shift;

                if (!v_update_element_single<T>(&a, sub_reps, start, end, new_val)))
            }
        }
    }*/
    return true;
}


template<typename T>
auto v_update_element_rand(size_t reps, const size_t sub_reps) -> bool {
    T a[sub_reps], changed_array[sub_reps], unchanged_array[sub_reps];
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    size_t total_bits = slot_size * sub_reps;
    size_t counter = 0;
    for (int i = 0; i < reps; ++i) {
        for (int k = 0; k < sub_reps; ++k) { unchanged_array[k] = changed_array[k] = a[k] = random(); }

        print_array_as_integers<T>(a, sub_reps);
        print_array_as_integers<T>(changed_array, sub_reps);
        print_array_as_integers<T>(unchanged_array, sub_reps);

        print_array_as_consecutive_memory<T>(a, sub_reps, cout);
        print_array_as_consecutive_memory<T>(changed_array, sub_reps, cout);
        print_array_as_consecutive_memory<T>(unchanged_array, sub_reps, cout);

        for (int j = 0; j < sub_reps; ++j) {
            size_t start = random() % (total_bits - 2 * slot_size);
            size_t prev_len = random() % slot_size;
            size_t new_len = random() % slot_size;
            size_t prev_end = start + prev_len;
            size_t new_end = start + new_len;
            size_t new_val = random() & MASK(new_len);

            bool cond = v_update_element_single<T>(a, sub_reps, start, prev_end, new_val);
            if (!cond)
                return false;

            cout << "Counter is: " << counter++ << "\n" + string(32, '$') << endl;

        }
    }
    return true;
/*

            auto prev_val = read_word<T>(a, sub_reps, start, prev_end);
            update_element_att<T>(a, start, prev_end, start, new_end, new_val, sub_reps);
            update_element<T>(changed_array, start, prev_end, start, new_end, new_val, sub_reps);
            auto res = read_word<T>(a, sub_reps, start, new_end);
*/

/*

            if (new_val != res) {
                */
/*
                cout << "Failed.\n" << endl;

                cout << "start is: " << start << "\t start index is :" << start / slot_size << "\t start_bit_pos is :"
                     << start % slot_size << endl;
                cout << "end is: " << end << "\t end index is :" << end / slot_size << "\t end_bit_pos is :"
                     << end % slot_size << endl;
                cout << "new_end is: " << new_end << "\t end index is :" << new_end / slot_size
                     << "\t new_end_bit_pos is :" << new_end % slot_size << endl;

                cout << "a[i] is: " << a[start / slot_size] << endl;
                if ((start / slot_size) < (end / slot_size))
                    cout << "a[i + 1] is: " << a[end / slot_size] << endl;

                cout << "prev_val is:" << prev_val << endl;
                cout << "new_val is:" << new_val << endl;
                cout << "res is:" << res << endl;

                cout << "(" << i << ", " << j << ")" << endl;
*//*

                return false;
            }

            update_element<T>(a, start, new_end, start, prev_end, prev_val, sub_reps);
//            bool unchanged = true;
            for (int k = 0; k < sub_reps; ++k) {
                if (a[k] != unchanged_array[k]) {
                    */
/*cout << a[k] << endl;
                    cout << bit_length_difference << endl;
                    cout << unchanged_array[k] << endl;

                    if (a[k] - 13 == unchanged_array[k]) {
                        cout << "\n\n\n\n\n\n!!!!!!!!!!!!!!!!!!!!! 13 BREAK !!!!!!!!!!!!!!!!!!!!!\n\n\n\n\n\n" << endl;
                        break;
                    }
                    print_array_as_integers<T>(a, sub_reps);
                    print_array_as_integers<T>(changed_array, sub_reps);
                    print_array_as_integers<T>(unchanged_array, sub_reps);

                    cout << "Failed.\n" << endl;
                    cout << "start is: " << start << "\t start index is :" << start / slot_size
                         << "\t start_bit_pos is :" << start % slot_size << endl;
                    cout << "end is: " << end << "\t end index is :" << end / slot_size << "\t end_bit_pos is :"
                         << end % slot_size << endl;
                    cout << "new_end is: " << new_end << "\t end index is :" << new_end / slot_size
                         << "\t new_end_bit_pos is :" << new_end % slot_size << endl;


                    cout << "a[i] is: " << a[start / slot_size] << endl;
                    if ((start / slot_size) < (end / slot_size))
                        cout << "a[i + 1] is: " << a[end / slot_size] << endl;

                    cout << "prev_val is:" << prev_val << endl;
                    cout << "new_val is:" << new_val << endl;


                    cout << "First different index : " << k << endl;
                    cout << "a[k]: " << a[k] << endl;
                    cout << "changed_array[k]: " << changed_array[k] << endl;
                    cout << "unchanged_array[k]: " << unchanged_array[k] << endl;

                    if (k + 1 < sub_reps) {
                        cout << "a[k + 1]: " << a[k + 1] << endl;
                        cout << "changed_array[k + 1]: " << changed_array[k + 1] << endl;
                        cout << "unchanged_array[k + 1]: " << unchanged_array[k + 1] << endl;
                    }*//*

                    return false;
                    */
/*for (int l = k; l < a_size; ++l) {
                        if (a[l] == changed_array[l])
                            break;
                        cout << "a[k]: " << a[k] << endl
                        cout << "a[k]: " << a[k] << endl
                    }*//*


                }
*/

}


auto v_vector_update_single(vector<bool> *vec, size_t prev_start, size_t prev_end, size_t new_start, size_t new_end,
                            size_t new_val) -> bool {

    vector<bool> copy(*vec);
    unsigned long long prev_val = read_T_word_from_vector<unsigned long long>(vec, prev_start, prev_end);
    const size_t n = 6;
    string var_names[n] = {"prev_start", "new_start", "prev_end", "new_end", "prev_val", "new_val"};
    size_t values[n] = {prev_start, new_start, prev_end, new_end, prev_val, new_val};
    table_print(n, var_names, values);

    cout << endl;
    print_bool_vector_no_spaces(vec);

    vector_update_element<size_t>(vec, prev_start, prev_end, new_start, new_end, new_val);
    print_bool_vector_no_spaces(vec);
    vector<bool> updated_copy(*vec);

    auto res = read_T_word_from_vector<unsigned long long>(vec, new_start, new_end);
    if (res != new_val) {
        cout << "Part 1 failed." << endl;
        return false;
    }

    vector_update_element<size_t>(vec, new_start, new_end, prev_start, prev_end, prev_val);

    print_bool_vector_no_spaces(vec);
    cout << endl;
    res = read_T_word_from_vector<unsigned long long>(vec, prev_start, prev_end);
    if (res != prev_val) {
        cout << "Part 2 failed." << endl;
        return false;
    }

    if (*vec != copy) {
        print_bool_vector_no_spaces(vec);
        print_bool_vector_no_spaces(&copy);

        print_bit_vector_as_words(vec);
        print_bit_vector_as_words(&copy);

        cout << "Part 3 failed." << endl;
        return false;
    }
    return true;
}

auto v_vector_update_rand(size_t reps) -> bool {
    vector<bool> vec(256);
    auto size = vec.size();
    auto counter = 0;
    for (int i = 0; i < (size - 128); ++i) { vec[i] = (i / 4) % 2; }

    for (int i = 0; i < reps; ++i) {
        auto start = random() % 64;
        auto prev_length = random() % 63;
        auto new_length = random() % 63;
        auto prev_end = start + prev_length;
        auto new_end = start + new_length;
        auto new_val = random() & MASK(new_length);
        if (!v_vector_update_single(&vec, start, prev_end, start, new_end, new_val)) {
//            cout << "Counter is: " << counter << endl;
            return (false);
        }
        counter++;
    }
    cout << "Counter is: " << counter << endl;
    return true;
}


template<typename T>
auto v_update_by_comparison_single(T *a, size_t a_size, vector<bool> *vec, size_t prev_start, size_t prev_end,
                                   size_t new_start, size_t new_end, size_t new_val) -> bool {

    assert(equality_vec_array<T>(vec, a, a_size, vec->size()) == -1);

    vector<bool> copy(*vec);
    auto prev_val = read_T_word_from_vector<T>(vec, prev_start, prev_end);
    auto prev_val_arr = read_word<T>(a, a_size, prev_start, prev_end);


    const size_t n = 6;
    string var_names[n] = {"prev_start", "new_start", "prev_end", "new_end", "prev_val", "new_val"};
    size_t values[n] = {prev_start, new_start, prev_end, new_end, prev_val, new_val};
    table_print(n, var_names, values);

    cout << endl;
    print_bool_vector_no_spaces(vec);
    print_array_as_consecutive_memory<T>(a, a_size, cout);

    assert(prev_val == prev_val_arr);

    /*const size_t n = 6;
    string var_names[n] = {"prev_start", "new_start", "prev_end", "new_end", "prev_val", "new_val"};
    size_t values[n] = {prev_start, new_start, prev_end, new_end, prev_val, new_val};
    table_print(n, var_names, values);

    cout << endl;
    print_bool_vector_no_spaces(vec);

    vector_update_element<size_t>(vec, prev_start, prev_end, new_start, new_end, new_val);
    print_bool_vector_no_spaces(vec);
    vector<bool> updated_copy(*vec);*/

    assert(equality_vec_array<T>(vec, a, a_size, vec->size()) == -1);
    update_element<T>(a, prev_start, prev_end, new_start, new_end, new_val, a_size);
    vector_update_element<size_t>(vec, prev_start, prev_end, new_start, new_end, new_val);
    assert(equality_vec_array<T>(vec, a, a_size, vec->size()) == -1);

    auto res_arr = read_word<T>(a, a_size, new_start, new_end);
    auto res = read_T_word_from_vector<unsigned long long>(vec, new_start, new_end);
    assert(res == res_arr);
    if (res != new_val) {
        cout << "Part 1 failed." << endl;
        return false;
    }

    assert(equality_vec_array<T>(vec, a, a_size, vec->size()) == -1);
    update_element<T>(a, new_start, new_end, prev_start, prev_end, prev_val, a_size);
    vector_update_element<size_t>(vec, new_start, new_end, prev_start, prev_end, prev_val);
    assert(equality_vec_array<T>(vec, a, a_size, vec->size()) == -1);

//    print_bool_vector_no_spaces(vec);
//    cout << endl;
    res_arr = read_word<T>(a, a_size, prev_start, prev_end);
    res = read_T_word_from_vector<unsigned long long>(vec, prev_start, prev_end);
    assert(res == res_arr);
    if (res != prev_val) {
        cout << "Part 2 failed." << endl;
        return false;
    }

    if (*vec != copy) {
        print_bool_vector_no_spaces(vec);
        print_bool_vector_no_spaces(&copy);

        print_bit_vector_as_words(vec);
        print_bit_vector_as_words(&copy);

        cout << "Part 3 failed." << endl;
        return false;
    }
    return true;

}


template<typename T>
auto v_update_by_comparison_rand(size_t reps, size_t a_size) -> bool {
    vector<bool> vec(256);
    const size_t slot_size = sizeof(T) * CHAR_BIT;
    T a[8];
    auto size = vec.size();
    auto counter = 0;
    for (int i = 0; i < (size - 128); ++i) { vec[i] = (i / 4) % 2; }
    for (int i = 0; i < a_size; ++i) {
        a[i] = read_T_word_from_vector<T>(&vec, i * slot_size, (i + 1) * slot_size);
    }


    for (int i = 0; i < reps; ++i) {
        auto start = random() % 64;
        auto prev_length = random() % slot_size;
        auto new_length = random() % slot_size;
        auto prev_end = start + prev_length;
        auto new_end = start + new_length;
        auto new_val = random() & MASK(new_length);
        if (!v_update_by_comparison_single<T>(a, a_size, &vec, start, prev_end, start, new_end, new_val)) {
            cout << "Counter is: " << counter << endl;
            return (false);
        }
        counter++;
    }
    cout << "Counter is: " << counter << endl;
    return true;
}

template<typename T>
auto v_read_k_words_fixed_length_single(const T *a, size_t a_size, size_t index, size_t element_length, T *res_array,
                                        size_t k) -> bool {
    uint32_t slot_size = sizeof(T) * CHAR_BIT;

    T val_array[k];
    auto bit_start_index = index * element_length;
    for (int i = 0; i < k; ++i) {
        val_array[i] = read_word(a, a_size, bit_start_index, bit_start_index + element_length);
        bit_start_index += element_length;
    }

    read_k_words_fixed_length_att(a, a_size, index, element_length, res_array, k);
    for (int i = 0; i < k; ++i) {
        if (res_array[i] != val_array[i]) {
            cout << i << endl;
            print_array_as_consecutive_memory(a, a_size, cout);
            cout << endl;
            print_array_as_consecutive_memory(res_array, k, cout);
            print_array_as_consecutive_memory(val_array, k, cout);
            cout << endl;
            print_array_as_integers<T>(a, a_size);
            cout << endl;
            print_array_as_integers<T>(res_array, k);
            print_array_as_integers<T>(val_array, k);
            return false;
        }
    }
    return true;
}

template<typename T>
auto v_read_k_words_fixed_length_rand(size_t reps, size_t element_length) -> bool {

    const auto a_size = 16;
    const size_t slot_size = sizeof(T) * CHAR_BIT;
    const size_t element_num = INTEGER_ROUND(a_size * slot_size, element_length);

    T a[a_size];
    auto counter = 0;
    for (int i = 0; i < a_size; ++i) {
        a[i] = random();
    }


    for (int k = 3; k < 10; ++k) {
        T res_array[k];
        assert(element_num >= k);
        for (int i = 0; i < reps; ++i) {
            auto element_index = random() % (element_num - k);
/*
//            auto prev_length = random() % slot_size;
//            auto new_length = random() % slot_size;
//            auto prev_end = start + prev_length;
//            auto new_end = start + new_length;
//            auto new_val = random() & MASK(new_length);
*/
            bool res = v_read_k_words_fixed_length_single<T>(a, a_size, element_index, element_length, res_array, k);
            if (!res) {
                cout << "Counter is: " << counter << endl;
                return (false);
            }
            counter++;
        }
    }
    cout << "Counter is: " << counter << endl;
    return true;
}

template<typename T>
auto v_from_array_to_vector_by_bit_limits_single(const T *a, size_t a_size, size_t abs_bit_start_index,
                                                 size_t abs_bit_end_index) -> bool {
    vector<bool> vec_start;
    vector<bool> vec_mid;
    vector<bool> vec_end;
    vector<bool> merged;

    from_array_to_vector_by_bit_limits(&vec_start, a, 0, abs_bit_start_index);
    assert(vec_start.size() == abs_bit_start_index);
    from_array_to_vector_by_bit_limits(&vec_mid, a, abs_bit_start_index, abs_bit_end_index);
    assert(vec_mid.size() == (abs_bit_end_index - abs_bit_start_index));
    from_array_to_vector_by_bit_limits(&vec_end, a, abs_bit_end_index, a_size * sizeof(T) * CHAR_BIT);
    assert(vec_end.size() == (a_size * sizeof(T) * CHAR_BIT - abs_bit_end_index));

    /*cout << "\nabs_bit_start_index " << abs_bit_start_index << endl;
    cout << "abs_bit_end_index " << abs_bit_end_index << endl;
    cout << "a_size * sizeof(T) * CHAR_BIT " << a_size * sizeof(T) * CHAR_BIT << endl;

    cout << "\nvec_start.size() " << vec_start.size() << endl;
    cout << "vec_mid.size() " << vec_mid.size() << endl;
    cout << "vec_end.size() " << vec_end.size() << endl;*/

    auto size_sum = vec_start.size() + vec_mid.size() + vec_end.size();
    assert (size_sum == a_size * sizeof(T) * CHAR_BIT);

    merged.reserve(vec_start.size() + vec_mid.size() + vec_end.size());
    merged.insert(merged.end(), vec_start.begin(), vec_start.end());
    merged.insert(merged.end(), vec_mid.begin(), vec_mid.end());
    merged.insert(merged.end(), vec_end.begin(), vec_end.end());

    T temp_a[a_size];
    from_bit_vector_to_array_of_words(&merged, temp_a, a_size);

    for (int i = 0; i < a_size; ++i) {
        if (a[i] != temp_a[i]) {
            cout << "first error in " << i << endl;
            return false;
        }
    }
    return true;
}

template<typename T>
auto v_from_array_to_vector_by_bit_limits_rand(size_t reps, size_t a_size, bool to_seed) -> bool {
    if (to_seed) {
        srand(clock());
    }
    T a[a_size];
    size_t total_bits = sizeof(T) * CHAR_BIT;
    size_t counter = 0;
    for (int j = 0; j < reps; ++j) {
        for (int i = 0; i < a_size; ++i) { a[i] = random(); }
        for (int k = 0; k < 64; ++k) {
            size_t abs_bit_start_index = random() % total_bits;
            size_t abs_bit_end_index = random() % total_bits;
            if (abs_bit_start_index > abs_bit_end_index) {
                auto temp = abs_bit_start_index;
                abs_bit_start_index = abs_bit_end_index;
                abs_bit_end_index = temp;
            }
            auto res = v_from_array_to_vector_by_bit_limits_single<T>(a, a_size, abs_bit_start_index,
                                                                      abs_bit_end_index);
            if (!res) {
                cout << "counter is " << counter << endl;
                return false;
            }
            counter++;
        }
    }
    return true;
}


template auto validate_find_first_and_second_set_bits_single<uint32_t>(uint32_t *a, size_t a_size) -> bool;

template auto validate_find_first_and_second_set_bits_iter<uint32_t>(size_t reps, size_t a_size) -> bool;

template auto validate_find_first_and_second_set_bits_rand<uint32_t>(size_t reps, size_t a_size) -> bool;

template auto v_vec_and_array_transformation<uint32_t>(size_t reps, size_t total_bits) -> bool;

template auto v_update_element_with_fixed_size_rand<uint32_t>(size_t reps, const size_t sub_reps) -> bool;


template auto
v_update_element_single<uint32_t>(uint32_t *a, size_t a_size, size_t start, size_t end, size_t new_val) -> bool;

template auto v_update_element_iter<uint32_t>(size_t reps, const size_t sub_reps) -> bool;

template auto v_update_element_rand<uint32_t>(size_t reps, const size_t sub_reps) -> bool;

template auto v_update_by_comparison_rand<uint32_t>(size_t reps, size_t a_size) -> bool;

template
auto v_update_by_comparison_single<uint32_t>(uint32_t *a, size_t a_size, vector<bool> *vec, size_t prev_start,
                                             size_t prev_end, size_t new_start, size_t new_end, size_t new_val) -> bool;


template auto
v_read_k_words_fixed_length_single<uint32_t>(const uint32_t *a, size_t a_size, size_t index, size_t element_length,
                                             uint32_t *res_array, size_t k) -> bool;

template
auto v_read_k_words_fixed_length_rand<uint32_t>(size_t reps, size_t element_length) -> bool;

template
auto v_from_array_to_vector_by_bit_limits_single<uint32_t>(const uint32_t *a, size_t a_size, size_t abs_bit_start_index,
                                                           size_t abs_bit_end_index) -> bool;

template
auto v_from_array_to_vector_by_bit_limits_rand<uint32_t>(size_t reps, size_t a_size, bool to_seed) -> bool;

/*

template<uint32_t>
auto v_update_element_with_fixed_size_rand<uint32_t>(size_t reps, const size_t sub_reps) -> bool;
*/
