
#ifndef CLION_CODE_VALIDATE_HASH_TABLE_HPP
#define CLION_CODE_VALIDATE_HASH_TABLE_HPP

#include <set>
#include <unordered_set>
#include <map>
#include <random>
#include "../hash_table.hpp"
#include "TPD_Filter/att_hTable.hpp"
//
//auto
//v_hash_table_rand(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
//                  double load_factor) -> bool;


auto
v_hash_table(size_t reps, size_t table_size, size_t max_capacity, size_t element_length, double load_factor) -> bool;

auto get_prob(size_t x, size_t y) -> bool;


template<class HashTable, typename slot_type>
auto v_HT_find(slot_type x, HashTable *ht, unordered_set<slot_type> *my_set) -> bool {
//    assert(x);
//    cout << "x is: " << x << endl;
    auto res = (ht->find(x) == (my_set->find(x) != my_set->end()));
    if (!res) {
        ht->find(x);
        cout << "x is: " << x << endl;
        cout << "ht->find(x) " << ht->find(x) << endl;
        cout << "(my_set->find(x) != my_set->end()) " << (my_set->find(x) != my_set->end()) << endl;
        cout << "here" << endl;
    }
    return ht->find(x) == (my_set->find(x) != my_set->end());
}

template<class HashTable, typename slot_type>
auto v_HT_insert(slot_type x, HashTable *ht, unordered_set<slot_type> *my_set) -> bool {
    assert(v_HT_find(x, ht, my_set));
    assert(my_set->find(x) == my_set->end());
    ht->insert(x);
    my_set->insert(x);
    assert(v_HT_find(x, ht, my_set));
}

template<class HashTable, typename slot_type>
auto v_HT_remove(slot_type x, HashTable *ht, unordered_set<slot_type> *my_set) -> bool {
    assert(v_HT_find(x, ht, my_set));
    assert(ht->find(x));
    ht->remove(x);
    my_set->erase(x);
    assert(my_set->find(x) == my_set->end());
    if (ht->find(x)) {
        ht->find(x);
    }
    assert(!ht->find(x));
    assert(v_HT_find(x, ht, my_set));
}

template<class HashTable, typename slot_type>
void single_op(HashTable *ht, unordered_set<slot_type> *my_set, unordered_set<slot_type> *out_set,
               unordered_set<slot_type> *in_set, bool to_insert) {
    std::unordered_set<slot_type> *source_set;
    std::unordered_set<slot_type> *dest_set;
    if (to_insert) {
        source_set = out_set;
        dest_set = in_set;
    } else {
        source_set = in_set;
        dest_set = out_set;
    }

    auto index = random() % source_set->size();
    auto it = std::begin(*source_set);
    std::advance(it, index);
    slot_type el = *it;
    /*size_t temp_counter = 0;
    while ((el == 0) and (temp_counter++ < 1E4)){
        index = random() % source_set->size();
        it = std::begin(*source_set);
        std::advance(it, index);
        el = *it;
    }
    if (temp_counter > 2){
        std::cout << "T1 many zeros" <<std::endl;
    }
    assert(temp_counter < 1E4);*/

    bool is_member = (my_set->find(el) != my_set->end());
    assert(v_HT_find(el, ht, my_set));

    assert(is_member ^ to_insert);
    source_set->erase(el);
    dest_set->insert(el);
    if (to_insert) {
        v_HT_insert(el, ht, my_set);
    } else {
        v_HT_remove(el, ht, my_set);
    }
    assert(v_HT_find(el, ht, my_set));

}


///////////////////////////////////////////////////////////////////////////////

template<class HashTable, typename slot_type>
auto
v_hash_table_rand(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                  double load_factor) -> bool {

    assert (sizeof(slot_type) * CHAR_BIT >= element_length);
    assert((1ULL << element_length) >= max_capacity);
    HashTable ht(max_capacity, element_length, load_factor, bucket_size);

    std::unordered_set<slot_type> my_set;
    std::unordered_set<slot_type> out_set;
    std::unordered_set<slot_type> in_set;

    size_t counter = 0;
    while (out_set.size() < max_capacity) {
        assert(counter++ < 1E5);
        out_set.insert(out_set.end(), random() & MASK(element_length));
    }

    counter = 0;
    while (my_set.size() < max_capacity / 4) {
        assert(counter++ < 1E6);
        auto index = random() % out_set.size();
        auto it = std::begin(out_set);
        std::advance(it, index);
        slot_type el = *it;
        /*if (el == 0)
            continue;*/
        if (my_set.find(el) != my_set.end())
            continue;
        out_set.erase(el);
        in_set.insert(el);

        v_HT_insert(el, &ht, &my_set);
    }

    for (int i = 0; i < reps; ++i) {

        bool to_insert;
        if (in_set.empty())
            to_insert = true;
        else if (out_set.empty())
            to_insert = false;
        else {
            to_insert = get_prob(in_set.size(), out_set.size());
        }

        single_op<HashTable, slot_type>(&ht, &my_set, &out_set, &in_set, to_insert);

        /*if (to_insert) {
            //insert element
            auto index = random() % out_set.size();
            auto it = std::begin(out_set);
            std::advance(it, index);
            uint32_t el = *it;
            assert(my_set.find(el) == my_set.end());
            out_set.erase(el);
            in_set.insert(el);
            v_HT_insert(el, &ht, &my_set);
        } else {
            //remove element.
            auto index = random() % in_set.size();
            auto it = std::begin(in_set);
            std::advance(it, index);
            uint32_t el = *it;
            assert(my_set.find(el) != my_set.end());
            in_set.erase(el);
            out_set.insert(el);
            v_HT_remove(el, &ht, &my_set);
        }*/

    }
    ht.get_data();
    return true;
}

template<class HashTable, typename slot_type>
auto v_hash_table_rand_gen_load(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                                double max_load_factor, double working_load_factor, double variance = 1) -> bool {
    HashTable ht(max_capacity, element_length, max_load_factor, bucket_size);

    std::unordered_set<slot_type> my_set;
    std::unordered_set<slot_type> out_set;
    std::unordered_set<slot_type> in_set;

//    std::random_device rd{};
//    std::mt19937 gen{rd()};
    std::mt19937 gen{1};

// values near the mean are the most likely
// standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<> d{working_load_factor * max_capacity, variance};


    size_t counter = 0;
    while (out_set.size() < max_capacity) {
        assert(counter++ < 1E5);
        out_set.insert(out_set.end(), random() & MASK(element_length));
    }
    counter = 0;
    while (my_set.size() < (max_capacity * working_load_factor)) {
        assert(counter++ < 1E5);
        auto index = random() % out_set.size();
        auto it = std::begin(out_set);
        std::advance(it, index);
        slot_type el = *it;
        if (my_set.find(el) != my_set.end())
            continue;

        out_set.erase(el);
        in_set.insert(el);

        v_HT_insert(el, &ht, &my_set);
    }


    size_t insertions_counter = 0;
    size_t remove_counter = 0;
    for (int i = 0; i < reps; ++i) {
        bool to_insert;
        if (in_set.empty())
            to_insert = true;
        else if (out_set.empty())
            to_insert = false;
        else {
            to_insert = d(gen) > my_set.size();
        }
//        (to_insert) ? insertions_counter++ : remove_counter++;
        if (to_insert) {
            insertions_counter++;
        } else {
            remove_counter++;
        }
        single_op<HashTable, slot_type>(&ht, &my_set, &out_set, &in_set, to_insert);


    }
    cout << "insertions_counter: " << insertions_counter << endl;
    cout << "remove_counter: " << remove_counter << endl;
//    ht.get_data();
    return true;
}

template<class HashTable, typename slot_type>
auto v_rand_core(HashTable *ht, size_t reps, size_t max_capacity, size_t element_length, double working_load_factor,
                 double variance = 1) -> bool {

    std::unordered_set<slot_type> my_set;
    std::unordered_set<slot_type> out_set;
    std::unordered_set<slot_type> in_set;

//    std::random_device rd{};
//    std::mt19937 gen{rd()};
    std::mt19937 gen{1};

// values near the mean are the most likely
// standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<> d{working_load_factor * max_capacity, variance};


    size_t counter = 0;
    while (out_set.size() < max_capacity) {
        assert(counter++ < 1E5);
        out_set.insert(out_set.end(), random() & MASK(element_length));
    }
    counter = 0;
    while (my_set.size() < (max_capacity * working_load_factor)) {
        assert(counter++ < 1E5);
        auto index = random() % out_set.size();
        auto it = std::begin(out_set);
        std::advance(it, index);
        slot_type el = *it;
        if (my_set.find(el) != my_set.end())
            continue;

        out_set.erase(el);
        in_set.insert(el);

        v_HT_insert<HashTable, slot_type>(el, ht, &my_set);
    }


    size_t insertions_counter = 0;
    size_t remove_counter = 0;
    for (int i = 0; i < reps; ++i) {
        bool to_insert;
        if (in_set.empty())
            to_insert = true;
        else if (out_set.empty())
            to_insert = false;
        else {
            to_insert = d(gen) > my_set.size();
        }
//        (to_insert) ? insertions_counter++ : remove_counter++;
        if (to_insert) {
            insertions_counter++;
        } else {
            remove_counter++;
        }
        single_op<HashTable, slot_type>(ht, &my_set, &out_set, &in_set, to_insert);

    }
    cout << "insertions_counter: " << insertions_counter << endl;
    cout << "remove_counter: " << remove_counter << endl;
//    ht.get_data();
    return true;
}


template<class HashTable, typename slot_type>
auto v_wrap_test(size_t reps, size_t max_capacity, size_t element_length, double expected_load_factor,
                 double actual_load_factor, size_t bucket_size = 4) -> bool {
    HashTable ht(max_capacity, element_length, expected_load_factor, bucket_size);
    return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, actual_load_factor);
}

template<typename slot_type>
auto v_wrap_test(size_t reps, size_t max_capacity, size_t element_length, double expected_load_factor,
                 double actual_load_factor, size_t bucket_size = 4) -> bool {
//    using slot_type = uint32_t;
    using HashTable = att_hTable<slot_type, 4>;
    HashTable ht(max_capacity, element_length, expected_load_factor);
    return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, actual_load_factor);
}
//template<>
//auto v_wrap_test<att_hTable<uint32_t, 4>,uint32_t>(size_t reps, size_t max_capacity, size_t element_length, double load_factor,
//                                                   size_t bucket_size) -> bool {
//    using slot_type = uint32_t;
//    using HashTable = att_hTable<slot_type, 4>;
//    HashTable ht(max_capacity, element_length, load_factor);
//    return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, load_factor);
//}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<typename T>
auto v_HT_find(T x, hash_table<T> *ht, unordered_set<T> *my_set) -> bool {
//    assert(x);
//    cout << "x is: " << x << endl;
    auto res = (ht->find(x) == (my_set->find(x) != my_set->end()));
    if (!res) {
        ht->find(x);
        cout << "x is: " << x << endl;
        cout << "ht->find(x) " << ht->find(x) << endl;
        cout << "(my_set->find(x) != my_set->end()) " << (my_set->find(x) != my_set->end()) << endl;
        cout << "here" << endl;
    }
    return ht->find(x) == (my_set->find(x) != my_set->end());
}

template<typename T>
auto v_HT_insert(T x, hash_table<T> *ht, unordered_set<T> *my_set) -> bool {
    assert(v_HT_find(x, ht, my_set));
    assert(my_set->find(x) == my_set->end());
    ht->insert(x);
    my_set->insert(x);
    assert(v_HT_find(x, ht, my_set));
}

template<typename T>
auto v_HT_remove(T x, hash_table<T> *ht, unordered_set<T> *my_set) -> bool {
    assert(v_HT_find(x, ht, my_set));
    assert(ht->find(x));
    ht->remove(x);
    my_set->erase(x);
    assert(my_set->find(x) == my_set->end());
    if (ht->find(x)) {
        ht->find(x);
    }
    assert(!ht->find(x));
    assert(v_HT_find(x, ht, my_set));
}

template<typename T>
void single_op(hash_table<T> *ht, unordered_set<T> *my_set, unordered_set<uint32_t> *out_set,
               unordered_set<uint32_t> *in_set, bool to_insert) {
    std::unordered_set<uint32_t> *source_set;
    std::unordered_set<uint32_t> *dest_set;
    if (to_insert) {
        source_set = out_set;
        dest_set = in_set;
    } else {
        source_set = in_set;
        dest_set = out_set;
    }

    auto index = random() % source_set->size();
    auto it = std::begin(*source_set);
    std::advance(it, index);
    uint32_t el = *it;
    /*size_t temp_counter = 0;
    while ((el == 0) and (temp_counter++ < 1E4)){
        index = random() % source_set->size();
        it = std::begin(*source_set);
        std::advance(it, index);
        el = *it;
    }
    if (temp_counter > 2){
        std::cout << "T1 many zeros" <<std::endl;
    }
    assert(temp_counter < 1E4);*/

    bool is_member = (my_set->find(el) != my_set->end());
    assert(v_HT_find(el, ht, my_set));

    assert(is_member ^ to_insert);
    source_set->erase(el);
    dest_set->insert(el);
    if (to_insert) {
        v_HT_insert(el, ht, my_set);
    } else {
        v_HT_remove(el, ht, my_set);
    }
    assert(v_HT_find(el, ht, my_set));

}

/*



template<typename T>
void single_op(hash_table<T> *ht, set<T> *my_set, set<uint32_t> *out_set, set<uint32_t> *in_set, bool to_insert);


template<typename T>
auto v_HT_insert(T x, hash_table<T> *ht, set<T> *my_set) -> bool;

template<typename T>
auto v_HT_remove(T x, hash_table<T> *ht, set<T> *my_set) -> bool;

template<typename T>
auto v_HT_find(T x, hash_table<T> *ht, set<T> *my_set) -> bool;
*/



//auto

#endif //CLION_CODE_VALIDATE_HASH_TABLE_HPP
