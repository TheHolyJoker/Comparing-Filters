
#ifndef FILTERS_DICT512_VER3_HPP
#define FILTERS_DICT512_VER3_HPP


#include "../hashutil.h"
#include "HashTables/hashTable_Aligned.hpp"
// #include "pd512.hpp"
#include "pd512_plus.hpp"
// #include <immintrin.h>

#define DICT512_VER3_DB1 (true)
#define DICT512_VER3_DB2 (true & DICT512_VER3_DB1)
#define DICT512_VER3_DB3 (true & DICT512_VER3_DB2)
// #define DICT512_VER3_SF_BITS (16)
// #define DICT512_VER3_SF_MASK (MASK(DICT512_VER3_SF_BITS))


typedef std::tuple<int, size_t, uint64_t, uint64_t, size_t> db_key;

/**
     * 1 : x > y
     * 0 : x == y
     * -1: x < y
     * */
template<typename T>
inline int my_cmp(T x, T y) {
    return (x == y) + 2 * (y < x) - 1;
}


template<typename T, typename S>
inline int my_cmp(T x, S y) {
    return (x == y) + 2 * (y < x) - 1;
}

// enum lookup_result {
//   found_on_l1 = 0,
//   l2_search_avoided = 1,
//   found_on_l2 = 2,
//   fu
// };


template<
        class TableType,
        typename spareItemType,
        typename itemType,
        // bool round_to_upperpower2 = false,
        typename HashFamily = hashing::TwoIndependentMultiplyShift,
        size_t bits_per_item = 8,
        size_t max_capacity = 51,
        size_t quot_range = 50>
class Dict512_Ver3 {

    // vector<uint16_t> last_q_vec;
    // vector<uint16_t> max_qr_could_insert;
    // vector<uint16_t> max_qr_did_insert;
    // vector<vector<uint16_t>> all_qr;
    // vector<uint16_t> pd_capacity_vec;
    // vector<uint64_t> pd_simple_filter_vec;
    // TableType *spare;
    hashTable_Aligned<uint64_t, 4> *spare;
    HashFamily Hasher;

    // size_t l1_capacity{0};
    // size_t total_capacity{0};
    const size_t filter_max_capacity;
    const size_t remainder_length{bits_per_item},
            quotient_range{quot_range},
            quotient_length{ceil_log2(quot_range)},
            single_pd_capacity{max_capacity};
    const size_t pd_index_length, number_of_pd;
    const size_t spare_element_length;
    double expected_pd_capacity;

    __m512i *pd_array;

    // size_t insert_existing_counter = 0;

    //{Found, Not Found}
    // size_t searched_on_l1[2] = {0, 0};
    // size_t searched_on_l2[2] = {0, 0};
    // size_t l2_search_avoided_counter = 0;
    // size_t l2_search_that_was_not_avoided_but_should_have = 0;
    // std::stringstream *counters_stream;

public:
    Dict512_Ver3(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
        : filter_max_capacity(max_number_of_elements),
          number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
          pd_index_length(ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))),
          spare_element_length(pd_index_length + quotient_length + remainder_length),
          Hasher() {
        // assert(upperpower2(number_of_pd) == number_of_pd);
        // std::cout << "filter_max_capacity: " << filter_max_capacity << std::endl;
        // std::cout << "number_of_PDs: " << number_of_pd << std::endl;
        // std::cout << "pd_index_length: " << pd_index_length << std::endl;
        // std::cout << "spare_element_length: " << spare_element_length << std::endl;
        // searched_on_l1[0] = 0;
        // searched_on_l2[0] = 0;
        // searched_on_l1[1] = 0;
        // searched_on_l2[1] = 0;

        expected_pd_capacity = max_capacity * level1_load_factor;
        //        hashing_test = false;
        assert(single_pd_capacity == 51);
        assert(spare_element_length < (sizeof(spareItemType) * CHAR_BIT));
        assert(sizeof(itemType) <= sizeof(spareItemType));

        // spare = new TableType(ceil(1.0 * max_number_of_elements / (1.5 * ceil_log2(max_number_of_elements))),
        spare = new hashTable_Aligned<uint64_t, 4>(ceil(2.5 * max_number_of_elements / (4.0 * ceil_log2(max_number_of_elements))),
                                                   ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)) + 14ul,
                                                   level2_load_factor);

        int ok = posix_memalign((void **) &pd_array, 64, 64 * number_of_pd);
        if (ok != 0) {
            cout << "Failed!!!" << endl;
            assert(false);
            return;
        }
        std::fill(pd_array, pd_array + number_of_pd, __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0});
        // last_q_vec.resize(number_of_pd, 0);
        // max_qr_could_insert.resize(number_of_pd, 0);
        // max_qr_did_insert.resize(number_of_pd, 0);
        // all_qr.resize(number_of_pd, vector<uint16_t>(51));
        // v_pd512_plus::decode_by_table_validator();
        // pd_capacity_vec.resize(number_of_pd, 0);
        // pd_simple_filter_vec.resize(number_of_pd, 0);
    }

    virtual ~Dict512_Ver3() {
        assert(get_capacity() >= 0);
        // lookup_count(0, 2);
        // auto ss = get_extended_info();
        // std::cout << ss.str();
        // std::cout << "squared chi test: " << squared_chi_test() << std::endl;

        // std::cout << counters_stream->str() << std::endl;
        // delete counters_stream;
        // pd_capacity_vec.clear();
        // pd_simple_filter_vec.clear();
        free(pd_array);
        delete spare;
    }

    inline void decrease_last_q_vec(uint64_t pd_index) {
        uint64_t new_quot = pd512_plus::decode_last_quot_wrapper(&pd_array[pd_index]);
        // assert(new_quot <= last_q_vec[pd_index]);
        // last_q_vec[pd_index] = new_quot;
    }


    inline auto lookup256(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint32_t pd_index2 = reduce32(out2, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;


        __m256i pd1 = _mm512_castsi512_si256(pd_array[pd_index]);
        __m256i pd2 = _mm512_castsi512_si256(pd_array[pd_index2]);

        // __m512i * pd3 = &pd_array[pd_index2];
        // const __m512i target = _mm512_set1_epi8(rem);
        // return _mm512_cmpeq_epu8_mask(target, *pd3);// || _mm256_cmpeq_epu8_mask(target, *pd2);


        const __m256i target = _mm256_set1_epi8(rem);
        return _mm256_cmpeq_epu8_mask(target, pd1) || _mm256_cmpeq_epu8_mask(target, pd2);
        // return
        // Simplest find
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index]));

        //One level search
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) ||
        //        pd512_plus::cmp_qr_smart(qr, &pd_array[pd_index]);

        //Two level search
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) ||
        //        (pd512_plus::cmp_qr_smart(qr, &pd_array[pd_index]) && spare->find(((uint64_t) pd_index << (14)) | qr));

        //Continguos TC  search
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) || (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index + 1]));
        // return (pd512_plus::pd_find_50_v31(quot, rem, &pd_array[pd_index])) || (pd512_plus::pd_find_50_v31(quot, rem, &pd_array[pd_index + 1]));
    }

    inline auto lookup(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        // const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) (quot_range << 8ul));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;

        // Simplest find
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index]));

        //One level search
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) ||
        //    pd512_plus::cmp_qr_smart(qr, &pd_array[pd_index]);

        // Two level search
        return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) ||
               (pd512_plus::cmp_qr_smart(qr, &pd_array[pd_index]) && spare->find(((uint64_t) pd_index << (14)) | qr));

        //Two level search Worst Case
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) || spare->find(((uint64_t) pd_index << (14)) | qr);

        //Continguos TC search
        // return (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index])) || (pd512_plus::pd_find_50_v25(quot, rem, &pd_array[pd_index + 1]));

        //Continguos TC search less branchy
        // return (pd512_plus::pd_find_50_v31(quot, rem, &pd_array[pd_index])) || (pd512_plus::pd_find_50_v31(quot, rem, &pd_array[pd_index + 1]));
    }


    inline auto lookup_minimal(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;
        // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        // const uint8_t rem = out2 & MASK(bits_per_item);
        // const uint64_t spare_element = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        return pd512_plus::pd_find_50_v18(quot, rem, &pd_array[pd_index]);
        // return pd512_plus::pd_find_50_v18(quot, rem, &pd_array[pd_index]);
        // return pd512_plus::pd_find_50_v21(quot, rem, &pd_array[pd_index]);
        // return pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index]);
    }

    auto lookup_count(const itemType s, int caser = 0) -> bool {
        static size_t l1_lookups[2] = {0, 0};
        static size_t l2_lookups[2] = {0, 0};
        static size_t OF_counter = 0;
        static size_t last_q_counter = 0;
        static size_t only_OF_counter = 0;
        static size_t only_last_q_counter = 0;
        if (caser == 1) {
            pd512_plus::pd_plus_find_v2_count(0, 0, &pd_array[0], caser);
            l1_lookups[0] = l1_lookups[1] = l2_lookups[0] = l2_lookups[1] = 0;
            OF_counter = 0;
            last_q_counter = 0;
            only_OF_counter = 0;
            only_last_q_counter = 0;
        }
        if (caser == 2) {
            pd512_plus::pd_plus_find_v2_count(0, 0, &pd_array[0], caser);
            auto line = std::string(64, '*');
            std::cout << line << std::endl;
            std::cout << "l1_lookups:(         \t" << l1_lookups[0] << ", " << l1_lookups[1] << ")" << std::endl;
            std::cout << "l2_lookups:(         \t" << l2_lookups[0] << ", " << l2_lookups[1] << ")" << std::endl;
            std::cout << "OF_counter:          \t" << OF_counter << std::endl;
            std::cout << "last_q_counter:      \t" << last_q_counter << std::endl;
            std::cout << "only_OF_counter:     \t" << only_OF_counter << std::endl;
            std::cout << "only_last_q_counter: \t" << only_last_q_counter << std::endl;
            std::cout << line << std::endl;
        }

        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;
        // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        // const uint8_t rem = out2 & MASK(bits_per_item);
        const uint64_t spare_element = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        auto res = pd512_plus::pd_plus_find_v2_count(quot, rem, &pd_array[pd_index]);
        l1_lookups[res & 1]++;
        if (res == 0) {
            auto l2_res = spare->find(spare_element);
            l2_lookups[l2_res]++;
            return l2_res;
        }

        if (res & 2) {
            OF_counter++;
            if ((res & 4) == 0)
                only_OF_counter++;
        }
        if (res & 4) {
            last_q_counter++;
            if ((res & 2) == 0)
                only_last_q_counter++;
        }

        return res;

        // const pd512_plus::pd_Status lookup_res = pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index]);
        // return (lookup_res == pd512_plus::pd_Status::Yes) || ((lookup_res == pd512_plus::pd_Status::look_in_the_next_level) && spare->find(spare_element));

        // uint64_t hash_res = Hasher(s);
        // uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        // const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        // const uint8_t rem = out2 & MASK(bits_per_item);

        // assert(pd_index < number_of_pd);
        // assert(quot <= 50);

        // bool is_in_l1 = pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index]);
        // if (is_in_l1) {
        //     searched_on_l1[0]++;
        //     return true;
        // }

        // searched_on_l1[1]++;
        // bool c1 = pd_simple_filter_vec[pd_index] & (1ull << quot);
        // if (!c1) {
        //     l2_search_avoided_counter++;
        //     return false;
        // }
        // auto spare_element = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        // bool res = spare->find(spare_element);
        // if (!res) {
        //     l2_search_that_was_not_avoided_but_should_have++;
        //     searched_on_l2[1]++;
        // }
        // searched_on_l2[0]++;
        // return res;
    }

    void insert(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;
        // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        // const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50);


        constexpr uint64_t Mask15 = 1ULL << 15;
        const uint64_t res_qr = pd512_plus::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
        const uint64_t temp_qr = (quot << 8u) | ((uint64_t) rem);
        // const uint64_t pd_last_qr = pd512_plus::get_last_qr_in_pd(&pd_array[pd_index]);


        //insertion succeeds
        if (res_qr == Mask15) {
            assert(lookup(s));
            return;
        } else if (res_qr == temp_qr) {
            // assert(pd_last_qr );
            assert(pd512_plus::pd_full(&pd_array[pd_index]));
            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | ((uint64_t) rem);
            assert((res_qr & spare_val) == res_qr);
            insert_to_spare_without_pop(spare_val);
            assert(spare->find(spare_val));
            assert(lookup(s));
        } else {
            // if (!pd512_plus::pd_full(&pd_array[pd_index])) {
            //     auto capacity = pd512_plus::get_capacity(&pd_array[pd_index]);
            //     auto last_quot = pd512_plus::decode_last_quot_wrapper(&pd_array[pd_index]);
            //     std::cout << "old_quot: " << last_quot << std::endl;
            //     std::cout << "new_quot: " << quot << std::endl;
            //     v_pd512_plus::print_headers(&pd_array[pd_index]);
            // }
            assert(pd512_plus::pd_full(&pd_array[pd_index]));
            // const uint64_t new_quot = res_qr >> 8ul;
            assert((res_qr >> 8ul) < 50);
            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res_qr;
            insert_to_spare_without_pop(spare_val);
            assert(spare->find(spare_val));
            assert(lookup(s));
        }
    }

    // void insert_for_validation(const itemType s) {

    //     // size_t att_capacity = get_capacity();
    //     // assert(att_capacity == l1_capacity);
    //     total_capacity++;

    //     uint64_t hash_res = Hasher(s);
    //     uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
    //     const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
    //     const uint8_t rem = out2 & MASK(bits_per_item);
    //     assert(pd_index < number_of_pd);
    //     assert(quot <= 50);


    //     uint64_t temp_qr = (quot << 8u) | rem;
    //     if (max_qr_could_insert[pd_index] < temp_qr) {
    //         max_qr_could_insert[pd_index] = temp_qr;
    //     }
    //     auto temp_vec = all_qr[pd_index];
    //     bool was_inserted = std::find(temp_vec.begin(), temp_vec.end(), temp_qr) != temp_vec.end();
    //     if (was_inserted) {
    //         // uint64_t pd_index = x >> 14;
    //         // uint64_t quot = (x >> 8) & 63;
    //         // uint16_t rem = x & 255;
    //         // std::cout << "x is already a member of l1: " << s;
    //         // std::cout << "\t(" << pd_index << ", " << quot << ", " << ((uint16_t) rem) << ")";
    //         // std::cout << "\t\t\tHasher(x): " << Hasher(s) << std::endl;
    //     }
    //     all_qr[pd_index].push_back(temp_qr);
    //     // uint64_t curr_qr = qr_vvec[pd_index];


    //     constexpr uint64_t Mask64 = -1;
    //     constexpr uint64_t Mask15 = 1ULL << 15;
    //     const size_t before_capacity = pd512_plus::get_capacity(&pd_array[pd_index]);
    //     const uint64_t res_qr = pd512_plus::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
    //     const size_t after_capacity = pd512_plus::get_capacity(&pd_array[pd_index]);
    //     assert(after_capacity >= before_capacity);
    //     assert(after_capacity <= before_capacity + 1);

    //     //insertion succeeds
    //     if (res_qr == Mask15) {
    //         if (last_q_vec[pd_index] < quot)
    //             last_q_vec[pd_index] = quot;
    //         // last_q_vec[pd_index] = (quot > last_q_vec[pd_index]) ? quot : last_q_vec[pd_index];
    //         if (max_qr_did_insert[pd_index] < temp_qr)
    //             max_qr_did_insert[pd_index] = temp_qr;
    //         assert(lookup(s));
    //         l1_capacity++;
    //         // size_t att_capacity_new = get_capacity();
    //         // assert(att_capacity_new == l1_capacity);
    //         return;
    //     } else if (res_qr == temp_qr) {
    //         assert(pd512_plus::pd_full(&pd_array[pd_index]));
    //         uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
    //         // std::cout << "k1" << std::endl;
    //         insert_to_spare_without_pop(spare_val);
    //         assert(spare->find(spare_val));
    //         assert(lookup(s));
    //         // const uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res_qr;// assert(res_qr);
    //     } else {
    //         assert(pd512_plus::pd_full(&pd_array[pd_index]));
    //         decrease_last_q_vec(pd_index);
    //         const uint64_t new_quot = res_qr >> 8ul;
    //         assert(new_quot < 50);
    //         uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res_qr;

    //         // std::cout << "k2" << std::endl;
    //         insert_to_spare_without_pop(spare_val);
    //         assert(spare->find(spare_val));
    //         assert(lookup(s));
    //     }

    //     // assert(pd512_plus::pd_full(&pd_array[pd_index]));
    //     // //Can be removed in some cases.
    //     // decrease_last_q_vec(pd_index);
    //     // const uint64_t new_quot = res >> 8ul;
    //     // assert(new_quot < 50);
    //     // const uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res;// assert(res);
    //     // constexpr uint64_t mask = (1ULL << 14ul) - 1;
    //     // const bool did_insertion_succeed = res ^ mask;
    //     // const bool is_full = pd512_plus::pd_full(&pd_array[pd_index]);
    //     // // if (!res) {
    //     // const uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res;
    //     // bool a_cond = ((uint64_t) pd_index << (quotient_length + bits_per_item)) & res;
    //     // assert(!a_cond);
    //     // insert_to_spare_without_pop(spare_val);
    //     // assert(spare->find(spare_val));
    //     // const uint64_t quot = res & 63;
    //     // const uint8_t rem = (res >> 6) & 255;
    //     // uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
    //     // insert_to_spare_with_pop(spare_val);
    //     // } else {
    //     // last_q_vec[pd_index] = (quot > last_q_vec[pd_index]) ? quot : last_q_vec[pd_index];
    //     // }
    //     // std::cout << "tomer" << std::endl;
    //     // assert(lookup(s));
    // }


    inline void remove(const itemType s) {
        assert(false);
        // total_capacity--;
        assert(false);
        uint64_t hash_res = Hasher(s);

        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50);

        if (pd512_plus::conditional_remove(quot, rem, &pd_array[pd_index])) {
            // l1_capacity--;
            // (pd_capacity_vec[pd_index]) -= 2;
            return;
        }

        uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        spare->remove(spare_val);
    }

    // void zero_out_search_counters() {
    //     searched_on_l1[0] = 0;
    //     searched_on_l1[1] = 0;
    //     searched_on_l2[0] = 0;
    //     searched_on_l2[1] = 0;
    //     l2_search_avoided_counter = 0;
    //     l2_search_that_was_not_avoided_but_should_have = 0;
    // }

    // void print_search_counters() {
    //     static int counter = 0;
    //     auto line = std::string(64, '*');
    //     *counters_stream << counter++ << ")" << std::endl;
    //     *counters_stream << line << std::endl;
    //     *counters_stream << "searched_on_l1: (" << searched_on_l1[0] << ", " << searched_on_l1[1] << ")" << std::endl;
    //     *counters_stream << "searched_on_l2: (" << searched_on_l2[0] << ", " << searched_on_l2[1] << ")" << std::endl;
    //     *counters_stream << "l2_search_avoided_counter: " << l2_search_avoided_counter << std::endl;
    //     *counters_stream << "l2_search_that_was_not_avoided_but_should_have: " << l2_search_that_was_not_avoided_but_should_have << std::endl;
    //     *counters_stream << line << "\n"
    //                      << std::endl;
    //     zero_out_search_counters();
    // }

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


        if (!pd512_plus::pd_full(&pd_array[pd_index])) {
            //            cout << " HERE!!!" << endl;
            // assert(!pd512_plus::pd_full(&pd_array[pd_index]));
            bool res = pd512_plus::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
            assert(res);

            // (pd_capacity_vec[pd_index]) += 2;
            spare->decrease_capacity();

            // cout << "element with hash_val: (" << element << ") was pop." << endl;
            return true;
        }
        if (DICT512_VER3_DB1) {
            assert(pd512_plus::pd_full(&pd_array[pd_index]));
        }
        return false;
    }


    // inline auto bitwise_lookup(const itemType s) const -> bool {
    //     using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
    //     uint32_t out1 = 647586, out2 = 14253653;
    //     Hash_ns::BobHash(&s, 8, &out1, &out2);
    //     const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
    //     // const uint16_t quot = reduce16((uint16_t) out2, (uint16_t) quot_range);
    //     const uint8_t rem = out2 & 8u;


    inline auto minimal_lookup(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = (uint64_t) reduce16((uint16_t) out2, quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50);
        return (pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index]));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// Validation functions.////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////


    auto squared_chi_test_basic() -> double {
        double res = 0;
        for (size_t i = 0; i < number_of_pd; i++) {
            auto temp = pd512_plus::get_capacity(&pd_array[i]) - expected_pd_capacity;
            res += temp * temp;
        }
        return res / (filter_max_capacity * expected_pd_capacity);
    }

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
            auto temp = pd512_plus::get_capacity(&pd_array[i]) - expected_pd_capacity;
            res += temp * temp;
        }
        return res / (filter_max_capacity * expected_pd_capacity);
    }


    auto validate_capacity_functions(size_t pd_index) -> bool {
        return true;
        // bool c = (pd512_plus::get_capacity(&pd_array[pd_index]) == (pd512_plus::get_capacity_naive(&pd_array[pd_index])));
        // return c & pd512_plus::get_capacity(&pd_array[pd_index]) == (pd_capacity_vec[pd_index] >> 1u);
    }
    auto get_extended_info() -> std::stringstream {
        // print_search_counters();
        std::stringstream ss;
        size_t temp_capacity = 0;
        for (size_t i = 0; i < number_of_pd; i++) {
            temp_capacity += (pd512_plus::get_capacity(&pd_array[i]));
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
        ss << "l1 byte size is: " << str_format(get_byte_size()) << std::endl;
        ss << "total byte size is: " << str_format(get_byte_size_with_spare()) << std::endl;
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
        size_t count_overflowing_PD = count_overflowing_PDs();
        size_t count_empty_PD = count_empty_PDs();

        const size_t num = 4;
        size_t val[num] = {spare_capacity, count_overflowing_PD, count_empty_PD, number_of_pd};

        string names[num] = {"spare_capacity", "count_overflowing_PD", "count_empty_PD", "number_of_pd"};
        std::stringstream ss = table_print(num, names, val);

        // const size_t dnum = 4;
        // double l0 = analyse_pd_status(0);
        // double l1 = analyse_pd_status(1);
        // double l2 = analyse_pd_status(2);
        // double l3 = analyse_pd_status(3);

        // double dval[dnum] = {l0, l1, l2, l3};

        // string dnames[dnum] = {"l0", "l1", "l2", "l3"};
        // table_print(dnum, dnames, dval, os);

        if (count_empty_PD > 1) {
            auto tp = find_empty_pd_interval();
            size_t start = std::get<0>(tp), end = std::get<1>(tp);
            ss << "longest_empty_interval length is: " << end - start << std::endl;
            ss << "start: " << start << std::endl;
            ss << "end: " << end << std::endl;
        }
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
            res += pd512_plus::get_capacity(&pd_array[i]);
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
        // return "Dict512_Ver3 find_21";
        // return "Dict512_Ver3 find_18";
        return "Dict512_Ver3";
        /* string a = "dict512:\t";
            string b = pd512_plus::get_name() + "\t";
            //        string b = pd_vec[0]->get_name() + "\t";
            string c = spare->get_name();
            return a + b + c; */
    }

    auto count_overflowing_PDs() -> size_t {
        size_t count_overflowing_PD = 0;
        for (int i = 0; i < number_of_pd; ++i) {
            bool add_cond = pd512_plus::pd_full(&pd_array[i]);
            count_overflowing_PD += add_cond;
            bool is_full = pd512_plus::pd_full(&pd_array[i]);
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
            /* bool temp_cond = (pd_capacity_vec[i] >> 1ul) == pd512_plus::get_capacity(&pd_array[i]);
                size_t r1 = pd_capacity_vec[i] >> 1ul;
                size_t r2 = pd512_plus::get_capacity(&pd_array[i]);
                size_t r3 = pd512_plus::get_capacity(&pd_array[i]);

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
            // assert((pd512_plus::get_capacity(pd_array[i])) == pd512_plus::get_capacity(&pd_array[i]));
            bool add_cond = (pd512_plus::get_capacity(&pd_array[i]) <= 0);
            count_empty_PD += add_cond;
            // bool is_full = pd512_plus::pd_full(&pd_array[i]);
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
            if (pd512_plus::get_capacity(&pd_array[index]) == 0) {
                size_t temp_length = 1;
                size_t temp_index = index + 1;
                while ((temp_index < number_of_pd) and (pd512_plus::get_capacity(&pd_array[temp_index]) == 0)) {
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

    auto get_byte_size_with_spare() {
        auto l1_size = get_byte_size();
        auto l2_size = spare->get_byte_size();
        return l1_size + l2_size;
    }


    auto analyse_pd_status(size_t p) -> double {
        return -42.0;
        // return m512i_lp_average(pd_array, number_of_pd, p);
    }

    /* auto case_validate() -> bool {
            size_t index = random() % number_of_pd;
            // case_validate_counter++;
            bool res = pd512_plus::validate_number_of_quotient(&pd_array[index]);
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
private:
    inline auto lookup_old(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        const uint8_t rem = out2 & MASK(bits_per_item);

        // const uint64_t lookup_qr = (quot << 8ul) | rem;
        // const uint64_t lim_qr = (quot << 8ul) | rem;

        // const int cmp = my_cmp<uint64_t>(quot, last_q_vec[pd_index]);
        // uint32_t spare_hash = Hasher.hash32(s);

        // const uint64_t lim_quot = last_q_vec[pd_index];
        const uint64_t spare_element = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        pd512_plus::pd_Status search_res = pd512_plus::pd_find_enums(quot, rem, &pd_array[pd_index]);
        switch (search_res) {
            case pd512_plus::Yes:
                return true;
            case pd512_plus::No:
                return false;
            case pd512_plus::look_in_the_next_level:
                return false;
                // return (quot >= lim_quot) && spare->find(spare_element);
                // return (quot >= 43);
                // return spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem);
            default:
                assert(0);
                break;
        }
        assert(0);
        return false;
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


    auto unSplit(uint32_t r, uint32_t quot, size_t pd_index) const -> spareItemType {
        assert(false);
        assert(quot_range == 50);
        spareItemType res = pd_index;
        res = (res << quotient_length) | quot;
        res = (res << remainder_length) | r;
        if (!flip) {
            flip = true;
            assert(check_unSplit(res));
            flip = false;
        }
        return res;
    }

    auto check_unSplit(itemType hash_val) const -> bool {
        size_t pd_index = -1;
        uint32_t quot = -1, r = -1;
        split(hash_val, &pd_index, &quot, &r);
        auto temp = hash_val & MASK(spare_element_length);
        bool res = temp == (hash_val & spare_element_length);
        if (!res) {
            std::cout << "temp: " << temp << std::endl;
            std::cout << "hash_val & spare_element_length: " << (hash_val & spare_element_length) << std::endl;
            std::cout << "hash_val " << (hash_val) << std::endl;
        }
        return (temp == (hash_val & spare_element_length));
    }

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
    //         // size_t cap_res = pd512_plus::get_capacity(&pd_array[pd_index]);
    //         // size_t cap_valid = pd_capacity_vec[pd_index] / 2;
    //         assert(pd512_plus::get_capacity(ppd) == pd_capacity_vec[pd_index] / 2);
    //         if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
    //             assert(pd512_plus::pd_full(ppd));
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
    //             pd512_plus::print512(ppd - 1);
    //             print_8array(temp_arr[0]);
    //             print_array(temp_arr[0], 8);
    //             std::cout << line << std::endl;
    //
    //             std::cout << 1 << ")" << std::endl;
    //             pd512_plus::print512(ppd);
    //             print_8array(temp_arr[1]);
    //             print_array(temp_arr[1], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 2 << ")" << std::endl;
    //             pd512_plus::print512(ppd);
    //             print_8array(temp_arr[2]);
    //             print_array(temp_arr[2], 8);
    //             std::cout << line << std::endl;
    //         }
    //  */
    //  // auto res = pd512_plus::pd_add_50(quot, r, ppd);
    //         auto res = inlining_pd_add_50(quot, r, ppd, pd_index);
    //         /* if (cond)
    //         {
    //             auto line = std::string(64, '*');
    //             _mm512_store_si512(temp_arr[3], *(ppd - 1));
    //             _mm512_store_si512(temp_arr[4], *ppd);
    //             _mm512_store_si512(temp_arr[5], *(ppd + 1));
    //             std::cout << 3 << ")" << std::endl;
    //             pd512_plus::print512(ppd - 1);
    //             print_8array(temp_arr[3]);
    //             print_array(temp_arr[3], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 4 << ")" << std::endl;
    //             pd512_plus::print512(ppd);
    //             print_8array(temp_arr[4]);
    //             print_array(temp_arr[4], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 5 << ")" << std::endl;
    //             pd512_plus::print512(ppd + 1);
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
    //         /* assert(pd512_plus::validate_number_of_quotient(&pd_array[pd_index - 1]));
    //         assert(pd512_plus::validate_number_of_quotient(&pd_array[pd_index]));
    //         bool temp_failed = pd512_plus::validate_number_of_quotient(&pd_array[pd_index + 1]);
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
    //             assert(pd512_plus::get_capacity(&pd_array[pd_index]) == pd_capacity_vec[pd_index] / 2);
    //             if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
    //                 assert(pd512_plus::pd_full(&pd_array[pd_index]));
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

#endif//FILTERS_DICT512_VER3_HPP


/*         // const bool in_pd = pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index]);
        // const bool did_OF = pd512_plus::did_pd_overflowed(&pd_array[pd_index]);
        // const uint64_t spare_item = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem;
        // const bool in_spare = spare->find(spare_item);

        switch (cmp) {
            case -1:
                // std::cout << "h-1" << std::endl;
                return pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index]);
            case 0:
                //todo replace with weaker find
                // std::cout << "h0" << std::endl;
                return (pd512_plus::pd_find_50(quot, rem, &pd_array[pd_index])) || (pd512_plus::did_pd_overflowed(&pd_array[pd_index]) && spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem));
            case 1:
                // std::cout << "h1" << std::endl;
                return (pd512_plus::did_pd_overflowed(&pd_array[pd_index]) && spare->find(((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) | rem));
            default:
                assert(0);
                break;
        }
        return false;
 */