
#ifndef FILTERS_SAFE512_HPP
#define FILTERS_SAFE512_HPP

#include "../TPD_Filter/pd512.hpp"
#include <cstddef>
#include <memory>
#include <string>

static const size_t capacity = 51u;
static uint counter = 0;

// class alignas(64) safe512 {
class safe512
{
    // __m512i x;
    //    __m512i x;

public:
    // __m512i x;// = {(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    // __m512i x __attribute__((aligned(512))) = { (INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0 };
    __m512i x = {(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    uint64_t y[8]{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    // __m512i x __attribute__((aligned(512))) = { (INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0 };

    safe512(size_t m, size_t f, size_t l) {}

    safe512() {}
    
    auto lookup(int64_t quot, char rem) -> bool
    {
        return pd512::pd_find_50(quot % capacity, rem, &x);
    }

    auto insert(int64_t quot, char rem) -> bool
    {
        return pd512::pd_add_50(quot % capacity, rem, &x);
    }

    void remove(int64_t quot, char rem)
    {
        assert(false);
    }

    auto conditional_remove(int64_t quot, char rem) -> bool
    {
        if (lookup(quot % capacity, rem))
        {
            remove(quot % capacity, rem);
            return true;
        }
        return false;
    }

    auto is_full() -> bool
    {
        return get_capacity() == 51;
    }

    auto get_capacity() -> size_t
    {
        // return get_capacity_naive();
        uint64_t header[2];
        // We need to copy (50+51) bits, but we copy slightly more and mask out the ones we
        // don't care about.
        //
        // memcpy is the only defined punning operation
        memcpy(header, &x, 13);
        header[1] = header[1] & ((UINT64_C(1) << (50 + 51 - 64)) - 1);

        auto h0_one_count = _mm_popcnt_u64(header[0]);
        if (h0_one_count == 50)
        {
            auto index = pd512::select64(header[0], 49);
            assert(index < 64);
            return index - 49;
            auto res = index - 49;
            assert(res == get_capacity_naive());
        }
        else
        {
            auto h0_zero_count = 64 - h0_one_count;
            auto ones_left = 49 - h0_one_count;
            // bool cond =
            assert(ones_left >= 0);
            auto index = pd512::select64(header[1], 49 - h0_one_count);
            assert(index < 64);
            assert(index <= 37);
            assert(index >= ones_left);
            return index - ones_left + h0_zero_count;
            auto res = index - ones_left + h0_zero_count;
            bool cond = res == get_capacity_naive();
            if (!cond)
            {
                std::cout << "h0_zero_count " << h0_zero_count << std::endl;
                std::cout << "h0_one_count " << h0_one_count << std::endl;
                std::cout << res << std::endl;
                std::cout << get_capacity_naive() << std::endl;
                assert(false);
            }
            // assert(res == get_capacity_naive());
            return res;
        }
    }

    auto get_capacity_naive() -> size_t
    {
        // std::cout << "here" << std::endl;
        uint64_t header[2];
        memcpy(header, &x, 13);
        // header[1] = header[1] & ((UINT64_C(1) << (50 + 51 - 64)) - 1);
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
        assert(false);
    }
    auto get_name() -> std::string
    {
        return "pd512 ";
    }

    /* void *operator new(std::size_t size)
    {
        //        int ok = posix_memalign((void**)&Table, sizeof(Bucket) * CHAR_BIT, sizeof(Bucket) * num_of_buckets);
        //        int ok = posix_memalign((void**)&safe512, )
        //        return posix_memalign()
        auto res = _mm_malloc(size, alignof(safe512));
        assert(res != NULL);
        return res;
        //        return _mm_malloc(size, alignof(safe512));
    }

    void *operator new[](std::size_t size)
    {
        return _mm_malloc(size, alignof(safe512));
    }

    void operator delete(void *ptr)
    {
        _mm_free(ptr);
    }

    void operator delete[](void *ptr)
    {
        _mm_free(ptr);
    } */
};

#endif //FILTERS_SAFE512_HPP
