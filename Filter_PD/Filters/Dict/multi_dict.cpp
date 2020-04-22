//
// Created by tomer on 4/8/20.
//

#include "multi_dict.hpp"

template<class D, class S, typename S_T>
multi_dict<D, S, S_T>::multi_dict(size_t max_number_of_elements, size_t error_power_inv, size_t level1_counter_size,
                                  size_t level2_counter_size, double level1_load_factor, double level2_load_factor):
        capacity(0),
        single_pd_capacity(DEFAULT_CPD_CAPACITY),
        number_of_pd(my_ceil(max_number_of_elements, (size_t) DEFAULT_CPD_CAPACITY)),
        quotient_range(DEFAULT_CPD_QUOTIENT_RANGE),
        remainder_length(error_power_inv),
        pd_index_length(ceil(log2(my_ceil(max_number_of_elements, (size_t) DEFAULT_CPD_CAPACITY)))),
        quotient_length(DEFAULT_CPD_QUOTIENT_LENGTH),
        level1_counter_size(level1_counter_size),
        level2_counter_size(level2_counter_size),
        spare_element_length(remainder_length + pd_index_length + quotient_length),
        sparse_counter_length(level2_counter_size) {

    assert(spare_element_length <= sizeof(D_TYPE) * CHAR_BIT);


    size_t spare_max_capacity = get_multi_spare_max_capacity(max_number_of_elements, level1_load_factor);
    spare = new S(spare_max_capacity, remainder_length + pd_index_length + quotient_length, level2_counter_size,
                  level2_load_factor);
//    spare2 = new multi_hash_table<uint32_t>(spare_max_capacity, remainder_length + pd_index_length + quotient_length, level2_counter_size,
//                   level2_load_factor);
    pd_capacity_vec.resize(number_of_pd);

    for (size_t i = 0; i < number_of_pd; ++i) {
        pd_vec.emplace_back(D(quotient_range, single_pd_capacity, remainder_length, level1_counter_size));
    }
//    get_info();
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::lookup(const string *s) -> bool {
    return lookup_helper(wrap_hash(s));

}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::lookup_helper(S_T hash_val) -> bool {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
    if (pd_vec[pd_index].lookup(quot, r)) return true;

    return spare->find(hash_val & MASK(spare_element_length));
}

template<class D, class S, typename S_T>
template<typename P>
auto multi_dict<D, S, S_T>::lookup_int(const P x) -> bool {
    return lookup_helper(wrap_hash(x));
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::lookup_multi(const string *s) -> size_t {
    return lookup_multi_helper(wrap_hash(s));
//    S_T hash_val = wrap_hash(s);


    /*size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
    auto res = pd_vec[pd_index].lookup_multi(quot, r);
    if (res) {
        if (MD_DB_MODE1)
            assert(res <= MASK(level1_counter_size));
        return res;
    }

    if (MD_DB_MODE1) {
        auto spare_res = spare->find_multiplicities(hash_val & MASK(spare_element_length));
        assert(spare_res <= MASK(level2_counter_size));
        return spare_res;
    }
    return spare->find_multiplicities(hash_val & MASK(spare_element_length));*/
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::lookup_multi_helper(S_T hash_val) -> size_t {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
    auto res = pd_vec[pd_index].lookup_multi(quot, r);
    if (res) {
        if (MD_DB_MODE1)
            assert(res <= MASK(level1_counter_size));
        return res;
    }

    if (MD_DB_MODE1) {
        auto spare_res = spare->find_multiplicities(hash_val & MASK(spare_element_length));
        assert(spare_res <= MASK(level2_counter_size));
        return spare_res;
    }
    return spare->find_multiplicities(hash_val & MASK(spare_element_length));
}

template<class D, class S, typename S_T>
template<typename P>
auto multi_dict<D, S, S_T>::lookup_multi_int(const P x) -> size_t {
    return lookup_multi_helper(wrap_hash(x));
}


template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::insert(const string *s) {
    //todo I think I need to enforce stronger policy when inserting, to make sure an element is not in more than one level at once.
//    auto hash_val = wrap_hash(s);
    return insert_helper(wrap_hash(s));
/*
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);

    if (pd_capacity_vec[pd_index] == single_pd_capacity) {
        insert_full_PD_helper(hash_val, pd_index, quot, r);
        return;
    }

    counter_status op_res = pd_vec[pd_index].insert_inc_attempt(quot, r);
    if (op_res == OK) {
        if (MD_DB_MODE2)
            assert(!spare->find(hash_val));
        return;
    }

    if (op_res == inc_overflow) {
        --pd_capacity_vec[pd_index];
        insert_level1_inc_overflow_handler(hash_val);
        return;
    }

    if (op_res == not_a_member) {
        auto spare_op_res = spare->insert_inc_att(hash_val);
        if (spare_op_res == OK)
            return;
        if (MD_DB_MODE1)
            assert(!spare->find(hash_val));
        pd_vec[pd_index].insert(quot, r);
        ++(pd_capacity_vec[pd_index]);
        return;
    }
    assert(false);
    *//*assert(!spare->find(hash_val));
    counter_status op_res = pd_vec[pd_index].insert(quot, r);
    if (op_res == OK)
        return;
    if (op_res == inc_overflow) {
        --pd_capacity_vec[pd_index];
        insert_level1_inc_overflow_handler(hash_val);
        return;
    }
    if (op_res == not_a_member)
        ++(pd_capacity_vec[pd_index]);*/
}

template<class D, class S, typename S_T>
template<typename P>
void multi_dict<D, S, S_T>::insert_int(const P x) {
    return insert_helper(wrap_hash(x));

}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::insert_helper(S_T hash_val) {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);

    if (pd_capacity_vec[pd_index] == single_pd_capacity) {
        insert_full_PD_helper(hash_val, pd_index, quot, r);
        return;
    }

    counter_status op_res = pd_vec[pd_index].insert_inc_attempt(quot, r);
    if (op_res == OK) {
        if (MD_DB_MODE2)
            assert(!spare->find(hash_val));
        return;
    }

    if (op_res == inc_overflow) {
        --pd_capacity_vec[pd_index];
        insert_level1_inc_overflow_handler(hash_val);
        return;
    }

    if (op_res == not_a_member) {
        auto spare_op_res = spare->insert_inc_att(hash_val);
        if (spare_op_res == OK)
            return;
        if (MD_DB_MODE1)
            assert(!spare->find(hash_val));
        pd_vec[pd_index].insert(quot, r);
        ++(pd_capacity_vec[pd_index]);
        return;
    }
    assert(false);
    /*assert(!spare->find(hash_val));
    counter_status op_res = pd_vec[pd_index].insert(quot, r);
    if (op_res == OK)
        return;
    if (op_res == inc_overflow) {
        --pd_capacity_vec[pd_index];
        insert_level1_inc_overflow_handler(hash_val);
        return;
    }
    if (op_res == not_a_member)
        ++(pd_capacity_vec[pd_index]);*/
}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::insert_full_PD_helper(S_T hash_val, size_t pd_index, uint32_t quot, uint32_t r) {
    if (MD_DB_MODE2)
        assert(pd_vec[pd_index].is_full());

    counter_status op_res = pd_vec[pd_index].insert_inc_attempt(quot, r);

    //"quot, r" is a member, and the incremental succeed.
    if (op_res == OK)
        return;

    //"quot, r" is a member, but the incremental failed.
    if (op_res == inc_overflow) {
        --pd_capacity_vec[pd_index];
        insert_level1_inc_overflow_handler(hash_val);
        return;
    }

        //"quot, r" is not a member
    else if (op_res == not_a_member) {
        spare->insert(hash_val);
        return;
    }

    assert(false);
}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::insert_to_spare(S_T y) {
//    assert(!spare->find(y & MASK(spare_element_length)));
    uint32_t b1 = -1, b2 = -1;
    spare->my_hash(y, &b1, &b2);

    auto att_res = insert_to_bucket_attempt(y, b2);
    if (att_res == OK)
        return;
    if (MD_DB_MODE1)
        assert(att_res != inc_overflow);

//    cout << "Cuckoo hashing start " << sum_pd_capacity() << endl;
    S_T hold = y;
    size_t bucket = b1;
    for (size_t i = 0; i < MAX_CUCKOO_LOOP_MULT; ++i) {
        auto temp_att_res = insert_to_bucket_attempt(hold, bucket, true);
        if (temp_att_res == OK) {
            spare->update_max_cuckoo_insert(i);
            spare->update_cuckoo_insert_counter(i);
            return;
        }
        if (MD_DB_MODE1) {
            assert(temp_att_res != inc_overflow);
            assert(spare->is_bucket_full_by_index(bucket));
        }
        //todo bucket_pop_attempt here.
        spare->cuckoo_swap(&hold, &bucket);
    }
    assert(false);

}


template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::insert_to_bucket_attempt(S_T y, size_t bucket_index) -> counter_status {
    auto tp = insert_inc_to_bucket_attempt(y, bucket_index);
    auto op_res = std::get<0>(tp);
    auto empty_bucket_location_nom = std::get<1>(tp);

    if (op_res == not_a_member) {
        if (empty_bucket_location_nom < spare->get_bucket_size()) {
            size_t table_index = spare->get_bucket_size() * bucket_index + empty_bucket_location_nom;
            spare->set_element(y, table_index);
            return OK;
        }

        //not a member, and no empty slot.
        return not_a_member;
    }
    return op_res;

    /*for (int i = 0; i < spare->get_bucket_size(); ++i) {
        if (spare->is_empty_by_bucket_index_and_location(bucket_index, i)) {
            spare->insert_by_bucket_index_and_location(y, bucket_index, i);
            return true;
        }
        if (single_pop_attempt(spare->get_element_without_counter_by_bucket_index_and_location(bucket_index, i))) {
            spare->insert_by_bucket_index_and_location(y, bucket_index, i);
            return true;
        }
    }
    return false;*/
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::insert_to_bucket_attempt(S_T y, size_t bucket_index, bool pop_attempt) -> counter_status {
    auto tp = insert_inc_to_bucket_attempt(y, bucket_index);
    auto op_res = std::get<0>(tp);
    auto empty_bucket_location_nom = std::get<1>(tp);

    if (op_res == not_a_member) {
        if (empty_bucket_location_nom < spare->get_bucket_size()) {
            size_t table_index = spare->get_bucket_size() * bucket_index + empty_bucket_location_nom;
            spare->set_element(y, table_index);
            return OK;
        }

        size_t table_index = pop_attempt_by_bucket(y, bucket_index);
        if (table_index < spare->get_table_size()) {
            spare->set_element(y, table_index);
            return OK;
        }
        //not a member, and no empty slot.
        return not_a_member;
    }
    return op_res;
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::insert_inc_to_bucket_attempt(S_T y,
                                                         size_t bucket_index) -> std::tuple<counter_status, size_t> {
    bool look_for_empty_location = true;
    size_t empty_location = spare->get_bucket_size();
    auto table_index = spare->get_bucket_size() * bucket_index;
    for (int i = 0; i < spare->get_bucket_size(); ++i) {
        if (look_for_empty_location and spare->is_empty_by_index(table_index + i)) {
            empty_location = i;
            look_for_empty_location = false;
        } else if (spare->is_equal_by_index(table_index + i, y)) {
            auto op_res = spare->increase_counter(table_index + i);
            return std::make_tuple(op_res, spare->get_bucket_size());
        }
    }
    return std::make_tuple(not_a_member, empty_location);

}

/*
template<class D, class S, typename S_T>
void multi_dict<D, S,S_T>::insert_to_spare_with_known_counter(S_T hash_val, size_t counter) {
    uint32_t b1 = -1, b2 = -1;
    spare->my_hash(hash_val, &b1, &b2);

    if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
        return;

    S_T hold = hash_val;
    size_t bucket = b1;
    for (size_t i = 0; i < MAX_CUCKOO_LOOP_MULT; ++i) {
        if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
            spare->update_max_cuckoo_insert(i);
            spare->update_cuckoo_insert_counter(i);
            return;
        }
        assert(spare->is_bucket_full_by_index(bucket));
        spare->cuckoo_swap(&hold, &bucket);
    }

}*/
/*
template<class D, class S, typename S_T>
void multi_dict<D, S,S_T>::insert_to_spare_with_pop(S_T hash_val) {
    uint32_t b1 = -1, b2 = -1;
    spare->my_hash(hash_val, &b1, &b2);

    if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
        return;

    S_T hold = hash_val;
    size_t bucket = b1;
    for (size_t i = 0; i < MAX_CUCKOO_LOOP_MULT; ++i) {
        if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
            spare->update_max_cuckoo_insert(i);
            spare->update_cuckoo_insert_counter(i);
            return;
        }
        assert(spare->is_bucket_full_by_index(bucket));
        spare->cuckoo_swap(&hold, &bucket);
    }

}*/


template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::pop_attempt_with_insertion_by_bucket(S_T hash_val, size_t bucket_index) -> bool {
    //todo continue from here.
    for (int i = 0; i < spare->get_bucket_size(); ++i) {
        if (spare->is_empty_by_bucket_index_and_location(bucket_index, i)) {
            spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
            return true;
        }
        if (single_pop_attempt(spare->get_element_without_counter_by_bucket_index_and_location(bucket_index, i))) {
//            assert(false);
            spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
            return true;
        }
    }
    return false;
}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::insert_level1_inc_overflow_handler(S_T hash_val) {
    assert(false);

    auto temp = spare->add_counter_to_element(hash_val & MASK(spare_element_length), SL(level1_counter_size));
    spare->insert_after_lower_memory_hierarchy_counter_overflow_with_counter(temp);
}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::remove(const string *s) {
    return insert_helper(wrap_hash(s));

//    auto hash_val = wrap_hash(s);

    /*size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
//    bool BPC = (hash_val == 80735794) or (*s == "]AHWVF]NK[X");
    auto op_res = pd_vec[pd_index].conditional_remove(quot, r);
    if (op_res == dec_underflow) {
        --(pd_capacity_vec[pd_index]);
        return;
    }
    if (op_res == not_a_member) {
        auto spare_op_res = spare->remove(hash_val);
        if (MD_DB_MODE0)
            assert((spare_op_res == OK) or (spare_op_res == dec_underflow));
    }
*/

}

template<class D, class S, typename S_T>
template<typename P>
void multi_dict<D, S, S_T>::remove_int(const P x) {
    return remove_helper(wrap_hash(x));

}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::remove_helper(S_T hash_val) {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
//    bool BPC = (hash_val == 80735794) or (*s == "]AHWVF]NK[X");
    auto op_res = pd_vec[pd_index].conditional_remove(quot, r);
    if (op_res == dec_underflow) {
        --(pd_capacity_vec[pd_index]);
        return;
    }
    if (op_res == not_a_member) {
        auto spare_op_res = spare->remove(hash_val);
        if (MD_DB_MODE0)
            assert((spare_op_res == OK) or (spare_op_res == dec_underflow));
    }


}

template<class D, class S, typename S_T>
void multi_dict<D, S, S_T>::get_info() {

    const size_t var_num = 12;
    string names[var_num] = {"capacity", "number_of_pd", "remainder_length", "quotient_range", "single_pd_capacity",
                             "quotient_length", "pd_index_length", "level1_counter_size", "level2_counter_size",
                             "spare_element_length", "sparse_counter_length", "spare_size"};
    size_t values[var_num] = {capacity, number_of_pd, remainder_length, quotient_range, single_pd_capacity,
                              quotient_length, pd_index_length, level1_counter_size, level2_counter_size,
                              spare_element_length, sparse_counter_length, spare->get_table_size()};

//    string columns[2] = {"name", "value"};
    table_print(var_num, names, values);
//    cout << "capacity: " << capacity << " number_of_pd: " << number_of_pd << " remainder_length: "
//         << remainder_length << " quotient_range: " << quotient_range << " single_pd_capacity: "
//         << single_pd_capacity << " quotient_length: " << quotient_length << " pd_index_length: "
//         << pd_index_length << " level1_counter_size: " << level1_counter_size << " level2_counter_size: "
//         << level2_counter_size << " spare_element_length: " << spare_element_length
//         << " sparse_counter_length: " << sparse_counter_length << endl;

}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::sum_pd_capacity() -> size_t {
    if (!MD_DB_MODE0)
        cout << "should not call this function" << endl;

    size_t res = 0;
    for (int i = 0; i < pd_capacity_vec.size(); ++i) {
        if (MD_DB_MODE2)
            assert(pd_vec[i].get_capacity() == pd_capacity_vec[i]);
        res += pd_capacity_vec[i];
    }
    return res;
}


template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::get_PDs_hash_val(const string *s) -> tuple<S_T, S_T, S_T> {
    return wrap_hash_split(s);
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::get_spare_hash_val(
        const string *s) -> tuple<std::tuple<size_t, size_t>, std::tuple<S_T, S_T, S_T>> {
    S_T hash_val = wrap_hash(s);
    auto t1 = spare->get_hash_buckets(hash_val);
    auto t2 = wrap_hash_split(s);
    return std::make_tuple(t1, t2);

}

template<class D, class S, typename S_T>
auto
multi_dict<D, S, S_T>::do_elements_collide(const string *s1, const string *s2) -> bool {
    return wrap_hash(s1) == wrap_hash(s2);
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::pop_attempt(string *s) -> S_T * {
    return nullptr;
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::pop_attempt_by_bucket(S_T y, size_t bucket_index) -> size_t {
    size_t table_index = bucket_index * spare->get_bucket_size();
    for (int i = 0; i < spare->get_bucket_size(); ++i) {
        S_T temp_el, temp_counter;
        std::tie(temp_el, temp_counter) = spare->get_split_element(table_index + i);
        if (MD_DB_MODE1) {
            if (temp_counter > MASK(level1_counter_size)) {
                cout << "counter to big(" << i << ") ";
                continue;
            }
        }
        if (single_pop_attempt(temp_el, temp_counter))
            return table_index + i;
    }
    return spare->get_table_size();
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::single_pop_attempt(S_T element) -> bool {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(element, &pd_index, &quot, &r);
    if (pd_capacity_vec[pd_index] != single_pd_capacity) {
        if (MD_DB_MODE1)
            assert(!pd_vec[pd_index].is_full());

        pd_vec[pd_index].insert(quot, r);
        ++(pd_capacity_vec[pd_index]);
        spare->decrease_capacity();
        return true;
    }
    if (MD_DB_MODE1)
        assert(pd_vec[pd_index].is_full());
    return false;
}

template<class D, class S, typename S_T>
auto multi_dict<D, S, S_T>::single_pop_attempt(S_T temp_el, S_T counter) -> bool {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(temp_el, &pd_index, &quot, &r);
//    cout << "(pd_index, quot, r) (" << pd_index << ", " << quot << ", " << r << ")";
    if (pd_capacity_vec[pd_index] != single_pd_capacity) {
        if (MD_DB_MODE1)
            assert(!pd_vec[pd_index].is_full());

        if (MD_DB_MODE2)
            assert(counter <= MASK(level1_counter_size));
        pd_vec[pd_index].insert_new_element_with_counter(quot, r, counter);
        ++(pd_capacity_vec[pd_index]);
        spare->decrease_capacity();
        return true;
    }
//    cout << " was full" << endl;
    if (MD_DB_MODE1)
        assert(pd_vec[pd_index].is_full());
    return false;
}


static auto get_multi_spare_max_capacity(size_t dict_max_capacity, double level1_load_factor) -> size_t {
    size_t log2_size = ceil(log2(dict_max_capacity));
//    auto res = my_ceil(dict_max_capacity, log2_size * log2_size * log2_size);
    auto res = my_ceil(dict_max_capacity, log2_size * log2_size) << 6u;
    return res;
//    return my_ceil(dict_max_capacity, log2_size * log2_size * log2_size);
}



//
//template <typename S_T>
//class multi_dict<CPD, multi_hash_table<S_T>, S_T>;
//
//template <typename S_T>
//class multi_dict<CPD_validator, multi_hash_table<S_T>, S_T>;

template
class multi_dict<CPD, multi_hash_table<uint32_t>, uint32_t>;


template
class multi_dict<CPD, multi_hash_table<uint64_t>, uint64_t>;
//
//template
//class multi_dict<CPD, multi_hash_table<size_t>, size_t>;


template
class multi_dict<CPD_validator, multi_hash_table<uint32_t>, uint32_t>;

template
class multi_dict<CPD_validator, multi_hash_table<uint64_t>, uint64_t>;


template void multi_dict64::remove_int<uint32_t>(uint32_t x);

template void multi_dict64::insert_int<uint32_t>(uint32_t x);

template auto multi_dict64::lookup_int<uint32_t>(uint32_t x) -> bool;

template auto multi_dict64::lookup_multi_int<uint32_t>(uint32_t x) -> size_t;


//template
//class multi_dict_ST<uint32_t>;
//
//
//template
//class multi_dict_ST<uint32_t>;

