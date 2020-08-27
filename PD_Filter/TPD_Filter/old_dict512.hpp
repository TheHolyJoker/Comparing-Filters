
#ifndef CLION_CODE_DICT512_HPP
#define CLION_CODE_DICT512_HPP

#include "Analyse/analyse.hpp"
#include "hash_table.hpp"
#include "printutil.hpp"
#include "att_hTable.hpp"
#include "basic_function_util.h"
#include "pd512_wrapper.hpp"
#include <cstring>

#define OLD_D512_DB1 (true)
#define OLD_D512_DB2 (true & OLD_D512_DB1)

template <
    class TableType, typename spareItemType,
    typename itemType,
    size_t bits_per_item = 8,
    size_t max_capacity = 51,
    size_t quot_range = 50>
class old_dict512 {
    //    using basic_tpd = temp_PD<slot_type, bits_per_item, max_capacity>;

    pd512_wrapper* pd_array;
    //    vector<pd512_wrapper *> pd_vec;
    vector<uint> pd_capacity_vec;
    //    using temp_spare = att_hTable<uint16_t, 4>;
    //    temp_spare *spare;
    TableType* spare;

    size_t capacity { 0 };
    const size_t remainder_length { bits_per_item },
        quotient_range { quot_range },
        quotient_length { ceil_log2(quot_range) },
        single_pd_capacity { max_capacity };

    const size_t pd_index_length, number_of_pd;
    const size_t sparse_element_length;

public:
    old_dict512(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
        : number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))
        , pd_index_length(ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)))
        , sparse_element_length(pd_index_length + quotient_length + remainder_length)
    {
        assert(single_pd_capacity == 51);
        assert(sparse_element_length <= sizeof(spareItemType) * CHAR_BIT);

        size_t log2_size = ceil_log2(max_number_of_elements);
        // auto res = my_ceil(max_number_of_elements, log2_size) << 6u;
        auto res = my_ceil(max_number_of_elements, log2_size) << 2u;
        // auto res = my_ceil(max_number_of_elements, log2_size) << 3u;
        // auto res = my_ceil(max_number_of_elements, log2_size);

        std::cout << "max_number_of_elements is: " << max_number_of_elements << std::endl;
        std::cout << "spare size is: " << res << std::endl;

        size_t spare_max_capacity = res;
        spare = new TableType(spare_max_capacity, sparse_element_length, level2_load_factor);

        // assert(sizeof(pd512_wrapper) == 64);

        // pd_array = new pd512_wrapper[number_of_pd];

        int ok = posix_memalign((void**)&pd_array, 512, sizeof(pd512_wrapper) * number_of_pd);

        if (ok != 0) {
            cout << "Failed!!!" << endl;
            assert(false);
            return;
        }

        for (size_t i = 0; i < number_of_pd; i++) {
            new (&(pd_array[i])) pd512_wrapper();
            assert(pd_array[i].get_capacity() == 0);
        }
        pd_capacity_vec.resize(number_of_pd, 0);
    }

    virtual ~old_dict512()
    {
        std::cout << "spare capacity is:" << spare->get_capacity() << std::endl;
        free(pd_array);
        // for (size_t i = 0; i < number_of_pd; i++) {
        //     delete (&(pd_array[i]));
        // }

        //        for (int i = 0; i < pd_vec.size(); ++i) {
        //            delete pd_vec[i];
        //        }
        pd_capacity_vec.clear();

        delete spare;
    }

    auto lookup(const itemType s) const -> bool
    {
        auto hash_val = wrap_hash(s);
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        if (pd_array[pd_index].lookup(quot, r))
            return true;

        //        bool c1 = pd_array[pd_index].lookup(quot, r);
        //        bool c2 = pd_vec[pd_index]->lookup(quot, r);
        //        assert(c1 == c2);
        //        if (c1) return true;

        if (pd_capacity_vec[pd_index] & 1u)
            return spare->find(hash_val & MASK(sparse_element_length));
        return false;
    }

    void insert(const itemType s)
    {
        spareItemType hash_val = wrap_hash(s);
      
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);

        assert(pd_array[pd_index].get_capacity() == pd_capacity_vec[pd_index] / 2);
        /* bool cond = pd_array[pd_index].get_capacity() == pd_capacity_vec[pd_index] / 2;
        if (!cond) {
            std::cout << pd_array[pd_index].get_capacity() << std::endl;
            std::cout << pd_capacity_vec[pd_index] / 2 << std::endl;
            assert(false);
        } */
        if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
            assert(pd_array[pd_index].is_full());
            pd_capacity_vec[pd_index] |= 1u;
            /**Todo!*/
            insert_to_spare_without_pop(hash_val);
            // insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
            //            insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
            return;
        }
        //        auto res2 = pd_vec[pd_index]->insert(quot, r);
        auto res = pd_array[pd_index].insert(quot, r);
        //        assert(res == res2);
        if (!res) {
            cout << "insertion failed!!!" << std::endl;
            assert(false);
        }
        (pd_capacity_vec[pd_index] += 2);
    }

    void remove(itemType x)
    {
        return remove_helper(wrap_hash(x));
    }

    auto lookup(const string* s) const -> bool
    {
        return lookup_helper(wrap_hash(s));
    }

    void insert(const string* s)
    {
        return insert_helper(wrap_hash(s));
    }

    void insert_to_spare_without_pop(spareItemType hash_val)
    {
        spare->insert(hash_val & MASK(sparse_element_length));
    }

    void remove(const string* s)
    {
        return remove_helper(wrap_hash(s));
    }

    void get_static_info()
    {
        const size_t num = 8;
        size_t val[num] = { number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
            pd_index_length,
            quotient_length, sparse_element_length };

        string names[num] = { "number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
            "pd_index_length", "quotient_length", "spare_element_length" };
        table_print(num, names, val);
    }

    void get_dynamic_info()
    {
        size_t spare_capacity = spare->get_capacity();
        size_t count_overflowing_PD = count_overflowing_PDs();

        const size_t num = 3;
        size_t val[num] = { spare_capacity, count_overflowing_PD ,number_of_pd};

        string names[num] = { "spare_capacity", "count_overflowing_PD" ,"number_of_pd"};
        table_print(num, names, val);

        const size_t dnum = 3;
        double l1 = analyse_pd_status(1);
        double l2 = analyse_pd_status(2);
        double l3 = analyse_pd_status(3);

        double dval[dnum] = { l1, l2, l3 };

        string dnames[dnum] = { "l1", "l2", "l3" };
        table_print(dnum, dnames, dval);
    }

    auto get_elements_buckets(itemType x) -> std::tuple<uint32_t, uint32_t>
    {
        return get_hash_val_buckets(wrap_hash(x));
    }

    auto get_hash_val_buckets(itemType hash_val) -> std::tuple<uint32_t, uint32_t>
    {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);
        return std::make_tuple(b1, b2);
    }

    auto get_second_level_capacity() -> std::size_t
    {
        return spare->get_capacity();
    }

    auto get_second_level_load_ratio() -> double
    {
        return spare->get_capacity() / ((double)spare->get_max_capacity());
    }

    auto get_name() -> std::string
    {
        string a = "dict512:\t";
        string b = pd_array[0].get_name() + "\t";
        //        string b = pd_vec[0]->get_name() + "\t";
        string c = spare->get_name();
        return a + b + c;
    }

    auto count_overflowing_PDs() -> size_t
    {
        size_t count_overflowing_PD = 0;
        for (int i = 0; i < number_of_pd; ++i) {
            bool add_cond = (pd_capacity_vec[i] & 1u);
            count_overflowing_PD += add_cond;
            bool is_full = pd_array[i].is_full();
            //            bool is_full2 = pd_vec[i]->is_full();
            //            assert(is_full == is_full2);
            bool final = (!add_cond or is_full);
            assert(final);
        }
        return count_overflowing_PD;
    }

    auto analyse_pd_status(size_t p) -> double
    {
        return lp_average(pd_array, number_of_pd, p);
    }

private:
    inline bool lookup_helper(spareItemType hash_val) const
    {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);

        if (pd_array[pd_index].lookup(quot, r))
            return true;
        //        bool c1 = pd_array[pd_index].lookup(quot, r);
        //        bool c2 = pd_vec[pd_index]->lookup(quot, r);
        //        assert(c1 == c2);
        //        if (c2) return true;

        return spare->find(hash_val & MASK(sparse_element_length));
    }

    inline void insert_helper(spareItemType hash_val)
    {

        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);

        assert(pd_array[pd_index].get_capacity() == pd_capacity_vec[pd_index] / 2);
        /* bool cond = pd_array[pd_index].get_capacity() == pd_capacity_vec[pd_index] / 2;
        if (!cond) {
            std::cout << pd_array[pd_index].get_capacity() << std::endl;
            std::cout << pd_capacity_vec[pd_index] / 2 << std::endl;
            assert(false);
        } */
        if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
            assert(pd_array[pd_index].is_full());
            pd_capacity_vec[pd_index] |= 1u;
            /**Todo!*/
            insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
            //            insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
            return;
        }
        //        auto res2 = pd_vec[pd_index]->insert(quot, r);
        auto res = pd_array[pd_index].insert(quot, r);
        //        assert(res == res2);
        if (!res) {
            cout << "insertion failed!!!" << std::endl;
            assert(false);
        }
        (pd_capacity_vec[pd_index] += 2);
    }

    void insert_to_spare_with_pop(spareItemType hash_val)
    {
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
            spare->cuckoo_swap(&hold, &bucket);
        }
        cout << spare->get_capacity() / ((double)spare->get_max_capacity()) << endl;
        assert(false);
    }

    inline void remove_helper(spareItemType hash_val)
    {
        if (OLD_D512_DB1)
            assert(lookup_helper(hash_val));
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);

        if (pd_array[pd_index].conditional_remove(quot, r)) {
            //            assert(pd_array[pd_index].conditional_remove(quot, r));
            (pd_capacity_vec[pd_index]) -= 2;
            return;
        }
        spare->remove(hash_val);
        //        pop_attempt_by_hash_val(hash_val);
    }

    //    void insert_full_PD_helper(spareItemType hash_val, size_t pd_index, uint32_t quot, uint32_t r);

    auto pop_attempt_by_element(const itemType s)
    {
        uint32_t b1, b2;
        std::tie(b1, b2) = get_elements_buckets(s);
        pop_attempt_by_bucket(b1);
        pop_attempt_by_bucket(b2);
    }

    auto pop_attempt_by_hash_val(const itemType hash_val)
    {
        uint32_t b1, b2;
        std::tie(b1, b2) = get_hash_val_buckets(hash_val);
        pop_attempt_by_bucket(b1);
        pop_attempt_by_bucket(b2);
    }

    auto pop_attempt_by_bucket(size_t bucket_index) -> void
    {
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

    auto pop_attempt_with_insertion_by_bucket(spareItemType hash_val, size_t bucket_index) -> bool
    {
        /* if (insert_if_bucket_not_full(hash_val, bucket_index)){
            return true;
        } */
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
    auto single_pop_attempt(spareItemType element) -> bool
    {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(element, &pd_index, &quot, &r);
        if (pd_capacity_vec[pd_index] / 2 < single_pd_capacity) {
            //            cout << " HERE!!!" << endl;
            assert(!pd_array[pd_index].is_full());
            //            bool r2 = pd_vec[pd_index]->is_full();
            //            assert(r1 == r2);
            //            assert(!pd_vec[pd_index]->is_full());

            //            pd_array[pd_index].insert(quot, r);
            bool res = pd_array[pd_index].insert(quot, r);
            assert(res);

            (pd_capacity_vec[pd_index]) += 2;
            spare->decrease_capacity();

            cout << "element with hash_val: (" << element << ") was pop." << endl;
            return true;
        }
        if (OLD_D512_DB1) {
            assert(pd_array[pd_index].is_full());

            /*             auto cap = pd_array[pd_index].get_capacity();
            if (cap != 51) {
                std::cout << "cap is: " << cap << std::endl;
                std::cout << "pd_capacity_vec[pd_index] / 2: " << pd_capacity_vec[pd_index] / 2 << std::endl;
                assert(false);
            }
 */
        }
        return false;
    }

    inline auto wrap_hash(itemType x) const -> spareItemType
    {
        assert(x == (x & MASK(sparse_element_length)));
        return s_pd_filter::hashint(x) & MASK(sparse_element_length);
    }

    inline auto wrap_hash(const string* s) const -> spareItemType
    {
        assert(false);
        assert(sparse_element_length <= 32);
        return s_pd_filter::my_hash(s, HASH_SEED) & MASK(sparse_element_length);
    }

    inline void wrap_split(const string* s, size_t* pd_index, D_TYPE* q, D_TYPE* r) const
    {
        auto h = s_pd_filter::my_hash(s, HASH_SEED);
        split(h, pd_index, q, r);
    }

    inline void split(ulong h, size_t* pd_index, D_TYPE* q, D_TYPE* r) const
    {
        *r = h & MASK(remainder_length);
        h >>= remainder_length;
        *q = h % (quotient_range);
        h >>= quotient_length;
        *pd_index = h % number_of_pd;
    }
};

#endif //CLION_CODE_DICT512_HPP
