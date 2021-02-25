/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#ifndef FILTERS_TS_PD512_HPP
#define FILTERS_TS_PD512_HPP

#include "pd512.hpp"


namespace v_ts_pd512 {
}// namespace v_ts_pd512

namespace ts_pd512 {

    constexpr size_t QUOTS = 50;
    constexpr size_t MAX_CAPACITY = 51;


    auto get_specific_quot_capacity_naive2(int64_t quot, const __m512i *pd) -> int;

    inline bool find(int64_t quot, uint8_t rem, const __m512i *pd) {
        return pd512::pd_find_50_v18(quot, rem, pd);
    }

    // inline auto conditional_remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
    //     return pd512::conditional_remove(quot, rem, pd);
    // }

    inline auto remove(int64_t quot, uint8_t rem, __m512i *pd) -> bool {
        // This assertion can not work, because we have false negative
        // assert(find(quot, rem, pd));
        auto res = pd512::conditional_remove_old(quot, rem, pd);
        return res;
        // assert(res);
        // return pd512::remove(quot, rem, pd);

    }

    inline bool add(int64_t quot, uint8_t rem, __m512i *pd) {
        return pd512::pd_add_50(quot, rem, pd);
    }

    inline void evict_random_element(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(0);
    }

    
    /**
     * @brief Finding the last `remainder` in the "run" of the quotient `quot`, and replacing it with rem.
     * This functions only changes one byte in the body. (Does not change the header).
     *  
     * Assertion error if there is no element in the filter with quotient `quot`.
     * This function probably induce non-uniform evacuation policy (w.r.t. insertion time), and therefore will probably be replaced.  
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void evict_random_element_with_the_same_quot_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(ts_pd512::get_specific_quot_capacity_naive2(quot, pd));
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        assert(pd512::popcount128(header) == 50);
        const unsigned fill = pd512::select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd512::pd_popcount(pd)));
        assert((fill == 51) == pd512::pd_full(pd));


        const uint64_t end = pd512::select128(header, quot);
        const uint64_t end_fingerprint = end - quot;

        // memmove(&((uint8_t *) pd)[kBytes2copy + end_fingerprint + 1],
        //         &((const uint8_t *) pd)[kBytes2copy + end_fingerprint],
        //         sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((uint8_t *) pd)[kBytes2copy + end_fingerprint] = rem;

        assert(find(quot, rem, pd));
        // return true;
    }


    inline void evict_random_element_with_new_quot_super_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        size_t index = (rand() % QUOTS);

        const size_t rand_quot = pd512::count_ones_up_to_the_kth_zero(pd, index);
        //todo: add in assertion here of something.
        const uint8_t rand_rem = ((uint8_t *) pd)[kBytes2copy + index];

        //todo: smart remove here
        remove(rand_quot, rand_rem, pd);
        add(quot, rem, pd);
    }


    inline void evict_random_element_with_the_same_quot(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(get_specific_quot_capacity_naive2(quot, pd));

        const __m512i target = _mm512_set1_epi8(rem);
        // compare_less or equal.
        uint64_t v = _mm512_cmp_epu8_mask(target, *pd, 2) >> 13ul;
    }

    inline void evict_random_element_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd512::pd_full_naive(pd));
        if (get_specific_quot_capacity_naive2(quot, pd)) {
            evict_random_element_with_the_same_quot_naive(quot, rem, pd);
            return;
        }
    }


}// namespace ts_pd512


#endif// FILTERS_TS_PD512_HPP
