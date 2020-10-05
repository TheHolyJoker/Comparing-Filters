//
// Created by tomer on 05/10/2020.
//

#ifndef COMPARING_FILTERS_SIMPLER_FIXED_DICT_VER2_HPP
#define COMPARING_FILTERS_SIMPLER_FIXED_DICT_VER2_HPP


//#include "../../cuckoofilter/src/cuckoofilter.h"
#include "../hashutil.h"
#include "HashTables/hashTable_Aligned.hpp"
#include "fixed_pd_45.hpp"
//#include <unordered_map>

#define FIXED_DICT_VER2_DB1 (true)
#define FIXED_DICT_VER2_DB2 (true & FIXED_DICT_VER2_DB1)
#define FIXED_DICT_VER2_DB3 (true & FIXED_DICT_VER2_DB2)


template<
        typename spareItemType,
        typename itemType>
class Fixed_Dict_Ver2 {
    static constexpr size_t bits_per_item = 8;
    static constexpr size_t max_capacity = Fixed_pd45::CAPACITY;
    static constexpr size_t quot_range = Fixed_pd45::QUOT_RANGE;


    hashTable_Aligned<uint64_t, 4> *spare;
    hashing::TwoIndependentMultiplyShift Hasher;


    const size_t filter_max_capacity;
    const size_t remainder_length{bits_per_item},
            quotient_range{quot_range},
            quotient_length{ceil_log2(quot_range)},
            single_pd_capacity{max_capacity};
    const size_t pd_index_length, number_of_pd;
    const size_t spare_element_length;
    double expected_pd_capacity;

    __m512i *pd_array;

    // unordered_map<itemType, uint64_t> map_db;
    // vector<size_t> capacity_vec;
    // vector<bool> rand_vec;

public:
    Fixed_Dict_Ver2(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
            : filter_max_capacity(max_number_of_elements),
              number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
              pd_index_length(
                      ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))),
              spare_element_length(pd_index_length + quotient_length + remainder_length),
//               Hasher(0,0){
              Hasher() {

        expected_pd_capacity = max_capacity * level1_load_factor;
        spare = new hashTable_Aligned<uint64_t, 4>(
                ceil(1.0 * max_number_of_elements / (1.0 * ceil_log2(max_number_of_elements))),
                ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)) +
                quotient_length + bits_per_item,
                level2_load_factor);

        int ok = posix_memalign((void **) &pd_array, 64, 64 * number_of_pd);
//        int ok = posix_memalign((void **) &packed_fpd_array, 64, 64 * 3 * number_of_pd);
        if (ok != 0) {
            cout << "Failed!!!" << endl;
            assert(false);
            return;
        }
        std::fill(pd_array, pd_array + number_of_pd, __m512i{0, 0, 0, 0, 0, 0, 0, 0});

//        for (int i = 0; i < number_of_pd; ++i) {
//            assert(Fixed_pd45::Header::get_capacity(&pd_array[i]) == 0);
//        }
    }

    virtual ~Fixed_Dict_Ver2() {
        assert(get_capacity() >= 0);
        auto ss = get_extended_info();
        std::cout << ss.str();
        free(pd_array);
        delete spare;
    }


    inline auto lookup(const itemType s) const -> bool {
        // return lookup_ver1(s);
        // return lookup_ver2(s);
        return lookup_ver3(s);
//        return lookup_ver4(s);
    }

    /*inline auto lookup_ver1(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        // uint64_t hash_res = s;
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const bool ind = out2 & (1 << 16);
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);

        int res = (ind) ?
                  Wrap_Fixed_pd::find2(quot, rem, packed_fpd_array[pd_index].header2, &packed_fpd_array[pd_index].body2)
                        :
                  Wrap_Fixed_pd::find2(quot, rem, packed_fpd_array[pd_index].header1,
                                       &packed_fpd_array[pd_index].body1);

        return (res != -1) ? res :
               (spare->find(
                       ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem));
    }
*/

    /* inline auto lookup_safe(const itemType s, int result_value) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        // const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) (quot_range << 8ul));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;
        const uint64_t spare_val = ((uint64_t) pd_index << (13)) | qr;

        bool a = pd_name::pd_find_25(quot, rem, &pd_array[pd_index]);
        bool b = spare_filter.Contain(spare_val) == cuckoofilter::Ok;
        bool b2 = spare->find(spare_val);
        bool c = pd_name::cmp_qr_smart(qr, &pd_array[pd_index]);
        bool d = (!pd_name::cmp_qr_smart(qr, &pd_array[pd_index])) ? pd_name::pd_find_25(quot, rem, &pd_array[pd_index]) : (spare_filter.Contain(spare_val) == cuckoofilter::Ok);
        bool e = pd_name::cmp_qr_smart(qr, &pd_array[pd_index]) ? (spare_filter.Contain(spare_val) == cuckoofilter::Ok) : pd_name::pd_find_25(quot, rem, &pd_array[pd_index]);

        assert(e == d);
        if (result_value == 1) {
            if (!d) {
                std::cout << "a: " << a << std::endl;
                std::cout << "b: " << b << std::endl;
                std::cout << "b2: " << b2 << std::endl;
                std::cout << "c: " << c << std::endl;
                std::cout << "d: " << d << std::endl;
                std::cout << "e: " << e << std::endl;
            }
            assert(d);
        }

        return d;
    }
 */

    /*inline auto lookup_ver2(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        // uint64_t hash_res = s;
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const bool ind = out2 & (1 << 16);
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);

        uint64_t *header = (ind) ? packed_fpd_array[pd_index].header2 : packed_fpd_array[pd_index].header1;
        size_t counter = Fixed_pd45::Header::read_counter(quot, header);
        // size_t counter = (ind) ?
        //     Fixed_pd45::Header::read_counter(quot, packed_fpd_array[pd_index].header2):
        //     Fixed_pd45::Header::read_counter(quot, packed_fpd_array[pd_index].header1);
        if (!counter) {
            return false;
        } else if (counter == Fixed_pd45::counter_overflowed_val) {
            const uint8_t rem = out2 & MASK(bits_per_item);
            return spare->find(
                    ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem);
        }
        uint64_t start = Fixed_pd45::Header::get_start(quot, header);

        const uint8_t rem = out2 & MASK(bits_per_item);
        uint64_t v = (ind) ? Fixed_pd45::Body::get_v(rem, &packed_fpd_array[pd_index].body2) : Fixed_pd45::Body::get_v(
                rem,
                &packed_fpd_array[pd_index].body1);

        v >>= start;
        if (!v)
            return false;
        return (v) & MASK(counter);
    }
*/
    inline auto lookup_ver3(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);


//        uint64_t *header = (ind) ? packed_fpd_array[pd_index].header2 : packed_fpd_array[pd_index].header1;
        size_t counter = Fixed_pd45::Header::read_counter(quot, &pd_array[pd_index]);
        if (!counter) {
            return false;
        } else if (counter == Fixed_pd45::counter_overflowed_val) {
            const uint8_t rem = out2 & MASK(bits_per_item);
            return spare->find(
                    ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem);
        } else {
            const uint8_t rem = out2 & MASK(bits_per_item);
            uint64_t v = Fixed_pd45::Body::get_v(rem, &pd_array[pd_index]);
            if (!v)
                return false;
            uint64_t start = Fixed_pd45::Header::get_start(quot, &pd_array[pd_index]);
            return (v >> start) & MASK(counter);
        }

    }

    inline auto lookup_ver4(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint8_t rem = out2 & MASK(bits_per_item);
        uint64_t v = Fixed_pd45::Body::get_v(rem, &pd_array[pd_index]);
        if (!v) {
            const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
            size_t counter = Fixed_pd45::Header::read_counter(quot, &pd_array[pd_index]);;
            return (counter == Fixed_pd45::counter_overflowed_val) && spare->find(
                    ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem);
        }


        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        size_t counter = Fixed_pd45::Header::read_counter(quot, &pd_array[pd_index]);
        if (!counter) {
            return false;
        } else {
            uint64_t start = Fixed_pd45::Header::get_start(quot, &pd_array[pd_index]);
            return (v >> start) & MASK(counter);
        }

    }

    void insert(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);


        auto res = Fixed_pd45::add(quot, rem, &pd_array[pd_index]);
        if (res == -1) {
            assert(lookup(s));
//             std::cout << "AA 1" << std::endl;
            return;
        } else if (res == -2) {
            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) |
                                 ((uint64_t) rem);
            insert_to_spare_without_pop(spare_val);
            assert(spare->find(spare_val));
            assert(lookup(s));
//             std::cout << "AA 2" << std::endl;
            return;
        } else if (res == -3) {
            const uint64_t partial_spare_item =
                    ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item);
            promote_quot_and_remove_from_body(quot, Fixed_pd45::max_valid_counter_value, partial_spare_item,
                                              &pd_array[pd_index]);
            const uint64_t spare_val = partial_spare_item | ((uint64_t) rem);
            insert_to_spare_without_pop(spare_val);

            assert(spare->find(spare_val));
            assert(Fixed_pd45::Header::did_quot_overflowed(quot, &pd_array[pd_index]));
            assert(lookup(s));
//             std::cout << "AA 3" << std::endl;
            return;
        } else if (res == -4) {

            size_t max_quot = Fixed_pd45::Header::get_max_quot(&pd_array[pd_index]);
            auto temp_val = Fixed_pd45::Header::read_counter(max_quot, &pd_array[pd_index]);
            assert(temp_val < Fixed_pd45::counter_overflowed_val);
            assert(temp_val);

            uint64_t partial_spare_item =
                    ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (max_quot << bits_per_item);
            promote_quot_and_remove_from_body(max_quot, temp_val, partial_spare_item, &pd_array[pd_index]);

            Fixed_pd45::Header::set_quot_as_overflow(max_quot, &pd_array[pd_index]);
            if (max_quot == quot) {
                uint64_t spare_val = partial_spare_item | ((uint64_t) rem);

                insert_to_spare_without_pop(spare_val);
                assert(lookup(s));
//                 std::cout << "AA 4" << std::endl;
                return;
            } else {
                //necessary for increasing relevent counter.
                int new_header_add_res = Fixed_pd45::Header::add(quot, &pd_array[pd_index]);
                assert(new_header_add_res == -1);
                uint64_t start = Fixed_pd45::Header::get_start(quot, &pd_array[pd_index]);
                uint64_t end = Fixed_pd45::Header::read_counter(quot, &pd_array[pd_index]);
                assert(end != Fixed_pd45::counter_overflowed_val);
                Fixed_pd45::Body::add(start, start + end - 1, rem, &pd_array[pd_index]);
                assert(lookup(s));
//                 std::cout << "AA 4.2" << std::endl;
            }
        }
    }


    void promote_quot_and_remove_from_body(uint64_t quot, size_t curr_counter_val, uint64_t partial_spare_item,
                                           __m512i *pd) {
        size_t begin_fingerprint = Fixed_pd45::Header::get_start(quot, pd);
        for (size_t i = 0; i < curr_counter_val; i++) {
            //todo: Error could be here.
            uint8_t temp_rem = ((uint8_t *) (pd))[Fixed_pd45::bodyStartIndex + begin_fingerprint + i];
            uint64_t spare_val = partial_spare_item | ((uint64_t) temp_rem);
            insert_to_spare_without_pop(spare_val);
            assert(spare->find(spare_val));
        }
//        assert(Fixed_pd45::Header::validate_capacity(pd));
        Fixed_pd45::Body::remove_quot(begin_fingerprint, begin_fingerprint + curr_counter_val, pd);
//        assert(Fixed_pd45::Header::validate_capacity(pd));

    }

    void promote_quot_and_remove_from_body(uint64_t quot, uint64_t partial_spare_item, __m512i *pd) {
        size_t counter_size = Fixed_pd45::Header::read_counter(quot, pd);
        assert(counter_size < Fixed_pd45::counter_overflowed_val);
        assert(0 < counter_size);
//        assert(Fixed_pd45::Header::validate_capacity(pd));
        promote_quot_and_remove_from_body(quot, counter_size, partial_spare_item, pd);
//        assert(Fixed_pd45::Header::validate_capacity(pd));
    }

    inline void remove(const itemType s) {
        assert(false);
    }

    inline void insert_to_spare_without_pop(spareItemType spare_val) {
        if (!spare->insert(spare_val)) {
            std::cout << "spare failed. Collecting Data" << std::endl;
            auto ss = get_extended_info();
            std::cout << ss.str();
            assert(false);
        }
    }

    void insert_to_spare_with_pop(spareItemType hash_val) {
        uint32_t b1 = -1, b2 = -1;
        spare->get_element_buckets(hash_val, &b1, &b2);
        if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
            std::cout << "AA 5" << std::endl;
        return;

        auto hold = hash_val;
        size_t bucket = b1;
        for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
                return;
            }
            spare->cuckoo_swap(&hold, &bucket);
        }
        cout << spare->get_capacity() / ((double) spare->get_max_capacity()) << endl;
        assert(false);
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
        /* Todo: optimize.
         * 1) copy the bucket using memcpy and pop every non empty element.
         * 2) another way.
         */

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
        const uint32_t pd_index = element >> (bits_per_item + quotient_length);
        assert(pd_index < number_of_pd);
        const uint64_t quot = (element >> bits_per_item) & MASK(quotient_length);
        assert(quot <= 50);
        const uint8_t rem = element & MASK(bits_per_item);

        assert(0);
        // if (!pd_name::pd_full(&pd_array[pd_index])) {
        //     //            cout << " HERE!!!" << endl;
        //     // assert(!pd_name::pd_full(&pd_array[pd_index]));
        //     bool res = pd_name::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
        //     assert(res);

        //     // (pd_capacity_vec[pd_index]) += 2;
        //     spare->decrease_capacity();

        //     // cout << "element with hash_val: (" << element << ") was pop." << endl;
        //     return true;
        // }
        // if (FIXED_DICT_DB1) {
        //     assert(pd_name::pd_full(&pd_array[pd_index]));
        // }
        return false;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// Validation functions.////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////


    auto squared_chi_test() -> double {
        // vector<uint16_t> temp_capacity_vec(pd_capacity_vec);
        vector<spareItemType> spare_elements;

        spare->get_all_elements(&spare_elements);
        for (size_t i = 0; i < spare_elements.size(); i++) {
            auto temp = spare_elements[i];
            size_t temp_pd_index = (temp >> (bits_per_item + quotient_length));
            // temp_capacity_vec[temp_pd_index]++;
        }

        double res = 0;
        for (size_t i = 0; i < number_of_pd; i++) {
            auto temp1 = Fixed_pd45::Header::get_capacity(&pd_array[i]) - expected_pd_capacity;
            res += temp1 * temp1;
        }
        return res / (filter_max_capacity * expected_pd_capacity);
    }


    auto validate_capacity_functions(size_t pd_index) -> bool {
        return true;
        // bool c = (pd_name::get_capacity(&pd_array[pd_index]) == (pd_name::get_capacity_naive(&pd_array[pd_index])));
        // return c & pd_name::get_capacity(&pd_array[pd_index]) == (pd_capacity_vec[pd_index] >> 1u);
    }

    auto get_extended_info() -> std::stringstream {
        // print_search_counters();
        std::stringstream ss;
        size_t temp_capacity = 0;
        for (size_t i = 0; i < number_of_pd; i++) {
            size_t cap_a = Fixed_pd45::Header::get_capacity(&pd_array[i]);
            assert(cap_a <= Fixed_pd45::CAPACITY);
            temp_capacity += cap_a;
            // temp_capacity += Fixed_pd45::Header::get_capacity(packed_fpd_array[i].header1) + Fixed_pd45::Header::get_capacity(packed_fpd_array[i].header2);
        }

        // std::sum(pd_capacity_vec);
        auto line = std::string(64, '*');
        ss << line << std::endl;

        ss << "filter max capacity is: " << str_format(filter_max_capacity) << std::endl;
        // ss << "total_capacity: " << str_format(total_capacity) << std::endl;
        ss << "l1_capacity (computed): " << str_format(temp_capacity) << std::endl;
        // ss << "l1_capacity (var): " << str_format(l1_capacity) << std::endl;
        // ss << "basic squared chi is: " << squared_chi_test_basic() << std::endl;
        // ss << "squared chi is: " << squared_chi_test() << std::endl;

        // ss << "total capacity is: " << str_format(temp_capacity + spare->get_capacity()) << std::endl;
        ss << "spare capacity is: " << str_format(spare->get_capacity()) << std::endl;
        ss << "spare load factor is: " << spare->get_load_factor() << std::endl;
        double ratio = 1.0 * spare->get_capacity() / (double) temp_capacity;
        ss << "l2/l1 capacity ratio is: " << ratio << std::endl;
        ss << "spare_element_length is: " << spare_element_length << std::endl;


        // if (insert_existing_counter) {
        //     ss << "insert_existing_counter: " << insert_existing_counter << std::endl;
        //     double ratio = insert_existing_counter / (double) filter_max_capacity;
        //     assert(ratio > 0);
        //     ss << "ratio to max capacity: " << ratio << std::endl;
        // }
        ss << std::string(64, '.') << std::endl;
        auto temp_ss = get_dynamic_info();
        // ss << "l1 byte size is: " << str_format(get_byte_size()) << std::endl;
        // ss << "total byte size is: " << str_format(get_byte_size_with_spare()) << std::endl;
        ss << std::string(80, '-') << std::endl;
        ss << temp_ss.str();
        ss << line << std::endl;

        // spare->get_info(&ss);
        return ss;
    }

    // void get_static_info() {
    //     const size_t num = 8;
    //     size_t val[num] = {number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
    //                        pd_index_length,
    //                        quotient_length, spare_element_length};

    //     string names[num] = {"number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
    //                          "pd_index_length", "quotient_length", "spare_element_length"};
    //     table_print(num, names, val);
    // }

    auto get_dynamic_info() -> std::stringstream {
        /* if (!hashing_test) {
                std::cout << std::string(120, '$') << std::endl;
                std::cout << "Probably did not hit all PD's. (hashing_test is false)." << std::endl;
                std::cout << std::string(120, '$') << std::endl;
            } */
        size_t spare_capacity = spare->get_capacity();
        // size_t count_overflowing_PD = count_overflowing_PDs();
        // size_t count_empty_PD = count_empty_PDs();

        // const size_t num = 4;
        // size_t val[num] = {spare_capacity, count_overflowing_PD, count_empty_PD, number_of_pd};

        // string names[num] = {"spare_capacity", "count_overflowing_PD", "count_empty_PD", "number_of_pd"};
        // std::stringstream ss = table_print(num, names, val);

        // const size_t dnum = 4;
        // double l0 = analyse_pd_status(0);
        // double l1 = analyse_pd_status(1);
        // double l2 = analyse_pd_status(2);
        // double l3 = analyse_pd_status(3);

        // double dval[dnum] = {l0, l1, l2, l3};

        // string dnames[dnum] = {"l0", "l1", "l2", "l3"};
        // table_print(dnum, dnames, dval, os);

        // if (count_empty_PD > 1) {
        //     // auto tp = find_empty_pd_interval();
        //     size_t start = std::get<0>(tp), end = std::get<1>(tp);
        //     ss << "longest_empty_interval length is: " << end - start << std::endl;
        //     ss << "start: " << start << std::endl;
        //     ss << "end: " << end << std::endl;
        // }
        std::stringstream ss;
        return ss;
        // return os;
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
        return spare->get_capacity() / ((double) spare->get_max_capacity());
    }

    auto get_capacity() -> size_t {
        size_t res = 0;
        // size_t validate_res = l1_capacity;
        // __m512i *ppd = &(pd_array[0]);
        for (int i = 0; i < number_of_pd; ++i) {
            res += Fixed_pd45::Header::get_capacity(&pd_array[i]);
            //  pd_name::get_capacity(&pd_array[i]);
        }
        // if (res != validate_res) {
        //     std::cout << "res: " << res << std::endl;
        //     std::cout << "validate_res: " << validate_res << std::endl;
        //     std::cout << "total_capacity: " << total_capacity << std::endl;
        //     std::cout << "Did not update <l1_capacity> inside a pop operation." << std::endl;

        //     assert(false);
        // }
        return res;
    }


    auto get_name() -> std::string {
        return "Fixed_Dict_Ver2";
    }

    // auto count_overflowing_PDs() -> size_t {
    //     size_t count_overflowing_PD = 0;
    //     for (int i = 0; i < number_of_pd; ++i) {
    //         bool add_cond = pd_name::pd_full(&pd_array[i]);
    //         count_overflowing_PD += add_cond;
    //         bool is_full = pd_name::pd_full(&pd_array[i]);
    //         //            bool is_full2 = pd_vec[i]->is_full();
    //         //            assert(is_full == is_full2);
    //         bool final = (!add_cond or is_full);
    //         // assert(final);
    //     }
    //     return count_overflowing_PD;
    // }

    //     auto count_empty_PDs() -> size_t {
    //         size_t count_empty_PD = 0;
    //         for (int i = 0; i < number_of_pd; ++i) {
    //             /* bool temp_cond = (pd_capacity_vec[i] >> 1ul) == pd_name::get_capacity(&pd_array[i]);
    //                 size_t r1 = pd_capacity_vec[i] >> 1ul;
    //                 size_t r2 = pd_name::get_capacity(&pd_array[i]);
    //                 size_t r3 = pd_name::get_capacity(&pd_array[i]);

    //                 if (!temp_cond) {
    //                     if (r1 == r2) {
    //                         std::cout << "r3 is wrong: " << r3 << "\t instead of " << r1 << std::endl;
    //                         // assert(false);

    //                     }
    //                     else {
    //                         std::cout << "r1 is: " << r1 << std::endl;
    //                         std::cout << "r2 is: " << r2 << std::endl;
    //                         std::cout << "r3 is: " << r3 << std::endl;
    //                         assert(false);
    //                     }
    //                 } */
    //             // assert((pd_name::get_capacity(pd_array[i])) == pd_name::get_capacity(&pd_array[i]));
    //             bool add_cond = (pd_name::get_capacity(&pd_array[i]) <= 0);
    //             count_empty_PD += add_cond;
    //             // bool is_full = pd_name::pd_full(&pd_array[i]);
    //             //            bool is_full2 = pd_vec[i]->is_full();
    //             //            assert(is_full == is_full2);
    //             // bool final = (!add_cond or is_full);
    //             // assert(final);
    //         }
    //         return count_empty_PD;
    //     }

    //     /**
    //          * For validation.
    //          * This test to see if the hash function could not map elements to certain interval, such as the higher ones.
    //          * Specifically by finding the longest interval of empty PDs.
    //          * @return
    //          */
    //     auto find_empty_pd_interval() -> std::tuple<size_t, size_t> {
    //         assert(0);
    //         return {42, 42};
    //     }

    //     auto get_byte_size() {
    //         //variables.
    //         size_t res = 8 * sizeof(size_t);

    //         //PD's
    //         res += sizeof(__m512i) * number_of_pd;
    //         // Capacity vec.
    //         res += sizeof(uint8_t) * number_of_pd;
    //         // Pointer to spare->
    //         res += 1;
    //         return res;
    //     }

    //     auto get_byte_size_with_spare() {
    //         auto l1_size = get_byte_size();
    //         auto l2_size = spare->get_byte_size();
    //         return l1_size + l2_size;
    //     }


    //     auto analyse_pd_status(size_t p) -> double {
    //         return -42.0;
    //         // return m512i_lp_average(pd_array, number_of_pd, p);
    //     }

    //     /* auto case_validate() -> bool {
    //             size_t index = random() % number_of_pd;
    //             // case_validate_counter++;
    //             bool res = pd_name::validate_number_of_quotient(&pd_array[index]);
    //             if (!res) {
    //                 // std::cout << "case_validate_counter: " << case_validate_counter << std::endl;
    //                 std::cout << "index: " << index << std::endl;
    //                 // std::cout << "actual val:" <<  << std::endl;
    //             }
    //             return res;
    //         }

    //     bool f(const itemType x) const {
    //         return false;
    //         return x == 1979170537;
    //     }
    // */
    // private:
    //     auto pop_attempt_by_element(const itemType s) {
    //         uint32_t b1, b2;
    //         std::tie(b1, b2) = get_elements_buckets(s);
    //         pop_attempt_by_bucket(b1);
    //         pop_attempt_by_bucket(b2);
    //     }

    //     auto pop_attempt_by_hash_val(const itemType hash_val) {
    //         uint32_t b1, b2;
    //         std::tie(b1, b2) = get_hash_val_buckets(hash_val);
    //         pop_attempt_by_bucket(b1);
    //         pop_attempt_by_bucket(b2);
    //     }


    /*
            inline auto wrap_hash(const string *s) const -> spareItemType {
                assert(false);
                assert(spare_element_length <= 32);
                return s_pd_filter::my_hash(s, HASH_SEED) & MASK(spare_element_length);
            }

            inline void wrap_split(const string *s, size_t *pd_index, D_TYPE *q, D_TYPE *r) const {
                auto h = s_pd_filter::my_hash(s, HASH_SEED);
                split(h, pd_index, q, r);
            }
        */
};


#endif //COMPARING_FILTERS_SIMPLER_FIXED_DICT_VER2_HPP
