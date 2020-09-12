
// #ifndef FILTERS_DICT320_V2_HPP
// #define FILTERS_DICT320_V2_HPP

// #include "../HashTables/hashTable_Aligned.hpp"
// #include "../hashutil.h"
// // #include "Analyse/analyse.hpp"
// // #include "pd320_v2.hpp"


// #define ATT_D320_V2_DB1 (false)
// #define ATT_D320_V2_DB2 (true & ATT_D320_V2_DB1)


// //int case, size_t pd_index, uint64_t quot, uint64_t rem,size_t insert_counter
// typedef std::tuple<int, size_t, uint64_t, uint64_t, size_t> db_key;

// template<
//         class TableType,
//         typename spareItemType,
//         typename itemType,
//         // bool round_to_upperpower2 = false,
//         typename HashFamily = hashing::TwoIndependentMultiplyShift,
//         size_t bits_per_item = 8,
//         size_t max_capacity = 32,
//         size_t quot_range = 32>
// class Dict320_v2 {

//     vector<uint16_t> pd_capacity_vec;
//     // hashTable_Aligned *spare;
//     TableType *spare;
//     HashFamily hasher;

//     size_t capacity{0};
//     const size_t filter_max_capacity;
//     const size_t remainder_length{bits_per_item},
//             quotient_range{quot_range},
//             quotient_length{ceil_log2(quot_range)},
//             single_pd_capacity{max_capacity};
//     // const uint64_t seed{12345};

//     const size_t pd_index_length, number_of_pd;
//     const size_t spare_element_length;
//     double expected_pd_capacity;
//     //    bool hashing_test;
//     __m512i *pd_array;

//     size_t insert_existing_counter = 0;

// public:
//     Dict320_v2(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
//         : filter_max_capacity(max_number_of_elements),
//           number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
//           pd_index_length(ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))),
//           spare_element_length(pd_index_length + quotient_length + remainder_length),
//           hasher()
//     //   ,
//     //   spare(ceil(1.0 * max_number_of_elements / (1.5 * ceil_log2(max_number_of_elements))),
//     //         ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)) + 14ul,
//     //         level2_load_factor)
//     {
//         // assert(upperpower2(number_of_pd) == number_of_pd);
//         // std::cout << "filter_max_capacity: " << filter_max_capacity << std::endl;
//         // std::cout << "number_of_PDs: " << number_of_pd << std::endl;
//         // std::cout << "pd_index_length: " << pd_index_length << std::endl;
//         // std::cout << "spare_element_length: " << spare_element_length << std::endl;

//         expected_pd_capacity = max_capacity * level1_load_factor;
//         assert(spare_element_length < (sizeof(spareItemType) * CHAR_BIT));
//         assert(sizeof(itemType) <= sizeof(spareItemType));

//         // size_t log2_size = ceil_log2(max_number_of_elements);
//         // size_t temp = ceil(max_number_of_elements / (double) 1.5);
//         // auto res = my_ceil(temp, log2_size);

//         // size_t spare_max_capacity = res;
//         spare = new TableType(ceil(1.0 * max_number_of_elements / (1.01 * ceil_log2(max_number_of_elements))),
//                               ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)) + 13ul,
//                               level2_load_factor);

//         int ok = posix_memalign((void **) &pd_array, 64, 64 * number_of_pd);
//         if (ok != 0) {
//             cout << "Failed!!!" << endl;
//             assert(false);
//             return;
//         }
//         std::fill(pd_array, pd_array + number_of_pd, __m512i{(INT64_C(1) << 32) - 1, 0, 0, 0, 0, 0, 0, 0});
//         pd_capacity_vec.resize(number_of_pd, 0);
//     }

//     virtual ~Dict320_v2() {
//         auto ss = get_extended_info();
//         std::cout << ss.str();

//         free(pd_array);
//         pd_capacity_vec.clear();
//         delete spare;
//     }

//     inline auto lookup(const itemType s) const -> bool {
//         uint64_t hash_res = hasher(s);

//         uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
//         const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
//         const uint64_t quot = (out2 >> 8) & 31;
//         const uint8_t rem = out2 & 255;
//         assert(pd_index < number_of_pd);
//         assert(quot < 32);
//         return pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index]);
//         // return (pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index])) || (spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem));
//         // return pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index]);
//         /* const int res = pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index]);
//         // if (res & 1) return true;
//         return (res & 1) ? true : ((res & 2) ? spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem) : false); */
//         // return ()) || spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem);
//         // return (pd_capacity_vec[pd_index] & 1u)  || (pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index]));
//         // return (pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index])) || (pd_capacity_vec[pd_index] & 1u);
//         /* return (pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index])) ||
//                ((pd_capacity_vec[pd_index] & 1u) &&
//                 spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem)); */
//     }


//     void insert(const itemType s) {
//         uint64_t hash_res = hasher(s);

//         uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
//         const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
//         const uint64_t quot = (out2 >> 8) & 31;
//         const uint8_t rem = out2 & 255;
//         assert(pd_index < number_of_pd);
//         assert(quot < 32);
//         assert(pd_index < number_of_pd);

//         if (pd320_v2::pd_add_32(quot, rem, &pd_array[pd_index]))
//             return;

//         uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
//         insert_to_spare_without_pop(spare_val);
//         // insert_to_spare_with_pop(spare_val);
//         if (ATT_D320_V2_DB2) {
//             uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
//             assert(spare->find(spare_val));
//         }
//     }


//     inline void remove(const itemType s) {
//         uint64_t hash_res = hasher(s);
//         uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
//         const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
//         const uint64_t quot = (out2 >> 8) & 31;
//         const uint8_t rem = out2 & 255;
//         assert(pd_index < number_of_pd);
//         assert(quot < 32);
//         if (pd320_v2::conditional_remove(quot, rem, &pd_array[pd_index])) {
//             // (pd_capacity_vec[pd_index]) -= 2;
//             return;
//         }

//         uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
//         spare->remove(spare_val);
//     }


//     inline void insert_to_spare_without_pop(spareItemType spare_val) {
//         spare->insert(spare_val);
//     }

//     void insert_to_spare_with_pop(spareItemType hash_val) {
//         uint32_t b1 = -1, b2 = -1;
//         spare->get_element_buckets(hash_val, &b1, &b2);
//         if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
//             return;

//         auto hold = hash_val;
//         size_t bucket = b1;
//         for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
//             if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
//                 return;
//             }
//             spare->cuckoo_swap(&hold, &bucket);
//         }
//         cout << spare->get_capacity() / ((double) spare->get_max_capacity()) << endl;
//         assert(false);
//     }


//     auto pop_attempt_by_bucket(size_t bucket_index) -> void {
//         for (int i = 0; i < spare->get_bucket_size(); ++i) {
//             if (spare->is_empty_by_bucket_index_and_location(bucket_index, i))
//                 continue;
//             auto temp_el = spare->get_element_by_bucket_index_and_location(bucket_index, i);
//             if (single_pop_attempt(temp_el)) {
//                 //                cout << "here" << endl;
//                 spare->clear_slot_bucket_index_and_location(bucket_index, i);
//             }
//         }
//     }

//     auto pop_attempt_with_insertion_by_bucket(spareItemType hash_val, size_t bucket_index) -> bool {
//         /* Todo: optimize.
//          * 1) copy the bucket using memcpy and pop every non empty element.
//          * 2) another way.
//          */

//         for (int i = 0; i < spare->get_bucket_size(); ++i) {
//             if (spare->is_empty_by_bucket_index_and_location(bucket_index, i)) {
//                 spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
//                 return true;
//             }
//             auto temp_el = spare->get_element_by_bucket_index_and_location(bucket_index, i);
//             if (single_pop_attempt(temp_el)) {
//                 spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
//                 return true;
//             }
//         }
//         return false;
//     }

//     /**
//          * reads the element if
//          * @param element
//          * @return
//          */
//     auto single_pop_attempt(spareItemType element) -> bool {
//         const uint32_t pd_index = element >> (bits_per_item + quotient_length);
//         const uint64_t quot = (element >> bits_per_item) & 31;
//         const uint8_t rem = element & 255;
//         assert(pd_index < number_of_pd);
//         assert(quot < 32);


//         if (pd320_v2::pd_add_32(quot, rem, &pd_array[pd_index])) {
//             spare->decrease_capacity();
//             return true;
//         }
//         if (ATT_D320_V2_DB1) {
//             assert(pd320_v2::is_full(&pd_array[pd_index]));
//         }
//         return false;
//     }


//     // inline auto bitwise_lookup(const itemType s) const -> bool {
//     //     uint64_t hash_res = hasher(s);
//     //     uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
//     //     const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
//     //     const uint64_t quot = (out2 >> 8) & 31;
//     //     const uint8_t rem = out2 & 255;
//     //     assert(pd_index < number_of_pd);
//     //     assert(quot < 32);
//     //     return pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index]);
//     // }

//     inline auto minimal_lookup(const itemType s) const -> bool {
//         uint64_t hash_res = hasher(s);
//         uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
//         const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
//         const uint64_t quot = (out2 >> 8) & 31;
//         const uint8_t rem = out2 & 255;
//         assert(pd_index < number_of_pd);
//         assert(quot < 32);
//         return pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index]);
//     }


//     /////////////////////////////////////////////////////////////////////////////////////////////////////////
//     /////////////////////////////////////////////////////////////////////////////////////////////////////////
//     /////////////////////////////// Validation functions.////////////////////////////////////////////////////
//     /////////////////////////////////////////////////////////////////////////////////////////////////////////
//     /////////////////////////////////////////////////////////////////////////////////////////////////////////

//     // int get_fp_level()

//     auto level_lookup(const itemType s) const -> int {
//         uint64_t hash_res = hasher(s);
//         uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
//         const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
//         const uint64_t quot = (out2 >> 8) & 31;
//         const uint8_t rem = out2 & 255;
//         assert(pd_index < number_of_pd);
//         assert(quot < 32);

//         if (pd320_v2::pd_find_32(quot, rem, &pd_array[pd_index])) {
//             return 1;
//         }
//         if (pd_capacity_vec[pd_index] & 1u) {
//             uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
//             return (spare->find(spare_val)) ? 2 : -1;
//         }
//         return -1;
//     }


//     auto squared_chi_test_basic() -> double {
//         double res = 0;
//         for (size_t i = 0; i < number_of_pd; i++) {
//             auto temp = pd320_v2::get_capacity(&pd_array[i]) - expected_pd_capacity;
//             res += temp * temp;
//         }
//         return res / (filter_max_capacity * expected_pd_capacity);
//     }

//     auto squared_chi_test() -> double {
//         vector<uint16_t> temp_capacity_vec(pd_capacity_vec);
//         vector<spareItemType> spare_elements;

//         spare->get_all_elements(&spare_elements);
//         for (size_t i = 0; i < spare_elements.size(); i++) {
//             auto temp = spare_elements[i];
//             size_t temp_pd_index = (temp >> (bits_per_item + quotient_length));
//             temp_capacity_vec[temp_pd_index]++;
//         }

//         double res = 0;
//         for (size_t i = 0; i < number_of_pd; i++) {
//             auto temp = temp_capacity_vec[i] - expected_pd_capacity;
//             res += temp * temp;
//         }
//         return res / (filter_max_capacity * expected_pd_capacity);
//     }


//     // auto validate_capacity_functions(size_t pd_index) -> bool {
//     //     bool c = (pd320_v2::get_capacity(&pd_array[pd_index]) == (pd320_v2::get_capacity_naive(&pd_array[pd_index])));
//     //     return c & pd320_v2::get_capacity(&pd_array[pd_index]) == (pd_capacity_vec[pd_index] >> 1u);
//     // }
//     auto get_extended_info() -> std::stringstream {
//         std::stringstream ss;
//         size_t temp_capacity = get_capacity();


//         // std::sum(pd_capacity_vec);
//         auto line = std::string(64, '*');
//         ss << line << std::endl;

//         ss << "filter max capacity is: " << str_format(filter_max_capacity) << std::endl;
//         ss << "l1_capacity is: " << str_format(temp_capacity) << std::endl;
//         // ss << "basic squared chi is: " << squared_chi_test_basic() << std::endl;
//         ss << "squared chi is: " << squared_chi_test() << std::endl;

//         // ss << "total capacity is: " << str_format(temp_capacity + spare->get_capacity()) << std::endl;
//         ss << "spare capacity is: " << str_format(spare->get_capacity()) << std::endl;
//         ss << "spare load factor is: " << spare->get_load_factor() << std::endl;
//         double ratio = spare->get_capacity() / (double) temp_capacity;
//         ss << "l2/l1 capacity ratio is: " << ratio << std::endl;
//         ss << "spare_element_length is: " << spare_element_length << std::endl;


//         if (insert_existing_counter) {
//             ss << "insert_existing_counter: " << insert_existing_counter << std::endl;
//             double ratio = insert_existing_counter / (double) filter_max_capacity;
//             assert(ratio > 0);
//             ss << "ratio to max capacity: " << ratio << std::endl;
//         }
//         ss << std::string(64, '.') << std::endl;
//         auto temp_ss = get_dynamic_info();
//         ss << "l1 byte size is: " << str_format(get_byte_size()) << std::endl;
//         ss << "total byte size is: " << str_format(get_byte_size_with_spare()) << std::endl;
//         ss << std::string(80, '-') << std::endl;
//         ss << temp_ss.str();
//         ss << line << std::endl;

//         // spare->get_info(&ss);
//         return ss;
//     }

//     // void get_static_info() {
//     //     const size_t num = 8;
//     //     size_t val[num] = {number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
//     //                        pd_index_length,
//     //                        quotient_length, spare_element_length};

//     //     string names[num] = {"number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
//     //                          "pd_index_length", "quotient_length", "spare_element_length"};
//     //     table_print(num, names, val);
//     // }

//     auto get_dynamic_info() -> std::stringstream {
//         /* if (!hashing_test) {
//                 std::cout << std::string(120, '$') << std::endl;
//                 std::cout << "Probably did not hit all PD's. (hashing_test is false)." << std::endl;
//                 std::cout << std::string(120, '$') << std::endl;
//             } */
//         size_t spare_capacity = spare->get_capacity();
//         size_t count_overflowing_PD = count_overflowing_PDs();
//         size_t count_empty_PD = count_empty_PDs();

//         const size_t num = 4;
//         size_t val[num] = {spare_capacity, count_overflowing_PD, count_empty_PD, number_of_pd};

//         string names[num] = {"spare_capacity", "count_overflowing_PD", "count_empty_PD", "number_of_pd"};
//         std::stringstream ss = table_print(num, names, val);
//         ss << "Overflowing PD ratio: " << (1.0 * number_of_pd / count_overflowing_PD) << std::endl; 

//         // const size_t dnum = 4;
//         // double l0 = analyse_pd_status(0);
//         // double l1 = analyse_pd_status(1);
//         // double l2 = analyse_pd_status(2);
//         // double l3 = analyse_pd_status(3);

//         // double dval[dnum] = {l0, l1, l2, l3};

//         // string dnames[dnum] = {"l0", "l1", "l2", "l3"};
//         // table_print(dnum, dnames, dval, os);

//         if (count_empty_PD > 1) {
//             auto tp = find_empty_pd_interval();
//             size_t start = std::get<0>(tp), end = std::get<1>(tp);
//             ss << "longest_empty_interval length is: " << end - start << std::endl;
//             ss << "start: " << start << std::endl;
//             ss << "end: " << end << std::endl;
//         }
//         return ss;
//         // return os;
//     }

//     auto get_elements_buckets(itemType x) -> std::tuple<uint32_t, uint32_t> {
//         return get_hash_val_buckets(wrap_hash(x));
//     }

//     auto get_hash_val_buckets(itemType hash_val) -> std::tuple<uint32_t, uint32_t> {
//         uint32_t b1 = -1, b2 = -1;
//         spare->my_hash(hash_val, &b1, &b2);
//         return std::make_tuple(b1, b2);
//     }

//     auto get_second_level_capacity() -> std::size_t {
//         return spare->get_capacity();
//     }

//     auto get_second_level_load_ratio() -> double {
//         return spare->get_capacity() / ((double) spare->get_max_capacity());
//     }

//     auto get_capacity() -> size_t {
//         size_t res = 0;

//         for (int i = 0; i < number_of_pd; ++i) {
//             res += pd320_v2::get_capacity(&pd_array[i]);
//         }
//         return res;
//     }

//     auto get_name() -> std::string {
//         return "Dict320_v2";
//     }

//     auto count_overflowing_PDs() -> size_t {
//         size_t count_overflowing_PD = 0;
//         for (int i = 0; i < number_of_pd; ++i) {
//             bool add_cond = pd320_v2::did_pd_overflowed(&pd_array[i]);
//             count_overflowing_PD += add_cond;
//         }
//         return count_overflowing_PD;
//     }

//     auto count_empty_PDs() -> size_t {
//         size_t count_empty_PD = 0;
//         for (int i = 0; i < number_of_pd; ++i) {
//             bool add_cond = pd320_v2::get_capacity(&pd_array[i]) <= 0;
//             count_empty_PD += add_cond;
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
//         size_t max_length = 0;
//         size_t start = 0, end = 0;
//         size_t temp_start = 0, temp_end = 0;
//         size_t index = 0;
//         __m512i *ppd = &pd_array[0];
//         while (index < number_of_pd) {
//             //            size_t temp_length = 0;
//             if (pd320_v2::get_capacity(&pd_array[index]) == 0) {
//                 size_t temp_length = 1;
//                 size_t temp_index = index + 1;
//                 while ((temp_index < number_of_pd) and (pd320_v2::get_capacity(&pd_array[temp_index]) == 0)) {
//                     temp_index++;
//                     temp_length++;
//                 }
//                 if (temp_index == number_of_pd) {
//                     std::cout << "h8!" << std::endl;
//                 }
//                 temp_length = temp_index - index;
//                 if (temp_length > max_length) {
//                     start = index;
//                     end = temp_index;
//                     max_length = temp_length;
//                 }
//                 index = temp_index + 1;
//             } else
//                 index++;
//         }
//         return {start, end};
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
//         return m512i_lp_average(pd_array, number_of_pd, p);
//     }

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
// };

// #endif//FILTERS_DICT320_V2_HPP
