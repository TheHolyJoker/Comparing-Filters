//
// Created by tomer on 4/3/20.
//

#include "hash_table_var.hpp"

template<typename T>
hash_table_var<T>::hash_table_var(const size_t max_capacity, const size_t element_size, const size_t bucket_size,
                                  const double max_load_factor) : max_capacity(max_capacity), element_size(element_size),
                                                          max_load_factor(max_load_factor), capacity(0),
                                                          bucket_size(bucket_size), table_size(
                compute_size(max_capacity, element_size, max_load_factor, sizeof(T) * CHAR_BIT)) {
    assert(sizeof(T) * CHAR_BIT <= element_size);
    table = new T[table_size];
}

template<typename T>
hash_table_var<T>::~hash_table_var() {
    delete[] table;
}

template<typename T>
void hash_table_var<T>::insert(T x) {
    if (capacity >= max_capacity) {
        cout << "Trying to insert into fully loaded hash table" << endl;
        assert(false);
    }


}

template<typename T>
void hash_table_var<T>::pop_attempt(T x) {

}

template<typename T>
void hash_table_var<T>::remove(T x) {

}

template<typename T>
auto hash_table_var<T>::find(T x) {
    uint32_t i1 = -1, i2 = -1;
    hash2_choice<T, uint32_t>(x, &i1, &i2);
    const size_t bucket_num = table_size / bucket_size;
    size_t b1 = (i1) % (bucket_num);
    if (find_helper(x, b1)) return true;

    size_t b2 = (i2) % (bucket_num);
    return find_helper(x, b2);

}

template<typename T>
auto hash_table_var<T>::find_helper(T x, size_t bucket_index) -> bool {
    T el_array[bucket_size];
    read_k_words_fixed_length_att<T>(table, table_size, bucket_index, element_size, el_array, bucket_size);
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(x, el_array[i]))
            return true;
    }
    return false;
}

template<typename T>
auto hash_table_var<T>::get_bucket_capacity(size_t bucket_index) {
    return capacity;
}


auto compute_size(const size_t max_capacity, const size_t element_size, const double max_load_factor,
                  const size_t slot_size) -> size_t {
    size_t total_slots_num = std::ceil((double) (max_capacity) / max_load_factor);
    auto total_bits = total_slots_num * element_size;
    return INTEGER_ROUND(total_bits, slot_size);
}
