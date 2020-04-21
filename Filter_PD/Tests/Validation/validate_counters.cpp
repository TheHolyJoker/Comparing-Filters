//
// Created by tomer on 3/17/20.
//

#include "validate_counters.hpp"
//#include "my_bit_op.hpp"

template<typename T>
bool v_extract_symbol_single(T *a, size_t a_size, size_t bit_start_index, size_t bit_end_index) {
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    vector<bool> vec(a_size * slot_size);
    from_array_of_words_to_bit_vector(&vec, a, a_size);
    auto v_res = sub_vector_to_word<T>(&vec, bit_start_index, bit_end_index);
    auto c_res = read_word<T>(a, a_size, bit_start_index, bit_end_index);

    if (c_res == v_res) return true;

    auto index = bit_start_index / slot_size;
    printf("start: %zu\nend: %zu\nindex: %lu\na[i]:%u\n", bit_start_index, bit_end_index, index, a[index]);
    if ((bit_start_index / 32) < (bit_end_index / 32)) {
        printf("a[i+1]:%u\n", a[index + 1]);
    }

    printf("\n[%zu,%zu,%lu,%u,%u]\n", bit_start_index, bit_end_index, index, a[index], a[index + 1]);
    printf("counter extract symbol: %u\nvector extract symbol: %u\n", c_res, v_res);

    return false;
}

template<typename T>
bool v_extract_symbol_iter(size_t reps, size_t a_size) {
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

template<typename T>
bool v_extract_symbol_rand(size_t reps, size_t a_size) {
    auto slot_size = (sizeof(T) * CHAR_BIT);
    size_t total_bits = slot_size * a_size;
    T a[a_size];
    for (size_t j = 0; j < reps; ++j) {
        for (size_t i = 0; i < a_size; ++i) { a[i] = random(); }

        /*Easy case "start" and "end" are in the same slot in A.*/
        for (int k = 0; k < a_size * a_size; ++k) {
            size_t start = random() % total_bits;
            size_t max_length = slot_size - (start % slot_size);
            size_t length = random() % max_length;
            assert((start % slot_size) + length < slot_size);
            size_t end = start + length;

            assert((start <= end) and (end <= total_bits) and (start + slot_size >= end));
            if (!v_extract_symbol_single(a, a_size, start, end)) {
                cout << "Case 0:\nFailed in the (" << j << ", " << k << ") rep." << endl;
                return false;
            }
        }

        for (int k = 0; k < a_size * a_size; ++k) {
            size_t start = random() % total_bits;
            size_t max_length = total_bits - start;
            size_t end = start + ((random() % (slot_size + 1))) % max_length;

//            cout << start << "," << end << "," << total_bits << endl;
            assert((start <= end) and (end <= total_bits) and (start + slot_size >= end));
            if (!v_extract_symbol_single(a, a_size, start, end)) {
                cout << "Case 1:\nFailed in the " << j << "'th rep." << endl;
                return false;
            }
        }

        /*Testing where "start" and "end" are not in the same slot in A.*/
        for (int k = 0; k < a_size; ++k) {
            size_t start = random() % (total_bits - slot_size);
            size_t s_rem = start % slot_size;
            size_t bits_to_next_slot = slot_size - s_rem;
            size_t end = start + bits_to_next_slot + (random() % (s_rem + 1));
            assert(start <= end <= total_bits);
            if (!v_extract_symbol_single(a, a_size, start, end)) {
                cout << "Case 2:\nFailed in the " << j << "'th rep." << endl;
                return false;
            }
        }


    }
    return true;

}

auto v_get_symbols_length_difference(size_t reps) -> bool {
    for (int i = 0; i < reps; ++i) {
        uint32_t x = random(), y = random();
        get_numbers_bit_length_difference<uint32_t>(x, y);
    }
    return true;
}

template bool v_extract_symbol_rand<COUNTER_TYPE>(size_t reps, size_t a_size);

