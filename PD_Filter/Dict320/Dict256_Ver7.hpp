
#ifndef FILTERS_DICT256_VER7_HPP
#define FILTERS_DICT256_VER7_HPP


#include "../../Bloom_Filter/simd-block.h"
#include "HashTables/HistoryLog.hpp"
#include "HashTables/Spare_Validator.hpp"
#include "HashTables/packed_spare.hpp"
#include "pd256_plus.hpp"
#include <map>
#include <unordered_set>
//#include <map>


//#include <compare>
//#include "compare"


// static size_t op_count = 0;

#define DICT256_VER71 (true)
#define DICT256_VER72 (true & DICT256_VER71)
#define DICT256_VER73 (true & DICT256_VER72)

using Simd_BF = SimdBlockFilter<hashing::TwoIndependentMultiplyShift>;
namespace pd_name = pd256_plus;

template<typename itemType>
class Dict256_Ver7 {
    enum lookup_correct_val {
        Yes = true,
        No = false,
        IDK = -42
    };
    static constexpr size_t bits_per_item = 8;
    static constexpr size_t max_capacity = 25;
    static constexpr size_t quot_range = 25;


    packed_spare<48, 32, bits_per_item, 4> *spare;
    // HistoryLog *Level2_Log;
    // HistoryLog *Level1_Log;


    const size_t filter_max_capacity;
    const size_t number_of_pd;
    // const size_t pd_index_length;
    // const size_t spare_element_length;

    const size_t remainder_length{bits_per_item},
            quotient_range{quot_range},
            quotient_length{ceil_log2(quot_range)},
            single_pd_capacity{max_capacity};
    hashing::TwoIndependentMultiplyShift Hasher;

    double expected_pd_capacity;

    __m256i *pd_array;
    Simd_BF *spare_filter;


public:
    Dict256_Ver7(size_t max_number_of_elements, double level1_load_factor, double level2_load_factor)
        : filter_max_capacity(max_number_of_elements),
          number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
          //   pd_index_length(
          //           ceil_log2(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor))),
          //   spare_element_length(pd_index_length + quotient_length + remainder_length),
          Hasher() {
        // op_count = 0;
        expected_pd_capacity = max_capacity * level1_load_factor;
        spare = new packed_spare<48, 32, bits_per_item, 4>(number_of_pd);
        // Level2_Log = new HistoryLog(number_of_pd, false);
        // Level1_Log = new HistoryLog(number_of_pd, false);
        //        spare_valid = new Spare_Validator(number_of_pd);


        int ok = posix_memalign((void **) &pd_array, 32, 32 * number_of_pd);
        if (ok != 0) {
            std::cout << "Failed!!!" << std::endl;
            std::cout << level2_load_factor << std::endl;
            ;
            assert(false);
            return;
        }

        constexpr uint64_t pd256_plus_init_header = (((INT64_C(1) << 25) - 1) << 6) | 32;
        std::fill(pd_array, pd_array + number_of_pd, __m256i{pd256_plus_init_header, 0, 0, 0});

        spare_filter = new Simd_BF(ceil(log2(max_number_of_elements * 0.1 * 8.0 / CHAR_BIT)));
    }


    virtual ~Dict256_Ver7() {
        std::cout << std::string(80, '=') << std::endl;
        print_data_on_space();
        std::cout << std::string(80, '=') << std::endl;

        // assert(get_capacity() >= 0);
        free(pd_array);
        delete spare;
        delete spare_filter;
        // std::cout << std::string(80, '!') << std::endl;
        // std::cout << std::string(80, '!') << std::endl;
        // op_count = 0;
        // delete Level2_Log;
        // delete Level1_Log;
        //        delete spare_valid;
    }

    item_key_t get_hash_res_as_key(const itemType s) const {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint64_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const uint64_t quot = qr >> bits_per_item;
        const uint64_t rem = qr & MASK(bits_per_item);
        item_key_t key = {pd_index, quot, rem};
        return key;
    }

    bool is_bad_element(const itemType s) const {
        return s == 372622566;
    }

    auto lookup_l2(const itemType s, lookup_correct_val correct_val) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const int64_t spare_quot = (QUOT_SIZE25 - 1) - quot;
        const uint8_t rem = qr;
        return lookup_l2(pd_index, spare_quot, rem, correct_val);
    }

    auto
    lookup_l2(uint64_t pd_index, uint8_t spare_quot, uint8_t rem, lookup_correct_val correct_val) const -> bool {
        // bool log_res = Level2_Log->Find(pd_index, quot, rem);
        // assert(!Level2_Log->using_flipped_quot);
        bool spare_res = spare->find(pd_index, spare_quot, rem);
        bool test_res = !(correct_val == IDK);
        if (test_res) {
            assert(spare_res == correct_val);
        }
        //        assert(spare_res || (!log_res)); // No false negative.
        //        assert(log_res == spare_res);

        const uint64_t quot = (QUOT_SIZE25 - 1) - spare_quot;
        uint64_t qr = (quot << bits_per_item) | static_cast<uint64_t>(rem);
        uint64_t spare_filter_key = (static_cast<uint64_t>(pd_index) << 13ul) | qr;
        bool spare_filter_res = spare_filter->Find(spare_filter_key);

        if (!spare_filter_res) {
            assert(!spare_res);
        }
        //        if (correct_val == Yes) {
        //            assert(spare_res);
        //        }
        return spare_res;
    }

    auto lookup_l1(const itemType s, lookup_correct_val correct_val) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;

        return lookup_l1(pd_index, quot, rem, correct_val);
    }

    auto lookup_l1(uint64_t pd_index, uint8_t quot, uint8_t rem, lookup_correct_val correct_val) const -> bool {
        bool res = pd_name::pd_find_25(quot, rem, &pd_array[pd_index]);
        bool test_res = !(correct_val == IDK);
        if (test_res) {
            assert(res == correct_val);
        }
        // bool v_res = Level1_Log->Find(pd_index, quot, rem);
        // if (v_res != res) {
        //     Level1_Log->print_bucket_log(pd_index);
        // }
        // assert(v_res == res);
        return res;
    }

    auto lookup_naive(uint64_t pd_index, uint8_t quot, uint8_t rem, lookup_correct_val correct_val) const -> bool {
        uint64_t qr = (static_cast<uint64_t>(quot) << bits_per_item) | static_cast<uint64_t>(rem);
        bool should_look_in_l1 = !pd_name::cmp_qr1(qr, &pd_array[pd_index]);

        if (should_look_in_l1)
            return lookup_l1(pd_index, quot, rem, correct_val);

        auto spare_quot = 24 - quot;
        return lookup_l2(pd_index, spare_quot, rem, correct_val);
    }

    auto lookup_naive(const itemType s, lookup_correct_val correct_val = IDK) const -> bool {
        return lookup_with_l2_filter(s);
    }

    inline auto lookup(const itemType s) const -> bool {
        return lookup_with_l2_filter(s);
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        assert(validate_lowers_quot_are_in_l1(pd_index));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> 8;
        const uint64_t spare_quot = 24 - quot;
        const uint8_t rem = qr;
        return (!pd_name::cmp_qr1(qr, &pd_array[pd_index])) ? pd_name::pd_find_25(quot, rem, &pd_array[pd_index])
                                                            : (spare->find(((uint64_t) pd_index), spare_quot, rem));
    }

    inline auto lookup_with_l2_filter(const itemType s) const -> bool {
        // op_count++;
        // bool bpc = is_bad_element(s);
        // if (bpc) {
        //     std::cout << "lookup. ";
        //     std::cout << "op_count: \t" << op_count << std::endl;
        // }
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        assert(validate_lowers_quot_are_in_l1(pd_index));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> 8;
        const uint8_t rem = qr;
        return (!pd_name::cmp_qr1(qr, &pd_array[pd_index])) ? pd_name::pd_find_25(quot, rem, &pd_array[pd_index])
                                                            : (spare_filter->Find(((uint64_t) pd_index << (13)) | qr));
    }

    int get_find_level(const itemType s) const {
        if (!lookup(s)) return 0;
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        bool should_look_in_l1 = !pd_name::cmp_qr1(qr, &pd_array[pd_index]);
        if (should_look_in_l1) {
            return (lookup_l1_safe(s, IDK)) ? 1 : 0;
        }
        return (lookup_l2_safe(s, IDK)) ? 2 : 0;
    }

    void spare_add(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        assert(pd_index < number_of_pd);
        assert(quot <= quot_range);

        spare_add(pd_index, quot, rem);

        assert(lookup(s, Yes));
        assert(lookup_l2_safe(s, Yes));
    }

    void spare_add(uint64_t pd_index, uint8_t quot, uint8_t rem) {
        uint64_t spare_quot = 24 - static_cast<uint64_t>(quot);
        //        insert_to_spare_without_pop(pd_index, spare_quot, rem);
        spare->insert(pd_index, spare_quot, rem);
        assert(spare->find(pd_index, spare_quot, rem));

        // Level2_Log->Add(pd_index, quot, rem);
        // assert(Level2_Log->Find(pd_index, quot, rem));
        // assert(!Level2_Log->using_flipped_quot);

        uint64_t qr = (static_cast<uint64_t>(quot) << bits_per_item) | rem;
        uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | qr;
        spare_filter->Add(spare_val);
        assert(spare_filter->Find(spare_val));
    }

    void spare_remove(const itemType s) {
        assert(lookup_naive(s, Yes));

        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        assert(pd_index < number_of_pd);
        assert(quot <= quot_range);

        spare_remove(pd_index, quot, rem);
    }

    void spare_remove(uint64_t pd_index, uint8_t quot, uint8_t rem) {
        uint64_t spare_quot = 24 - static_cast<uint64_t>(quot);
        assert(lookup_l2(pd_index, spare_quot, rem, Yes));

        spare->remove(pd_index, spare_quot, rem);
        // Level2_Log->Remove(pd_index, quot, rem);
        // assert(!Level2_Log->using_flipped_quot);
    }

    void insert(const itemType s) {
        // op_count++;
        // bool bpc = is_bad_element(s);
        // if (bpc) {
        //     std::cout << "insert. ";
        //     std::cout << "op_count: \t" << op_count << std::endl;
        // }
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        assert(validate_lowers_quot_are_in_l1(pd_index));
        if (pd_name::pd_full(&pd_array[pd_index])) {
            insert_into_full_pd_helper(s);
            assert(lookup_naive(s, Yes));
        } else {
            insert_into_not_full_pd_helper(s);
            assert(lookup_naive(s, Yes));
        }
    }

    inline void insert_into_not_full_pd_helper(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        assert(!pd_name::pd_full(&pd_array[pd_index]));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        assert(pd_index < number_of_pd);
        assert(quot <= quot_range);

        const uint64_t res_qr = pd_name::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
        constexpr uint64_t Mask15 = 1ULL << 15ULL;
        assert(res_qr == Mask15);

        // Level1_Log->Add(pd_index, quot, rem);
        // assert(Level1_Log->Find(pd_index, quot, rem));
        // assert(lookup(s, Yes));
    }

    inline void insert_into_full_pd_helper(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        assert(pd_name::pd_full(&pd_array[pd_index]));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const int64_t spare_quot = quot_range - 1 - quot;
        const uint8_t rem = qr;
        assert(pd_index < number_of_pd);
        assert(quot <= quot_range);

        const uint64_t res_qr = pd_name::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
        constexpr uint64_t Mask15 = 1ULL << 15ULL;
        assert(res_qr != Mask15);

        if (res_qr != qr) {
            return insertion_swap_case(s, pd_index, quot, rem, res_qr);
        }

        /** PD did not change!
         * Element is bigger then pd's biggest_element
         * */

        assert(pd_name::pd_full(&pd_array[pd_index]));
        insert_to_spare_without_pop(pd_index, spare_quot, rem);
        // Level2_Log->Add(pd_index, quot, rem);
        // assert(Level2_Log->Find(pd_index, quot, rem));
        // assert(!Level2_Log->using_flipped_quot);

        uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | qr;
        assert((res_qr & spare_val) == res_qr);
        spare_filter->Add(spare_val);
        assert(spare_filter->Find(spare_val));

        assert(lookup_naive(s, Yes));
        // assert(lookup_l2_safe(s, Yes));
    }

    inline void insertion_swap_case(const itemType s, uint64_t pd_index, uint8_t l1_quot, uint8_t l1_rem,
                                    uint64_t pd_add_qr_result) {
        assert(pd_name::pd_full(&pd_array[pd_index]));

        // Level1_Log->Add(pd_index, l1_quot, l1_rem);
        // assert(Level1_Log->Find(pd_index, l1_quot, l1_rem));
        // assert(!Level1_Log->using_flipped_quot);


        const uint64_t new_quot = pd_add_qr_result >> bits_per_item;
        const uint64_t new_spare_quot = (quot_range - 1) - new_quot;
        const uint64_t new_rem = pd_add_qr_result & MASK(bits_per_item);
        assert(new_quot < QUOT_SIZE25);

        insert_to_spare_without_pop(pd_index, new_spare_quot, new_rem);
        // Level2_Log->Add(pd_index, new_quot, new_rem);
        // assert(Level2_Log->Find(pd_index, new_quot, new_rem));
        // assert(!Level2_Log->using_flipped_quot);


        uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | pd_add_qr_result;
        spare_filter->Add(spare_val);
        assert(spare_filter->Find(spare_val));
        assert(lookup_naive(s, Yes));
        // assert(lookup_l1_safe(s, Yes));

        assert(lookup_naive(pd_index, new_quot, new_rem, Yes));
        // assert(lookup_l2_safe(pd_index, new_spare_quot, new_rem, Yes));
    }


    void remove(const itemType s) {
        // op_count++;
        // static int remove_counter = 0;
        // remove_counter++;
        // bool bpc = is_bad_element(s);
        // if (bpc) {
        //     std::cout << "remove. ";
        //     std::cout << "op_count: \t" << op_count << std::endl;
        // }

        assert(lookup_naive(s, Yes));

        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        assert(validate_lowers_quot_are_in_l1(pd_index));
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        assert(pd_index < number_of_pd);
        assert(quot <= quot_range);


        bool did_pd_overflowed = pd_name::did_pd_overflowed(&pd_array[pd_index]);
        if (!did_pd_overflowed) {
            auto res = pd_name::delete_element_wrapper(quot, rem, &pd_array[pd_index]);
            assert(res);
            // Level1_Log->Remove(pd_index, quot, rem);
            return;
        }

        bool did_deletion_succeed = pd_name::delete_element_wrapper(quot, rem, &pd_array[pd_index]);
        if (!did_deletion_succeed) {
            spare_remove(s);
            return;
        }

        // Level1_Log->Remove(pd_index, quot, rem);
        pop_obtain_element_and_insert_into_l1(pd_index, quot, rem);
    }

    void pop_add_to_l1(item_key_t item, uint64_t removed_quot, uint8_t removed_rem, uint64_t pd_index) {
        assert(item.pd_index == pd_index);
        assert(removed_quot <= item.quot);
        pd_name::pd_pop_add(item.quot, item.rem, removed_quot, removed_rem, &pd_array[pd_index]);
        // Level1_Log->Add(pd_index, item.quot, item.rem);
        // assert(lookup(pd_index, item.quot, item.rem, Yes));
        // assert(lookup_l1_safe(pd_index, item.quot, item.rem, Yes));
    }

    item_key_t spare_pop(uint64_t pd_index) {
        auto pop_item = spare->pop(pd_index);
        if (pop_item.pd_index != static_cast<uint64_t>(-1))
            flip_quot(&pop_item);
        return pop_item;
    }

    void pop_obtain_element_and_insert_into_l1(uint64_t pd_index, uint64_t removed_quot, uint8_t removed_rem) {

        item_key_t item = spare_pop(pd_index);
        if (item.pd_index == static_cast<uint64_t>(-1)) {
            return pop_helper_no_element_to_pop(pd_index, removed_quot, removed_rem, item);
        } else
            pop_add_to_l1(item, removed_quot, removed_rem, pd_index);
    }


    void
    pop_helper_no_element_to_pop(uint64_t pd_index, uint64_t removed_quot, uint8_t removed_rem, item_key_t pop_res) {
        pd_name::clear_overflow_bit(&pd_array[pd_index]);
        pd_name::update_quot_after_pop(&pd_array[pd_index]);
    }


    /* item_key_t test_for_equality_of_the_pop_item(uint64_t pd_index) const {
        static int eq_c = 0;
        eq_c++;

        item_key_t v_pop_item = Level2_Log->get_pop_element(pd_index);
        item_key_t pop_item = spare->get_pop_element(pd_index);
        if (pop_item.pd_index == -1) {
            assert(v_pop_item.pd_index == -1);
            return pop_item;
        }

        bool are_qs_flipped = (24 - pop_item.quot) == v_pop_item.quot;
        assert(are_qs_flipped);
        pop_item.quot = 24 - pop_item.quot;
        assert(pop_item.rem == v_pop_item.rem);
        assert(pop_item == v_pop_item);
        return pop_item;
    }
 */

    inline void insert_to_spare_without_pop(uint64_t pd_index, uint8_t spare_quot, uint8_t rem) {
        spare->insert(pd_index, spare_quot, rem);
        assert(spare->find(pd_index, spare_quot, rem));
    }


    bool validate_lowers_quot_are_in_l1(uint64_t pd_index) const {
        // const __m256i *pd = &((const __m256i *)pd_array)[pd_index];

        item_key_t pop_item = spare->get_pop_element(pd_index);
        if (pop_item.pd_index == static_cast<uint64_t>(-1))
            return true;

        if (!pd_name::did_pd_overflowed(&pd_array[pd_index]))
            return true;

        flip_quot(&pop_item);

        const __m256i *pd = &pd_array[pd_index];
        uint64_t pd_quot = pd_name::decode_last_quot(pd);
        uint64_t pd_rem = pd_name::read_last_rem(pd);
        item_key_t pd_last_item = {pd_index, pd_quot, pd_rem};

        if (pd_last_item.pd_index != pop_item.pd_index) {
            size_t b1 = pd_index / 32;
            size_t b2 = (pd_index / 32) + (pd_index & 31) + 1;
            bool validation = ((pop_item.pd_index == b1) && (pop_item.pd_index == b2));
            if (!validation) {
                std::cout << "b1: " << b1 << std::endl;
                std::cout << "b2: " << b2 << std::endl;
                std::cout << "pd_index: " << pd_index << std::endl;
                std::cout << "pop_item:\n" << pop_item << std::endl;
                std::cout << "pd_last_item:\n" << pd_last_item << std::endl;
                assert(0);
            }
            assert(validation);
            pd_last_item.pd_index = pop_item.pd_index;
            // std::cout << "pop_item.pd_index: " << pop_item.pd_index << std::endl;
        }
        assert(pd_last_item <= pop_item);
        return true;
    }

    /*void insert_old(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const int64_t spare_quot = quot_range - 1 - quot;
        const uint8_t rem = qr;
        assert(pd_index < number_of_pd);
        assert(quot <= quot_range);
//        auto key = std::make_tuple<uint64_t, int, int>(pd_index, quot, rem);


        constexpr uint64_t Mask15 = 1ULL << 15;
        const uint64_t res_qr = pd_name::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);

        //insertion succeeds
        if (res_qr == Mask15) {
            Level1_Log->Add(pd_index, quot, rem);
            assert(Level1_Log->Find(pd_index, quot, rem));
            assert(lookup(s, Yes));
            return;
        } else if (res_qr == qr) {
            assert(pd_name::pd_full(&pd_array[pd_index]));
            insert_to_spare_without_pop(pd_index, spare_quot, rem);
            Level2_Log->Add(pd_index, quot, rem);
            assert(Level2_Log->Find(pd_index, quot, rem));

            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | qr;
            assert((res_qr & spare_val) == res_qr);
            spare_filter->Add(spare_val);
            assert(spare_filter->Find(spare_val));
            assert(lookup(s, Yes));

        } else {
            assert(pd_name::pd_full(&pd_array[pd_index]));
            const uint64_t new_quot = res_qr >> 8ul;
            const uint64_t new_spare_quot = quot_range - 1 - new_quot;
            assert((res_qr >> 8ul) < quot_range);


            insert_to_spare_without_pop(pd_index, new_spare_quot, rem);
            Level2_Log->Add(pd_index, new_quot, rem);
            assert(Level2_Log->Find(pd_index, new_quot, rem));

            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res_qr;
            spare_filter->Add(spare_val);
            assert(spare_filter->Find(spare_val));
            assert(lookup(s, Yes));

        }
        assert(lookup(s, Yes));
//        assert(lookup_naive(s, 1));
//        assert(lookup_exact(s));
    }*/

    void insert_db(const itemType s) {
        // static int counter = 0;
        // static int c1 = 0;
        // static int c2 = 0;
        // static int c3 = 0;
        // counter++;

        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 6400);
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        // const uint64_t quot = reduce32((uint32_t) out2, (uint32_t) quot_range);
        // const uint8_t rem = out2 & MASK(bits_per_item);
        assert(pd_index < number_of_pd);
        assert(quot <= 50);


        constexpr uint64_t Mask15 = 1ULL << 15;
        const uint64_t res_qr = pd_name::pd_conditional_add_50(quot, rem, &pd_array[pd_index]);
        const uint64_t temp_qr = (quot << 8u) | ((uint64_t) rem);
        // const uint64_t pd_last_qr = pd_name::get_last_qr_in_pd(&pd_array[pd_index]);


        //insertion succeeds
        if (res_qr == Mask15) {
            // c1++;
            assert(lookup_naive(s, 1));
            assert(lookup(s));
            return;
        } else if (res_qr == temp_qr) {
            // c2++;
            // assert(pd_last_qr );
            assert(pd_name::pd_full(&pd_array[pd_index]));
            insert_to_spare_without_pop(pd_index, quot_range - 1 - quot, rem);
            assert(spare->find(pd_index, quot_range - 1 - quot, rem));

            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | (quot << bits_per_item) |
                                 ((uint64_t) rem);
            assert((res_qr & spare_val) == res_qr);
            spare_filter->Add(spare_val);
            assert(spare_filter->Find(spare_val));

            assert(lookup_naive(s, 1));
            assert(lookup(s));
        } else {
            // c3++;
            // if (!pd_name::pd_full(&pd_array[pd_index])) {
            //     auto capacity = pd_name::get_capacity(&pd_array[pd_index]);
            //     auto last_quot = pd_name::decode_last_quot_wrapper(&pd_array[pd_index]);
            //     std::cout << "old_quot: " << last_quot << std::endl;
            //     std::cout << "new_quot: " << quot << std::endl;
            //     v_pd_name::print_headers(&pd_array[pd_index]);
            // }
            assert(pd_name::pd_full(&pd_array[pd_index]));
            const uint64_t new_quot = res_qr >> 8ul;
            assert((res_qr >> 8ul) < 50);

            insert_to_spare_without_pop(pd_index, quot_range - 1 - new_quot, rem);
            assert(spare->find(pd_index, quot_range - 1 - new_quot, rem));

            uint64_t spare_val = ((uint64_t) pd_index << (quotient_length + bits_per_item)) | res_qr;
            spare_filter->Add(spare_val);
            assert(spare_filter->Find(spare_val));
            assert(lookup_naive(s, 1));
            assert(lookup(s));
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// Validation functions.////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* void pop_helper_too_complex(uint64_t pd_index, uint64_t removed_quot, uint8_t removed_rem) {
        static int pop_helper_c = 0;
        pop_helper_c++;

        auto removed_spare_quot = (QUOT_SIZE25 - 1) - removed_quot;
        //        uint64_t pop_res = spare->get_pop_element(pd_index, removed_spare_quot, removed_rem);
        item_key_t v_pop_item = Level2_Log->get_pop_element(pd_index);
        item_key_t pop_item = spare->get_pop_element(pd_index, removed_spare_quot, removed_rem);
        bool are_qs_flipped = (24 - pop_item.quot) == v_pop_item.quot;
        assert(are_qs_flipped);
        pop_item.quot = 24 - pop_item.quot;
        assert(pop_item.rem == v_pop_item.rem);
        assert(pop_item == v_pop_item);
        if (pop_item.pd_index == -1) {
            return pop_helper_no_element_to_pop(pd_index, removed_quot, removed_rem, pop_item);
        }
        auto vv_pop_item = Level2_Log->pop(pd_index);
        //        auto spare_vv_pop_item = spare->pop
        assert(vv_pop_item == v_pop_item);

        assert(removed_quot <= pop_item.quot);
        pd_name::pd_pop_add(pop_item.quot, pop_item.rem, removed_quot, removed_rem, &pd_array[pd_index]);
        Level1_Log->Add(pd_index, pop_item.quot, pop_item.rem);
        assert(lookup(pd_index, pop_item.quot, pop_item.rem, Yes));
        assert(lookup_l1_safe(pd_index, pop_item.quot, pop_item.rem, Yes));
    }
 */

    auto squared_chi_test_basic() -> double {
        double res = 0;
        for (size_t i = 0; i < number_of_pd; i++) {
            auto temp = pd_name::get_capacity(&pd_array[i]) - expected_pd_capacity;
            res += temp * temp;
        }
        return res / (filter_max_capacity * expected_pd_capacity);
    }

    auto squared_chi_test() -> double {
        // vector<uint16_t> temp_capacity_vec(pd_capacity_vec);
        /* vector<uint64_t> spare_elements;

        spare->get_all_elements(&spare_elements);
        for (size_t i = 0; i < spare_elements.size(); i++) {
            auto temp = spare_elements[i];
            size_t temp_pd_index = (temp >> (bits_per_item + quotient_length));
            // temp_capacity_vec[temp_pd_index]++;
        } */

        double res = 0;
        for (size_t i = 0; i < number_of_pd; i++) {
            auto temp = pd_name::get_capacity(&pd_array[i]) - expected_pd_capacity;
            res += temp * temp;
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
            temp_capacity += (pd_name::get_capacity(&pd_array[i]));
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
        // ss << "spare_element_length is: " << spare_element_length << std::endl;


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

    /* auto get_hash_val_buckets(itemType hash_val) -> std::tuple<uint32_t, uint32_t> {
        uint32_t b1 = -1, b2 = -1;
        spare->my_hash(hash_val, &b1, &b2);
        return std::make_tuple(b1, b2);
    } */

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
        for (size_t i = 0; i < number_of_pd; ++i) {
            res += pd_name::get_capacity(&pd_array[i]);
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
        return "Dict256_Ver7";
    }

    auto count_overflowing_PDs() -> size_t {
        size_t count_overflowing_PD = 0;
        for (int i = 0; i < number_of_pd; ++i) {
            bool add_cond = pd_name::pd_full(&pd_array[i]);
            count_overflowing_PD += add_cond;
            bool is_full = pd_name::pd_full(&pd_array[i]);
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
            /* bool temp_cond = (pd_capacity_vec[i] >> 1ul) == pd_name::get_capacity(&pd_array[i]);
                size_t r1 = pd_capacity_vec[i] >> 1ul;
                size_t r2 = pd_name::get_capacity(&pd_array[i]);
                size_t r3 = pd_name::get_capacity(&pd_array[i]);

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
            // assert((pd_name::get_capacity(pd_array[i])) == pd_name::get_capacity(&pd_array[i]));
            bool add_cond = (pd_name::get_capacity(&pd_array[i]) <= 0);
            count_empty_PD += add_cond;
            // bool is_full = pd_name::pd_full(&pd_array[i]);
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
        assert(0);
        return {42, 42};
    }

    auto get_l2_byte_size() const -> size_t {
        return spare->get_byte_size();
    }

    auto get_l1_byte_size() const -> size_t {
        size_t res = sizeof(__m256i) * number_of_pd;
        size_t simd_filter_size = spare_filter->SizeInBytes();
        res += simd_filter_size;
        return res;
    }

    auto get_byte_size() const -> size_t {
        //variables.
        // size_t res = 8 * sizeof(size_t);

        //PD's
        size_t res = sizeof(__m256i) * number_of_pd;
        // Capacity vec.
        size_t simd_filter_size = spare_filter->SizeInBytes();
        res += simd_filter_size;
        res += get_l2_byte_size();
        return res;
    }

    void print_data_on_space() const {
        auto total_size = get_byte_size();
        std::cout << "Total byte size is: \t" << get_byte_size() << std::endl;
        std::cout << "L1 size:            \t" << get_l1_byte_size();
        std::cout << ".\t" << (1.0 * get_l1_byte_size() / total_size) << std::endl;

        std::cout << "spare_filter size:  \t" << spare_filter->SizeInBytes();
        std::cout << ".\t" << (1.0 * spare_filter->SizeInBytes() / total_size) << std::endl;

        std::cout << "L2 size:            \t" << get_l2_byte_size();
        std::cout << ".\t" << (1.0 * get_l2_byte_size() / total_size) << std::endl;
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
            bool res = pd_name::validate_number_of_quotient(&pd_array[index]);
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
    /* auto pop_attempt_by_element(const itemType s) {
        uint32_t b1, b2;
        std::tie(b1, b2) = get_elements_buckets(s);
        pop_attempt_by_bucket(b1);
        pop_attempt_by_bucket(b2);
    } */

    /* auto pop_attempt_by_hash_val(const itemType hash_val) {
        uint32_t b1, b2;
        std::tie(b1, b2) = get_hash_val_buckets(hash_val);
        pop_attempt_by_bucket(b1);
        pop_attempt_by_bucket(b2);
    } */

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
    //         // size_t cap_res = pd_name::get_capacity(&pd_array[pd_index]);
    //         // size_t cap_valid = pd_capacity_vec[pd_index] / 2;
    //         assert(pd_name::get_capacity(ppd) == pd_capacity_vec[pd_index] / 2);
    //         if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
    //             assert(pd_name::pd_full(ppd));
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
    //             pd_name::print512(ppd - 1);
    //             print_8array(temp_arr[0]);
    //             print_array(temp_arr[0], 8);
    //             std::cout << line << std::endl;
    //
    //             std::cout << 1 << ")" << std::endl;
    //             pd_name::print512(ppd);
    //             print_8array(temp_arr[1]);
    //             print_array(temp_arr[1], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 2 << ")" << std::endl;
    //             pd_name::print512(ppd);
    //             print_8array(temp_arr[2]);
    //             print_array(temp_arr[2], 8);
    //             std::cout << line << std::endl;
    //         }
    //  */
    //  // auto res = pd_name::pd_add_50(quot, r, ppd);
    //         auto res = inlining_pd_add_50(quot, r, ppd, pd_index);
    //         /* if (cond)
    //         {
    //             auto line = std::string(64, '*');
    //             _mm512_store_si512(temp_arr[3], *(ppd - 1));
    //             _mm512_store_si512(temp_arr[4], *ppd);
    //             _mm512_store_si512(temp_arr[5], *(ppd + 1));
    //             std::cout << 3 << ")" << std::endl;
    //             pd_name::print512(ppd - 1);
    //             print_8array(temp_arr[3]);
    //             print_array(temp_arr[3], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 4 << ")" << std::endl;
    //             pd_name::print512(ppd);
    //             print_8array(temp_arr[4]);
    //             print_array(temp_arr[4], 8);
    //             std::cout << line << std::endl;
    //             std::cout << 5 << ")" << std::endl;
    //             pd_name::print512(ppd + 1);
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
    //         /* assert(pd_name::validate_number_of_quotient(&pd_array[pd_index - 1]));
    //         assert(pd_name::validate_number_of_quotient(&pd_array[pd_index]));
    //         bool temp_failed = pd_name::validate_number_of_quotient(&pd_array[pd_index + 1]);
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
    //             assert(pd_name::get_capacity(&pd_array[pd_index]) == pd_capacity_vec[pd_index] / 2);
    //             if (pd_capacity_vec[pd_index] / 2 == (single_pd_capacity)) {
    //                 assert(pd_name::pd_full(&pd_array[pd_index]));
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

#endif//FILTERS_DICT256_VER7_HPP
