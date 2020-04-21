//
// Created by tomer on 4/10/20.
//

#include "bit_word_converter.hpp"


template<typename T>
void print_vector(const vector<T> *v) {
    std::cout << "<" << v->at(0);
    for (std::size_t i = 1; i < v->size(); ++i) {
        std::cout << ", " << v->at(i);
    }
    std::cout << ">" << std::endl;
}


template<typename T>
void print_array_as_integers(const T *a, std::size_t size) {
    std::cout << "[" << (int) a[0];
    for (std::size_t i = 1; i < size; ++i) {
        std::cout << ", " << (int) a[i];
    }
    std::cout << "]" << std::endl;
}

template<typename T>
auto print_array_as_consecutive_memory(const T *a, std::size_t size, std::ostream &os) -> std::ostream & {
    for (std::size_t i = 0; i < size; ++i) {
        os << my_bin(a[i]);
    }
    os << std::endl;
    return os;
}


template<typename T>
void from_array_of_words_to_bit_vector(vector<bool> *vec, const T *a, size_t a_size) {
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
void from_bit_vector_to_array_of_words(const vector<bool> *v, T *a, size_t a_size) {
    size_t slot_size = sizeof(T) * CHAR_BIT;
    size_t expected_size = INTEGER_ROUND(v->size(), slot_size);
    assert(a_size == expected_size);

    for (size_t i = 0; i < a_size - 1; ++i) {
        ulong b = (1ul << (slot_size - 1));
        for (int j = 0; j < slot_size; ++j) {
            assert(b > 0);
            if (v->at(slot_size * i + j)) {
                a[i] |= b;
            }
            b >>= 1ul;
        }
    }
    ulong b = (1ul << (slot_size - 1));
    for (size_t k = 0; k < v->size() % slot_size; ++k) {
        if (v->at(slot_size * (a_size - 1) + k)) {
            a[a_size - 1] |= b;
        }
        b >>= 1ul;
    }
}

template<typename T>
auto compare_vector_and_array(const vector<bool> *v, const T *a, size_t a_size) -> int {
    size_t slot_size = sizeof(T) * CHAR_BIT;
    size_t number_of_words_to_read = min(a_size, INTEGER_ROUND(v->size(), slot_size));
//    size_t size = min(v->size(), sizeof(a) / sizeof(a[0]));
    for (size_t i = 0; i < number_of_words_to_read; i += slot_size) {
        if (a[i] != read_FP_from_vector_by_index(v, i, slot_size))
            return i;
    }
    return -1;
}

template<typename T>
auto read_T_word_from_vector(const vector<bool> *v, size_t start, size_t end) -> T {
    assert(start <= end);
    assert(end <= v->size());
    size_t length = end - start;
    assert(sizeof(T) * CHAR_BIT >= length);

    T res = 0;
    for (size_t i = start; i < end; ++i) {
        res <<= 1ul;
        res |= v->at(i);
    }
    return res;
}

template<typename T>
void write_val_of_length_k_to_bit_vector(vector<bool> *v, T val, size_t abs_bit_start_index, size_t val_length) {
    assert((abs_bit_start_index + val_length) <= v->size());
    assert(sizeof(T) * CHAR_BIT >= val_length);
    ulong b = SL(val_length - 1u);
    for (size_t i = 0; i < val_length; ++i) {
        v->at(abs_bit_start_index + i) = ((val & b) != 0);
        b >>= 1ul;
    }
}


template<typename T>
void
from_val_vector_to_bit_vector_with_given_word_size(const vector<T> *val_vec, vector<bool> *dest, size_t word_size) {
    dest->resize(word_size * val_vec->size());
    size_t mask = MASK(word_size);
    for (int i = 0; i < val_vec->size(); ++i) {
        T val = val_vec->at(i);
        assert((val & mask) == val);
        write_val_of_length_k_to_bit_vector(dest, val, i * word_size, word_size);
    }
}

//template<typename T>
//void print_T_vector_as_words(const vector<T> *v, size_t word_size) {
//    dest->resize(word_size * val_vec->size());
//    size_t mask = MASK(word_size);
//    for (int i = 0; i < val_vec->size(); ++i) {
//        T val = val_vec->at(i);
//        assert((val & mask) == val);
//        write_val_of_length_k_to_bit_vector(dest, val, i * word_size, word_size);
//    }
//}

template<typename T>
void
from_val_vector_to_bit_vector_representing_PD_header(const vector<T> *val_vec, vector<bool> *dest) {
    for (int i = 0; i < val_vec->size(); ++i) {
        auto val = val_vec->at(i);
        dest->insert(dest->end(), val, true);
        dest->insert(dest->end(), false);
    }
    /*size_t num_of_zeros_to_add = val_vec->size() - dest->size();
    assert(num_of_zeros_to_add > 0);
    dest->insert(dest->end(), num_of_zeros_to_add, 0);
    */
}

template<typename T>
void
from_val_vector_to_bit_vector_representing_PD_header_inverse(const T *arr, size_t abs_bit_size, size_t max_capacity,
                                                             vector<T> *dest) {

    if (!dest->empty()) {
        cout << "dest is not empty" << endl;
    }
//    auto slot_size = sizeof(T) * CHAR_BIT;
    if (abs_bit_size != (max_capacity << 1u)) {
        cout << "abs_bit_size != (max_capacity << 1u): values are " << abs_bit_size << ", " << (max_capacity << 1u)
             << endl;
        assert(false);
    }
    vector<bool> temp_vec;
    from_array_to_vector_by_bit_limits(&temp_vec, arr, 0, abs_bit_size);

    size_t temp_vec_index = 0;
    while (temp_vec_index < temp_vec.size()) {
        if (temp_vec[temp_vec_index]) {
            size_t counter = 1;
            while (temp_vec[temp_vec_index + counter]) {
                counter++;
            }
            dest->insert(dest->end(), counter);
            temp_vec_index += counter;
        } else {
            ++temp_vec_index;
        }
    }

    if (dest->size() != max_capacity){
        cout << "dest->size()=" << dest->size() << endl;
        cout << "max_capacity=" << max_capacity << endl;
    }
    /*
    //    size_t a_size = INTEGER_ROUND(abs_bit_size, slot_size);
    size_t last_on_bit = 0;
    for (int k = 0; k < max_capacity; ++k) {
        auto tp = find_kth_interval_simple(arr, a_size, k);
        auto ones_count = std::get<1>(tp) - std::get<0>(tp);
        dest->insert(dest->end(), ones_count);
        if (k + 1 == max_capacity)
            last_on_bit == std::get<1>(tp) - 1;
    }
    //todo validate every bit after that is zero.*/

}

template<typename T>
auto equality_vec_array(const vector<bool> *v, const T *a, size_t a_size, size_t bits_lim) -> int {
    auto slot_size = sizeof(a[0]) * CHAR_BIT;
    size_t size = min(v->size() / slot_size, a_size);
    size = min(size, bits_lim / slot_size);
    size_t step = slot_size;
    for (size_t i = 0; i < size; ++i) {
        if (a[i] != read_T_word_from_vector<T>(v, i * step, (i + 1) * step)) {
//            print_array_as_consecutive_memory(&a[i],a_size - i);

            cout << "\n\n equality_vec_array error in index: " << i << endl;
            print_array_as_consecutive_memory(a, a_size, cout);
            print_bool_vector_no_spaces(v);
            cout << "\n" << endl;

            return i;
        }
    }
    return -1;
}


void print_bit_vector_as_words(const vector<bool> *v, size_t word_size) {
    size_t number_of_words = INTEGER_ROUND(v->size(), word_size);
    uint32_t a[number_of_words];
    for (size_t j = 0; j < number_of_words; ++j) {
        a[j] = 0;
    }
    from_bit_vector_to_array_of_words(v, a, number_of_words);

    print_array_as_integers(a, number_of_words);
    /*cout << "[" << a[0];
    for (size_t i = 1; i < number_of_words; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;*/
}

template<typename T>
void from_array_to_vector_by_bit_limits(vector<bool> *vec, const T *a, size_t abs_bit_start_index,
                                        size_t abs_bit_end_index) {

    size_t slot_size = sizeof(T) * CHAR_BIT;
    size_t diff = abs_bit_end_index - abs_bit_start_index;

    if (diff == 0)
        return;

    bool is_simple_case = (abs_bit_end_index % slot_size == 0) and (abs_bit_start_index % slot_size == 0);
    if (is_simple_case) {
        assert(diff % slot_size == 0);
        from_array_of_words_to_bit_vector(vec, &(a[abs_bit_start_index / slot_size]), diff / slot_size);
        return;
    }

    size_t A_index_diff = (abs_bit_end_index / slot_size) - (abs_bit_start_index / slot_size);
    bool are_in_the_same_slot = A_index_diff == 0;
    bool are_almost_in_the_same_slot = (A_index_diff == 1) and !(abs_bit_end_index % slot_size);

    if (are_in_the_same_slot or are_almost_in_the_same_slot) {
        size_t rel_bit_start_index = abs_bit_start_index % slot_size;
        size_t b = 1ULL << (slot_size - 1 - rel_bit_start_index);
        size_t vec_index = 0;
        size_t start_A_index = abs_bit_start_index / slot_size;
        for (size_t j = 0; j < diff; ++j) {
            auto val = (b & a[start_A_index]) ? 1 : 0;
            vec->insert(vec->begin() + vec_index, val);
            b >>= 1ULL;
            vec_index++;
        }
        return;
    }


    /*bool skip_mid = false;

    int skip_mid_val_att = (abs_bit_end_index / slot_size) - (abs_bit_start_index / slot_size);
    bool dont_skip_mid_att = (abs_bit_start_index % slot_size) ? skip_mid_val_att > 1 : skip_mid_val_att > 0;

    if (diff < slot_size)
        skip_mid = true;
    else if (diff >= slot_size * 2)
        skip_mid = false;
    else {

        if (abs_bit_start_index % slot_size)
            skip_mid = (abs_bit_end_index / slot_size) > (abs_bit_start_index / slot_size) + 1;
        else {
            skip_mid = (abs_bit_end_index / slot_size) > (abs_bit_start_index / slot_size);
        }
    }
    assert(dont_skip_mid_att != skip_mid);
    if (!skip_mid) {
        size_t mid_start_index, length;
        std::tie(mid_start_index, length) = from_array_to_vector_by_bit_limits_mid(vec, a, abs_bit_start_index,
                                                                                   abs_bit_end_index);
        from_array_to_vector(vec, &(a[mid_start_index]), length);
    }
*/
    size_t mid_start_index, length;
    std::tie(mid_start_index, length) = from_array_to_vector_by_bit_limits_mid(abs_bit_start_index,
                                                                               abs_bit_end_index, sizeof(T) * CHAR_BIT);
    from_array_of_words_to_bit_vector(vec, &(a[mid_start_index]), length);

    if (abs_bit_start_index % slot_size != 0) {
        size_t rel_bit_start_index = abs_bit_start_index % slot_size;
        size_t b = 1ULL << (slot_size - 1 - rel_bit_start_index);
        size_t vec_index = 0;
        size_t start_A_index = abs_bit_start_index / slot_size;
        for (size_t j = rel_bit_start_index; j < slot_size; ++j) {
            auto val = (b & a[start_A_index]) ? 1 : 0;
            vec->insert(vec->begin() + vec_index, val);
            b >>= 1ULL;
            vec_index++;
        }
    }
    if (abs_bit_end_index % slot_size != 0) {
        size_t rel_bit_end_index = abs_bit_end_index % slot_size;
        size_t b = 1ULL << (slot_size - 1);
        size_t end_A_index = abs_bit_end_index / slot_size;
        for (size_t j = 0; j < rel_bit_end_index; ++j) {
            auto val = (b & a[end_A_index]) ? 1 : 0;

            vec->insert(vec->end(), val);
            b >>= 1ULL;
        }
    }


}


auto from_array_to_vector_by_bit_limits_mid(size_t abs_bit_start_index, size_t abs_bit_end_index,
                                            size_t slot_size) -> std::tuple<size_t, size_t> {

    if (abs_bit_start_index % slot_size) {
        abs_bit_start_index = abs_bit_start_index - (abs_bit_start_index % slot_size) + slot_size;
    }
    if (abs_bit_end_index % slot_size) {
        abs_bit_end_index = abs_bit_end_index - (abs_bit_end_index % slot_size);
    }
    assert (abs_bit_start_index % slot_size == 0);
    size_t mid_start_index = abs_bit_start_index / slot_size;
    size_t length = (abs_bit_end_index - abs_bit_start_index) / slot_size;
    auto res = std::make_tuple(mid_start_index, length);
    return res;

}


/*
auto compare_vector_and_array(const vector<bool> *v, const uint8_t *a) -> int {
    assert(false);
    size_t size = min(v->size(), sizeof(a) / sizeof(a[0]));
    size_t step = sizeof(a[0]) * sizeof(char);
    for (size_t i = 0; i < size; i += step) {
        if (a[i] != read_FP_from_vector_by_index(v, i, step)) return i;
    }
    return -1;

}

auto compare_vector_and_array(const vector<bool> *v, const uint32_t *a) -> int {
    size_t size = min(v->size(), sizeof(a) / sizeof(a[0]));
    size_t step = sizeof(a[0]) * sizeof(char);
    for (size_t i = 0; i < size; i += step) {
        if (a[i] != read_FP_from_vector_by_index(v, i, step)) return i;
    }
    return -1;
}

*/

template<typename T>
auto my_bin(T n, size_t length) -> string {
    string s;
    ulong shift = sizeof(T) * CHAR_BIT;
    ulong b = 1ull << (ulong) (32 - 1);
    for (int i = 0; i < 32; ++i) {
        if (b & n)
            s.append("1");
        else
            s.append("0");
        b >>= 1ul;
    }
    return s;
}

void vector_differences_printer(const vector<bool> *valid_vec, const vector<bool> *att_vec) {
    print_bit_vector_as_words(att_vec);
    print_bit_vector_as_words(valid_vec);
    cout << endl;
    print_bool_vector_no_spaces(att_vec);
    print_bool_vector_no_spaces(valid_vec);
    cout << endl;

    cout << "THPS2" << endl;
}

void vector_differences_printer(const vector<bool> *valid_vec, const vector<bool> *att_vec, size_t word_size) {
    print_bit_vector_as_words(att_vec);
    print_bit_vector_as_words(valid_vec);
    cout << endl;

    print_vector_by_unpacking(att_vec, word_size);
    print_vector_by_unpacking(valid_vec, word_size);
    cout << endl;

    print_bool_vector_no_spaces(att_vec);
    print_bool_vector_no_spaces(valid_vec);
    cout << endl;

    cout << "THPS3" << endl;
}

void print_vector_by_unpacking(const vector<bool> *v, size_t unpack_size) {
    size_t a_size = INTEGER_ROUND(v->size(), unpack_size);
    uint32_t a[a_size];
    assert(unpack_size <= 32);
    for (size_t i = 0; i < a_size; ++i) {
        a[i] = read_FP_from_vector_by_index(v, i * unpack_size, unpack_size);
    }
    print_array_as_integers(a, a_size);
}

void print_bool_vector_no_spaces(const vector<bool> *v) {
    for (auto b : *v)
        cout << b;
    cout << endl;
}


void to_vector(vector<bool> *vec, const uint32_t *a, size_t a_size) {
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

void vector_to_word_array(const vector<bool> *v, uint32_t *a, size_t a_size) {
    size_t expected_size = (v->size() / 32) + (v->size() % 32 != 0);
    assert(a_size == expected_size);
    for (size_t i = 0; i < a_size - 1; ++i) {
        ulong b = (1ul << 31ul);
        for (int j = 0; j < 32; ++j) {
            assert(b > 0);
            if (v->at(32 * i + j)) {
                a[i] |= b;
            }
            b >>= 1ul;
        }
    }
    ulong b = (1ul << 31ul);
    for (size_t k = 0; k < v->size() % 32; ++k) {
        if (v->at(32 * (a_size - 1) + k)) {
            a[a_size - 1] |= b;
        }
        b >>= 1ul;
    }
}

auto read_FP_from_vector_by_index(const vector<bool> *v, size_t bit_start_index, size_t fp_size) -> uint32_t {
    assert(bit_start_index + fp_size <= v->size());

//    BODY_BLOCK_TYPE res = v->at(bit_start_index);
    uint32_t res = 0;
    for (size_t i = 0; i < fp_size; ++i) {
        res <<= 1ul;
        res |= v->at(bit_start_index + i);
    }
    return res;
}


void write_FP_to_vector_by_index(vector<bool> *v, size_t index, uint32_t remainder, size_t fp_size) {
    assert(index + fp_size <= v->size());

    ulong b = 1ULL << (fp_size - 1);
    for (size_t i = 0; i < fp_size; ++i) {
        v->at(index + i) = (remainder & b) != 0;
        b >>= 1ul;
    }
}

template<typename T>
void store_header(vector<T> *vec) {

}

template<typename T>
void store_remainders(vector<T> *vec) {

}

template<typename T>
void store_counters(vector<T> *vec) {

}

/***********************************************************************************************************************
Templates
***********************************************************************************************************************/

template string my_bin<uint32_t>(uint32_t n, size_t length);

template void print_vector<uint32_t>(const vector<uint32_t> *v);

template void print_array_as_integers<uint32_t>(const uint32_t *a, size_t size);

template void print_array_as_integers<size_t>(const size_t *a, size_t size);

template auto print_array_as_consecutive_memory<uint32_t>(const uint32_t *a, size_t size, ostream &os) -> ostream &;

template auto print_array_as_consecutive_memory<uint64_t>(const uint64_t *a, size_t size, ostream &os) -> ostream &;

template
auto compare_vector_and_array<uint32_t>(const vector<bool> *v, const uint32_t *a, size_t a_size) -> int;

template
void from_array_of_words_to_bit_vector<uint32_t>(vector<bool> *vec, const uint32_t *a, size_t a_size);

template
void from_array_of_words_to_bit_vector<size_t>(vector<bool> *vec, const size_t *a, size_t a_size);

template
void from_bit_vector_to_array_of_words<uint32_t>(const vector<bool> *v, uint32_t *a, size_t a_size);


template
void from_bit_vector_to_array_of_words<uint64_t>(const vector<bool> *v, uint64_t *a, size_t a_size);


template auto
equality_vec_array<uint32_t>(const vector<bool> *v, const uint32_t *a, size_t a_size, size_t bits_lim) -> int;


template auto
equality_vec_array<uint64_t>(const vector<bool> *v, const uint64_t *a, size_t a_size, size_t bits_lim) -> int;



template void
from_array_to_vector_by_bit_limits<uint32_t>(vector<bool> *vec, const uint32_t *a, size_t abs_bit_start_index,
                                             size_t abs_bit_end_index);


template void
from_array_to_vector_by_bit_limits<uint64_t>(vector<bool> *vec, const uint64_t *a, size_t abs_bit_start_index,
                                             size_t abs_bit_end_index);


template auto
read_T_word_from_vector<unsigned long long>(const vector<bool> *v, size_t start, size_t end) -> unsigned long long;

template auto read_T_word_from_vector<uint32_t>(const vector<bool> *v, size_t start, size_t end) -> uint32_t;

template
void write_val_of_length_k_to_bit_vector<uint32_t>(vector<bool> *v, uint32_t val, size_t abs_bit_start_index,
                                                   size_t val_length);

template void
from_val_vector_to_bit_vector_with_given_word_size<uint32_t>(const vector<uint32_t> *val_vec, vector<bool> *dest,
                                                             size_t word_size);

template void
from_val_vector_to_bit_vector_with_given_word_size<uint64_t>(const vector<uint64_t> *val_vec, vector<bool> *dest,
                                                             size_t word_size);


template void
from_val_vector_to_bit_vector_representing_PD_header<uint32_t>(const vector<uint32_t> *val_vec, vector<bool> *dest);


template void
from_val_vector_to_bit_vector_representing_PD_header<uint64_t>(const vector<uint64_t> *val_vec, vector<bool> *dest);