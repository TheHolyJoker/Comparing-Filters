
#include "validate_hash_table.hpp"
template<>
auto v_hash_table_rand_gen_load<hash_table<uint32_t>, uint32_t>(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                                double max_load_factor, double working_load_factor, double variance) -> bool {
    hash_table<uint32_t> ht(max_capacity, element_length, max_load_factor, bucket_size);

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

        v_HT_insert<hash_table<uint32_t>,uint32_t>(el, &ht, &my_set);
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
        }*/
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
        /*if (el == 0)
            continue;*/
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

//template<>
auto v_wrap_test(size_t reps, size_t max_capacity, size_t element_length, double load_factor) -> bool {
    using slot_type = uint32_t;
    using HashTable = att_hTable<slot_type, 4>;
    HashTable ht(max_capacity, element_length, load_factor);
    return v_rand_core<HashTable, slot_type>(&ht, reps, max_capacity, element_length, load_factor);
}


auto get_prob(size_t x, size_t y) -> bool {
    if (x > y) {
        auto temp = y;
        y = x;
        x = temp;
    }
    float r = static_cast <float> (random()) / static_cast <float> (RAND_MAX);

    double thres = ((double) (x) / (double) (x + y));
//    cout << "(thres > r)" << (thres > r) << endl;
    return thres > r;

}


//auto
//v_hash_table(size_t reps, size_t table_size, size_t max_capacity, size_t element_length, double load_factor) -> bool {
//    return false;
//}

