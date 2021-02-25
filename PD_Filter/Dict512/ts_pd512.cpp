#include "ts_pd512.hpp"

namespace ts_pd512 {

auto get_specific_quot_capacity_naive2(int64_t quot, const __m512i *pd) -> int {
        assert(quot < 50);
        // assert(quot <= _mm_popcnt_u64(get_clean_header(pd)));
        if (quot == 0) {
            return pd512::count_zeros_up_to_the_kth_one(pd, 0);
        }
        return pd512::count_zeros_up_to_the_kth_one(pd, quot) - pd512::count_zeros_up_to_the_kth_one(pd, quot - 1);
    }
}