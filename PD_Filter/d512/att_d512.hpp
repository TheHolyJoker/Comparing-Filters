
#ifndef CLION_CODE_ATT_D512_HP
#define CLION_CODE_ATT_D512_HP

#include "Analyse/analyse.hpp"
#include "hash_table.hpp"
#include "printutil.hpp"
#include "TPD_Filter/att_hTable.hpp"
#include "TPD_Filter/basic_function_util.h"
#include "TPD_Filter/pd512.hpp"
// #include "pd512_wrapper.hpp"
#include <cstring>

#define ATT_D512_DB1 (true)
#define ATT_D512_DB2 (true & D512DB1)

static size_t insert_counter = 0;
static size_t lookup_counter = 0;
static size_t remove_counter = 0;
static bool hashing_test = false;
static size_t case_validate_counter = 0;
//static size_t case_validate_counter = 0;

//int case, size_t pd_index, uint64_t quot, uint64_t rem,size_t insert_counter
typedef std::tuple<int, size_t, uint64_t, uint64_t, size_t> db_key;

template<
    class TableType, typename spareItemType,
    typename itemType,
    size_t bits_per_item = 8,
    size_t max_capacity = 51,
    size_t quot_range = 50>
    class att_d512 {
    //    using basic_tpd = temp_PD<slot_type, bits_per_item, max_capacity>;

    //    vector<pd512_wrapper *> pd_vec;
    vector<uint16_t> pd_capacity_vec;
    //    using temp_spare = att_hTable<uint16_t, 4>;
    //    temp_spare *spare;
    att_hTable<uint64_t, 4> *spare;
    //    TableType *spare;

    size_t capacity{ 0 };
    const size_t filter_max_capacity;
    const size_t remainder_length{ bits_per_item },
        quotient_range{ quot_range },
        quotient_length{ ceil_log2(quot_range) },
        single_pd_capacity{ max_capacity };
    const uint32_t seed{ 12345 };

    const size_t pd_index_length, number_of_pd;
    const size_t sparse_element_length;
    //    bool hashing_test;
    __m512i *pd_array;

    public:
        att_d512(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
            : filter_max_capacity(max_number_of_elements),
            number_of_pd(compute_number_of_PD(max_number_of_elements,
                max_capacity, level1_load_factor)),
            pd_index_length(ceil_log2(compute_number_of_PD(max_number_of_elements,
                max_capacity, level1_load_factor))),
            sparse_element_length(pd_index_length + quotient_length + remainder_length) {
            //        hashing_test = false;
            assert(single_pd_capacity == 51);
            assert(sparse_element_length <= (sizeof(spareItemType) * CHAR_BIT));
            assert(sizeof(itemType) <= sizeof(spareItemType));

            size_t log2_size = ceil_log2(max_number_of_elements);
            size_t temp = ceil(max_number_of_elements *1);
            auto res = my_ceil(temp, log2_size);

            // std::cout << "max_number_of_elements is: " << max_number_of_elements << std::endl;
            // std::cout << "spare size is: " << res << std::endl;
            size_t spare_max_capacity = res;
            spare = new TableType(spare_max_capacity, sparse_element_length, level2_load_factor);

            // assert(sizeof(pd512_wrapper) == 64);
            // pd_array = new pd512_wrapper[number_of_pd];

            /* pd_array = new __m512i[number_of_pd];
            assert(is_aligned<__m512i>(pd_array));
            std::fill(pd_array, pd_array + number_of_pd,
                      __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0});
            */
            int ok = posix_memalign((void **)&pd_array, 64, 64 * number_of_pd);

            if (ok != 0) {
                cout << "Failed!!!" << endl;
                assert(false);
                return;
            }
            std::fill(pd_array, pd_array + number_of_pd, __m512i{ (INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0 });

            /* for (size_t i = 0; i < number_of_pd; i++)
            {
                assert(pd512::get_capacity(&pd_array[i]) == 0);
                assert(pd512::validate_number_of_quotient(&pd_array[i]));
            }

            std::cout << "pass!" << std::endl;
     */
            pd_capacity_vec.resize(number_of_pd, 0);
        }

        virtual ~att_d512() {
            size_t temp_capacity = 0;
            for (size_t i = 0; i < number_of_pd; i++) {
                temp_capacity += (pd_capacity_vec[i] >> 1u);
            }

            // std::sum(pd_capacity_vec);
            auto line = std::string(64, '*');
            std::cout << line << std::endl;

            std::cout << "filter max capacity is: " << str_format(filter_max_capacity) << std::endl;
            std::cout << "l1_capacity is: " << str_format(temp_capacity) << std::endl;
            std::cout << "spare capacity is: " << str_format(spare->get_capacity()) << std::endl;
            std::cout << "total capacity is: " << str_format(temp_capacity + spare->get_capacity()) << std::endl;
            std::cout << "spare load factor is: " << spare->get_load_factor() << std::endl;

            std::cout << line << std::endl;
            get_dynamic_info();
            std::cout << "byte size is: " << str_format(get_byte_size()) << std::endl;
            std::cout << line << std::endl;


            free(pd_array);
            // for (size_t i = 0; i < number_of_pd; i++) {
            //     delete (&(pd_array[i]));
            // }

            //        for (int i = 0; i < pd_vec.size(); ++i) {
            //            delete pd_vec[i];
            //        }
            pd_capacity_vec.clear();
            //        assert(hashing_test);
            hashing_test = false;
            delete spare;
        }

        auto lookup(const itemType s) const -> bool {
            lookup_counter++;
            /*bool db_cond = f(s);
            if (db_cond)
                std::cout << "lookup counter is: " << lookup_counter++ << std::endl;
            */
            auto hash_val = wrap_hash(s);
            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(hash_val, &pd_index, &quot, &r);

            __m512i *temp_pd = &pd_array[pd_index];
            if (pd512::pd_find_50(quot, r, temp_pd)) {
                assert(pd512::validate_number_of_quotient(temp_pd));
                return true;
            }
            if (pd_capacity_vec[pd_index] & 1u) {
                /*if (db_cond) {
                    std::cout << "h8" << std::endl;
                    std::cout << "h8" << std::endl;
                    assert ((hash_val & MASK(sparse_element_length)) == hash_val);
                }*/
                return spare->find(hash_val & MASK(sparse_element_length));
            }
            return false;
        }

        void insert(const itemType s) {
            insert_counter++;
            //        bool printer = false;
            itemType hash_val = wrap_hash(s);

            /*if (f(s)) {
                std::cout << "insert_counter: " << insert_counter << std::endl;
                std::cout << "hash_val: " << hash_val << std::endl;
                std::cout << "spare load is: " << spare->get_load_factor() << std::endl;
            }

            if (s == 300257437) {
                printer = true;
                std::cout << "hash_val: " << hash_val << std::endl;
            }
    */
            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(hash_val, &pd_index, &quot, &r);
            assert(pd_index < number_of_pd);
            // __m512i temp_pd = pd_array[pd_index];
            assert(pd512::get_capacity(&pd_array[pd_index]) == pd_capacity_vec[pd_index] / 2);

            if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
                /*if (printer) {
                    std::cout << "h6" << std::endl;
                }*/
                assert(pd512::is_full(&pd_array[pd_index]));
                pd_capacity_vec[pd_index] |= 1u;
                /**Todo! this is a mistake?*/
                insert_to_spare_without_pop(hash_val);
                // insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
                //            insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
                return;
            }
            //        if (printer) {
            //            std::cout << "h7" << std::endl;
            //        }

            auto res = pd512::pd_add_50(quot, r, &pd_array[pd_index]);
            if (!res) {
                cout << "insertion failed!!!" << std::endl;
                assert(false);
            }
            (pd_capacity_vec[pd_index] += 2);
        }

        void remove(itemType x) {
            std::cout << "remove counter is: " << remove_counter++ << std::endl;
            return remove_helper(wrap_hash(x));
        }

        /*auto lookup(const string *s) const -> bool {
            return lookup_helper(wrap_hash(s));
        }*/

        //    void insert(const string *s) {
        //        return insert_helper(wrap_hash(s));
        //    }

        void insert_to_spare_without_pop(spareItemType hash_val) {
            spare->insert(hash_val & MASK(sparse_element_length));
        }

        void get_static_info() {
            const size_t num = 8;
            size_t val[num] ={ number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
                pd_index_length,
                quotient_length, sparse_element_length };

            string names[num] ={ "number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
                "pd_index_length", "quotient_length", "spare_element_length" };
            table_print(num, names, val);
        }

        void get_dynamic_info() {

            if (!hashing_test) {
                std::cout << std::string(120, '$') << std::endl;
                std::cout << "Probably did not hit all PD's. (hashing_test is false)." << std::endl;
                std::cout << std::string(120, '$') << std::endl;
            }

            size_t spare_capacity = spare->get_capacity();
            size_t count_overflowing_PD = count_overflowing_PDs();
            size_t count_empty_PD = count_empty_PDs();

            const size_t num = 4;
            size_t val[num] ={ spare_capacity, count_overflowing_PD, count_empty_PD, number_of_pd };

            string names[num] ={ "spare_capacity", "count_overflowing_PD", "count_empty_PD", "number_of_pd" };
            table_print(num, names, val);

            const size_t dnum = 4;
            double l0 = analyse_pd_status(0);
            double l1 = analyse_pd_status(1);
            double l2 = analyse_pd_status(2);
            double l3 = analyse_pd_status(3);

            double dval[dnum] ={ l0, l1, l2, l3 };

            string dnames[dnum] ={ "l0", "l1", "l2", "l3" };
            table_print(dnum, dnames, dval);
            if (count_empty_PD > 1) {
                auto tp = find_empty_pd_interval();
                size_t start = std::get<0>(tp), end = std::get<1>(tp);
                std::cout << "longest_empty_interval length is: " << end - start << std::endl;
                std::cout << "start: " << start << std::endl;
                std::cout << "end: " << end << std::endl;
            }
        }

        auto get_elements_buckets(itemType x) -> std::tuple<uint32_t, uint32_t> {
            return get_hash_val_buckets(wrap_hash(x));
        }

        auto get_hash_val_buckets(itemType hash_val) -> std::tuple<uint32_t, uint32_t> {
            uint32_t b1 = -1, b2 = -1;
            spare->my_hash(hash_val, &b1, &b2);
            return std::make_tuple(b1, b2);
        }

        auto get_second_level_capacity() -> std::size_t {
            return spare->get_capacity();
        }

        auto get_second_level_load_ratio() -> double {
            return spare->get_capacity() / ((double)spare->get_max_capacity());
        }

        auto get_capacity() -> size_t {
            size_t res = 0;
            size_t validate_res = 0;
            __m512i *ppd = &(pd_array[0]);
            for (int i = 0; i < number_of_pd; ++i) {
                res += pd512::get_capacity(ppd++);
            }
            for (int i = 0; i < number_of_pd; ++i) {
                validate_res += (pd_capacity_vec[i] >> 1u);
            }
            if (res != validate_res) {
                std::cout << "res: " << res << std::endl;
                std::cout << "validate_res: " << validate_res << std::endl;
                assert(false);
            }
            return res;
        }

        auto get_name() -> std::string {
            string a = "dict512:\t";
            string b = pd512::get_name() + "\t";
            //        string b = pd_vec[0]->get_name() + "\t";
            string c = spare->get_name();
            return a + b + c;
        }

        auto count_overflowing_PDs() -> size_t {
            size_t count_overflowing_PD = 0;
            for (int i = 0; i < number_of_pd; ++i) {
                bool add_cond = (pd_capacity_vec[i] & 1u);
                count_overflowing_PD += add_cond;
                bool is_full = pd512::is_full(&pd_array[i]);
                //            bool is_full2 = pd_vec[i]->is_full();
                //            assert(is_full == is_full2);
                bool final = (!add_cond or is_full);
                assert(final);
            }
            return count_overflowing_PD;
        }

        auto count_empty_PDs() -> size_t {
            size_t count_empty_PD = 0;
            for (int i = 0; i < number_of_pd; ++i) {
                assert((pd_capacity_vec[i] >> 1ul) == pd512::get_capacity(&pd_array[i]));
                bool add_cond = (pd_capacity_vec[i] <= 0);
                count_empty_PD += add_cond;
                // bool is_full = pd512::is_full(&pd_array[i]);
                //            bool is_full2 = pd_vec[i]->is_full();
                //            assert(is_full == is_full2);
                // bool final = (!add_cond or is_full);
                // assert(final);
            }
            return count_empty_PD;
        }

        /**
         * For validation.
         * This test to see if the hash function could not map elements to certain interval, such as the higher ones.
         * Specifically by finding the longest interval of empty PDs.
         * @return
         */
        auto find_empty_pd_interval() -> std::tuple<size_t, size_t> {
            size_t max_length = 0;
            size_t start = 0, end = 0;
            size_t temp_start = 0, temp_end = 0;
            size_t index = 0;
            __m512i *ppd = &pd_array[0];
            while (index < number_of_pd) {
                //            size_t temp_length = 0;
                if (pd512::get_capacity(&pd_array[index]) == 0) {
                    size_t temp_length = 1;
                    size_t temp_index = index + 1;
                    while ((temp_index < number_of_pd) and (pd512::get_capacity(&pd_array[temp_index]) == 0)) {
                        temp_index++;
                        temp_length++;
                    }
                    if (temp_index == number_of_pd) {
                        std::cout << "h8!" << std::endl;
                    }
                    temp_length = temp_index - index;
                    if (temp_length > max_length) {
                        start = index;
                        end = temp_index;
                        max_length = temp_length;
                    }
                    index = temp_index + 1;
                }
                else
                    index++;
            }
            return { start, end };
        }

        auto get_byte_size() {
            //variables.
            size_t res = 8 * sizeof(size_t);

            //PD's
            res += sizeof(__m512i) * number_of_pd;
            // Capacity vec.
            res += sizeof(uint8_t) * number_of_pd;
            // Pointer to spare.
            res += 1;
            return res;
        }

        auto analyse_pd_status(size_t p) -> double {
            return m512i_lp_average(pd_array, number_of_pd, p);
        }

        auto case_validate() -> bool {
            case_validate_counter++;
            bool res = pd512::validate_number_of_quotient(&pd_array[2279]);
            if (!res) {
                std::cout << "case_validate_counter: " << case_validate_counter << std::endl;
            }
            return res;
        }

        bool f(const itemType x) const {
            return x == 675346436;
        }

    private:
        auto inlining_pd_add_50(int64_t quot, char rem, __m512i *pd, size_t pd_index) {
            using namespace pd512;
            __m512i *ppd = &pd_array[pd_index];
            // assert(validate_number_of_quotient(ppd - 1));
            // assert(validate_number_of_quotient(ppd));
            // assert(validate_number_of_quotient(ppd + 1));

            // print512(*pd);
            // if (pd)
            __m512i slot0 = pd_array[pd_index + 1];
            assert(quot < 50);
            // The header has size 50 + 51
            unsigned __int128 header = 0;
            // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
            // don't care about.
            //
            // memcpy is the only defined punning operation
            const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
            assert(kBytes2copy < sizeof(header));
            memcpy(&header, pd, kBytes2copy);
            auto my_temp = popcount128(header);

            // std::cout << "my_temp: " << my_temp << std::endl;
            // Number of bits to keep. Requires little-endianness
            const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
            const unsigned __int128 kLeftoverMask = (((unsigned __int128)1) << (50 + 51)) - 1;
            header = header & kLeftoverMask;
            /* TODO!!! */
            assert(popcount128(header) == 50);
            const unsigned fill = select128(header, 50 - 1) - (50 - 1);
            assert((fill <= 14) || (fill == pd_popcount(pd)));
            assert((fill == 51) == pd_full(pd));
            if (fill == 51)
                return false;
            // [begin,end) are the zeros in the header that correspond to the fingerprints with
            // quotient quot.
            const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
            const uint64_t end = select128(header, quot);
            assert(begin <= end);
            // assert(end <= 50 + 51);
            unsigned __int128 new_header = header & ((((unsigned __int128)1) << begin) - 1);
            new_header |= ((header >> end) << (end + 1));
            assert(popcount128(new_header) == 50);
            assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
            /* Error is here OR in line 176! */
            __m512i slot1 = pd_array[pd_index + 1];
            assert(my_equal(slot0, slot1));
            // assert(slot0 == slot1);

            memcpy(pd, &new_header, kBytes2copy);

            __m512i slot2 = pd_array[pd_index + 1];
            // assert(validate_number_of_quotient(ppd - 1));
            // assert(validate_number_of_quotient(ppd));
            // assert(validate_number_of_quotient(ppd + 1));
            assert(my_equal(slot1, slot2));
            // assert(slot1 == slot2);

            const uint64_t begin_fingerprint = begin - quot;
            const uint64_t end_fingerprint = end - quot;
            assert(begin_fingerprint <= end_fingerprint);
            assert(end_fingerprint <= 51);
            uint64_t i = begin_fingerprint;
            for (; i < end_fingerprint; ++i) {
                if (rem <= ((const uint8_t *)pd)[kBytes2copy + i])
                    break;
            }
            assert((i == end_fingerprint) || (rem <= ((const uint8_t *)pd)[kBytes2copy + i]));
            /* or here! */
            __m512i slot3 = pd_array[pd_index + 1];
            // assert(validate_number_of_quotient(ppd - 1));
            // assert(validate_number_of_quotient(ppd));
            // assert(validate_number_of_quotient(ppd + 1));
            assert(my_equal(slot0, slot3));
            // assert(slot0 == slot3);
            memmove(&((uint8_t *)pd)[kBytes2copy + i + 1],
                &((const uint8_t *)pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));

            ((uint8_t *)pd)[kBytes2copy + i] = rem;
            __m512i slot4 = pd_array[pd_index + 1];

            assert(my_equal(slot3, slot4));

            // assert(slot4 == slot3);

            assert(pd_find_50(quot, rem, pd));
            return true;
            // //// jbapple: STOPPED HERE
            // const __m512i target = _mm512_set1_epi8(rem);
            // uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
            // // round up to remove the header
            // v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
            // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
        }

        inline bool lookup_helper(spareItemType hash_val) const {
            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(hash_val, &pd_index, &quot, &r);

            if (pd512::pd_find_50(quot, r, &pd_array[pd_index]))
                return true;
            //        bool c1 = pd_array[pd_index].lookup(quot, r);
            //        bool c2 = pd_vec[pd_index]->lookup(quot, r);
            //        assert(c1 == c2);
            //        if (c2) return true;

            return spare->find(hash_val & MASK(sparse_element_length));
        }

        inline void insert_helper(spareItemType hash_val) {

            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(hash_val, &pd_index, &quot, &r);

            assert(pd512::get_capacity(&pd_array[pd_index]) == pd_capacity_vec[pd_index] / 2);
            if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
                assert(pd512::is_full(&pd_array[pd_index]));
                pd_capacity_vec[pd_index] |= 1u;
                /**Todo!*/
                // insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
                insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
                return;
            }
            auto res = pd512::pd_add_50(quot, r, &pd_array[pd_index]);
            if (!res) {
                cout << "insertion failed!!!" << std::endl;
                assert(false);
            }
            pd_capacity_vec[pd_index] += 2;
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
                spare->cuckoo_swap(&hold, &bucket);
            }
            cout << spare->get_capacity() / ((double)spare->get_max_capacity()) << endl;
            assert(false);
        }

        inline void remove_helper(spareItemType hash_val) {
            if (ATT_D512_DB1)
                assert(lookup_helper(hash_val));
            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(hash_val, &pd_index, &quot, &r);

            if (pd512::conditional_remove(quot, r, &pd_array[pd_index])) {
                (pd_capacity_vec[pd_index]) -= 2;
                return;
            }
            spare->remove(hash_val);
            //        pop_attempt_by_hash_val(hash_val);
        }

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
        auto single_pop_attempt(spareItemType element) -> bool {
            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(element, &pd_index, &quot, &r);
            if (pd_capacity_vec[pd_index] / 2 < single_pd_capacity) {
                //            cout << " HERE!!!" << endl;
                assert(!pd512::is_full(&pd_array[pd_index]));
                bool res = pd512::pd_add_50(quot, r, &pd_array[pd_index]);
                assert(res);

                (pd_capacity_vec[pd_index]) += 2;
                spare->decrease_capacity();

                cout << "element with hash_val: (" << element << ") was pop." << endl;
                return true;
            }
            if (ATT_D512_DB1) {
                assert(pd512::is_full(&pd_array[pd_index]));
            }
            return false;
        }

        inline auto wrap_hash(itemType x) const -> spareItemType {
            /*if (insert_counter < 10){
                spareItemType h1 = s_pd_filter::hashint(x);
                spareItemType h2 = s_pd_filter::my_hash64(x, 42) & MASK(sparse_element_length);
                spareItemType h3 = s_pd_filter::hashint64(x) & MASK(sparse_element_length);
                std::cout << "x: " << x << std::endl;
                std::cout << "h1: " << h1 << "\t(" << (h1 & MASK(sparse_element_length)) << ")" <<  std::endl;
                std::cout << "h2: " << h2 << "\t(" << s_pd_filter::my_hash64(x, 42) << ")" << std::endl;
                std::cout << "h3: " << h3 << "\t(" << s_pd_filter::hashint64(x) << ")" << std::endl;
                std::cout << std::string(32,'-') << "\n" << std::endl;
            }*/
            //        return s_pd_filter::my_hash64(x, 42) & MASK(sparse_element_length);
            //        return s_pd_filter::hashint(x);
            //        return s_pd_filter::hashint(x) & MASK(sparse_element_length);;
            return s_pd_filter::hashint64(x);
            //        return s_pd_filter::hashint64(x) & MASK(sparse_element_length);

                    /* Todo:
                     * x &= MASK(sparse_element_length);
                     * return s_pd_filter::hashint(x) & MASK(sparse_element_length);
                     * is this necessary?
                     * */

                     //        assert(x == (x & MASK(sparse_element_length)));
                             // assert(x == (x & MASK(sparse_element_length)));
                             // return s_pd_filter::hashint(x) & MASK(sparse_element_length);
                             // if (sparse_element_length <= 32ul)
                             // {

                             // }
                             // return s_pd_filter::my_hash64(x, seed);
        }

        /*
            inline auto wrap_hash(const string *s) const -> spareItemType {
                assert(false);
                assert(sparse_element_length <= 32);
                return s_pd_filter::my_hash(s, HASH_SEED) & MASK(sparse_element_length);
            }

            inline void wrap_split(const string *s, size_t *pd_index, D_TYPE *q, D_TYPE *r) const {
                auto h = s_pd_filter::my_hash(s, HASH_SEED);
                split(h, pd_index, q, r);
            }
        */

        void split(ulong h, size_t *pd_index, D_TYPE *q, D_TYPE *r) const {
            *r = h & MASK(remainder_length);
            h >>= remainder_length;
            *q = h % (quotient_range);
            h >>= quotient_length;
            if ((!hashing_test) and (h >= number_of_pd)) {
                std::cout << "h3!" << std::endl;
                hashing_test = true;
            }
            *pd_index = h % number_of_pd;
        }

        auto w_insert(const itemType s) -> db_key {
            if (insert_counter == 0) {
                std::cout << "number of pd:" << number_of_pd << std::endl;
            }
            using namespace pd512;
            db_key w_res ={ 0, 0, 0, 0, 0 };
            uint64_t __attribute__((aligned(64))) temp_arr[6][8];
            std::get<4>(w_res) = insert_counter;

            insert_counter++;
            spareItemType hash_val = wrap_hash(s);

            size_t pd_index = -1;
            uint32_t quot = -1, r = -1;
            split(hash_val, &pd_index, &quot, &r);

            bool limits_cond = (pd_index == 0) or (pd_index + 1 == number_of_pd);
            if (limits_cond) {
                std::get<0>(w_res) = -1;
                return w_res;
            }
            std::get<1>(w_res) = pd_index;
            std::get<2>(w_res) = quot;
            std::get<3>(w_res) = r;
            int *v_res = &(std::get<0>(w_res));
            int b = 1ul;
            __m512i *ppd = &pd_array[pd_index];

            bool BPC = (pd_index == 91379);
            if (BPC) {
                std::cout << "h2" << std::endl;
            }

            /* bool cond = (pd_index == 2278) or (pd_index == 9249);
            if (cond)
            {
                std::cout << "HERE! " << std::endl;
                ;
                std::cout << "insert counter is: " << insert_counter << std::endl;
                std::cout << "pd_index:\t" << pd_index << std::endl;
                std::cout << "quot:\t" << quot << std::endl;
                std::cout << "r:\t" << r << std::endl;
            } */

            assert(pd_index < number_of_pd);
            // size_t cap_res = pd512::get_capacity(&pd_array[pd_index]);
            // size_t cap_valid = pd_capacity_vec[pd_index] / 2;

            assert(pd512::get_capacity(ppd) == pd_capacity_vec[pd_index] / 2);

            if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
                assert(pd512::is_full(ppd));
                pd_capacity_vec[pd_index] |= 1u;
                /**Todo!*/
                insert_to_spare_without_pop(hash_val);
                // insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
                return w_res;
            }
            //        auto res2 = pd_vec[pd_index]->insert(quot, r);

            bool c1 = validate_number_of_quotient(ppd - 1);
            bool c2 = validate_number_of_quotient(ppd);
            bool c3 = validate_number_of_quotient(ppd + 1);
            *v_res |= (!c1 * b);
            b <<= 1u;
            *v_res |= (!c2 * b);
            b <<= 1u;
            *v_res |= (!c3 * b);
            b <<= 1u;

            /* if (cond)
            {
                auto line = std::string(64, '*');

                _mm512_store_si512(temp_arr[0], *(ppd - 1));
                _mm512_store_si512(temp_arr[1], *(ppd));
                _mm512_store_si512(temp_arr[2], *(ppd + 1));

                std::cout << 0 << ")" << std::endl;
                pd512::print512(ppd - 1);
                print_8array(temp_arr[0]);
                print_array(temp_arr[0], 8);
                std::cout << line << std::endl;

                std::cout << 1 << ")" << std::endl;
                pd512::print512(ppd);
                print_8array(temp_arr[1]);
                print_array(temp_arr[1], 8);
                std::cout << line << std::endl;

                std::cout << 2 << ")" << std::endl;
                pd512::print512(ppd);
                print_8array(temp_arr[2]);
                print_array(temp_arr[2], 8);
                std::cout << line << std::endl;
            }

     */
     // auto res = pd512::pd_add_50(quot, r, ppd);
            auto res = inlining_pd_add_50(quot, r, ppd, pd_index);
            /* if (cond)
            {
                auto line = std::string(64, '*');

                _mm512_store_si512(temp_arr[3], *(ppd - 1));
                _mm512_store_si512(temp_arr[4], *ppd);
                _mm512_store_si512(temp_arr[5], *(ppd + 1));

                std::cout << 3 << ")" << std::endl;
                pd512::print512(ppd - 1);
                print_8array(temp_arr[3]);
                print_array(temp_arr[3], 8);
                std::cout << line << std::endl;
                std::cout << 4 << ")" << std::endl;
                pd512::print512(ppd);
                print_8array(temp_arr[4]);
                print_array(temp_arr[4], 8);
                std::cout << line << std::endl;
                std::cout << 5 << ")" << std::endl;
                pd512::print512(ppd + 1);
                print_8array(temp_arr[5]);
                print_array(temp_arr[5], 8);
                std::cout << line << std::endl;

                std::cout << "0,3: " << (memcmp(temp_arr[0], temp_arr[3], 64)) << std::endl;
                std::cout << "1,4: " << (memcmp(temp_arr[1], temp_arr[4], 64)) << std::endl;
                std::cout << "2,5: " << (memcmp(temp_arr[2], temp_arr[5], 64)) << std::endl;
            } */

            c1 = validate_number_of_quotient(ppd - 1);
            c2 = validate_number_of_quotient(ppd);
            c3 = validate_number_of_quotient(ppd + 1);
            *v_res |= (!c1 * b);
            b <<= 1u;
            *v_res |= (!c2 * b);
            b <<= 1u;
            *v_res |= (!c3 * b);

            /* assert(pd512::validate_number_of_quotient(&pd_array[pd_index - 1]));
            assert(pd512::validate_number_of_quotient(&pd_array[pd_index]));
            bool temp_failed = pd512::validate_number_of_quotient(&pd_array[pd_index + 1]);
            if (!temp_failed)
            {
                std::cout << "in Failed:" << std::endl;
                std::cout << "pd_index:\t" << pd_index << std::endl;
                std::cout << "cond:\t" << cond << std::endl;
                // assert(false);
            } */
            // assert();

            if (!res) {
                cout << "insertion failed!!!" << std::endl;
                assert(false);
            }
            (pd_capacity_vec[pd_index] += 2);
            return w_res;
        }

        void old_insert(const itemType s) {
            using namespace std;
            auto w_res = w_insert(s);

            int ans = get<0>(w_res);
            if (ans > 0) {
                std::cout << "h1" << std::endl;
                const size_t var_num = 5;
                size_t vals[var_num] ={ get<0>(w_res), get<1>(w_res), get<2>(w_res), get<3>(w_res), get<4>(w_res) };
                string names[var_num] ={ "indicator", "pd_index", "quot", "rem", "insert_count" };
                size_t pd_index = get<1>(w_res);
                table_print(var_num, names, vals);
                bin_print(get<0>(w_res));
                std::cout << "pd_capacity_vec[pd_index - 1]: " << pd_capacity_vec[pd_index - 1] << "(" << pd_index - 1
                    << ")" << std::endl;
                std::cout << "pd_capacity_vec[pd_index]: " << pd_capacity_vec[pd_index] << "(" << pd_index << ")"
                    << std::endl;
                std::cout << "pd_capacity_vec[pd_index + 1]: " << pd_capacity_vec[pd_index + 1] << "(" << pd_index + 1
                    << ")" << std::endl;
                assert(false);
            }
            if (ans == 0)
                return;
            if (ans == -1) {
                spareItemType hash_val = wrap_hash(s);

                size_t pd_index = -1;
                uint32_t quot = -1, r = -1;
                split(hash_val, &pd_index, &quot, &r);
                assert(pd_index < number_of_pd);
                // __m512i temp_pd = pd_array[pd_index];
                assert(pd512::get_capacity(&pd_array[pd_index]) == pd_capacity_vec[pd_index] / 2);

                if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
                    assert(pd512::is_full(&pd_array[pd_index]));
                    pd_capacity_vec[pd_index] |= 1u;
                    /**Todo!*/
                    insert_to_spare_without_pop(hash_val);
                    // insert_to_spare_with_pop(hash_val & MASK(sparse_element_length));
                    //            insert_to_spare_without_pop(hash_val & MASK(sparse_element_length));
                    return;
                }

                auto res = inlining_pd_add_50(quot, r, &pd_array[pd_index], pd_index);
                if (!res) {
                    cout << "insertion failed!!!" << std::endl;
                    assert(false);
                }
                (pd_capacity_vec[pd_index] += 2);
            }
        }
};

#endif //CLION_CODE_ATT_D512_HP
