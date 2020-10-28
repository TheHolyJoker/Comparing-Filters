#include "validate_hash_table.hpp"

auto get_prob(size_t x, size_t y) -> bool {
    if (x > y) {
        auto temp = y;
        y = x;
        x = temp;
    }
    float r = static_cast <float> (random()) / static_cast <float> (RAND_MAX);

    assert(x + y);
    // std::cout << "x: " << x << std::endl;
    // std::cout << "y: " << y << std::endl;
    double thres = ((double) (x) / (double) (x + y));
//    cout << "(thres > r)" << (thres > r) << endl;
    return thres > r;

}


//auto v_HT_find(item_key_t x, default_ps *ht, all_map::umapset_t *my_set) -> bool {
//    bool ht_res = ht->find(x);
//    bool my_set_res = my_set->count(x);
//    auto res = (ht_res == my_set_res);
//    if (!res) {
//
//        ht->find(x);
//        std::cout << std::string(80, '*') << std::endl;
//        std::cout << x << std::endl;
//        std::cout << "ht_res:     \t" << ht_res << std::endl;
//        std::cout << "my_set_res: \t" << my_set_res << std::endl;
//        std::cout << std::string(80, '*') << std::endl;
//
//    }
//    return res;
//}
//
//auto v_HT_find(item_key_t x, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool {
//    bool ht_res = ht->find(x);
//    bool my_set_res = my_set->count(x);
//    if (my_set_res)
//        assert(my_set->operator[](x));
//    auto res = (ht_res == my_set_res);
//    return res;
//}
//
//auto v_HT_insert(item_key_t x, default_ps *ht, all_map::umapset_t *my_set) -> bool {
//    assert(v_HT_find(x, ht, my_set));
//    assert(my_set->count(x) == 0);
//
//    ht->insert(x);
//    my_set->insert({x, true});
//    assert(v_HT_find(x, ht, my_set));
//    return true;
//}
//
//auto v_HT_insert(item_key_t x, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool {
//    assert(v_HT_find(x, ht, my_set));
//    // assert(my_set->count(x) == 0);
//
//    ht->insert(x);
//    my_set->operator[](x) += 1;
//    assert(v_HT_find(x, ht, my_set));
//    return true;
//}
//
//auto v_HT_remove(item_key_t x, default_ps *ht, all_map::umapset_t *my_set) -> bool {
//    static int remove_counter = 0;
//    remove_counter++;
//    item_key_t bad_key = {static_cast<uint64_t>(-1), 30, 223};
//    if (x == bad_key) {
////        ht->print_both_joint_bucket_matching_pd_index(x);
//        ht->print_joint_bucket(bad_key.pd_index / 32 + 32);
////        ht->print_both_joint_bucket_matching_pd_index(bad_key.pd_index);
////        ht->print_joint_bucket(bad_key.pd_index / 32)
//    }
//    assert(v_HT_find(x, ht, my_set));
//    assert(my_set->count(x));
//    ht->remove(x);
//    if (x == bad_key)
//        ht->print_joint_bucket(bad_key.pd_index / 32 + 32);
//    my_set->erase(x);
//    assert(my_set->count(x) == 0);
//    if (ht->find(x)) {
//        ht->find(x);
//    }
////    assert(!ht->find(x)); //todo deal with this.
//    assert(v_HT_find(x, ht, my_set));
//    return true;
//}
//
//auto v_HT_remove(item_key_t x, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool {
//    static int remove_counter = 0;
//    remove_counter++;
//    item_key_t bad_key = {static_cast<uint64_t>(-1), 30, 223};
//    assert(v_HT_find(x, ht, my_set));
//    assert(my_set->at(x));
//    ht->remove(x);
//    my_set->operator[](x)--;
//    if (!my_set->at(x)){
//        my_set->erase(x);
//    }
//    assert(v_HT_find(x, ht, my_set));
//    return true;
//}
//
//
//auto validate_pop_item(size_t pd_index, default_ps *ht, all_map::umapset_t *my_set) -> bool {
//    static int pop_counter = 0;
//    pop_counter++;
//    ht->get_pop_element(pd_index);
//    return true;
//}
//
//auto validate_pop_item(size_t pd_index, default_ps *ht, all_map::umap_multiset_t *my_set) -> bool {
//    ht->get_pop_element(pd_index);
//    return true;
//}
//
//
//void single_op(default_ps *ht, all_map::umapset_t *my_set, all_map::umapset_t *out_set,
//               all_map::umapset_t *in_set, bool to_insert) {
//    all_map::umapset_t *source_set;
//    all_map::umapset_t *dest_set;
//    if (to_insert) {
//        source_set = out_set;
//        dest_set = in_set;
//    } else {
//        source_set = in_set;
//        dest_set = out_set;
//    }
//
//    auto index = random() % source_set->size();
//    auto it = std::begin(*source_set);
//    std::advance(it, index);
//    auto el = *it;
//    item_key_t el_key = el.first;
//    /*size_t temp_counter = 0;
//    while ((el == 0) and (temp_counter++ < 1E4)){
//        index = random() % source_set->size();
//        it = std::begin(*source_set);
//        std::advance(it, index);
//        el = *it;
//    }
//    if (temp_counter > 2){
//        std::cout << "T1 many zeros" <<std::endl;
//    }
//    assert(temp_counter < 1E4);*/
//
//    assert(validate_pop_item(el_key.pd_index, ht, my_set));
//    bool is_member = (my_set->count(el_key));
//    assert(v_HT_find(el_key, ht, my_set));
//
//    assert(is_member ^ to_insert);
//    source_set->erase(el_key);
//    dest_set->insert(el);
//    if (to_insert) {
//        v_HT_insert(el_key, ht, my_set);
//    } else {
//        v_HT_remove(el_key, ht, my_set);
//    }
//    assert(v_HT_find(el_key, ht, my_set));
//
//}
//
//void single_op(default_ps *ht, all_map::umap_multiset_t *my_set, all_map::umap_multiset_t *out_set,
//               all_map::umap_multiset_t *in_set, bool to_insert) {
//    all_map::umap_multiset_t *source_set;
//    all_map::umap_multiset_t *dest_set;
//    if (to_insert) {
//        source_set = out_set;
//        dest_set = in_set;
//    } else {
//        source_set = in_set;
//        dest_set = out_set;
//    }
//
//    auto index = random() % source_set->size();
//    auto it = std::begin(*source_set);
//    std::advance(it, index);
//    auto el = *it;
//    item_key_t el_key = el.first;
//
//    assert(validate_pop_item(el_key.pd_index, ht, my_set));
//    assert(v_HT_find(el_key, ht, my_set));
//    assert(source_set->at(el_key));
//    source_set->operator[](el_key)--;
//    if (!source_set->at(el_key)){
//        source_set->erase(el_key);
//    }
//    dest_set->operator[](el_key)++;
//    if (to_insert) {
//        v_HT_insert(el_key, ht, my_set);
//    } else {
//        v_HT_remove(el_key, ht, my_set);
//    }
//    assert(v_HT_find(el_key, ht, my_set));
//
//}
//
//
//auto v_hash_table_rand(default_ps *packed_s, size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool {
//
//    auto pd_index_range = packed_s->getNumberOfBuckets() * 32;
//    item_key_t lim_item = {pd_index_range, 16, 255};
//
//    all_map::umapset_t my_set;
//    all_map::umapset_t out_set;
//    all_map::umapset_t in_set;
//
//    size_t counter = 0;
//    while (out_set.size() < l2_max_capacity) {
//        if (counter++ > 5E7) {
//            break;
//        }
//        const item_key_t temp = rand_item_key_simple(lim_item);
//        auto temp_pair = std::make_pair(temp, true);
//        out_set.insert(temp_pair);
//    }
//    std::cout << "passed out_set initialization. \tSet size:\t" << out_set.size() << std::endl;
//
//    counter = 0;
//    while (my_set.size() < l2_max_capacity / 4) {
//        if (counter % 2500 == 0) {
//            std::cout << "counter: " << counter << std::endl;
//            std::cout << "my_set size: " << my_set.size() << std::endl;
//        }
//        if (counter++ > 5E7) {
//            break;
//        }
//        auto index = random() % out_set.size();
//        auto it = std::begin(out_set);
//        std::advance(it, index);
//        auto el = *it;
//        item_key_t el_key = el.first;
//        /*if (el == 0)
//            continue;*/
//        if (my_set.count(el_key) != 0)
//            continue;
//
//        out_set.erase(el_key);
//        in_set.insert({el_key, true});
//        v_HT_insert(el_key, packed_s, &my_set);
//    }
//    std::cout << "passed my_set init. \tSet size:\t" << my_set.size() << std::endl;
//
//    for (size_t i = 0; i < op_reps; ++i) {
//        bool to_insert;
//        if (in_set.empty())
//            to_insert = true;
//        else if (out_set.empty())
//            to_insert = false;
//        else {
//            to_insert = get_prob(in_set.size(), out_set.size());
//        }
//
//        single_op(packed_s, &my_set, &out_set, &in_set, to_insert);
//
//        /*if (to_insert) {
//            //insert element
//            auto index = random() % out_set.size();
//            auto it = std::begin(out_set);
//            std::advance(it, index);
//            uint32_t el = *it;
//            assert(my_set.find(el) == my_set.end());
//            out_set.erase(el);
//            in_set.insert(el);
//            v_HT_insert(el, &ht, &my_set);
//        } else {
//            //remove element.
//            auto index = random() % in_set.size();
//            auto it = std::begin(in_set);
//            std::advance(it, index);
//            uint32_t el = *it;
//            assert(my_set.find(el) != my_set.end());
//            in_set.erase(el);
//            out_set.insert(el);
//            v_HT_remove(el, &ht, &my_set);
//        }*/
//
//    }
////    ht.get_data();
//    return true;
//}
//
//auto v_hash_table_rand_multi(default_ps *packed_s, size_t op_reps, size_t l2_max_capacity, double l2_load) -> bool {
//    //todo:continue from here!
//    auto pd_index_range = packed_s->getNumberOfBuckets() * 32;
//    item_key_t lim_item = {pd_index_range, 16, 255};
//
//    all_map::umap_multiset_t my_set;
//    all_map::umap_multiset_t out_set;
//    all_map::umap_multiset_t in_set;
//
//    size_t counter = 0;
//    while (out_set.size() < l2_max_capacity) {
//        const item_key_t temp = rand_item_key_simple(lim_item);
//        out_set[temp]++;
//    }
//    std::cout << "passed out_set initialization. \tSet size:\t" << out_set.size() << std::endl;
//
//    counter = 0;
//    while (my_set.size() < l2_max_capacity / 4) {
//        if (counter % 100 == 0) {
//            std::cout << "counter: " << counter << std::endl;
//            std::cout << "my_set size: " << my_set.size() << std::endl;
//        }
//        counter++;
//        auto index = random() % out_set.size();
//        auto it = std::begin(out_set);
//        std::advance(it, index);
//        auto el = *it;
//        item_key_t el_key = el.first;
//
//        assert(out_set[el_key]);
//        out_set[el_key]--;
//        if (!out_set[el_key]){
//            out_set.erase(el_key);
//        }
//        in_set[el_key]++;
//        v_HT_insert(el_key, packed_s, &my_set);
//    }
//    std::cout << "passed my_set init. \tSet size:\t" << my_set.size() << std::endl;
//
//    for (size_t i = 0; i < op_reps; ++i) {
//        bool to_insert;
//        if (in_set.empty())
//            to_insert = true;
//        else if (out_set.empty())
//            to_insert = false;
//        else {
//            to_insert = get_prob(in_set.size(), out_set.size());
//        }
//
//        single_op(packed_s, &my_set, &out_set, &in_set, to_insert);
//
//
//    }
//    return true;
//}
//
//
//auto v_rand_core(default_ps *ht, size_t reps, size_t max_capacity, double working_load_factor,
//                 double variance) -> bool {
//
//    item_key_t lim_item = {ht->number_of_buckets * 32, 16, 255};
//    all_map::umapset_t my_set;
//    all_map::umapset_t out_set;
//    all_map::umapset_t in_set;
//
////    std::random_device rd{};
////    std::mt19937 gen{rd()};
//    std::mt19937 gen{1};
//
//// values near the mean are the most likely
//// standard deviation affects the dispersion of generated values from the mean
//    std::normal_distribution<> d{working_load_factor * max_capacity, variance};
//
//
//    size_t counter = 0;
//    while (out_set.size() < max_capacity) {
//        assert(counter++ < 1E5);
//        const item_key_t temp = rand_item_key_simple(lim_item);
//        auto temp_pair = std::make_pair(temp, true);
//        out_set.insert(temp_pair);
////        out_set.insert(out_set.end(), random() & MASK(element_length));
//    }
//    counter = 0;
//    while (my_set.size() < (max_capacity * working_load_factor)) {
//        assert(counter++ < 1E5);
//        auto index = random() % out_set.size();
//        auto it = std::begin(out_set);
//        std::advance(it, index);
//        auto el_tp = *it;
//        item_key_t el = el_tp.first;
//        if (my_set.count(el))
//            continue;
//
//        out_set.erase(el);
//        in_set.insert({el, true});
//        v_HT_insert(el, ht, &my_set);
//    }
//
//
//    size_t insertions_counter = 0;
//    size_t remove_counter = 0;
//    for (size_t i = 0; i < reps; ++i) {
//        bool to_insert;
//        if (in_set.empty())
//            to_insert = true;
//        else if (out_set.empty())
//            to_insert = false;
//        else {
//            to_insert = d(gen) > my_set.size();
//        }
////        (to_insert) ? insertions_counter++ : remove_counter++;
//        if (to_insert) {
//            insertions_counter++;
//        } else {
//            remove_counter++;
//        }
//        single_op(ht, &my_set, &out_set, &in_set, to_insert);
//
//    }
//    cout << "insertions_counter: " << insertions_counter << endl;
//    cout << "remove_counter: " << remove_counter << endl;
////    ht.get_data();
//    return true;
//}


/*

template<>
auto v_hash_table_rand_gen_load<hashTable_Aligned<uint32_t,4>, uint32_t>(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                                double max_load_factor, double working_load_factor, double variance) -> bool {
    hashTable_Aligned<uint32_t, 4> ht(max_capacity, element_length, max_load_factor);

    std::unordered_set<uint32_t> my_set;
    std::unordered_set<uint32_t> out_set;
    std::unordered_set<uint32_t> in_set;

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
        uint32_t el = *it;
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

        */
/*bool my_set_contain_x = my_set.find(x) != my_set.end();
        bool ht_contains_x = ht.find(x);
        if(my_set_contain_x and !ht_contains_x){
            ht.find(x);
            assert(false);
        }
        my_set_contain_x = my_set.find(x) != my_set.end();
        ht_contains_x = ht.find(x);
        if(my_set_contain_x and !ht_contains_x){
            ht.find(x);
            assert(false);
        }*//*

    }
    cout << "insertions_counter: " << insertions_counter << endl;
    cout << "remove_counter: " << remove_counter << endl;
//    ht.get_data();
    return true;
}

template<>
auto
v_hash_table_rand<hash_table<uint32_t>, uint32_t>(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                  double load_factor) -> bool {

    assert((1ULL << element_length) >= max_capacity);
    hash_table<uint32_t> ht(max_capacity, element_length, load_factor, bucket_size);

    std::unordered_set<uint32_t> my_set;
    std::unordered_set<uint32_t> out_set;
    std::unordered_set<uint32_t> in_set;

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
        uint32_t el = *it;
        */
/*if (el == 0)
            continue;*//*

        if (my_set.find(el) != my_set.end())
            continue;
        out_set.erase(el);
        in_set.insert(el);

        v_HT_insert<hash_table<uint32_t>,uint32_t>(el, &ht, &my_set);
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

        single_op<hash_table<uint32_t>, uint32_t>(&ht, &my_set, &out_set, &in_set, to_insert);

        */
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
        }*//*


    }
    ht.get_data();
    return true;
}

//template<>
auto v_wrap_test(size_t reps, size_t max_capacity, size_t element_length, double load_factor) -> bool {
    using slot_type = uint32_t;
    using HashTable = att_hTable<slot_type, 4>;
    HashTable ht(max_capacity, element_length, load_factor);
    return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, load_factor);
}




//auto
//v_hash_table(size_t reps, size_t table_size, size_t max_capacity, size_t element_length, double load_factor) -> bool {
//    return false;
//}

*/
