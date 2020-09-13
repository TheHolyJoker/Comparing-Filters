
#ifndef FILTERS_DICT512_WITH_CF_HPP
#define FILTERS_DICT512_WITH_CF_HPP

// #include "Analyse/analyse.hpp"
// #include "TPD_Filter/att_hTable.hpp"
// #include "../hashutil.h"
// #include "hashTable_Aligned.hpp"
#include "../HashTables/hashTable_CuckooFilter.hpp"
#include "pd512.hpp"

// #include "hash_table.hpp"
// #include "pd512_wrapper.hpp"
// #include <cstring>

#define D512_WCF_DB1 (true)
#define D512_WCF_DB2 (true & D512_WCF_DB1)

// static size_t insert_counter = 0;
// static size_t lookup_counter = 0;
// static size_t remove_counter = 0;
// static bool hashing_test = false;
// static size_t case_validate_counter = 0;
//static size_t case_validate_counter = 0;


//int case, size_t pd_index, uint64_t quot, uint64_t rem,size_t insert_counter
typedef std::tuple<int, size_t, uint64_t, uint64_t, size_t> db_key;
template<typename itemType,
         // bool round_to_upperpower2 = false,
         typename HashFamily = hashing::TwoIndependentMultiplyShift,
         size_t bits_per_item = 8,
         size_t max_capacity = 51,
         size_t quot_range = 50>
class Dict512_With_CF {

    vector<uint16_t> pd_capacity_vec;
    hashTable_CuckooFilter *spare;
    HashFamily hasher;

    size_t capacity{0};
    const size_t filter_max_capacity;
    const size_t remainder_length{bits_per_item},
            quotient_range{quot_range},
            quotient_length{ceil_log2(quot_range)},
            single_pd_capacity{max_capacity};

    const size_t pd_index_length, number_of_pd;
    const size_t spare_element_length;
    double expected_pd_capacity;
    //    bool hashing_test;
    __m512i *pd_array;

    size_t insert_existing_counter = 0;

public:
    Dict512_With_CF(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
        : filter_max_capacity(max_number_of_elements),
          number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
          pd_index_length(ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))),
          spare_element_length(pd_index_length + quotient_length + remainder_length),
          hasher()
    //   ,
    //   spare(ceil(1.0 * max_number_of_elements / (1.5 * ceil_log2(max_number_of_elements))),
    //         spare_element_length,
    //         // ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)) + quotient_length + bits_per_item,
    //         level2_load_factor)
    {
        std::cout << "spare_element_length: " << spare_element_length << std::endl;
        // assert(spare_element_length <= 32);
        // assert(pd_index_length + bits_per_item + quotient_length <= 32);
        // assert(upperpower2(number_of_pd) == number_of_pd);
        // std::cout << "filter_max_capacity: " << filter_max_capacity << std::endl;
        // std::cout << "number_of_PDs: " << number_of_pd << std::endl;
        // std::cout << "pd_index_length: " << pd_index_length << std::endl;
        // std::cout << "spare_element_length: " << spare_element_length << std::endl;
        // expected_pd_capacity = max_capacity * level1_load_factor;
        // assert(single_pd_capacity == 51);
        // assert(spare_element_length < (sizeof(spareItemType) * CHAR_BIT));
        // assert(sizeof(itemType) <= sizeof(spareItemType));
        // using TableType = cuckoofilter<itemType, bits_per_item_l2>;
        // size_t log2_size = ceil_log2(max_number_of_elements);
        // size_t temp = ceil(max_number_of_elements / (double) 1.5);
        // auto res = my_ceil(temp, log2_size);
        // size_t spare_max_capacity = res;
        // spare = new CuckooFilter(spare_max_capacity, level2_load_factor);
        size_t log2_size = ceil_log2(max_number_of_elements);
        size_t temp = ceil(1.0 * max_number_of_elements / (1.5 * log2_size));
        // auto res = ceil(temp, log2_size);
        size_t spare_max_capacity = temp;
        spare = new hashTable_CuckooFilter(spare_max_capacity, spare_element_length, level2_load_factor);


        int ok = posix_memalign((void **) &pd_array, 64, 64 * number_of_pd);
        if (ok != 0) {
            cout << "Failed!!!" << endl;
            assert(false);
            return;
        }
        std::fill(pd_array, pd_array + number_of_pd, __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0});
        pd_capacity_vec.resize(number_of_pd, 0);
    }

    virtual ~Dict512_With_CF() {
        auto ss = get_extended_info();
        // std::cout << ss.str();
        // std::cout << "squared chi test: " << squared_chi_test() << std::endl;
        free(pd_array);
        pd_capacity_vec.clear();
        delete spare;
    }

    inline auto lookup(const itemType s) const -> bool {
        /* using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
        uint32_t out1 = 647586, out2 = 14253653;
        Hash_ns::BobHash(&s, 8, &out1, &out2);
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50); */

        uint64_t hash_res = hasher(s);
        // uint64_t hash_res = XXHash64::hash(&s, 8, seed);
        // uint64_t hash_res = wyhash64(s, seed);

        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        // const uint32_t pd_index = out1 & (number_of_pd - 1);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50);

        return (pd512::pd_find_50(quot, rem, &pd_array[pd_index])) ||
               ((pd_capacity_vec[pd_index] & 1u) &&
                spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem));
    }


    void insert(const itemType s) {
        /* using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
        uint32_t out1 = 647586, out2 = 14253653;
        Hash_ns::BobHash(&s, 8, &out1, &out2);
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        assert(quot <= 50);
        const uint8_t rem = out2 & MASK(bits_per_item);
         */

        uint64_t hash_res = hasher(s);
        // uint64_t hash_res = XXHash64::hash(&s, 8, seed);
        // uint64_t hash_res = wyhash64(s, seed);

        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        // const uint32_t pd_index = out1 & (number_of_pd - 1);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);
        // if (pd_index >= number_of_pd)
        assert(pd_index < number_of_pd);
        assert(quot <= 50);
        // assert(validate_capacity_functions(pd_index));

        if ((pd_capacity_vec[pd_index] >> 1u) == (single_pd_capacity)) {
            assert(pd512::is_full(&pd_array[pd_index]));

            pd_capacity_vec[pd_index] |= 1u;
            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
            spare->insert(spare_val);
            // insert_to_spare_without_pop(spare_val);
            // insert_to_spare_with_pop(spare_val);
            if (D512_WCF_DB2) {
                uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
                assert(spare->find(spare_val));
            }
            return;
        }

        const bool res = pd512::pd_add_50(quot, rem, &pd_array[pd_index]);
        assert(res);
        pd_capacity_vec[pd_index] += 2;
        // assert(validate_capacity_functions(pd_index));
    }

    inline void remove(const itemType s) {
        // assert(lookup(s));
        // auto level_lookup_res = level_lookup(s);

        /* using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
        uint32_t out1 = 647586, out2 = 14253653;
        Hash_ns::BobHash(&s, 8, &out1, &out2);
        uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        assert(quot <= 50);
        uint8_t rem = out2 & MASK(bits_per_item); */

        uint64_t hash_res = hasher(s);
        // uint64_t hash_res = XXHash64::hash(&s, 8, seed);
        // uint64_t hash_res = wyhash64(s, seed);

        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        // const uint32_t pd_index = out1 & (number_of_pd - 1);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50);

        if (pd512::conditional_remove(quot, rem, &pd_array[pd_index])) {
            (pd_capacity_vec[pd_index]) -= 2;
            return;
        }

        uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        spare->remove(spare_val);
    }

    auto get_extended_info() -> std::stringstream {
        std::stringstream ss;
        return ss;
    }

    // inline void insert_to_spare_without_pop(spareItemType spare_val) {
    //     spare->insert(spare_val);
    // }

    // void insert_to_spare_with_pop(spareItemType hash_val) {
    //     uint32_t b1 = -1, b2 = -1;
    //     spare->get_element_buckets(hash_val, &b1, &b2);
    //     if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
    //         return;

    //     auto hold = hash_val;
    //     size_t bucket = b1;
    //     for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
    //         if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
    //             return;
    //         }
    //         spare->cuckoo_swap(&hold, &bucket);
    //     }
    //     cout << spare->get_capacity() / ((double) spare->get_max_capacity()) << endl;
    //     assert(false);
    // }


    // auto pop_attempt_by_bucket(size_t bucket_index) -> void {
    //     for (int i = 0; i < spare->get_bucket_size(); ++i) {
    //         if (spare->is_empty_by_bucket_index_and_location(bucket_index, i))
    //             continue;
    //         auto temp_el = spare->get_element_by_bucket_index_and_location(bucket_index, i);
    //         if (single_pop_attempt(temp_el)) {
    //             //                cout << "here" << endl;
    //             spare->clear_slot_bucket_index_and_location(bucket_index, i);
    //         }
    //     }
    // }

    // auto pop_attempt_with_insertion_by_bucket(spareItemType hash_val, size_t bucket_index) -> bool {
    //     /* Todo: optimize.
    //      * 1) copy the bucket using memcpy and pop every non empty element.
    //      * 2) another way.
    //      */

    //     for (int i = 0; i < spare->get_bucket_size(); ++i) {
    //         if (spare->is_empty_by_bucket_index_and_location(bucket_index, i)) {
    //             spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
    //             return true;
    //         }
    //         auto temp_el = spare->get_element_by_bucket_index_and_location(bucket_index, i);
    //         if (single_pop_attempt(temp_el)) {
    //             spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    // /**
    //      * reads the element if
    //      * @param element
    //      * @return
    //      */
    // auto single_pop_attempt(spareItemType element) -> bool {
    //     const uint32_t pd_index = element >> (bits_per_item + quotient_length);
    //     assert(pd_index < number_of_pd);
    //     const uint64_t quot = (element >> bits_per_item) & MASK(quotient_length);
    //     assert(quot <= 50);
    //     const uint8_t rem = element & MASK(bits_per_item);


    //     if (pd_capacity_vec[pd_index] / 2 < single_pd_capacity) {
    //         //            cout << " HERE!!!" << endl;
    //         assert(!pd512::is_full(&pd_array[pd_index]));
    //         bool res = pd512::pd_add_50(quot, rem, &pd_array[pd_index]);
    //         assert(res);

    //         (pd_capacity_vec[pd_index]) += 2;
    //         spare->decrease_capacity();

    //         // cout << "element with hash_val: (" << element << ") was pop." << endl;
    //         return true;
    //     }
    //     if (D512_WCF_DB1) {
    //         assert(pd512::is_full(&pd_array[pd_index]));
    //     }
    //     return false;
    // }

    // inline auto new_hasher(const itemType x) const -> uint64_t {
    //     using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     uint32_t out1 = 647586, out2 = 14253653;
    //     Hash_ns::BobHash(&x, 8, &out1, &out2);
    //     uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
    //     assert(quot <= 50);
    //     // uint32_t quot = (out2 >> bits_per_item) & MASK(quotient_length);
    //     // uint32_t quot = (out2 >> bits_per_item) % quot_range;

    //     uint8_t rem = out2 & MASK(bits_per_item);

    //     // size_t pd_index = reduce2(h1, number_of_pd);
    //     // uint32_t quot = reduce32((uint32_t)(out2 >> bits_per_item), (uint32_t)quot_range);
    //     // uint64_t quot = reduce2((uint32_t) h2, (uint32_t) quot_range);
    //     // uint8_t rem = h3 & MASK(bits_per_item);

    //     // size_t pd_index = h1 % number_of_pd;
    //     // uint64_t quot = h2 % quot_range;
    //     // uint8_t rem = h3 & MASK(bits_per_item);
    //     // unsigned long long h1 = all_hash & MASK(32ull);
    //     // unsigned long long h2 = all_hash >> 32ull;
    //     // *pd_index1 = h1 % number_of_pd;
    //     assert(quotient_length == 6);
    //     uint64_t upper = pd_index << (quotient_length + bits_per_item);
    //     uint64_t mid = quot << bits_per_item;
    //     uint8_t lower = rem;
    //     bool c = ((upper & mid) == 0);
    //     c &= ((upper & lower) == 0);
    //     c &= ((mid & lower) == 0);
    //     if (!c) {
    //         std::cout << "before shift pd_index: " << pd_index << std::endl;
    //         std::cout << "after shift pd_index: " << (pd_index << (quotient_length + bits_per_item)) << std::endl;
    //         std::cout << "before shift quot: " << quot << std::endl;
    //         std::cout << "after shift quot: " << (quot << (bits_per_item)) << std::endl;
    //         assert(false);
    //     }
    //     // assert(upper & mid == 0);
    //     // assert(upper & lower == 0);
    //     // assert(mid & lower == 0);
    //     return upper | mid | lower;
    // }


    // inline auto new_hasher2(const itemType x) const -> uint64_t {
    //     // uint64_t out[2];
    //     // s_pd_filter::MurmurHash3_x86_128(&x, sizeof(x), seed, out);
    //     // uint64_t upper = (out[0] % number_of_pd) << (quotient_length + bits_per_item);
    //     // uint64_t mid = ((out[1] >> 32ul) % quot_range) << bits_per_item;
    //     // uint8_t lower = out[1] & MASK(bits_per_item);
    //     // return upper | mid | lower;

    //     uint64_t out[2];
    //     s_pd_filter::MurmurHash3_x86_128(&x, sizeof(x), -42, out);

    //     assert(reduce32(out[0], number_of_pd) < number_of_pd);
    //     uint64_t upper = reduce32(out[0], number_of_pd) << (quotient_length + bits_per_item);

    //     assert((out[1] >> 32ul) != 0);
    //     uint64_t new_quot = reduce32((uint32_t)(out[1] >> 32ul), (uint32_t) quotient_range);
    //     assert(new_quot < quotient_range);
    //     uint64_t mid = new_quot << bits_per_item;

    //     // if (new_quot == 49) {
    //     //     std::cout << "h1!" << std::endl;
    //     // }
    //     assert((out[1] & MASK(bits_per_item)) < 256);
    //     uint8_t lower = out[1] & MASK(bits_per_item);
    //     // if (lower == 133) {
    //     //     std::cout << "h2!" << std::endl;
    //     // }
    //     return upper | mid | lower;
    //     // uint8_t rem =
    //     // size_t pd_index = out[0] % number_of_pd;
    //     // uint64_t quot = (out[1] >> 32ul) % quot_range;
    //     // assert (rem == (rem & MASK(remainder_length)));
    //     // return std::make_tuple(pd_index, quot, rem);
    // }


    // inline auto merge_hash_result(const itemType x) const -> spareItemType {
    //     size_t pd_index;
    //     uint64_t quot;
    //     uint8_t rem;
    //     std::tie(pd_index, quot, rem);
    //     return merge_hash_result(pd_index, quot, rem);
    // }

    // inline auto merge_hash_result(const size_t pd_index, uint64_t quot, uint8_t rem) const -> spareItemType {
    //     spareItemType res = (pd_index << (quotient_length + remainder_length)) | (quot << quotient_length) | rem;
    //     bool cond = (res == (res & MASK(spare_element_length)));
    //     if (!cond) {
    //         bool c1 = (pd_index == pd_index & MASK(pd_index_length));
    //         bool c2 = (quot == quot & MASK(quotient_length));
    //         bool c3 = (rem == rem & MASK(remainder_length));
    //         if (!c1) {
    //             std::cout << "c1 failed" << std::endl;
    //         }
    //         if (!c2) {
    //             std::cout << "c2 failed" << std::endl;
    //         }
    //         if (!c3) {
    //             std::cout << "c3 failed" << std::endl;
    //         }
    //         // if (pd_index)
    //         bool c4 = c1 & c2 & c3;
    //         if (!c4) {
    //             std::cout << "pd_index: " << pd_index << std::endl;
    //             std::cout << "quot: " << quot << std::endl;
    //             std::cout << "rem: " << rem << std::endl;
    //             assert(false);
    //         }
    //     }
    //     return res;
    // }


    // inline auto wrap_hash(itemType x) const -> spareItemType {
    //     using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     uint64_t upper = (Hash_ns::MurmurHash(&x, 8, 124369));
    //     uint64_t lower = (Hash_ns::MurmurHash(&x, 8, 45931));
    //     return (upper << 32ul) | lower;
    // }

    // void split(itemType h, size_t *pd_index, uint32_t *q, uint32_t *r) const {
    //     *pd_index = h % number_of_pd;
    //     *q = (h >> pd_index_length) % (quotient_range);
    //     *r = h & MASK(remainder_length);// r is has dependency in pd_index, and q. not sure how to solve it without an PRG
    // }


    // inline auto bitwise_lookup(const itemType s) const -> bool {
    //     using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     uint32_t out1 = 647586, out2 = 14253653;
    //     Hash_ns::BobHash(&s, 8, &out1, &out2);
    //     const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     // const uint16_t quot = reduce16((uint16_t) out2, (uint16_t) quot_range);
    //     const uint8_t rem = out2 & 8u;


    //     // const uint32_t pd_index = out1 % number_of_pd;
    //     // const uint64_t quot = (out2 >> 8) % quot_range;
    //     // const uint8_t rem = out2 % 255;
    //     //
    //     // const uint64_t quot = (((out2 >> 8u) & 63u) < 51) ? ((out2 >> 8u) & 63u) : 63 - ((out2 >> 8u) & 63u);
    //     const uint64_t quot = (out2 >> 8u) & 63u;

    //     // const uint8_t rem = out2 & 255u;

    //     // const uint32_t pd_index = out1 & number_of_pd;
    //     // const uint64_t quot = (out2 >> 8u) & quot_range;
    //     // const uint8_t rem = out2 & 255u;

    //     // return pd512::pd_find_50(quot, rem, &pd_array[pd_index]);

    //     // return (pd512::pd_find_50(quot, rem, &pd_array[pd_index])) ||
    //     //        ((pd_capacity_vec[pd_index] & 1u) &&
    //     //         spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem));// const uint32_t pd_index =
    //     // const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     // assert(pd_index < number_of_pd);
    //     // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
    //     // assert(quot <= 50);
    //     // const uint8_t rem = out2 & MASK(bits_per_item);

    //     return pd512::pd_find_50(quot, rem, &pd_array[pd_index]);
    // }

    // inline auto minimal_lookup(const itemType s) const -> bool {
    //     uint64_t hash_res = hasher(s);
    //     uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
    //     const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     const uint64_t quot = (uint64_t) reduce16((uint16_t) out2, quot_range);
    //     const uint8_t rem = out2 & MASK(bits_per_item);
    //     assert(pd_index < number_of_pd);
    //     assert(quot <= 50);
    //     return (pd512::pd_find_50(quot, rem, &pd_array[pd_index]));

    //     /*
    //     // using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     // uint32_t out1 = 647586, out2 = 14253653;
    //     // uint64_t out = xorshf96();
    //     const uint32_t out1 = s_pd_filter::hashint((uint64_t) s);
    //     const uint32_t out2 = s_pd_filter::hashint2((uint64_t) s);
    //     // Hash_ns::BobHash(&s, 8, &out1, &out2);
    //     // return pd512::pd_find_50(
    //     //         reduce32((uint32_t) out2, (uint32_t) quot_range),
    //     //         out2 & MASK(bits_per_item),
    //     //         &pd_array[reduce32(out1, (uint32_t) number_of_pd)]);


    //     return pd512::pd_find_50(
    //             (((out2 >> 8u) & 63) < 51) ? ((out2 >> 8u) & 63) : 63 - ((out2 >> 8u) & 63),
    //             // (out2 >> 8u) & 63u,
    //             out2 & 255u,
    //             &pd_array[out1 & 524287ul]);
    //     // const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     // assert(pd_index < number_of_pd);
    //     // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
    //     // assert(quot <= 50);
    //     // const uint8_t rem = out2 & MASK(bits_per_item);

    //     // return pd512::pd_find_50(quot, rem, &pd_array[pd_index]); */
    // }

    // /* inline void remove_helper(spareItemType hash_val) {
    //     // if (D512_WCF_DB1)
    //     //     assert(lookup_helper(hash_val));
    //     using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     uint32_t out1 = 647586, out2 = 14253653;
    //     Hash_ns::BobHash(&x, 8, &out1, &out2);
    //     uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
    //     assert(quot <= 50);
    //     uint8_t rem = out2 & MASK(bits_per_item);


    //     if (pd512::conditional_remove(quot, r, &pd_array[pd_index])) {
    //         (pd_capacity_vec[pd_index]) -= 2;
    //         return;
    //     }
    //     // assert(false);
    //     spare->remove(hash_val);
    //     //        pop_attempt_by_hash_val(hash_val);
    // } */

    // /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // /////////////////////////////// Validation functions.////////////////////////////////////////////////////
    // /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // /////////////////////////////////////////////////////////////////////////////////////////////////////////

    // // int get_fp_level()

    // auto level_lookup(const itemType s) const -> int {
    //     using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     uint32_t out1 = 647586, out2 = 14253653;

    //     Hash_ns::BobHash(&s, 8, &out1, &out2);

    //     uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     assert(pd_index < number_of_pd);
    //     uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
    //     assert(quot <= 50);
    //     uint8_t rem = out2 & MASK(bits_per_item);

    //     if (pd512::pd_find_50(quot, rem, &pd_array[pd_index])) {
    //         return 1;
    //     }
    //     if (pd_capacity_vec[pd_index] & 1u) {
    //         uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
    //         return (spare->find(spare_val)) ? 2 : -1;
    //     }
    //     return -1;
    // }


    // auto squared_chi_test_basic() -> double {
    //     double res = 0;
    //     for (size_t i = 0; i < number_of_pd; i++) {
    //         auto temp = pd512::get_capacity(&pd_array[i]) - expected_pd_capacity;
    //         res += temp * temp;
    //     }
    //     return res / (filter_max_capacity * expected_pd_capacity);
    // }

    // auto squared_chi_test() -> double {
    //     vector<uint16_t> temp_capacity_vec(pd_capacity_vec);
    //     vector<spareItemType> spare_elements;

    //     spare->get_all_elements(&spare_elements);
    //     for (size_t i = 0; i < spare_elements.size(); i++) {
    //         auto temp = spare_elements[i];
    //         size_t temp_pd_index = (temp >> (bits_per_item + quotient_length));
    //         temp_capacity_vec[temp_pd_index]++;
    //     }

    //     double res = 0;
    //     for (size_t i = 0; i < number_of_pd; i++) {
    //         auto temp = temp_capacity_vec[i] - expected_pd_capacity;
    //         res += temp * temp;
    //     }
    //     return res / (filter_max_capacity * expected_pd_capacity);
    // }


    // auto validate_capacity_functions(size_t pd_index) -> bool {
    //     bool c = (pd512::get_capacity(&pd_array[pd_index]) == (pd512::get_capacity_naive(&pd_array[pd_index])));
    //     return c & pd512::get_capacity(&pd_array[pd_index]) == (pd_capacity_vec[pd_index] >> 1u);
    // }
    // auto get_extended_info() -> std::stringstream {
    //     std::stringstream ss;
    //     size_t temp_capacity = 0;
    //     for (size_t i = 0; i < number_of_pd; i++) {
    //         temp_capacity += (pd_capacity_vec[i] >> 1u);
    //     }

    //     // std::sum(pd_capacity_vec);
    //     auto line = std::string(64, '*');
    //     ss << line << std::endl;

    //     ss << "filter max capacity is: " << str_format(filter_max_capacity) << std::endl;
    //     ss << "l1_capacity is: " << str_format(temp_capacity) << std::endl;
    //     ss << "basic squared chi is: " << squared_chi_test_basic() << std::endl;
    //     ss << "squared chi is: " << squared_chi_test() << std::endl;

    //     // ss << "total capacity is: " << str_format(temp_capacity + spare->get_capacity()) << std::endl;
    //     ss << "spare capacity is: " << str_format(spare->get_capacity()) << std::endl;
    //     ss << "spare load factor is: " << spare->get_load_factor() << std::endl;
    //     double ratio = spare->get_capacity() / (double) temp_capacity;
    //     ss << "l2/l1 capacity ratio is: " << ratio << std::endl;
    //     ss << "spare_element_length is: " << spare_element_length << std::endl;


    //     if (insert_existing_counter) {
    //         ss << "insert_existing_counter: " << insert_existing_counter << std::endl;
    //         double ratio = insert_existing_counter / (double) filter_max_capacity;
    //         assert(ratio > 0);
    //         ss << "ratio to max capacity: " << ratio << std::endl;
    //     }
    //     ss << std::string(64, '.') << std::endl;
    //     auto temp_ss = get_dynamic_info();
    //     ss << "l1 byte size is: " << str_format(get_byte_size()) << std::endl;
    //     ss << "total byte size is: " << str_format(get_byte_size_with_spare()) << std::endl;
    //     ss << std::string(80, '-') << std::endl;
    //     ss << temp_ss.str();
    //     ss << line << std::endl;

    //     // spare->get_info(&ss);
    //     return ss;
    // }

    // // void get_static_info() {
    // //     const size_t num = 8;
    // //     size_t val[num] = {number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length,
    // //                        pd_index_length,
    // //                        quotient_length, spare_element_length};

    // //     string names[num] = {"number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
    // //                          "pd_index_length", "quotient_length", "spare_element_length"};
    // //     table_print(num, names, val);
    // // }

    // auto get_dynamic_info() -> std::stringstream {
    //     /* if (!hashing_test) {
    //             std::cout << std::string(120, '$') << std::endl;
    //             std::cout << "Probably did not hit all PD's. (hashing_test is false)." << std::endl;
    //             std::cout << std::string(120, '$') << std::endl;
    //         } */
    //     size_t spare_capacity = spare->get_capacity();
    //     size_t count_overflowing_PD = count_overflowing_PDs();
    //     size_t count_empty_PD = count_empty_PDs();

    //     const size_t num = 4;
    //     size_t val[num] = {spare_capacity, count_overflowing_PD, count_empty_PD, number_of_pd};

    //     string names[num] = {"spare_capacity", "count_overflowing_PD", "count_empty_PD", "number_of_pd"};
    //     std::stringstream ss = table_print(num, names, val);

    //     // const size_t dnum = 4;
    //     // double l0 = analyse_pd_status(0);
    //     // double l1 = analyse_pd_status(1);
    //     // double l2 = analyse_pd_status(2);
    //     // double l3 = analyse_pd_status(3);

    //     // double dval[dnum] = {l0, l1, l2, l3};

    //     // string dnames[dnum] = {"l0", "l1", "l2", "l3"};
    //     // table_print(dnum, dnames, dval, os);

    //     if (count_empty_PD > 1) {
    //         auto tp = find_empty_pd_interval();
    //         size_t start = std::get<0>(tp), end = std::get<1>(tp);
    //         ss << "longest_empty_interval length is: " << end - start << std::endl;
    //         ss << "start: " << start << std::endl;
    //         ss << "end: " << end << std::endl;
    //     }
    //     return ss;
    //     // return os;
    // }

    // auto get_elements_buckets(itemType x) -> std::tuple<uint32_t, uint32_t> {
    //     return get_hash_val_buckets(wrap_hash(x));
    // }

    // auto get_hash_val_buckets(itemType hash_val) -> std::tuple<uint32_t, uint32_t> {
    //     uint32_t b1 = -1, b2 = -1;
    //     spare->my_hash(hash_val, &b1, &b2);
    //     return std::make_tuple(b1, b2);
    // }

    // auto get_second_level_capacity() -> std::size_t {
    //     return spare->get_capacity();
    // }

    // auto get_second_level_load_ratio() -> double {
    //     return spare->get_capacity() / ((double) spare->get_max_capacity());
    // }

    auto get_capacity() -> size_t {
        size_t res = 0;
        size_t validate_res = 0;
        // __m512i *ppd = &(pd_array[0]);
        for (int i = 0; i < number_of_pd; ++i) {
            res += pd512::get_capacity(&pd_array[i]);
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
        return "Dict512_With_CF";
        /* string a = "dict512:\t";
            string b = pd512::get_name() + "\t";
            //        string b = pd_vec[0]->get_name() + "\t";
            string c = spare->get_name();
            return a + b + c; */
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
            // assert(final);
        }
        return count_overflowing_PD;
    }

    auto count_empty_PDs() -> size_t {
        size_t count_empty_PD = 0;
        for (int i = 0; i < number_of_pd; ++i) {
            /* bool temp_cond = (pd_capacity_vec[i] >> 1ul) == pd512::get_capacity(&pd_array[i]);
                size_t r1 = pd_capacity_vec[i] >> 1ul;
                size_t r2 = pd512::get_capacity(&pd_array[i]);
                size_t r3 = pd512::get_capacity(&pd_array[i]);

                if (!temp_cond) {
                    if (r1 == r2) {
                        std::cout << "r3 is wrong: " << r3 << "\t instead of " << r1 << std::endl;
                        // assert(false);

                    }
                    else {
                        std::cout << "r1 is: " << r1 << std::endl;
                        std::cout << "r2 is: " << r2 << std::endl;
                        std::cout << "r3 is: " << r3 << std::endl;
                        assert(false);
                    }
                } */
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
            } else
                index++;
        }
        return {start, end};
    }

    auto get_byte_size() {
        //variables.
        size_t res = 8 * sizeof(size_t);

        //PD's
        res += sizeof(__m512i) * number_of_pd;
        // Capacity vec.
        res += sizeof(uint8_t) * number_of_pd;
        // Pointer to spare->
        res += 1;
        return res;
    }

    // auto get_byte_size_with_spare() {
    //     auto l1_size = get_byte_size();
    //     auto l2_size = spare->get_byte_size();
    //     return l1_size + l2_size;
    // }


    // auto analyse_pd_status(size_t p) -> double {
    //     return m512i_lp_average(pd_array, number_of_pd, p);
    // }

    /* auto case_validate() -> bool {
            size_t index = random() % number_of_pd;
            // case_validate_counter++;
            bool res = pd512::validate_number_of_quotient(&pd_array[index]);
            if (!res) {
                // std::cout << "case_validate_counter: " << case_validate_counter << std::endl;
                std::cout << "index: " << index << std::endl;
                // std::cout << "actual val:" <<  << std::endl;
            }
            return res;
        } 

    bool f(const itemType x) const {
        return false;
        return x == 1979170537;
    }
*/
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


    //     /*
    //             inline auto wrap_hash(const string *s) const -> spareItemType {
    //                 assert(false);
    //                 assert(spare_element_length <= 32);
    //                 return s_pd_filter::my_hash(s, HASH_SEED) & MASK(spare_element_length);
    //             }

    //             inline void wrap_split(const string *s, size_t *pd_index, D_TYPE *q, D_TYPE *r) const {
    //                 auto h = s_pd_filter::my_hash(s, HASH_SEED);
    //                 split(h, pd_index, q, r);
    //             }
    //         */


    //     auto unSplit(uint32_t r, uint32_t quot, size_t pd_index) const -> spareItemType {
    //         assert(false);
    //         assert(quot_range == 50);
    //         spareItemType res = pd_index;
    //         res = (res << quotient_length) | quot;
    //         res = (res << remainder_length) | r;
    //         if (!flip) {
    //             flip = true;
    //             assert(check_unSplit(res));
    //             flip = false;
    //         }
    //         return res;
    //     }

    //     auto check_unSplit(itemType hash_val) const -> bool {
    //         size_t pd_index = -1;
    //         uint32_t quot = -1, r = -1;
    //         split(hash_val, &pd_index, &quot, &r);
    //         auto temp = hash_val & MASK(spare_element_length);
    //         bool res = temp == (hash_val & spare_element_length);
    //         if (!res) {
    //             std::cout << "temp: " << temp << std::endl;
    //             std::cout << "hash_val & spare_element_length: " << (hash_val & spare_element_length) << std::endl;
    //             std::cout << "hash_val " << (hash_val) << std::endl;
    //         }
    //         return (temp == (hash_val & spare_element_length));
    //     }

    //     auto w_insert(const itemType s) -> db_key {
    //         if (insert_counter == 0) {
    //             std::cout << "number of pd:" << number_of_pd << std::endl;
    //         }
    //         using namespace pd512;
    //         db_key w_res ={ 0, 0, 0, 0, 0 };
    //         uint64_t __attribute__((aligned(64))) temp_arr[6][8];
    //         std::get<4>(w_res) = insert_counter;
    //         insert_counter++;
    //         spareItemType hash_val = wrap_hash(s);
    //         size_t pd_index = -1;
    //         uint32_t quot = -1, r = -1;
    //         split(hash_val, &pd_index, &quot, &r);
    //         bool limits_cond = (pd_index == 0) or (pd_index + 1 == number_of_pd);
    //         if (limits_cond) {
    //             std::get<0>(w_res) = -1;
    //             return w_res;
    //         }
    //         std::get<1>(w_res) = pd_index;
    //         std::get<2>(w_res) = quot;
    //         std::get<3>(w_res) = r;
    //         int *v_res = &(std::get<0>(w_res));
    //         int b = 1ul;
    //         __m512i *ppd = &pd_array[pd_index];
    //         bool BPC = (pd_index == 91379);
    //         if (BPC) {
    //             std::cout << "h2" << std::endl;
    //         }
    //         /* bool cond = (pd_index == 2278) or (pd_index == 9249);
    //         if (cond)
    //         {
    //             std::cout << "HERE! " << std::endl;
    //             ;
    //             std::cout << "insert counter is: " << insert_counter << std::endl;
    //             std::cout << "pd_index:\t" << pd_index << std::endl;
    //             std::cout << "quot:\t" << quot << std::endl;
    //             std::cout << "r:\t" << r << std::endl;
    //         } */
    //         assert(pd_index < number_of_pd);
    //         // size_t cap_res = pd512::get_capacity(&pd_array[pd_index]);
    //         // size_t cap_valid = pd_capacity_vec[pd_index] / 2;
    //         assert(pd512::get_capacity(ppd) == pd_capacity_vec[pd_index] / 2);
    //         if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
    //             assert(pd512::is_full(ppd));
    //             pd_capacity_vec[pd_index] |= 1u;
    //             /**Todo!*/
    //             insert_to_spare_without_pop(hash_val);
    //             // insert_to_spare_with_pop(hash_val & MASK(spare_element_length));
    //             return w_res;
    //         }
    //         //        auto res2 = pd_vec[pd_index]->insert(quot, r);
    //         bool c1 = validate_number_of_quotient(ppd - 1);
    //         bool c2 = validate_number_of_quotient(ppd);
    //         bool c3 = validate_number_of_quotient(ppd + 1);
    //         *v_res |= (!c1 * b);
    //         b <<= 1u;
    //         *v_res |= (!c2 * b);
    //         b <<= 1u;
    //         *v_res |= (!c3 * b);
    //         b <<= 1u;
    //         /* if (cond)
    //         {
    //             auto line = std::string(64, '*');
    //             _mm512_store_si512(temp_arr[0], *(ppd - 1));
    //             _mm512_store_si512(temp_arr[1], *(ppd));
    //             _mm512_store_si512(temp_arr[2], *(ppd + 1));
    //             std::cout << 0 << ")" << std::endl;
    //             pd512::print512(ppd - 1);
    //             print_8array(temp_arr[0]);
    //             print_array(temp_arr[0], 8);
    //             std::cout << line << std::endl;
    //
    //             std::cout << 1 << ")" << std::endl;
    //             pd512::print512(ppd);
    //             print_8array(temp_arr[1]);
    //             print_array(temp_arr[1], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 2 << ")" << std::endl;
    //             pd512::print512(ppd);
    //             print_8array(temp_arr[2]);
    //             print_array(temp_arr[2], 8);
    //             std::cout << line << std::endl;
    //         }
    //  */
    //  // auto res = pd512::pd_add_50(quot, r, ppd);
    //         auto res = inlining_pd_add_50(quot, r, ppd, pd_index);
    //         /* if (cond)
    //         {
    //             auto line = std::string(64, '*');
    //             _mm512_store_si512(temp_arr[3], *(ppd - 1));
    //             _mm512_store_si512(temp_arr[4], *ppd);
    //             _mm512_store_si512(temp_arr[5], *(ppd + 1));
    //             std::cout << 3 << ")" << std::endl;
    //             pd512::print512(ppd - 1);
    //             print_8array(temp_arr[3]);
    //             print_array(temp_arr[3], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 4 << ")" << std::endl;
    //             pd512::print512(ppd);
    //             print_8array(temp_arr[4]);
    //             print_array(temp_arr[4], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 5 << ")" << std::endl;
    //             pd512::print512(ppd + 1);
    //             print_8array(temp_arr[5]);
    //             print_array(temp_arr[5], 8);
    //             std::cout << line << std::endl;
    //             std::cout << "0,3: " << (memcmp(temp_arr[0], temp_arr[3], 64)) << std::endl;
    //             std::cout << "1,4: " << (memcmp(temp_arr[1], temp_arr[4], 64)) << std::endl;
    //             std::cout << "2,5: " << (memcmp(temp_arr[2], temp_arr[5], 64)) << std::endl;
    //         } */
    //         c1 = validate_number_of_quotient(ppd - 1);
    //         c2 = validate_number_of_quotient(ppd);
    //         c3 = validate_number_of_quotient(ppd + 1);
    //         *v_res |= (!c1 * b);
    //         b <<= 1u;
    //         *v_res |= (!c2 * b);
    //         b <<= 1u;
    //         *v_res |= (!c3 * b);
    //         /* assert(pd512::validate_number_of_quotient(&pd_array[pd_index - 1]));
    //         assert(pd512::validate_number_of_quotient(&pd_array[pd_index]));
    //         bool temp_failed = pd512::validate_number_of_quotient(&pd_array[pd_index + 1]);
    //         if (!temp_failed)
    //         {
    //             std::cout << "in Failed:" << std::endl;
    //             std::cout << "pd_index:\t" << pd_index << std::endl;
    //             std::cout << "cond:\t" << cond << std::endl;
    //             // assert(false);
    //         } */
    //         // assert();
    //         if (!res) {
    //             cout << "insertion failed!!!" << std::endl;
    //             assert(false);
    //         }
    //         (pd_capacity_vec[pd_index] += 2);
    //         return w_res;
    //     }

    //     void old_insert(const itemType s) {
    //         using namespace std;
    //         auto w_res = w_insert(s);
    //         int ans = get<0>(w_res);
    //         if (ans > 0) {
    //             std::cout << "h1" << std::endl;
    //             const size_t var_num = 5;
    //             size_t vals[var_num] ={ get<0>(w_res), get<1>(w_res), get<2>(w_res), get<3>(w_res), get<4>(w_res) };
    //             string names[var_num] ={ "indicator", "pd_index", "quot", "rem", "insert_count" };
    //             size_t pd_index = get<1>(w_res);
    //             table_print(var_num, names, vals);
    //             bin_print(get<0>(w_res));
    //             std::cout << "pd_capacity_vec[pd_index - 1]: " << pd_capacity_vec[pd_index - 1] << "(" << pd_index - 1
    //                 << ")" << std::endl;
    //             std::cout << "pd_capacity_vec[pd_index]: " << pd_capacity_vec[pd_index] << "(" << pd_index << ")"
    //                 << std::endl;
    //             std::cout << "pd_capacity_vec[pd_index + 1]: " << pd_capacity_vec[pd_index + 1] << "(" << pd_index + 1
    //                 << ")" << std::endl;
    //             assert(false);
    //         }
    //         if (ans == 0)
    //             return;
    //         if (ans == -1) {
    //             spareItemType hash_val = wrap_hash(s);
    //             size_t pd_index = -1;
    //             uint32_t quot = -1, r = -1;
    //             split(hash_val, &pd_index, &quot, &r);
    //             assert(pd_index < number_of_pd);
    //             // __m512i temp_pd = pd_array[pd_index];
    //             assert(pd512::get_capacity(&pd_array[pd_index]) == pd_capacity_vec[pd_index] / 2);
    //             if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
    //                 assert(pd512::is_full(&pd_array[pd_index]));
    //                 pd_capacity_vec[pd_index] |= 1u;
    //                 /**Todo!*/
    //                 insert_to_spare_without_pop(hash_val);
    //                 // insert_to_spare_with_pop(hash_val & MASK(spare_element_length));
    //                 //            insert_to_spare_without_pop(hash_val & MASK(spare_element_length));
    //                 return;
    //             }
    //             auto res = inlining_pd_add_50(quot, r, &pd_array[pd_index], pd_index);
    //             if (!res) {
    //                 cout << "insertion failed!!!" << std::endl;
    //                 assert(false);
    //             }
    //             (pd_capacity_vec[pd_index] += 2);
    //         }
    //     }
};

#endif//FILTERS_DICT512_WITH_CF_HPP
