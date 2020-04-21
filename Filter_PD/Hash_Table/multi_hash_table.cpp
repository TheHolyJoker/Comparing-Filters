//
// Created by tomer on 4/7/20.
//

#include "multi_hash_table.hpp"


template<typename T>
multi_hash_table<T>::multi_hash_table(size_t max_distinct_capacity, size_t element_size, size_t counter_size,
                                      double max_load_factor, size_t bucket_size):
        max_distinct_capacity(max_distinct_capacity), element_size(element_size), bucket_size(bucket_size),
        max_load_factor(max_load_factor), capacity(0), distinct_capacity(0),
        table_size(std::ceil(((double) max_distinct_capacity) / max_load_factor)),
        max_cuckoo_insert(0), cuckoo_insert_counter(0), max_capacity_reached(0), counter_size(counter_size) {
    table = new T[table_size];
//    cout << "table_size is: " << table_size << endl;
    for (int i = 0; i < table_size; ++i) {
        table[i] = 0;
    }
}


template<typename T>
auto multi_hash_table<T>::find(const T x) -> bool {
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    return (find_in_bucket(x, b2) || find_in_bucket(x, b1));

}


template<typename T>
auto multi_hash_table<T>::find_element_table_index(const T x) -> size_t {
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    //todo find index and look for empty slot at the same time.
    auto res = find_element_location_in_bucket(x, b2);
    if (res != bucket_size)
        return b2 * bucket_size + res;
    res = find_element_location_in_bucket(x, b1);
    if (res != bucket_size)
        return b1 * bucket_size + res;
    return table_size;

}

template<typename T>
auto multi_hash_table<T>::find_element_location_in_bucket(const T x, size_t bucket_index) -> size_t {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x))
            return i;
    }
    return bucket_size;
}


template<typename T>
auto multi_hash_table<T>::find_multiplicities(const T x) -> uint32_t {
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    auto res = find_multiplicity_in_bucket(x, b2);
    return (res) ? res : find_multiplicity_in_bucket(x, b1);

}

template<typename T>
auto multi_hash_table<T>::insert(const T x) -> counter_status {
    auto op_res = insert_inc_att(x);
    if (op_res == OK)
        return OK;
    if (MHT_DB_MODE1)
        assert(op_res != inc_overflow);

    insert_new(add_counter_to_new_element(x));
    return OK;
}

template<typename T>
auto multi_hash_table<T>::insert_inc_att(T x) -> counter_status {
    auto index = find_element_table_index(x);
    if (index != table_size)
        return increase_counter(index);

    return not_a_member;
}

template<typename T>
void multi_hash_table<T>::insert_new(const T y) {
//    assert((x & MASK(element_size)) == x);

    if (distinct_capacity >= max_distinct_capacity) {
        cout << "Trying to insert into fully loaded hash table " << endl;
        assert(false);
    }
    distinct_capacity++;
//    capacity++;
    /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

    max_capacity_reached = (max_capacity_reached >= distinct_capacity) ? max_capacity_reached : distinct_capacity;
    uint32_t b1 = -1, b2 = -1;

    my_hash(y & MASK(element_size), &b1, &b2);

    if (insert_element_with_counter_if_bucket_not_full(y, b2)) return;

    T hold = y;
    size_t bucket_index = b1;
    for (int i = 0; i < MAX_CUCKOO_LOOP_MULT; ++i) {
        if (insert_element_with_counter_if_bucket_not_full(hold, bucket_index)) {
            max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
            this->cuckoo_insert_counter += (bucket_size * i);
            return;
        }
        cuckoo_swap(&hold, &bucket_index);
    }

    assert(false);
}

template<typename T>
void multi_hash_table<T>::insert_after_lower_memory_hierarchy_counter_overflow_with_counter(T y) {
    if (MHT_DB_MODE1) {
        T x = y & MASK(element_size);
        assert(!find(x));
    }
    insert_new(y);
}

//template<typename T>
//auto multi_hash_table<T>::insert_inc_attempt(T x) -> int {
//    auto index = find_element_table_index(x);
//    if (index != table_size) {
//        return increase_counter(index);
//    }
//}

template<typename T>
auto multi_hash_table<T>::insert_new_element_if_bucket_not_full(const T x, size_t bucket_index) -> bool {
    if (MHT_DB_MODE1)
        assert((x & MASK(element_size)) == x);
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_empty_by_index(table_index + i)) {
            store_new_element(x, table_index + i);
            return true;
        }
        /*Add pop attempt*/
    }
    return false;
}

template<typename T>
auto multi_hash_table<T>::insert_element_with_counter_if_bucket_not_full(T y, size_t bucket_index) -> bool {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_empty_by_index(table_index + i)) {
            table[table_index + i] = y;
            return true;
        }
    }
    return false;
}


template<typename T>
void multi_hash_table<T>::insert_by_bucket_index_and_location(const T x, size_t bucket_index, size_t location) {
    cout << "4.11 15:21! " << endl;
}


template<typename T>
void multi_hash_table<T>::insert_new_element_without_counter_by_index(T x, size_t table_index) {
    T y = add_counter_to_new_element(x);
    set_element(y, table_index);
}

template<typename T>
void multi_hash_table<T>::insert_new_element_with_counter_by_index(T y, size_t table_index) {
    set_element(y, table_index);
}

template<typename T>
auto multi_hash_table<T>::remove(const T x) -> counter_status {
    if (MHT_DB_MODE2)
        assert(find(x));
    if (distinct_capacity == 0) {
        cout << "Trying to delete from empty hash table" << endl;
//        assert(false);
    }
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    auto res = remove_helper(x, b1);
    if (res == not_a_member)
        return remove_helper(x, b2);

    return res;
}

template<typename T>
auto multi_hash_table<T>::remove_helper(const T x, size_t bucket_index) -> counter_status {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x)) {
            return decrease_counter(table_index + i);
/*
            auto prev_val = table[table_index + i];
            table[table_index + i] = 0;
            if (find(x)) {
//                auto res = find_table_location(x);
                find(x);
                assert(false);
            }
            return true;
*/
        }
    }
    return not_a_member;
}

template<typename T>
void multi_hash_table<T>::cuckoo_swap(T *hold, size_t *bucket_index) {
    auto rand_table_index = (*bucket_index * bucket_size) + (random() % bucket_size);

    //todo swap here.
    auto temp = table[rand_table_index];
    table[rand_table_index] = *hold;
    *hold = temp;

    uint32_t temp_b1 = -1, temp_b2 = -1;
    my_hash((*hold) & MASK(element_size), &temp_b1, &temp_b2);

    if (MHT_DB_MODE2)
        assert(temp_b2 != temp_b1);

    if (temp_b1 == *bucket_index)
        *bucket_index = temp_b2;
    else if (temp_b2 == *bucket_index)
        *bucket_index = temp_b1;
    else
        assert(false);

}

template<typename T>
auto multi_hash_table<T>::get_element_without_counter_by_bucket_index_and_location(size_t bucket_index,
                                                                                   size_t location) -> T {
    cout << "Sat 11.4 15:25" << endl;
    assert(false);
//    return nullptr;
}

template<typename T>
auto
multi_hash_table<T>::get_element_with_counter_by_bucket_index_and_location(size_t bucket_index, size_t location) -> T {
    cout << "Sat 11.4 15:25" << endl;
    assert(false);
//    return nullptr;
}

template<typename T>
auto multi_hash_table<T>::get_counter_by_table_index(size_t table_index) -> size_t {
    if (MHT_DB_MODE1)
        assert(is_occupied(table_index));
    auto res = table[table_index] >> element_size;
    if (MHT_DB_MODE1)
        assert (res <= MASK(counter_size));
    return table[table_index] >> element_size;
}

template<typename T>
auto multi_hash_table<T>::is_equal(T with_counter, T without_counter) -> bool {
    //todo assert((without_counter & MASK(element_size) == without_counter);
    return (with_counter & MASK(element_size)) == (without_counter & MASK(element_size));
}

template<typename T>
auto multi_hash_table<T>::is_equal_by_index(size_t table_index, T without_counter) -> bool {
    return is_equal(table[table_index], without_counter);
}

template<typename T>
auto
multi_hash_table<T>::is_equal_by_bucket_and_location(size_t bucket_index, size_t location, T without_counter) -> bool {
    return is_equal_by_index(bucket_index * bucket_index + location, without_counter);
}

template<typename T>
auto multi_hash_table<T>::if_equal_return_multiplicity_by_table_index(T without_counter, size_t table_index) -> bool {
    return ((table[table_index] & MASK(element_size)) == without_counter) ? table[table_index] >> element_size : 0;
}

template<typename T>
auto multi_hash_table<T>::is_occupied(size_t table_index) -> bool {
    if (MHT_DB_MODE2) {
        T slot = table[table_index];
        T element = slot & MASK(element_size);
        T counter = slot >> element_size;
        if (!counter)
            assert(!element);
    }
    return ((table[table_index] >> element_size) != 0);
}

template<typename T>
auto multi_hash_table<T>::is_empty_by_index(size_t table_index) -> bool {
    return !is_occupied(table_index);
}

template<typename T>
auto multi_hash_table<T>::is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool {
    return is_empty_by_index(bucket_index * bucket_size + location);
}

template<typename T>
auto multi_hash_table<T>::is_bucket_full_by_index(size_t bucket_index) -> bool {
    return get_bucket_capacity(bucket_index) == bucket_size;
}

template<typename T>
auto multi_hash_table<T>::find_in_bucket(T x, size_t bucket_index) -> bool {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x))
            return true;
    }
    return false;
}

template<typename T>
auto multi_hash_table<T>::find_multiplicity_in_bucket(T x, size_t bucket_index) -> size_t {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x))
            return get_counter_by_table_index(table_index + i);
    }
    return false;
}

template<typename T>
auto multi_hash_table<T>::get_bucket_capacity(size_t bucket_index) -> size_t {
    auto table_index = bucket_index * bucket_size;
    size_t counter = 0;
    for (int i = 0; i < bucket_size; ++i) {
        if (!is_empty_by_index(table_index + i))
            counter++;
    }
    return counter;
}

template<typename T>
auto multi_hash_table<T>::get_bucket_size() -> size_t {
    return bucket_size;
}

template<typename T>
auto multi_hash_table<T>::get_table_size() -> size_t {
    return table_size;
}

template<typename T>
void multi_hash_table<T>::increase_capacity() {
    ++distinct_capacity;
}

template<typename T>
void multi_hash_table<T>::decrease_capacity() {
    --distinct_capacity;
}

template<typename T>
auto multi_hash_table<T>::increase_counter(size_t table_index) -> counter_status {
    T counter = table[table_index] >> element_size;
    if (counter == MASK(counter_size)) {
        assert(false);
        //todo: what happens when an element multiplicity is too big for the hash_table?
        // Adding it as a new element can cause:
        // 1) Cuckoo chain problems.
        // 2) Under-estimate on multiplicity (or possibly longer lookup - by at most factor 2.)
        // 3) a merge operator should be defined. (also reduce the counter of the smaller element).
        // Different solution: Ignoring this insertion.
        return inc_overflow;
    }
    auto prev_val = table[table_index];
    auto new_val = ((counter + 1) << element_size) | (table_index & MASK(element_size));
    table[table_index] = ((counter + 1) << element_size) | (table_index & MASK(element_size));
    if (MHT_DB_MODE1)
        assert(is_occupied(table_index));
    return OK;
}

template<typename T>
auto multi_hash_table<T>::decrease_counter(size_t table_index) -> counter_status {
    T counter = table[table_index] >> element_size;
    if (counter == 1) {
        //todo: Maybe the element needs to drop one level, and therefore should not be deleted.
        table[table_index] = 0;
        --distinct_capacity;
        return dec_underflow;
    }

    table[table_index] = ((--counter) << element_size) | (table_index & MASK(element_size));
    if (MHT_DB_MODE1)
        assert(is_occupied(table_index));
    return OK;

}

template<typename T>
void multi_hash_table<T>::store_new_element(const T x, size_t table_index) {
    distinct_capacity++;
    table[table_index] = x | SL(element_size);
    if (MHT_DB_MODE1)
        assert(is_occupied(table_index));


}

template<typename T>
auto multi_hash_table<T>::add_counter_to_new_element(T x) -> T {
    if (MHT_DB_MODE1) {
        assert((x & MASK(element_size)) == x);
        assert(SL(element_size) > x);
    }
    return x | SL(element_size);
}

template<typename T>
auto multi_hash_table<T>::add_counter_to_element(T x, T counter) -> T {
    if (MHT_DB_MODE2) {
        assert((x & MASK(element_size)) == x);
        unsigned long long temp = x | (counter << element_size);
        unsigned long long slot_size = sizeof(T) * CHAR_BIT;
        assert (temp <= MASK(slot_size));
    }
    return x | (counter << element_size);
}


template<typename T>
void multi_hash_table<T>::update_max_cuckoo_insert(size_t i) {
    max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
}

template<typename T>
void multi_hash_table<T>::update_cuckoo_insert_counter(size_t cuckoo_chain_length) {
    cuckoo_insert_counter += cuckoo_chain_length * bucket_size;
}

template<typename T>
multi_hash_table<T>::~multi_hash_table() {
    delete[] table;
}

template<typename T>
auto multi_hash_table<T>::get_element(size_t table_index) -> T {
    return table[table_index];
}

template<typename T>
auto multi_hash_table<T>::get_split_element(size_t table_index) -> tuple<T, T> {
    auto element = get_element(table_index);
    T mask = MASK(element_size);
    T val = element & mask;
    T counter = (element & ~mask) >> element_size;
    if (MHT_DB_MODE1)
        assert(counter > 0);
    return tuple<T, T>(val, counter);
}

template<typename T>
void multi_hash_table<T>::set_element(T x, size_t table_index) {
    table[table_index] = x;
    if (MHT_DB_MODE2)
        assert(is_occupied(table_index));

}

template<typename T>
void multi_hash_table<T>::set_element(T x, size_t counter, size_t table_index) {
    T y = add_counter_to_element(x, counter);
    table[table_index] = y;
}

template<typename T>
auto multi_hash_table<T>::get_hash_buckets(T x) -> std::tuple<size_t, size_t> {
    x &= MASK(element_size);
    size_t number_of_buckets_in_each_table = (table_size / bucket_size) / 2;
    size_t b1 = (hashint(x)) % number_of_buckets_in_each_table;
    size_t b2 = (hashint2(x) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
    return std::make_tuple(b1, b2);
}


template
class multi_hash_table<uint32_t>;

template
class multi_hash_table<size_t>;
