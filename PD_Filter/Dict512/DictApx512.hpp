
#ifndef FILTERS_DICTAPX512_HPP
#define FILTERS_DICTAPX512_HPP


// #include "../../Bloom_Filter/simd-block.h"
// #include "HashTables/HistoryLog.hpp"
// #include "HashTables/Spare_Validator.hpp"
// #include "HashTables/packed_spare.hpp"
#include "ts_pd512.hpp"
// #include <map>
// #include <unordered_set>


// static size_t op_count = 0;

#define DICTAPX5121 (true)
#define DICTAPX5122 (true & DICTAPX5121)
#define DICTAPX5123 (true & DICTAPX5122)

namespace pd_apx_name = ts_pd512;

template<typename itemType>
class DictApx512 {

    static constexpr size_t bits_per_item = 8;
    static constexpr size_t max_capacity = pd_apx_name::MAX_CAPACITY;
    static constexpr size_t quot_range = pd_apx_name::QUOTS;

    const size_t filter_max_capacity;
    const size_t number_of_pd;

    const size_t remainder_length{bits_per_item},
            quotient_range{quot_range},
            quotient_length{ceil_log2(quot_range)},
            single_pd_capacity{max_capacity};
    hashing::TwoIndependentMultiplyShift Hasher;

    double expected_pd_capacity;

    __m512i *pd_array;

public:
    DictApx512(size_t max_number_of_elements, double level1_load_factor)
        : filter_max_capacity(max_number_of_elements),
          number_of_pd(compute_number_of_PD(max_number_of_elements, max_capacity, level1_load_factor)),
          Hasher() {
        // op_count = 0;
        expected_pd_capacity = max_capacity * level1_load_factor;

        int ok = posix_memalign((void **) &pd_array, 64, 64 * number_of_pd);
        if (ok != 0) {
            std::cout << "Failed!!!" << std::endl;
            // std::cout << level2_load_factor << std::endl;
            assert(false);
            return;
        }

        constexpr uint64_t ts_pd512_init_header = (INT64_C(1) << 50) - 1;
        std::fill(pd_array, pd_array + number_of_pd, __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0});
    }

    virtual ~DictApx512() {
        // std::cout << std::string(80, '=') << std::endl;
        // // print_data_on_space();
        // std::cout << std::string(80, '=') << std::endl;

        free(pd_array);
    }

    auto lookup(const itemType s) const -> bool {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;

        return pd_apx_name::find(quot, rem, &pd_array[pd_index]);
    }

    void insert(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;

        pd_apx_name::add(quot, rem, &pd_array[pd_index]);
    }

    bool remove(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;

        return pd_apx_name::remove(quot, rem, &pd_array[pd_index]);
    }

    auto get_name() -> std::string {
        return "DictApx512";
    }

};

#endif//FILTERS_DICTAPX512_HPP
