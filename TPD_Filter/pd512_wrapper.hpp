//
// Created by tomer on 7/12/20.
//

#ifndef FILTERS_PD512_WRAPPER_HPP
#define FILTERS_PD512_WRAPPER_HPP

#include <string>

extern "C" {
#include "pd512.h"
}


class pd512_wrapper {
    __m512i x{~INT64_C(0), 0, 0, 0, 0, 0, 0, 0};

public:
    pd512_wrapper(size_t m, size_t f, size_t l) {}

    auto lookup(int64_t quot, char rem) -> bool {
        return pd_find_50(quot, rem, &x);
    }

    auto insert(int64_t quot, char rem) -> bool {
        return pd_add_50(quot, rem, &x);
    }

    void remove(int64_t quot, char rem) {
        assert(false);
    }

    auto conditional_remove(int64_t quot, char rem) -> bool {
        if (lookup(quot, rem)) {
            remove(quot, rem);
            return true;
        }
        return false;
    }

    auto is_full() -> bool {
        return get_capacity() == 50;
    }

    auto get_capacity() -> size_t {
        unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, &x, kBytes2copy);
        // Number of bits to keep. Requires little-endianness
        const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        return popcount128(header);

    }

    auto get_name() -> std::string {
        return "pd512 ";
    }
};


#endif //FILTERS_PD512_WRAPPER_HPP
