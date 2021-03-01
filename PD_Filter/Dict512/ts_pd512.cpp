#include "ts_pd512.hpp"


namespace v_ts_pd512 {
    size_t popcount_up_to_index(uint64_t word, size_t index) {
        assert(index <= sizeof(word) * CHAR_BIT);
        if (index == sizeof(word) * CHAR_BIT)
            return _mm_popcnt_u64(word);

        uint64_t mask = MSK(index);
        return _mm_popcnt_u64(word & mask);
    }

    size_t popcount_up_to_index(uint64_t *arr, size_t a_size, size_t index);

    bool v_write_header_cond12(uint64_t h0, size_t index, uint64_t quot) {
        size_t res1 = popcount_up_to_index(h0, index);
        bool cond1 = res1 == (quot - 1);

        size_t res2 = popcount_up_to_index(h0, index + 1);
        bool cond2 = res2 == quot;

        assert(cond1 && cond2);
        return cond1 && cond2;
    }

    void print_body(const __m512i *pd) {
        std::cout << std::string(80, '=') << std::endl;
        uint8_t temp_arr[ts_pd512::MAX_CAPACITY];
        memcpy(temp_arr, &((uint8_t *) pd)[ts_pd512::kBytes2copy], ts_pd512::MAX_CAPACITY);
        for (size_t i = 0; i < 6; i++) {
            for (size_t j = 0; j < 8; j++) {
                size_t temp_index = i * 8 + j;
                std::cout << std::setw(3) << (uint16_t) temp_arr[temp_index] << " | ";
            }
            std::cout << std::endl;
        }
        for (size_t i = 0; i < 3; i++) {
            size_t temp_index = 48 + i;
            std::cout << std::setw(3) << (uint16_t) temp_arr[temp_index] << " | ";
        }
        std::cout << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }

    bool validate_body_add(size_t end_fingerprint, uint8_t rem, __m512i *pd) {
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;

        ts_pd512::body_add_naive(end_fingerprint, rem, &temp_pd0);
        ts_pd512::body_add(end_fingerprint, rem, &temp_pd1);
        bool cond1 = my_equal(temp_pd0, temp_pd1);
        ts_pd512::body_add2(end_fingerprint, rem, &temp_pd2);

        // bool cond1 = my_equal(temp_pd0,temp_pd1);
        bool cond2 = my_equal(temp_pd0, temp_pd2);

        assert(cond1);
        assert(cond2);
        return true;
    }


    void pd_to_array(const __m512i *pd, uint64_t *arr, size_t a_size) {
        assert(a_size == pd512::get_capacity(pd) + 2);
        size_t capacity = pd512::get_capacity(pd);
        uint8_t rem_arr[a_size];
        zero_array(rem_arr, a_size);

        size_t quots_caps_arr[ts_pd512::QUOTS];
        zero_array(quots_caps_arr, ts_pd512::QUOTS);

        // std::cout << "capacity: " << capacity << std::endl;
        // quots_caps_arr[0] = ts_pd512::get_specific_quot_capacity_naive2(0, pd);
        // std::cout << quots_caps_arr[0];
        for (size_t i = 0; i < ts_pd512::QUOTS; i++) {
            quots_caps_arr[i] = ts_pd512::get_specific_quot_capacity_naive2(i, pd);
            // std::cout << ", " << quots_caps_arr[i];
        }
        // std::cout << std::endl;

        memcpy(rem_arr, &((const uint8_t *) pd)[ts_pd512::kBytes2copy], capacity);
        size_t index = 0;
        while (index < capacity) {
            // std::cout << "here!" << std::endl;
            for (size_t i = 0; i < ts_pd512::QUOTS; i++) {
                for (size_t j = 0; j < quots_caps_arr[i]; j++) {
                    uint16_t val = ((uint16_t) rem_arr[index]) | (i << 8u);
                    arr[index] = (uint64_t) val;
                    index++;
                }
            }
            if (index < capacity) {
                assert(0);
            }
        }
    }

    size_t array_sym_diff_single(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size) {
        size_t counter = 0;
        for (size_t i = 0; i < a_size; i++) {
            for (size_t j = 0; j < b_size; j++) {
                if (a[i] == b[j]) {
                    counter++;
                    break;
                }
            }
        }
        return a_size - counter;
    }

    size_t array_sym_diff(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size) {
        size_t a_minus_b = array_sym_diff_single(a, a_size, b, b_size);
        size_t b_minus_a = array_sym_diff_single(b, b_size, a, a_size);
        return a_minus_b + b_minus_a;
    }


    size_t sorted_sym_diff(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size) {
        std::sort(a, a + a_size);
        std::sort(b, b + b_size);

        size_t a_index = 0;
        size_t b_index = 0;
        size_t counter = 0;

        while ((a_index < a_size) && (b_index < b_size)) {
            if (a[a_index] == b[b_index]) {
                a_index++;
                b_index++;
            } else if (a[a_index] < b[b_index]) {
                counter++;
                a_index++;
            } else {
                counter++;
                b_index++;
            }
        }
        size_t a_left = a_size - a_index;
        size_t b_left = b_size - b_index;
        size_t res = counter + a_left + b_left;
        assert(res <= a_size + b_size);
        return res;

        // size_t counter = 0;
        // for (size_t i = 0; i < a_size; i++) {
        //     for (size_t j = 0; j < b_size; j++) {
        //         if (a[i] == b[j]) {
        //             counter++;
        //             break;
        //         }
        //     }
        // }
        // size_t a_minus_b = array_sym_diff_single(a, a_size, b, b_size);
        // size_t b_minus_a = array_sym_diff_single(b, b_size, a, a_size);
        // return a_minus_b + b_minus_a;
    }

    size_t pd_distance(const __m512i *pd1, const __m512i *pd2) {
        size_t capacity1 = pd512::get_capacity(pd1);
        size_t capacity2 = pd512::get_capacity(pd2);

        //maybe capacity is too small (< 2).
        size_t a1_size = capacity1 + 2;
        size_t a2_size = capacity2 + 2;
        uint64_t arr1[a1_size];
        zero_array(arr1, a1_size);
        pd_to_array(pd1, arr1, a1_size);

        uint64_t arr2[a2_size];
        zero_array(arr2, a2_size);

        pd_to_array(pd2, arr2, a2_size);

        // size_t sanity_check1 = sorted_sym_diff(arr1, capacity1, arr1, capacity1);
        // assert(sanity_check1 == 0);
        // size_t sanity_check2 = sorted_sym_diff(arr2, capacity2, arr2, capacity2);
        // assert(sanity_check2 == 0);

        return sorted_sym_diff(arr1, capacity1, arr2, capacity2);
        // return array_sym_diff(arr1, capacity1, arr2, capacity2);
    }

    size_t val_pd_distance(int64_t quot, uint8_t rem, const __m512i *old_pd, const __m512i *new_pd) {
        auto res = v_ts_pd512::pd_distance(new_pd, old_pd);
        if (res != 1) {
            std::cout << "old capacity: " << pd512::get_capacity_naive(old_pd) << std::endl;
            std::cout << "new capacity: " << pd512::get_capacity_naive(new_pd) << std::endl;
            std::cout << "quot: " << quot << std::endl;
            std::cout << "rem: " << ((uint64_t) rem) << std::endl;
            std::cout << "res:  " << res << std::endl;
            v_pd512_plus::print_headers_extended(old_pd);
            v_ts_pd512::print_body(old_pd);
            v_pd512_plus::print_headers_extended(new_pd);
            v_ts_pd512::print_body(new_pd);
            v_ts_pd512::pd_distance(new_pd, old_pd);
        }
        return res;
    }

}// namespace v_ts_pd512


namespace ts_pd512 {

    auto get_specific_quot_capacity_naive2(int64_t quot, const __m512i *pd) -> int {
        assert(quot < 50);
        // assert(quot <= _mm_popcnt_u64(get_clean_header(pd)));
        if (quot == 0) {
            return pd512_plus::count_zeros_up_to_the_kth_one(pd, 0);
        }
        return pd512_plus::count_zeros_up_to_the_kth_one(pd, quot) -
               pd512_plus::count_zeros_up_to_the_kth_one(pd, quot - 1);
    }
}// namespace ts_pd512