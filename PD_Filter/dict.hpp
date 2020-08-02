
#ifndef CLION_CODE_DICT_HPP
#define CLION_CODE_DICT_HPP

#include <cstring>
#include "PD.h"
#include "../TPD_Filter/TPD.hpp"
#include "hash_table.hpp"

#define DICT_DB_MODE0 (false)
#define DICT_DB_MODE1 (true & DICT_DB_MODE0)
#define DICT_DB_MODE2 (true & DICT_DB_MODE1)

/*
static inline auto get_single_pd_max_capacity(size_t error_power_inv, double level1_load_factor) -> size_t {
    if (error_power_inv <= 8)
        return DEFAULT_PD_CAPACITY;
    assert(false);
//    auto temp = (512u / (error_power_inv + 2)) * level1_load_factor;
//    return std::floor(temp);
//    return my_ceil<size_t>(512u, error_power_inv + 2);
}*/

//template<class PDType, template<typename> class TableType, typename itemType, size_t bits_per_item>
template<class PDType, template<typename> class spareType, typename itemType, typename spareItemType>
class dict {
    vector<PDType *> pd_vec;
    vector<uint> pd_capacity_vec;
    hash_table<spareItemType> *spare;
//    TableType<spareItemType> *spare;

    size_t capacity;
//    const size_t number_of_pd, remainder_length{bits_per_item}, quotient_range, single_pd_capacity;
    const size_t number_of_pd, remainder_length, quotient_range, single_pd_capacity;
    const size_t quotient_length, pd_index_length;
    const size_t sparse_element_length, sparse_counter_length;

public:
    dict(size_t max_number_of_elements, size_t error_power_inv, double level1_load_factor, double level2_load_factor, size_t pd_max_capacity = DEFAULT_PD_CAPACITY) :
            capacity(0),
            single_pd_capacity(pd_max_capacity),
            number_of_pd(std::ceil(max_number_of_elements / pd_max_capacity)),
            quotient_range(DEFAULT_QUOTIENT_RANGE),
            remainder_length(error_power_inv),
            pd_index_length(ceil(log2(my_ceil(max_number_of_elements, (size_t) single_pd_capacity)))),
            quotient_length(DEFAULT_QUOTIENT_LENGTH),
            sparse_element_length(remainder_length + pd_index_length + quotient_length),
            sparse_counter_length(sizeof(D_TYPE) * CHAR_BIT - sparse_element_length)
//        spare(get_spare_max_capacity(max_number_of_elements, level1_load_factor),
//              remainder_length + pd_index_length + quotient_length, level2_load_factor)
    {
        assert(sparse_element_length <= sizeof(spareItemType) * CHAR_BIT);

        size_t log2_size = ceil(log2(max_number_of_elements));
//        auto res = my_ceil(max_number_of_elements, log2_size * log2_size)  << 8u;
        // auto res = my_ceil(max_number_of_elements, log2_size) << 6u;
        auto res = my_ceil(max_number_of_elements, log2_size);

        size_t spare_max_capacity = res;
        std::cout << "max_number_of_elements is: " << max_number_of_elements << std::endl;
        std::cout << "spare size is: " << res << std::endl;
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
        auto hash_val = wrap_hash(s);
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->lookup(quot, r)) return true;

        return spare->find(hash_val & MASK(sparse_element_length));
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

    void insert_to_spare_without_pop(spareItemType hash_val) {
        spare->insert(hash_val & MASK(sparse_element_length));
    }

    void insert_to_spare_with_pop(spareItemType hash_val) {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);

        if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
            return;

        spareItemType hold = hash_val;
        size_t bucket = b1;
        for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
                if (i > 4) {
//                    cout << "here with hash_val: " << hash_val << endl;
                    spare->update_max_cuckoo_insert(i);
                    spare->update_cuckoo_insert_counter(i);
                }
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

    auto get_elements_buckets(itemType x) -> std::tuple<uint32_t, uint32_t> {
        return get_hash_val_buckets(wrap_hash(x));
        /*auto hash_val = wrap_hash(x);
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);
        return std::make_tuple(b1, b2);*/
    }

    auto get_hash_val_buckets(itemType hash_val) -> std::tuple<uint32_t, uint32_t> {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);
        return std::make_tuple(b1, b2);
    }
/*
    auto get_second_level_max_reached_capacity()->std::size_t {
        return spare->get_max_capacity_reached();
    }*/

    auto get_second_level_capacity()->std::size_t {
        return spare->get_capacity();
    }

    auto get_second_level_load_ratio() ->double {
        return spare->get_capacity() / ((double) spare->get_max_capacity());
    }
private:


    inline bool lookup_helper(spareItemType hash_val) const {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->lookup(quot, r)) return true;

        return spare->find(hash_val & MASK(sparse_element_length));
    }

    inline void insert_helper(spareItemType hash_val) {

        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
//        size_t a = pd_capacity_vec[pd_index];
//        size_t b = pd_vec[pd_index]->get_capacity();
//        assert(pd_capacity_vec[pd_index] == pd_vec[pd_index]->get_capacity());
        if (pd_capacity_vec[pd_index] == single_pd_capacity) {
            insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
//            insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
            return;
        }
        pd_vec[pd_index]->insert(quot, r);
//        a = pd_capacity_vec[pd_index];
//        b = pd_vec[pd_index]->get_capacity();
//        assert(pd_capacity_vec[pd_index] + 1 == pd_vec[pd_index]->get_capacity());
        ++(pd_capacity_vec[pd_index]);

    }

    inline void remove_helper(spareItemType hash_val) {
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
//        pop_attempt_by_hash_val(hash_val);
    }

//    void insert_full_PD_helper(spareItemType hash_val, size_t pd_index, uint32_t quot, uint32_t r);
/*

    void insert_to_spare(spareItemType y) {
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
    auto insert_to_bucket_attempt(spareItemType y, size_t bucket_index, bool pop_attempt) -> counter_status;

    auto insert_inc_to_bucket_attempt(spareItemType y, size_t bucket_index) -> std::tuple<counter_status, size_t>;

    auto pop_attempt_by_bucket(spareItemType y, size_t bucket_index) -> size_t;

//    auto single_pop_attempt(spareItemType element) -> bool;

    auto single_pop_attempt(spareItemType temp_el, spareItemType counter) -> bool;




*/

    auto pop_attempt_by_element(const itemType s) {
        uint32_t b1, b2;
        std::tie(b1, b2) = get_elements_buckets(s);
        pop_attempt_by_bucket(b1);
        pop_attempt_by_bucket(b2);
    }

    auto pop_attempt_by_hash_val(const itemType hash_val) {
        uint32_t b1, b2;
        std::tie(b1, b2) = get_hash_val_buckets(hash_val);
        pop_attempt_by_bucket(b1);
        pop_attempt_by_bucket(b2);
    }

    auto pop_attempt_by_bucket(size_t bucket_index) -> void {
        for (int i = 0; i < spare->get_bucket_size(); ++i) {
            if (spare->is_empty_by_bucket_index_and_location(bucket_index, i))
                continue;
            auto temp_el = spare->get_element_by_bucket_index_and_location(bucket_index, i);
            if (single_pop_attempt(temp_el)) {
//                cout << "here" << endl;
                spare->clear_slot_bucket_index_and_location(bucket_index, i); }

        }
    }

    auto pop_attempt_with_insertion_by_bucket(spareItemType hash_val, size_t bucket_index) -> bool {
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
    auto single_pop_attempt(spareItemType element) -> bool {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(element, &pd_index, &quot, &r);
        if (pd_capacity_vec[pd_index] != single_pd_capacity) {
            assert(!pd_vec[pd_index]->is_full());

            pd_vec[pd_index]->insert(quot, r);
            ++(pd_capacity_vec[pd_index]);
            spare->decrease_capacity();
//            cout << "element with hash_val: (" << element << ") was pop." << endl;
            return true;
        }
        if (DICT_DB_MODE1)
            assert(pd_vec[pd_index]->is_full());
        return false;
    }


    inline auto wrap_hash(itemType x) const -> spareItemType {
        return s_pd_filter::hashint(x) & MASK(sparse_element_length);
    }

//    auto wrap_hash(uint64_t x) const -> spareItemType; {
//    inline auto wrap_hash(uint64_t x) const -> spareItemType {
//        return s_pd_filter::hashint3(x) & MASK(sparse_element_length);
//    }

    inline auto wrap_hash(const string *s) const -> spareItemType {
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
