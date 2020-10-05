
#ifndef FILTERS_WRAP_FIXED_PD_HPP
#define FILTERS_WRAP_FIXED_PD_HPP

#include "fpd.hpp"
#include <algorithm>
#include <tuple>
#include <vector>

namespace Wrap_Fixed_pd {
    // using namespace Fixed_pd;

    // typedef std::tuple<uint64_t, uint16_t> vec_key;

    struct packed_fpd {
        __m512i body1 __attribute__((aligned(64)));
        uint64_t header1[4];
        uint64_t header2[4];
        __m512i body2 __attribute__((aligned(64)));


        packed_fpd() {
            body1 = __m512i{0, 0, 0, 0, 0, 0, 0, 0};
            body2 = __m512i{0, 0, 0, 0, 0, 0, 0, 0};
            for (size_t i = 0; i < 4; i++) {
                header1[i] = 0;
                header2[i] = 0;
            }
        }

    } __attribute__((aligned(64)));
    static_assert(sizeof(struct packed_fpd) == (64 * 3), "Check your assumptions");
    // static_assert(sizeof(struct packed_fpd) == (64 * 4), "Check your assumptions");


    // void init_fpd(packed_fpd *pd) {
    //     pd->body = __m512i{0, 0, 0, 0, 0, 0, 0, 0};
    //     for (size_t i = 0; i < counter_size; i++) {
    //         pd->header[i] = 0;
    //     }
    // }

    auto validate_init_packed_fpd(packed_fpd *pd) -> bool;


    /*     auto add(uint64_t quot, uint8_t rem, uint64_t *header, __m512i *body) -> int;

    auto find(uint64_t quot, uint8_t rem, const uint64_t *header, const __m512i *body) -> int;

    auto get_capacity(uint64_t *header, __m512i *body) -> size_t;
 */

    inline auto find(uint64_t quot, uint8_t rem, const uint64_t *header, const __m512i *body) -> int {
        auto counter = Fixed_pd::Header::read_counter(quot, header);
        if (counter == 0) {
            return 0;
        } else if (counter == Fixed_pd::counter_overflowed_val) {
            return -1;
        }

        uint64_t v = Fixed_pd::Body::get_v(rem, body);
        if (!v) {
            return 0;
        }

        uint64_t start = Fixed_pd::Header::get_start(quot, header);
        return (v >> start) & MASK(counter);
    }

    inline auto find2(uint64_t quot, uint8_t rem, const uint64_t *header, const __m512i *body) -> int {
        const size_t counter = Fixed_pd::Header::read_counter(quot, header);
        if (counter != Fixed_pd::counter_overflowed_val) {
            if (!counter)
                return 0;
            uint64_t v = Fixed_pd::Body::get_v(rem, body);
            if (!v)
                return 0;

            uint64_t start = Fixed_pd::Header::get_start(quot, header);
            return (v >> start) & MASK(counter);
        }
        return -1;
    }

    inline auto find3(uint64_t quot, uint8_t rem, const uint64_t *header, const __m512i *body) -> int {
        const size_t counter = Fixed_pd::Header::read_counter(quot, header);
        switch (counter) {
            case 0:
                return 0;

            case Fixed_pd::counter_overflowed_val:
                return -1;

            default:
                break;
        }
        uint64_t v = Fixed_pd::Body::get_v(rem, body);
        if (!v)
            return 0;

        uint64_t start = Fixed_pd::Header::get_start(quot, header);
        return (v >> start) & MASK(counter);
    }

    inline auto add(uint64_t quot, uint8_t rem, uint64_t *header, __m512i *body) -> int {
        assert(quot < Fixed_pd::QUOT_RANGE);
        int header_add_res = Fixed_pd::Header::add(quot, header);
        if (header_add_res == -1) {
            uint64_t start = Fixed_pd::Header::get_start(quot, header);
            assert(start <= Fixed_pd::CAPACITY);
            uint64_t end = Fixed_pd::Header::read_counter(quot, header);
            Fixed_pd::Body::add(start, start + end - 1, rem, body);
            return -1;
        }
        return header_add_res;
    }


    inline auto get_capacity(uint64_t *header, __m512i *body) -> size_t {
        return Fixed_pd::Header::get_capacity(header);
    }
}// namespace Wrap_Fixed_pd
#endif// FILTERS_WRAP_FIXED_PD_HPP
