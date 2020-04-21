
#include <map>
#include "validate_hash_table.hpp"

auto v_hash_table_rand_gen_load(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                                double max_load_factor, double working_load_factor, double variance) -> bool {
    hash_table<uint32_t> ht(max_capacity, element_length, bucket_size, max_load_factor);

    std::set<uint32_t> my_set;
    std::set<uint32_t> out_set;
    std::set<uint32_t> in_set;

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
    uint32_t x = 190353236;
    if(my_set.find(x) != my_set.end())
        assert(ht.find(x));


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
        (to_insert) ? insertions_counter++ : remove_counter++;
        /*
//        if (to_insert){
//            insertions_counter++;
//        }else{
//            remove_counter++;
//        }
*/

        bool my_set_contain_x = my_set.find(x) != my_set.end();
        bool ht_contains_x = ht.find(x);
        if(my_set_contain_x and !ht_contains_x){
            ht.find(x);
            assert(false);
        }

        single_op(&ht, &my_set, &out_set, &in_set, to_insert);
        my_set_contain_x = my_set.find(x) != my_set.end();
        ht_contains_x = ht.find(x);
        if(my_set_contain_x and !ht_contains_x){
            ht.find(x);
            assert(false);
        }
    }
    cout << "insertions_counter: " << insertions_counter << endl;
    cout << "remove_counter: " << remove_counter << endl;
    ht.get_data();
    return true;
}

auto
v_hash_table_rand(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                  double load_factor) -> bool {

    hash_table<uint32_t> ht(max_capacity, element_length, bucket_size, load_factor);

    std::set<uint32_t> my_set;
    std::set<uint32_t> out_set;
    std::set<uint32_t> in_set;

    size_t counter = 0;
    while (out_set.size() < max_capacity) {
        assert(counter++ < 1E5);
        out_set.insert(out_set.end(), random() & MASK(element_length));
    }

    counter = 0;
    while (my_set.size() < max_capacity / 4) {
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

    for (int i = 0; i < reps; ++i) {

        bool to_insert;
        if (in_set.empty())
            to_insert = true;
        else if (out_set.empty())
            to_insert = false;
        else {
            to_insert = get_prob(in_set.size(), out_set.size());
        }

        single_op(&ht, &my_set, &out_set, &in_set, to_insert);

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


template<typename T>
void single_op(hash_table<T> *ht, set<T> *my_set, set<uint32_t> *out_set, set<uint32_t> *in_set, bool to_insert) {
    set<uint32_t> *source_set;
    set<uint32_t> *dest_set;
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

template<typename T>
auto v_HT_insert(T x, hash_table<T> *ht, set<T> *my_set) -> bool {
    assert(v_HT_find(x, ht, my_set));
    assert(my_set->find(x) == my_set->end());
    ht->insert(x);
    my_set->insert(x);
    assert(v_HT_find(x, ht, my_set));
}

template<typename T>
auto v_HT_remove(T x, hash_table<T> *ht, set<T> *my_set) -> bool {
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
auto v_HT_find(T x, hash_table<T> *ht, set<T> *my_set) -> bool {
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


//auto
//v_hash_table(size_t reps, size_t table_size, size_t max_capacity, size_t element_length, double load_factor) -> bool {
//    return false;
//}

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
