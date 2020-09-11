/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#include "pd320_v2.hpp"

namespace v_pd320_v2 {
    void bin_print_header(uint64_t header) {
        // assert(_mm_popcnt_u64(header) == 32);
        uint64_t b = 1ULL << (64ul - 1u);
        while (b) {
            std::string temp = (b & header) ? "1" : "0";
            std::cout << temp;
            b >>= 1ul;
        }
    }
    auto bin_print_header_spaced(uint64_t header) -> std::string {
        // assert(_mm_popcnt_u64(header) == 32);
        uint64_t b = 1ULL << (64ul - 1u);
        std::string res = "";
        while (b) {
            for (size_t i = 0; i < 4; i++) {
                res += (b & header) ? "1" : "0";
                b >>= 1ul;
            }
            if (b)
                res += ".";
        }
        return res;
    }

}// namespace v_pd320_v2

namespace pd320_v2 {
    void print512(const __m512i *var) {
        uint64_t val[8];
        memcpy(val, var, 64);
        printf("[%zu, %zu, %zu, %zu, %zu, %zu, %zu, %zu] \n",
               val[0], val[1], val[2], val[3], val[4], val[5],
               val[6], val[7]);
        //     long long unsigned int val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        //     assert(sizeof(val) == 64);
        //     memcpy(val, &var, sizeof(val));
        //     for (size_t i = 0; i < 8; i++) {
        //         std::cout << val[i] << ", ";
        //         /* code */
        //     }
        //     std::cout << std::endl;
        // }

        /* bool
    is_aligned(const __m512i *ptr) noexcept
    {
        auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
        return !(iptr % alignof(T));
    } */

        // cout <<
        // printf("Numerical: %i %i %i %i %i %i %i %i \n",
        //        val[0], val[1], val[2], val[3], val[4], val[5],
        //        val[6], val[7]);
    }

    auto validate_number_of_quotient(const __m512i *pd) -> bool {
        // std::cout << "h128: " << std::endl;

        const uint64_t header = ((uint64_t *) pd)[0];
        auto pop_count = _mm_popcnt_u64(header);

        // std::cout << "my_temp: " << my_temp << std::endl;
        // Number of bits to keep. Requires little-endianness
        // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        // const unsigned __int128 kLeftoverMask = (((unsigned __int128)1) << (50 + 51)) - 1;
        // header = header & kLeftoverMask;
        // size_t res = popcount128(header);
        if (pop_count != 32) {
            std::cout << "pop_count: " << pop_count << std::endl;
        }
        return pop_count != 32;
        // return true;
    }

    auto get_capacity_att(const __m512i *x) -> size_t {
        validate_number_of_quotient(x);
        // return get_capacity_naive();
        uint64_t header;
        memcpy(&header, x, 8);
        assert(_mm_popcnt_u64(header) == 32);
        auto temp = _lzcnt_u64(header);
        assert(0 <= temp);
        assert(temp <= 32);
        auto res = 32 - temp;

        if (res != get_capacity_naive(x)) {
            auto valid_res = get_capacity_naive(x);
            std::cout << "In get_capacity_att:" << std::endl;
            std::cout << "res: " << res << std::endl;
            std::cout << "valid_res: " << valid_res << std::endl;
        }
        return res;
    }

    auto get_capacity_naive(const __m512i *x) -> size_t {
        uint64_t header;
        memcpy(&header, x, 8);
        size_t zero_count = 0, one_count = 0;
        uint64_t temp = header;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & temp) {
                one_count++;
                if (one_count == 32)
                    return zero_count;
            } else {
                zero_count++;
            }
            b <<= 1ul;
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false);
    }

    auto is_pd_full_naive(const __m512i *pd) -> bool {
        return get_capacity_naive(pd) == 32;
    }

    auto get_name() -> std::string {
        return "pd320_v2 ";
    }

    auto remove_naive(int64_t quot, char rem, __m512i *pd) -> bool {
        assert(false);
        return false;
    }
}// namespace pd320_v2
