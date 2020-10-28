
#ifndef CLION_CODE_VALIDATE_HASH_TABLE_HPP
#define CLION_CODE_VALIDATE_HASH_TABLE_HPP

#include "../hashTable_Aligned.hpp"
//#include "../packed_spare.hpp"
#include <map>
#include <random>
#include <set>
#include <unordered_set>
//#include "TPD_Filter/att_hTable.hpp"
//
//auto
//v_hash_table_rand(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
//                  double load_factor) -> bool;

//using default_ps = packed_spare<48, 32, 8, 4>;
using default_HTA = hashTable_Aligned<uint64_t, 4>;
using simpler_HTA = hashTable_Aligned<uint64_t, 4, hashing::SimpleMixSplit>;


auto
v_hash_table(size_t reps, size_t table_size, size_t max_capacity, size_t element_length, double load_factor) -> bool;

auto get_prob(size_t x, size_t y) -> bool;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//auto v_HT_find(item_key_t x, default_ps *ht, all_map::umapset_t *my_set) -> bool;
//
//auto v_HT_insert(item_key_t x, default_ps *ht, all_map::umapset_t *my_set) -> bool;
//
//auto v_HT_remove(item_key_t x, default_ps *ht, all_map::umapset_t *my_set) -> bool;
//
//void single_op(default_ps *ht, all_map::umapset_t *my_set, all_map::umapset_t *out_set,
//               all_map::umapset_t *in_set, bool to_insert);
//
//auto v_hash_table_rand(default_ps *packed_s, size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool;


//auto v_hash_table_rand(default_HTA *packed_s, size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool;
//
//auto v_HT_find(item_key_t x, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool;
//
//auto v_HT_insert(item_key_t x, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool;
//
//auto v_HT_remove(item_key_t x, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool;
//
//void single_op(default_ps *ht, all_map::umap_multiset_t *my_set, all_map::umap_multiset_t *out_set,
//               all_map::umap_multiset_t *in_set, bool to_insert);
//
//auto v_hash_table_rand_multi(default_ps *packed_s, size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool;
//
//
//auto v_rand_core(default_ps *ht, size_t reps, size_t max_capacity, double working_load_factor,
//                 double variance = 1) -> bool;
//
//
//template<size_t bucket_capacity = 48, size_t batch_size = 32, size_t bits_per_item = 8, size_t quot_length = 4>
//auto packed_spare_wrapper(size_t op_reps, size_t l1_max_capacity, double l1_load, double l2_load) -> bool {
//
//    /*Compute number of buckets.*/
//    double b = 25 * l1_load;
//    auto l1_bins = (std::size_t) ceil(1.0 * l1_max_capacity / b);
//    default_ps packed_s(l1_bins);
//
//    auto l2_max_capacity = packed_s.getNumberOfBuckets() * bucket_capacity * l2_load;
//    return v_hash_table_rand(&packed_s, op_reps, l2_max_capacity, l2_load);
//}
//
//
//template<size_t bucket_capacity = 48, size_t batch_size = 32, size_t bits_per_item = 8, size_t quot_length = 4>
//auto packed_spare_wrapper_multi(size_t op_reps, size_t l1_max_capacity, double l1_load, double l2_load) -> bool {
//
//    /*Compute number of buckets.*/
//    double b = 25 * l1_load;
//    auto l1_bins = (std::size_t) ceil(1.0 * l1_max_capacity / b);
//    default_ps packed_s(l1_bins);
//
//    auto l2_max_capacity = packed_s.getNumberOfBuckets() * bucket_capacity * l2_load;
//    return v_hash_table_rand_multi(&packed_s, op_reps, l2_max_capacity, l2_load);
//}
//

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
    return true;
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
    return true;
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


template<class HashTable, typename slot_type>
auto v_hash_table_rand(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                       double load_factor) -> bool {

    assert(sizeof(slot_type) * CHAR_BIT >= element_length);
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

        // if (el == 0)
        //     continue;


        if (my_set.find(el) != my_set.end())
            continue;
        out_set.erase(el);
        in_set.insert(el);

        v_HT_insert(el, &ht, &my_set);
    }

    for (size_t i = 0; i < reps; ++i) {
        bool to_insert;
        if (in_set.empty())
            to_insert = true;
        else if (out_set.empty())
            to_insert = false;
        else {
            to_insert = get_prob(in_set.size(), out_set.size());
        }

        single_op<HashTable, slot_type>(&ht, &my_set, &out_set, &in_set, to_insert);


        if (to_insert) {
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
        }
    }
    ht.get_data();
    return true;
}


template<class HashTable, typename slot_type>
auto v_hash_table_rand_gen_load(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                                double max_load_factor, double working_load_factor, double variance = 1) -> bool {
    HashTable ht(max_capacity, element_length, max_load_factor);

    std::unordered_set<slot_type> my_set;
    std::unordered_set<slot_type> out_set;
    std::unordered_set<slot_type> in_set;

    //    std::random_device rd{};
    //    std::mt19937 gen{rd()};
    // std::mt19937 gen{1};

    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    // std::normal_distribution<> d{working_load_factor * max_capacity, variance};


    size_t counter = 0;
    while (out_set.size() < max_capacity) {
        if (counter++ >= 1'000'0000ul) {
            std::cout << "out_set.size(): " << out_set.size() << "/" << max_capacity << std::endl;
            break;
        }
        // assert(counter++ < 1'000'0000ul);
        out_set.insert(out_set.end(), random() & MASK(31));
    }
    counter = 0;

    const auto temp_size = (max_capacity >> 1ul);
    while (my_set.size() < temp_size) {
        if (counter++ >= 1'000'0000ul) {
            std::cout << "my_set.size(): " << my_set.size() << "/" << max_capacity << std::endl;
            std::cout << "out_set.size(): " << out_set.size() << "/" << max_capacity << std::endl;
            break;
        }
        assert(counter++ < 1'000'0000ul);
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
    for (size_t i = 0; i < reps; ++i) {
        bool to_insert;
        if (in_set.empty())
            to_insert = true;
        else if (out_set.empty())
            to_insert = false;
        else {
            to_insert = get_prob(in_set.size(), out_set.size());
            // to_insert = d(gen) > my_set.size();
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


// template<class HashTable, typename slot_type>
// auto v_wrap_test(size_t reps, size_t max_capacity, size_t element_length, double expected_load_factor,
//                  double actual_load_factor, size_t bucket_size = 4) -> bool {
//     HashTable ht(max_capacity, element_length, expected_load_factor, bucket_size);
//     return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, actual_load_factor);
// }

// template<typename slot_type>
// auto v_wrap_test(size_t reps, size_t max_capacity, size_t element_length, double expected_load_factor,
//                  double actual_load_factor, size_t bucket_size = 4) -> bool {
//     //    using slot_type = uint32_t;
//     using HashTable = hashTable_Aligned<slot_type, 4>;
//     HashTable ht(max_capacity, element_length, expected_load_factor);
//     return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, actual_load_factor);
// }

// template<typename bucket_type = uint64_t, size_t bucket_size = 4, typename HashFamily = hashing::TwoIndependentMultiplyShift>
// auto HT_aligned_wrapper(size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool {
//     hashTable_Aligned<bucket_type, bucket_size> temp_ht(l2_max_capacity, 32, l2_load);
//     return v_hash_table_rand(&packed_s, op_reps, l2_max_capacity, l2_load);
// }

// template<class HashTable, typename slot_type>
// auto v_hash_table_rand(HashTable *packed_s, size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool {
//     //incomplete
//     assert(0);
//     // auto pd_index_range = packed_s->getNumberOfBuckets() * 32;
//     // item_key_t lim_item = {pd_index_range, 16, 255};

//     unordered_set<slot_type> my_set;
//     unordered_set<slot_type> out_set;
//     unordered_set<slot_type> in_set;

//     size_t counter = 0;
//     while (out_set.size() < l2_max_capacity) {
//         if (counter++ > 5E7) {
//             break;
//         }
//         out_set.insert(rand());
//     }
//     std::cout << "passed out_set initialization. \tSet size:\t" << out_set.size() << std::endl;

//     counter = 0;
//     while (my_set.size() < l2_max_capacity / 4) {
//         if (counter % 2500 == 0) {
//             std::cout << "counter: " << counter << std::endl;
//             std::cout << "my_set size: " << my_set.size() << std::endl;
//         }
//         if (counter++ > 5E7) {
//             break;
//         }
//         auto index = random() % out_set.size();
//         auto it = std::begin(out_set);
//         std::advance(it, index);
//         auto el = *it;
//         item_key_t el_key = el.first;
//         /*if (el == 0)
//             continue;*/
//         if (my_set.count(el_key) != 0)
//             continue;

//         out_set.erase(el_key);
//         in_set.insert({el_key, true});
//         v_HT_insert(el_key, packed_s, &my_set);
//     }
//     std::cout << "passed my_set init. \tSet size:\t" << my_set.size() << std::endl;

//     for (size_t i = 0; i < op_reps; ++i) {
//         bool to_insert;
//         if (in_set.empty())
//             to_insert = true;
//         else if (out_set.empty())
//             to_insert = false;
//         else {
//             to_insert = get_prob(in_set.size(), out_set.size());
//         }

//         single_op(packed_s, &my_set, &out_set, &in_set, to_insert);

//     }
// //    ht.get_data();
//     return true;
// }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// template<>
// auto v_wrap_test<att_hTable<uint32_t, 4>,uint32_t>(size_t reps, size_t max_capacity, size_t element_length, double load_factor,
//                                                   size_t bucket_size) -> bool {
//    using slot_type = uint32_t;
//    using HashTable = att_hTable<slot_type, 4>;
//    HashTable ht(max_capacity, element_length, load_factor);
//    return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, load_factor);
// }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
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
    return true;
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
    return true;
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
    */
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
/*


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
*/

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

#endif//CLION_CODE_VALIDATE_HASH_TABLE_HPP
