//
// Created by tomer on 05/10/2020.
//

#include "fixed_pd_45.hpp"

namespace Fixed_pd45 {
    namespace Header {
        auto sum_words_naive(size_t w_end_index, const header_type *header) -> size_t {
            size_t res = 0;
            for (size_t i = 0; i < w_end_index; i++) {
                res += sum_word_v1(header[i]);
            }
            return res;
        }

        auto validate_sum_words_naive(size_t w_end_index, const header_type *header) -> size_t {
            auto v_res = validate_sum_words_naive(w_end_index, header);
            return -1;
        }

        auto validate_capacity(const __m512i *pd) -> bool {
            auto temp = get_capacity(pd);
            assert(temp <= CAPACITY);
            return true;
        }

    }
}
