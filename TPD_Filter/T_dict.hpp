
#ifndef CLION_CODE_T_DICT_HPP
#define CLION_CODE_T_DICT_HPP

#include <cstring>
#include <printutil.hpp>
#include "../PD_Filter/PD.h"
#include "../PD_Filter/hash_table.hpp"
#include "TPD.hpp"
#include "att_hTable.hpp"
#include "basic_function_util.h"

#define DICT_DB_MODE0 (false)
#define DICT_DB_MODE1 (true & DICT_DB_MODE0)
#define DICT_DB_MODE2 (true & DICT_DB_MODE1)


//template<class <> PDType, template<typename> class TableType, typename itemType, typename spareItemType>
template<
        class temp_PD,
        typename slot_type,
        size_t bits_per_item,
        size_t max_capacity,
        class spareType, typename spareItemType,
        typename itemType
>
class T_dict {
//    using basic_tpd = temp_PD<slot_type, bits_per_item, max_capacity>;

    vector<temp_PD *> pd_vec;
    vector<uint> pd_capacity_vec;
//    using temp_spare = att_hTable<uint16_t, 4>;
//    temp_spare *spare;
    spareType *spare;

    size_t capacity{0};
    const size_t remainder_length{bits_per_item},
            quotient_range{64},
            quotient_length{6},
            single_pd_capacity{max_capacity};

    const size_t pd_index_length, number_of_pd;
    const size_t sparse_element_length;

public:
    T_dict(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor) :
//            number_of_pd(std::ceil(max_number_of_elements / (double) (max_capacity * level1_load_factor))),
            number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
            pd_index_length(ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))),
            sparse_element_length(pd_index_length + quotient_length + remainder_length)
/*
//            capacity(0),
//            single_pd_capacity(pd_max_capacity),
//            quotient_range(DEFAULT_QUOTIENT_RANGE),
//            remainder_length(error_power_inv),
//            quotient_length(DEFAULT_QUOTIENT_LENGTH),
//            sparse_counter_length(sizeof(D_TYPE) * CHAR_BIT - sparse_element_length)
//        spare(get_spare_max_capacity(max_number_of_elements, level1_load_factor),
//              remainder_length + pd_index_length + quotient_length, level2_load_factor)
*/
    {
        assert(sparse_element_length <= sizeof(spareItemType) * CHAR_BIT);

        size_t log2_size = ceil_log2(max_number_of_elements);
        // auto res = my_ceil(max_number_of_elements, log2_size) << 6u;
        auto res = my_ceil(max_number_of_elements, log2_size);

        size_t spare_max_capacity = res;
        spare = new spareType(spare_max_capacity, sparse_element_length, level2_load_factor);

        pd_capacity_vec.resize(number_of_pd, 0);

        for (size_t i = 0; i < number_of_pd; ++i) {
            auto *temp = new temp_PD(quotient_range, single_pd_capacity, remainder_length);
            pd_vec.push_back(temp);
        }
        assert(pd_vec.size() == pd_capacity_vec.size());
//    get_info();
    }


//    virtual ~T_dict() {
//        for (int i = 0; i < pd_vec.size(); ++i) {
//            delete pd_vec[i];
//        }
//        pd_vec.clear();
//
//        delete spare;
//    }


    auto lookup(const itemType s) const -> bool {
        auto hash_val = wrap_hash(s);
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->lookup(quot, r)) return true;

        if (pd_capacity_vec[pd_index] & 1u)
            return spare->find(hash_val & MASK(sparse_element_length));
        return false;
    }

    void insert(const itemType s) {
        /*auto a = count_overflowing_PDs();
        auto b = spare->get_capacity();
        auto c = b >= a;
        */
        return insert_helper(wrap_hash(s));
        /*
        auto new_b = spare->get_capacity();
        auto new_a = count_overflowing_PDs();
        assert(new_b = b + 1);
        assert((new_a == a) or (new_a == a + 1));*/
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


    void remove(const string *s) {
        return remove_helper(wrap_hash(s));
    }

    void get_static_info() {
        const size_t num = 8;
        size_t val[num] = {number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
                           pd_index_length,
                           quotient_length, sparse_element_length};

        string names[num] = {"number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
                             "pd_index_length", "quotient_length", "spare_element_length"};
        table_print(num, names, val);
    }

    void get_dynamic_info() {
        size_t spare_capacity = spare->get_capacity();
        size_t count_overflowing_PD = count_overflowing_PDs();

        const size_t num = 2;
        size_t val[num] = {spare_capacity, count_overflowing_PD};

        string names[num] = {"spare_capacity", "count_overflowing_PD"};
        table_print(num, names, val);

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

    auto get_second_level_capacity() -> std::size_t {
        return spare->get_capacity();
    }

    auto get_second_level_load_ratio() -> double {
        return spare->get_capacity() / ((double) spare->get_max_capacity());
    }


    auto get_name() -> std::string {
        string a = "T_dict:\t";
        string b = pd_vec[0]->get_name() + "\t";
        string c = spare->get_name();
        return a + b + c;
    }

    auto count_overflowing_PDs() ->size_t{
        size_t count_overflowing_PD = 0;
        for (int i = 0; i < pd_vec.size(); ++i) {
            bool add_cond = (pd_capacity_vec[i] & 1u);
            count_overflowing_PD += add_cond;
            bool is_full = pd_vec[i]->is_full();
            bool final = (!add_cond or is_full);
            assert(final);
        }
        return count_overflowing_PD;
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
        /*
//        size_t a = pd_capacity_vec[pd_index];
//        size_t b = pd_vec[pd_index]->get_capacity();
//        assert(pd_capacity_vec[pd_index] == pd_vec[pd_index]->get_capacity());
        */
        if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
            pd_capacity_vec[pd_index] |= 1u;
            /**Todo!*/
            insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
//            insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
            return;
        }
        pd_vec[pd_index]->insert(quot, r);
        (pd_capacity_vec[pd_index] += 2);
        /*  a = pd_capacity_vec[pd_index];
          b = pd_vec[pd_index]->get_capacity();
          assert(pd_capacity_vec[pd_index] + 1 == pd_vec[pd_index]->get_capacity());*/

    }

    void insert_to_spare_with_pop(spareItemType hash_val) {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);

        if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
            return;

        auto hold = hash_val;
        size_t bucket = b1;
        for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
                return;
            }
            /*if (DICT_DB_MODE1)
                assert(spare->is_bucket_full_by_index(bucket));*/
            spare->cuckoo_swap(&hold, &bucket);
        }
        cout << spare->get_capacity() / ((double) spare->get_max_capacity()) << endl;
        assert(false);
    }


    inline void remove_helper(spareItemType hash_val) {
        if (DICT_DB_MODE2)
            assert (lookup_helper(hash_val));
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_vec[pd_index]->conditional_remove(quot, r)) {
            (pd_capacity_vec[pd_index]) -= 2;
            return;
        }
        spare->remove(hash_val);
//        pop_attempt_by_hash_val(hash_val);
    }

//    void insert_full_PD_helper(spareItemType hash_val, size_t pd_index, uint32_t quot, uint32_t r);

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
                spare->clear_slot_bucket_index_and_location(bucket_index, i);
            }

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
        if (pd_capacity_vec[pd_index] / 2 < single_pd_capacity) {
            cout << " HERE!!!" << endl;
            assert(!pd_vec[pd_index]->is_full());

            pd_vec[pd_index]->insert(quot, r);
            (pd_capacity_vec[pd_index]) += 2;
            spare->decrease_capacity();
            cout << "element with hash_val: (" << element << ") was pop." << endl;
            return true;
        }
        if (DICT_DB_MODE1)
            assert(pd_vec[pd_index]->is_full());
        return false;
    }


    inline auto wrap_hash(itemType x) const -> spareItemType {
        return s_pd_filter::hashint(x) & MASK(sparse_element_length);
    }


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


#endif //CLION_CODE_T_DICT_HPP
