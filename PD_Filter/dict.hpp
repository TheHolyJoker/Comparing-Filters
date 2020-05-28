
#ifndef CLION_CODE_DICT_HPP
#define CLION_CODE_DICT_HPP

#include <cstring>
#include "PD.h"
#include "hash_table.hpp"

#define DICT_DB_MODE0 (false)
#define DICT_DB_MODE1 (true & DICT_DB_MODE0)
#define DICT_DB_MODE2 (true & DICT_DB_MODE1)


template<typename T>
auto my_ceil(T x, T y) -> T {
    return (x + y - 1) / y;
}


//template<class PDType, template<typename> class spareType, typename itemType, size_t bits_per_item>
template<class PDType, template<typename> class spareType, typename itemType,  typename spareItemType>
class dict {
    vector<PDType *> pd_vec;
    vector<uint> pd_capacity_vec;
    spareType<spareItemType> *spare;

    size_t capacity;
//    const size_t number_of_pd, remainder_length{bits_per_item}, quotient_range, single_pd_capacity;
    const size_t number_of_pd, remainder_length, quotient_range, single_pd_capacity;
    const size_t quotient_length, pd_index_length;
    const size_t sparse_element_length, sparse_counter_length;

public:
    dict(size_t max_number_of_elements, size_t error_power_inv, double level1_load_factor, double level2_load_factor) :
            capacity(0),
            single_pd_capacity(DEFAULT_PD_CAPACITY),
            number_of_pd(my_ceil(max_number_of_elements, (size_t) DEFAULT_PD_CAPACITY)),
            quotient_range(DEFAULT_QUOTIENT_RANGE),
            remainder_length(error_power_inv),
            pd_index_length(ceil(log2(my_ceil(max_number_of_elements, (size_t) DEFAULT_PD_CAPACITY)))),
            quotient_length(DEFAULT_QUOTIENT_LENGTH),
            sparse_element_length(remainder_length + pd_index_length + quotient_length),
            sparse_counter_length(sizeof(D_TYPE) * CHAR_BIT - sparse_element_length)
//        spare(get_spare_max_capacity(max_number_of_elements, level1_load_factor),
//              remainder_length + pd_index_length + quotient_length, level2_load_factor)
    {

        assert(sparse_element_length <= sizeof(D_TYPE) * CHAR_BIT);

        size_t log2_size = ceil(log2(max_number_of_elements));
        auto res = my_ceil(max_number_of_elements, log2_size * log2_size) << 6u;

        size_t spare_max_capacity = res;
        spare = new spareType<spareItemType>(spare_max_capacity, sparse_element_length, level2_load_factor);

        pd_capacity_vec.resize(number_of_pd);

        for (size_t i = 0; i < number_of_pd; ++i) {
            PDType *temp = new PDType(quotient_range, single_pd_capacity, remainder_length);
            pd_vec.push_back(temp);
        }
//    get_info();
    }


    virtual ~dict() {
        for (int i = 0; i < pd_vec.size(); ++i) {
            delete pd_vec[i];
        }
        pd_vec.clear();

        delete spare;
    }


    auto lookup(const itemType s) const -> bool {
        return lookup_helper(wrap_hash(s));
    }

    void insert(const itemType s) {
        return insert_helper(wrap_hash(s));
    }

    void remove(itemType x) {
        return remove_helper(wrap_hash(x));

    }

    auto lookup(const string *s) const -> bool {
        return lookup_helper(wrap_hash(s));
        /*auto hash_val = wrap_hash(s);

        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->lookup(quot, r)) return true;

        return spare->find(hash_val % SL(sparse_element_length));*/
    }

    void insert(const string *s) {
        return insert_helper(wrap_hash(s));
    }

    void insert_to_spare_without_pop(S_TYPE hash_val) {
        spare->insert(hash_val & MASK(sparse_element_length));
    }

    void insert_to_spare_with_pop(S_TYPE hash_val) {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);

        if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
            return;

        S_TYPE hold = hash_val;
        size_t bucket = b1;
        for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
                spare->update_max_cuckoo_insert(i);
                spare->update_cuckoo_insert_counter(i);
                return;
            }
            if (DICT_DB_MODE1)
                assert(spare->is_bucket_full_by_index(bucket));
            spare->cuckoo_swap(&hold, &bucket);
        }
        cout << spare->get_capacity() / ((double) spare->get_max_capacity()) << endl;
        assert(false);
    }

    void remove(const string *s) {
        return remove_helper(wrap_hash(s));
    }

    template<typename P>
    auto lookup_int(P x) const -> bool {
        return lookup_helper(wrap_hash(x));
    }

    template<typename P>
    void insert_int(P x) {
        return insert_helper(wrap_hash(x));
    }

    template<typename P>
    void remove_int(P x) {
        return remove_helper(wrap_hash(x));

    }


    void get_info() {
        const size_t num = 9;
        size_t val[num] = {number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
                           pd_index_length,
                           quotient_length, sparse_element_length, sparse_counter_length};

        string names[num] = {"number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
                             "pd_index_length", "quotient_length", "spare_element_length", "sparse_counter_length"};
        for (int i = 0; i < num; ++i) {
            cout << names[i] << ": " << val[i] << endl;
        }
    }

private:


    inline bool lookup_helper(S_TYPE hash_val) const {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->lookup(quot, r)) return true;

        return spare->find(hash_val & MASK(sparse_element_length));
    }

    inline void insert_helper(S_TYPE hash_val) {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_capacity_vec[pd_index] == single_pd_capacity) {
            insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
            return;
        }
        pd_vec[pd_index]->insert(quot, r);
        ++(pd_capacity_vec[pd_index]);

    }

    inline void remove_helper(S_TYPE hash_val) {
        if (DICT_DB_MODE2)
            assert (lookup_helper(hash_val));
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->conditional_remove(quot, r)) {
            --(pd_capacity_vec[pd_index]);
            return;
        }
        spare->remove(hash_val);
    }

//    void insert_full_PD_helper(S_TYPE hash_val, size_t pd_index, uint32_t quot, uint32_t r);
/*

    void insert_to_spare(S_TYPE y) {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(y, &b1, &b2);

        auto att_res = insert_to_bucket_attempt(y, b2);
        if (att_res == OK)
            return;

//    cout << "Cuckoo hashing start " << sum_pd_capacity() << endl;
        auto hold = y;
        size_t bucket = b1;
        for (size_t i = 0; i < MAX_CUCKOO_LOOP_MULT; ++i) {
            auto temp_att_res = pop_attempt_with_insertion_by_bucket(hold, bucket);
            if (temp_att_res) {
                spare->update_max_cuckoo_insert(i);
                spare->update_cuckoo_insert_counter(i);
                return;
            }
            //todo bucket_pop_attempt here.
            spare->cuckoo_swap(&hold, &bucket);
        }
        assert(false);

    }
*/



/*
    auto insert_to_bucket_attempt(S_TYPE y, size_t bucket_index, bool pop_attempt) -> counter_status;

    auto insert_inc_to_bucket_attempt(S_TYPE y, size_t bucket_index) -> std::tuple<counter_status, size_t>;

    auto pop_attempt_by_bucket(S_TYPE y, size_t bucket_index) -> size_t;

//    auto single_pop_attempt(S_TYPE element) -> bool;

    auto single_pop_attempt(S_TYPE temp_el, S_TYPE counter) -> bool;


    auto pop_attempt(string *s) -> S_TYPE *;

    auto pop_attempt_by_bucket(size_t bucket_index) -> S_TYPE *;

*/

    auto pop_attempt_with_insertion_by_bucket(S_TYPE hash_val, size_t bucket_index) -> bool {
        for (int i = 0; i < spare->get_bucket_size(); ++i) {
            if (spare->is_empty_by_bucket_index_and_location(bucket_index, i)) {
                spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
                return true;
            }
            auto temp_el = spare->get_element_by_bucket_index_and_location(bucket_index, i);
            if (single_pop_attempt(temp_el)) {
                spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
                return true;
            }
        }
        return false;
    }

    /**
     * reads the element if
     * @param element
     * @return
     */
    auto single_pop_attempt(S_TYPE element) -> bool {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(element, &pd_index, &quot, &r);
        if (pd_capacity_vec[pd_index] != single_pd_capacity) {
            assert(!pd_vec[pd_index]->is_full());

            pd_vec[pd_index]->insert(quot, r);
            ++(pd_capacity_vec[pd_index]);
            spare->decrease_capacity();
            cout << "element with hash_val: (" << element << ") was pop." << endl;
            return true;
        }
        if (DICT_DB_MODE1)
            assert(pd_vec[pd_index]->is_full());
        return false;
    }


    inline auto wrap_hash(itemType x) const -> S_TYPE {
        return s_pd_filter::hashint(x) & MASK(sparse_element_length);
    }

//    inline auto wrap_hash(uint64_t x) const -> S_TYPE {
//        return s_pd_filter::my_hash(x, HASH_SEED) & MASK(sparse_element_length);
//    }

    inline auto wrap_hash(const string *s) const -> S_TYPE {
        return s_pd_filter::my_hash(s, HASH_SEED) & MASK(sparse_element_length);
    }


    inline void wrap_split(const string *s, size_t *pd_index, D_TYPE *q, D_TYPE *r) const {
        auto h = s_pd_filter::my_hash(s, HASH_SEED);
        split(h, pd_index, q, r);
    }

    inline void split(ulong h, size_t *pd_index, D_TYPE *q, D_TYPE *r) const {
        *r = h & MASK(remainder_length);
        h >>= remainder_length;
        *q = h % (quotient_range);
        h >>= quotient_length;
        *pd_index = h % pd_vec.size();
    }
};


static auto
get_max_elements_in_level1(size_t number_of_pd, size_t single_pd_capacity, double level1_load_factor) -> size_t;

static auto get_max_elements_in_level2(size_t number_of_pd, size_t single_pd_capacity, double level1_load_factor,
                                       double level2_load_factor) -> size_t;

//typedef dict<PD, hash_table<uint32_t>> s_dict32;


//typedef dict<PD, hash_table<uint32_t>> dict32;
//typedef dict<PD, hash_table<uint32_t>> dict64;


#endif //CLION_CODE_DICT_HPP
