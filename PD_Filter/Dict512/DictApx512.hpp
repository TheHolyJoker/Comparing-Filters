
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
    size_t insertions_total_counter;
    size_t insertions_eviction_counter;
    size_t insertions_to_full_pd_counter;

    size_t total_remove_counter;
    size_t true_remove_counter;
    // size_t number_of_pd

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
          Hasher(), true_remove_counter(0), total_remove_counter(0), insertions_total_counter(0),
          insertions_eviction_counter(0), insertions_to_full_pd_counter(0) {
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
        assert(_mm_popcnt_u64(ts_pd512_init_header) == pd_apx_name::QUOTS);
        std::fill(pd_array, pd_array + number_of_pd, __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0});
    }

    virtual ~DictApx512() {
        double insertion_ratio = 1.0 * insertions_eviction_counter / insertions_total_counter;
        std::cout << "Failed insertion ratio is: " << insertion_ratio << std::endl;

        double insertion_to_full_pd_ratio = 1.0 * insertions_to_full_pd_counter / insertions_total_counter;
        std::cout << "Insertions to full pd ratio is: " << insertion_to_full_pd_ratio << std::endl;

        double remove_ratio = 1.0 * true_remove_counter / total_remove_counter;
        std::cout << "Yes-remove-counter-ratio: " << remove_ratio << std::endl;
        // std::cout << std::string(80, '=') << std::endl;
        // // print_data_on_space();
        // std::cout << std::string(80, '=') << std::endl;

        free(pd_array);
    }

    inline void get_qr(const itemType s, int64_t *p_quot, uint8_t *p_rem) const {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        // uint8_t rem = qr;
        // const uint8_t rem = qr * (qr & 255 != 255);
        uint8_t rem = qr;
        rem += (rem == 255);

        // if (((uint16_t) rem) == 255)
        //     rem = 1;
        // if (((uint16_t) rem) == 0)
        //     rem = 1;

        *p_quot = quot;
        *p_rem = rem;
    }

    inline auto lookup(const itemType s) const -> bool {
        /* int64_t quot;
        uint8_t rem;
        get_qr(s, &quot, &rem); */
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        //  const uint8_t rem = qr;
        // const uint8_t rem = qr * (qr & 255 != 255);
        uint8_t rem = qr;
        rem += (rem == 255);

        // if (((uint16_t) rem) == 255)
        //     rem = 1;
        // if (((uint16_t) rem) == 0)
        //     rem = 1;
        // // // const uint8_t rem = qr * (rem != 255);

        return pd_apx_name::find(quot, rem, &pd_array[pd_index]);
    }

    inline void insert_plus_tombstones(const itemType s) {
        /* int64_t quot;
        uint8_t rem;
        get_qr(s, &quot, &rem); */
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        // uint8_t rem = qr;
        // rem += (rem == 255);
        const uint8_t rem = qr + ((uint8_t) qr == ((uint8_t) 255));

        pd_apx_name::add_plus_tombs(quot, rem, &pd_array[pd_index]);
    }


    inline void insert_without_tombstone_handling(const itemType s) {
        /* int64_t quot;
        uint8_t rem;
        get_qr(s, &quot, &rem); */
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        //     rem = 1;
        // if (((uint16_t) rem) == 0)
        //     rem = 1;
        //     // const uint8_t rem = qr * (rem != 255);

#ifdef COUNT
        return insert_count(s);
#endif//COUNT
        pd_apx_name::add(quot, rem, &pd_array[pd_index]);
    }

    inline void insert(const itemType s) {
        //  insert_without_tombstone_handling(s);
        insert_plus_tombstones(s);
    }

    inline bool remove_without_tombstone_handling(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        const uint8_t rem = qr;
        return pd_apx_name::remove(quot, rem, &pd_array[pd_index]);
    }

    inline bool remove_plus_tombstones(const itemType s) {
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        uint8_t rem = qr;
        rem += (rem == 255);

        pd_apx_name::bad_tombstoning_idea(quot, rem, &pd_array[pd_index]);
        return true;

        if (pd_apx_name::find(quot, rem, &pd_array[pd_index])) {
            // pd_apx_name::remove(quot, rem, &pd_array[pd_index]);
            pd_apx_name::remove_by_tombstoning(quot, rem, &pd_array[pd_index]);
            return true;
        }
        return false;
    }

    inline bool remove(const itemType s) {
        return remove_plus_tombstones(s);
        // return remove_without_tombstone_handling(s);
    }

    void insert_count(const itemType s) {
        insertions_total_counter++;
        /* int64_t quot;
        uint8_t rem;
        get_qr(s, &quot, &rem); */
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        // uint8_t rem = qr;
        // const uint8_t rem = qr * (qr & 255 != 255);
        uint8_t rem = qr;
        rem += (rem == 255);

        // if (((uint16_t) rem) == 255)
        //     rem = 1;
        // if (((uint16_t) rem) == 0)
        //     rem = 1;
        // // // const uint8_t rem = qr * (rem != 255);

        insertions_to_full_pd_counter += pd_apx_name::pd_full(&pd_array[pd_index]);
        bool res = pd_apx_name::add(quot, rem, &pd_array[pd_index]);
        if (res) {
            insertions_eviction_counter++;
        }
    }


    void insert_count_plus_tombstones(const itemType s) {
        insertions_total_counter++;

        /* int64_t quot;
        uint8_t rem;
        get_qr(s, &quot, &rem); */
        uint64_t hash_res = Hasher(s);
        uint32_t out1 = hash_res >> 32u, out2 = hash_res & MASK32;
        const uint32_t pd_index = reduce32(out1, (uint32_t) number_of_pd);
        const uint16_t qr = reduce16((uint16_t) out2, (uint16_t) 12800);// 12800 == QUOTS << 8;
        const int64_t quot = qr >> bits_per_item;
        // uint8_t rem = qr;
        // const uint8_t rem = qr * (qr & 255 != 255);
        uint8_t rem = qr;
        rem += (rem == 255);

        // if (((uint16_t) rem) == 255)
        //     rem = 1;
        // if (((uint16_t) rem) == 0)
        //     rem = 1;
        // // const uint8_t rem = qr * (rem != 255);

        insertions_to_full_pd_counter += pd_apx_name::pd_full(&pd_array[pd_index]);
        bool res = pd_apx_name::add_plus_tombs(quot, rem, &pd_array[pd_index]);
        if (res) {
            insertions_eviction_counter++;
        }
    }


    auto get_name() -> std::string {
        return "DictApx512";
    }
};

#endif//FILTERS_DICTAPX512_HPP
