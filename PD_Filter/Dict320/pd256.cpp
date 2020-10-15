/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#include "pd256.hpp"

namespace v_pd256 {
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

}// namespace v_pd256

namespace pd256 {

    auto validate_number_of_quotient(const __m256i *pd) -> bool {
        return validate_number_of_quotient(get_clean_header(pd));
    }

    auto validate_number_of_quotient(uint64_t clean_header) -> bool {
        auto pop_count = _mm_popcnt_u64(clean_header);
        if (pop_count != QUOT_SIZE22) {
            std::cout << "pop_count: " << pop_count << std::endl;
        }
        return pop_count == QUOT_SIZE22;
    }


    auto get_capacity_att(const __m256i *x) -> size_t {
        validate_number_of_quotient(x);
        // return get_capacity_naive();

        uint64_t header = get_clean_header(x);
        assert(_mm_popcnt_u64(header) == QUOT_SIZE22);
        auto temp = _lzcnt_u64(header);
        assert(14 <= temp);
        assert(temp <= (64 - 22));
        auto res = 42 - temp;

        if (res != get_capacity_naive(x)) {
            auto valid_res = get_capacity_naive(x);
            std::cout << "In get_capacity_att:" << std::endl;
            std::cout << "res: " << res << std::endl;
            std::cout << "valid_res: " << valid_res << std::endl;
            assert(false);
        }
        return res;
    }

    auto get_capacity_naive(const __m256i *x) -> size_t {
        uint64_t header = get_clean_header(x);

        size_t zero_count = 0, one_count = 0;
        uint64_t temp = header;
        uint64_t b = 1ULL;
        for (size_t i = 0; i < 64; i++) {
            if (b & temp) {
                one_count++;
                if (one_count == QUOT_SIZE22)
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

    auto is_pd_full_naive(const __m256i *pd) -> bool {
        return get_capacity_naive(pd) == CAPACITY26;
    }

    auto get_name() -> std::string {
        return "pd256 ";
    }

    auto remove_naive(int64_t quot, char rem, __m256i *pd) -> bool {
        assert(false);
        return false;
    }
}// namespace pd256
