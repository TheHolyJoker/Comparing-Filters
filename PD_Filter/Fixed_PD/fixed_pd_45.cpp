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


        // auto read_counter_switch(uint64_t quot, const __m512i *pd) -> size_t {
        //     switch (quot) {
        //         case 0:
        //             return (((uint64_t *) pd)[0] >> 0ul) & MASK(counter_size);
        //         case 1:
        //             return (((uint64_t *) pd)[0] >> 3ul) & MASK(counter_size);
        //         case 2:
        //             return (((uint64_t *) pd)[0] >> 6ul) & MASK(counter_size);
        //         case 3:
        //             return (((uint64_t *) pd)[0] >> 9ul) & MASK(counter_size);
        //         case 4:
        //             return (((uint64_t *) pd)[0] >> 12ul) & MASK(counter_size);
        //         case 5:
        //             return (((uint64_t *) pd)[0] >> 15ul) & MASK(counter_size);
        //         case 6:
        //             return (((uint64_t *) pd)[0] >> 18ul) & MASK(counter_size);
        //         case 7:
        //             return (((uint64_t *) pd)[0] >> 21ul) & MASK(counter_size);
        //         case 8:
        //             return (((uint64_t *) pd)[0] >> 24ul) & MASK(counter_size);
        //         case 9:
        //             return (((uint64_t *) pd)[0] >> 27ul) & MASK(counter_size);
        //         case 10:
        //             return (((uint64_t *) pd)[0] >> 30ul) & MASK(counter_size);
        //         case 11:
        //             return (((uint64_t *) pd)[0] >> 33ul) & MASK(counter_size);
        //         case 12:
        //             return (((uint64_t *) pd)[0] >> 36ul) & MASK(counter_size);
        //         case 13:
        //             return (((uint64_t *) pd)[0] >> 39ul) & MASK(counter_size);
        //         case 14:
        //             return (((uint64_t *) pd)[0] >> 42ul) & MASK(counter_size);
        //         case 15:
        //             return (((uint64_t *) pd)[0] >> 45ul) & MASK(counter_size);
        //         case 16:
        //             return (((uint64_t *) pd)[0] >> 48ul) & MASK(counter_size);
        //         case 17:
        //             return (((uint64_t *) pd)[0] >> 51ul) & MASK(counter_size);
        //         case 18:
        //             return (((uint64_t *) pd)[0] >> 54ul) & MASK(counter_size);
        //         case 19:
        //             return (((uint64_t *) pd)[0] >> 57ul) & MASK(counter_size);
        //         case 20:
        //             return (((uint64_t *) pd)[0] >> 60ul) & MASK(counter_size);
        //         case 21:
        //             return (((uint64_t *) pd)[1] >> 0ul) & MASK(counter_size);
        //         case 22:
        //             return (((uint64_t *) pd)[1] >> 3ul) & MASK(counter_size);
        //         case 23:
        //             return (((uint64_t *) pd)[1] >> 6ul) & MASK(counter_size);
        //         case 24:
        //             return (((uint64_t *) pd)[1] >> 9ul) & MASK(counter_size);
        //         case 25:
        //             return (((uint64_t *) pd)[1] >> 12ul) & MASK(counter_size);
        //         case 26:
        //             return (((uint64_t *) pd)[1] >> 15ul) & MASK(counter_size);
        //         case 27:
        //             return (((uint64_t *) pd)[1] >> 18ul) & MASK(counter_size);
        //         case 28:
        //             return (((uint64_t *) pd)[1] >> 21ul) & MASK(counter_size);
        //         case 29:
        //             return (((uint64_t *) pd)[1] >> 24ul) & MASK(counter_size);
        //         case 30:
        //             return (((uint64_t *) pd)[1] >> 27ul) & MASK(counter_size);
        //         case 31:
        //             return (((uint64_t *) pd)[1] >> 30ul) & MASK(counter_size);
        //         case 32:
        //             return (((uint64_t *) pd)[1] >> 33ul) & MASK(counter_size);
        //         case 33:
        //             return (((uint64_t *) pd)[1] >> 36ul) & MASK(counter_size);
        //         case 34:
        //             return (((uint64_t *) pd)[1] >> 39ul) & MASK(counter_size);
        //         case 35:
        //             return (((uint64_t *) pd)[1] >> 42ul) & MASK(counter_size);
        //         case 36:
        //             return (((uint64_t *) pd)[1] >> 45ul) & MASK(counter_size);
        //         case 37:
        //             return (((uint64_t *) pd)[1] >> 48ul) & MASK(counter_size);
        //         case 38:
        //             return (((uint64_t *) pd)[1] >> 51ul) & MASK(counter_size);
        //         case 39:
        //             return (((uint64_t *) pd)[1] >> 54ul) & MASK(counter_size);
        //         case 40:
        //             return (((uint64_t *) pd)[1] >> 57ul) & MASK(counter_size);
        //         case 41:
        //             return (((uint64_t *) pd)[1] >> 60ul) & MASK(counter_size);
        //         case 42:
        //             return (((uint64_t *) pd)[2] >> 0ul) & MASK(counter_size);
        //         case 43:
        //             return (((uint64_t *) pd)[2] >> 3ul) & MASK(counter_size);
        //         case 44:
        //             return (((uint64_t *) pd)[2] >> 6ul) & MASK(counter_size);
        //         case 45:
        //             return (((uint64_t *) pd)[2] >> 9ul) & MASK(counter_size);
        //         case 46:
        //             return (((uint64_t *) pd)[2] >> 12ul) & MASK(counter_size);
        //         case 47:
        //             return (((uint64_t *) pd)[2] >> 15ul) & MASK(counter_size);
        //         case 48:
        //             return (((uint64_t *) pd)[2] >> 18ul) & MASK(counter_size);
        //         case 49:
        //             return (((uint64_t *) pd)[2] >> 21ul) & MASK(counter_size);
        //         default:
        //             assert(false);
        //             return 0;
        //     }
        // }
    }
}
