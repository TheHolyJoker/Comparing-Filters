
#ifndef FILTERS_WRAP_FIXED_PD_HPP
#define FILTERS_WRAP_FIXED_PD_HPP

#include "fpd.hpp"
#include <algorithm>
#include <tuple>
#include <vector>

namespace Wrap_Fixed_pd {
    // using namespace fixed_pd;

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


    auto add(uint64_t quot, uint8_t rem, uint64_t *header, __m512i *body) -> int;

    auto find(uint64_t quot, uint8_t rem, const uint64_t *header, const __m512i *body) -> int;

    // auto find(uint64_t quot, uint8_t rem, packed_fpd * pd) -> int;
    // auto add(uint64_t quot, packed_fpd * pd) -> int;

    // auto vector_find(uint64_t quot, uint8_t rem, packed_fpd *pd) -> bool;

    // void vector_add(uint64_t quot, uint8_t rem, packed_fpd *pd);
    // void vector_promote_quot(uint64_t quot, packed_fpd *pd) {
    //     size_t begin_fingerprint = Header::get_start(quot, pd->header);
    //     for (size_t i = 0; i < last_val - 1; i++) {
    //         uint8_t temp_rem = ((uint8_t *) (&pd->body))[begin_fingerprint + i];
    //         vector_add(quot, temp_rem, promote_quot_and_remove_from_bodyd);
    //     }
    // }

    // void promote_quot_and_remove_from_body(uint64_t quot, size_t counter_size,uint64_t *header, __m512i* body);

    // void promote_quot_and_remove_from_body(uint64_t quot, uint64_t *header, __m512i* body);

    auto get_capacity(uint64_t *header, __m512i *body) -> size_t;
}// namespace Wrap_Fixed_pd
#endif// FILTERS_WRAP_FIXED_PD_HPP
