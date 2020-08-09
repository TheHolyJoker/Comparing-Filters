/*
 * Taken from this repository.
 * https://github.com/jbapple/crate-dictionary
 * */

#include "pd512.hpp"

namespace pd512
{
    // returns the position (starting from 0) of the jth set bit of x.
    uint64_t select64(uint64_t x, int64_t j)
    {
        assert(j < 64);
        const uint64_t y = _pdep_u64(UINT64_C(1) << j, x);
        return _tzcnt_u64(y);
    }

    // returns the position (starting from 0) of the jth set bit of x.
    uint64_t select128(unsigned __int128 x, int64_t j)
    {
        const int64_t pop = _mm_popcnt_u64(x);
        if (j < pop)
            return select64(x, j);
        return 64 + select64(x >> 64, j - pop);
    }

    // returns the number of zeros before the jth (counting from 0) set bit of x
    uint64_t nth64(uint64_t x, int64_t j)
    {
        const uint64_t y = select64(x, j);
        assert(y < 64);
        const uint64_t z = x & ((UINT64_C(1) << y) - 1);
        return y - _mm_popcnt_u64(z);
    }

    // returns the number of zeros before the jth (counting from 0) set bit of x
    uint64_t nth128(unsigned __int128 x, int64_t j)
    {
        const uint64_t y = select128(x, j);
        assert(y < 128);
        const unsigned __int128 z = x & ((((unsigned __int128)1) << y) - 1);
        return y - _mm_popcnt_u64(z) - _mm_popcnt_u64(z >> 64);
    }

    int popcount64(uint64_t x)
    {
        return _mm_popcnt_u64(x);
    }

    int popcount128(unsigned __int128 x)
    {
        const uint64_t hi = x >> 64;
        const uint64_t lo = x;
        return popcount64(lo) + popcount64(hi);
    }

    // find an 8-bit value in a pocket dictionary with quotients in [0,64) and 49 values
    bool pd_find_64(int64_t quot, char rem, const __m512i *pd)
    {
        assert(quot < 64);
        // The header has size 64 + 49
        uint64_t header[2];
        // We need to copy (64+49) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        memcpy(header, pd, 16);
        header[1] = header[1] & ((UINT64_C(1) << 49) - 1);
        assert(_mm_popcnt_u64(header[0]) + _mm_popcnt_u64(header[1]) == 64);
        // [begin,end) is the indexes of the values that have quot as their quotient. the range
        // is exclusive and end is at most 49
        const int64_t p = _mm_popcnt_u64(header[0]);
        uint64_t begin = 0;
        if (quot > 0)
        {
            begin = ((quot <= p) ? nth64(header[0], quot - 1)
                                 : ((64 - p) + nth64(header[1], quot - 1 - p)));
        }
        const uint64_t end =
            (quot < p) ? nth64(header[0], quot) : ((64 - p) + nth64(header[1], quot - p));
        assert(begin <= end);
        assert(end <= 49);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // round up to remove the header
        v = v >> ((64 + 49 + CHAR_BIT - 1) / CHAR_BIT);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    // find an 8-bit value in a pocket dictionary with quotients in [0,50) and 51 values
    bool pd_find_50(int64_t quot, char rem, const __m512i *pd)
    {
        assert(quot < 50);
        // The header has size 50 + 51
        uint64_t header[2];
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        memcpy(header, pd, 13);
        // Requires little-endianness
        header[1] = header[1] & ((UINT64_C(1) << (50 + 51 - 64)) - 1);
        assert(_mm_popcnt_u64(header[0]) + _mm_popcnt_u64(header[1]) == 50);
        // [begin,end) is the indexes of the values that have quot as their quotient. the range
        // is exclusive and end is at most 51
        const int64_t p = _mm_popcnt_u64(header[0]);
        uint64_t begin = 0;
        if (quot > 0)
        {
            begin = ((quot <= p) ? nth64(header[0], quot - 1)
                                 : ((64 - p) + nth64(header[1], quot - 1 - p)));
        }
        const uint64_t end =
            (quot < p) ? nth64(header[0], quot) : ((64 - p) + nth64(header[1], quot - p));
        assert(begin <= end);
        assert(end <= 51);
        const __m512i target = _mm512_set1_epi8(rem);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // round up to remove the header
        v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
        return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    // insert a pair of a quotient (mod 50) and an 8-bit remainder in a pocket dictionary.
    // Returns false if the dictionary is full.
    bool pd_add_50(int64_t quot, char rem, __m512i *pd)
    {
        // print512(*pd);
        assert(quot < 50);
        // The header has size 50 + 51
        unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, pd, kBytes2copy);
        auto my_temp = popcount128(header);

        // std::cout << "my_temp: " << my_temp << std::endl;
        // Number of bits to keep. Requires little-endianness
        const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128)1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        /* TODO!!! */
        assert(popcount128(header) == 50);
        const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51)
            return false;
        // [begin,end) are the zeros in the header that correspond to the fingerprints with
        // quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        // assert(end <= 50 + 51);
        unsigned __int128 new_header = header & ((((unsigned __int128)1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        /* Error is here OR in line 176! */
        memcpy(pd, &new_header, kBytes2copy);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i)
        {
            if (rem <= ((const char *)pd)[kBytes2copy + i])
                break;
        }
        assert((i == end_fingerprint) || (rem <= ((const char *)pd)[kBytes2copy + i]));
        /* or here! */
        memmove(&((char *)pd)[kBytes2copy + i + 1],
                &((const char *)pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((char *)pd)[kBytes2copy + i] = rem;

        assert(pd_find_50(quot, rem, pd));
        return true;
        // //// jbapple: STOPPED HERE
        // const __m512i target = _mm512_set1_epi8(rem);
        // uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd);
        // // round up to remove the header
        // v = v >> ((51 + 50 + CHAR_BIT - 1) / CHAR_BIT);
        // return (v & ((UINT64_C(1) << end) - 1)) >> begin;
    }

    /* bool pd_add_50_v2(int64_t quot, uint8_t rem, __m512i *pd)
    {
        assert(0 == (reinterpret_cast<uintptr_t>(pd) % 64));
        assert(quot < 50);
        // The header has size 50 + 51
        unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the
        // ones we don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, pd, kBytes2copy);
        // Number of bits to keep. Requires little-endianness
        // const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask =
            (((unsigned __int128)1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        assert(50 == popcount128(header));
        const unsigned fill = select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd_popcount(pd)));
        assert((fill == 51) == pd_full(pd));
        if (fill == 51)
            return false;
        // [begin,end) are the zeros in the header that correspond to the fingerprints
        // with quotient quot.
        const uint64_t begin = quot ? (select128(header, quot - 1) + 1) : 0;
        const uint64_t end = select128(header, quot);
        assert(begin <= end);
        assert(end <= 50 + 51);
        unsigned __int128 new_header =
            header & ((((unsigned __int128)1) << begin) - 1);
        new_header |= ((header >> end) << (end + 1));
        assert(popcount128(new_header) == 50);
        assert(select128(new_header, 50 - 1) - (50 - 1) == fill + 1);
        memcpy(pd, &new_header, kBytes2copy);
        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= 51);
        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i)
        {
            if (rem <= ((const uint8_t *)pd)[kBytes2copy + i])
                break;
        }
        assert((i == end_fingerprint) ||
               (rem <= ((const uint8_t *)pd)[kBytes2copy + i]));
        memmove(&((uint8_t *)pd)[kBytes2copy + i + 1],
                &((const uint8_t *)pd)[kBytes2copy + i],
                sizeof(*pd) - (kBytes2copy + i + 1));
        ((uint8_t *)pd)[kBytes2copy + i] = rem;

        assert(pd_find_50(quot, rem, pd) == pd_find_50_alt(quot, rem, pd));
        assert(pd_find_50_alt2(quot, rem, pd) == pd_find_50_alt(quot, rem, pd));
        assert(pd_find_50_alt3(quot, rem, pd) == pd_find_50_alt(quot, rem, pd));
        assert(pd_find_50_alt4(quot, rem, pd) == pd_find_50_alt(quot, rem, pd));
        assert(pd_find_50_alt5(quot, rem, pd) == pd_find_50_alt(quot, rem, pd));
        assert(pd_find_50(quot, rem, pd));
        assert(pd_find_50_alt(quot, rem, pd));
        assert(pd_find_50_alt2(quot, rem, pd));
        assert(pd_find_50_alt3(quot, rem, pd));
        assert(pd_find_50_alt4(quot, rem, pd));
        assert(pd_find_50_alt5(quot, rem, pd));
        return true;
    }
 */

    auto remove(int64_t quot, char rem, __m512i *pd) -> bool
    {
        assert(false);
        return false;
    }
    auto conditional_remove(int64_t quot, char rem, __m512i *pd) -> bool
    {
        if (pd_find_50(quot % 51, rem, pd))
        {
            remove(quot % 51, rem, pd);
            return true;
        }
        return false;
    }

    void print512(const __m512i *var)
    {
        long int val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        memcpy(val, &var, sizeof(val));
        for (size_t i = 0; i < 8; i++)
        {
            std::cout << val[i] << ", ";
            /* code */
        }
        std::cout << std::endl;

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
    auto is_full(const __m512i *x) -> bool
    {
        assert(get_capacity_naive(x) == get_capacity(x));
        return get_capacity(x) == 51;
    }

    auto validate_number_of_quotient(const __m512i *pd) -> bool
    {
        // std::cout << "h128: " << std::endl;

        unsigned __int128 header = 0;
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        const unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        assert(kBytes2copy < sizeof(header));
        memcpy(&header, pd, kBytes2copy);
        auto my_temp = popcount128(header);

        // std::cout << "my_temp: " << my_temp << std::endl;
        // Number of bits to keep. Requires little-endianness
        const unsigned __int128 kLeftover = sizeof(header) * CHAR_BIT - 50 - 51;
        const unsigned __int128 kLeftoverMask = (((unsigned __int128)1) << (50 + 51)) - 1;
        header = header & kLeftoverMask;
        size_t res = popcount128(header);
        if (res != 50)
        {
            std::cout << "popcount128: " << res << std::endl;
        }
        return popcount128(header) == 50;
        // return true;
    }

    auto get_capacity(const __m512i *x) -> size_t
    {
        validate_number_of_quotient(x);
        // return get_capacity_naive();
        uint64_t header[2];
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        memcpy(header, x, 13);
        auto old_header = header[1];
        // header[1] = header[1] & ((UINT64_C(1) << (50 + 51 - 64)) - 1);

        auto h0_one_count = _mm_popcnt_u64(header[0]);
        if (h0_one_count == 50)
        {
            auto index = pd512::select64(header[0], 49);
            assert(index < 64);
            auto res = index - 49;
            assert(res == get_capacity_naive(x));
            return res;
        }
        else
        {
            auto h0_zero_count = 64 - h0_one_count;
            auto ones_left = 49 - h0_one_count;
            // bool cond =
            assert(ones_left >= 0);
            auto index = pd512::select64(header[1], 49 - h0_one_count);
            auto att_index = pd512::select64(header[1], 49 - h0_one_count - 1);
            if (index == 64)
            {
                std::cout << header[0] << std::endl;
                std::cout << header[1] << std::endl;
                std::cout << old_header << std::endl;
                std::cout << h0_one_count << std::endl;
                std::cout << ones_left << std::endl;
                std::cout << get_capacity_naive(x) << std::endl;
                return -1;
                assert(false);
            }
            assert(index < 64);
            assert(index <= 37);
            assert(index >= ones_left);
            auto res = index - ones_left + h0_zero_count;
            bool cond = res == get_capacity_naive(x);
            if (!cond)
            {
                std::cout << "h0_zero_count " << h0_zero_count << std::endl;
                std::cout << "h0_one_count " << h0_one_count << std::endl;
                std::cout << res << std::endl;
                std::cout << get_capacity_naive(x) << std::endl;
                return -1;
                assert(false);
            }
            return (res == get_capacity_naive(x)) ? res : -1;
            // assert(res == get_capacity_naive(x));
            // return res;
            return index - ones_left;
        }
    }

    auto get_capacity_naive(const __m512i *x) -> size_t
    {
        uint64_t header[2];
        memcpy(header, x, 13);
        size_t zero_count = 0, one_count = 0;
        for (size_t j = 0; j < 2; j++)
        {
            uint64_t temp = header[j];
            uint64_t b = 1ULL;
            for (size_t i = 0; i < 64; i++)
            {
                if (b & temp)
                {
                    one_count++;
                    if (one_count == 50)
                        return zero_count;
                }
                else
                {
                    zero_count++;
                }
                b <<= 1ul;
            }
        }
        std::cout << zero_count << std::endl;
        std::cout << one_count << std::endl;
        return -1;
        assert(false);
    }
    auto get_name() -> std::string
    {
        return "pd512 ";
    }

    ////New functions
    int pd_popcount(const __m512i *pd)
    {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        constexpr uint64_t kLeftoverMask = (UINT64_C(1) << (50 + 51 - 64)) - 1;
        header_end = header_end & kLeftoverMask;
        const int result = 128 - 51 - _lzcnt_u64(header_end) + 1;
        return result;
    }

    bool pd_full(const __m512i *pd)
    {
        uint64_t header_end;
        memcpy(&header_end, reinterpret_cast<const uint64_t *>(pd) + 1,
               sizeof(header_end));
        return 1 & (header_end >> (50 + 51 - 64 - 1));
    }

} // namespace pd512

auto my_equal(__m512i x, __m512i y) -> bool
{
    bool res = (_mm512_cmpeq_epi64_mask(x, y) == 255);
    __mmask8 temp = _mm512_cmpeq_epi64_mask(x, y);
    assert(res == (temp == 255));
    return res;
}
