// Copied from Apache Impala (incubating), usable under the terms in the Apache License,
// Version 2.0.

// This is a block Bloom filter (from Putze et al.'s "Cache-, Hash- and Space-Efficient
// Bloom Filters") with some twists:
//
// 1. Each block is a split Bloom filter - see Section 2.1 of Broder and Mitzenmacher's
// "Network Applications of Bloom Filters: A Survey".
//
// 2. The number of bits set per Add() is contant in order to take advantage of SIMD
// instructions.

#pragma once

#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <new>

#include "../hashutil.h"
#include <immintrin.h>


using uint32_t = ::std::uint32_t;
using uint64_t = ::std::uint64_t;

namespace Helpers {
    [[gnu::always_inline]] inline size_t popcount_MaskMask(const __m256i *mask) {
        const uint64_t *temp_arr = (const uint64_t *) mask;
        uint64_t db_vals[4] = {temp_arr[0], temp_arr[1], temp_arr[2], temp_arr[3]};
        size_t res = 0;
        size_t val1 = _mm_popcnt_u64(temp_arr[0]);
        size_t val2 = _mm_popcnt_u64(temp_arr[1]);
        size_t val3 = _mm_popcnt_u64(temp_arr[2]);
        size_t val4 = _mm_popcnt_u64(temp_arr[3]);
        assert(val1 <= 64);
        assert(val2 <= 64);
        assert(val3 <= 64);
        assert(val4 <= 64);
        return val1 + val2 + val3 + val4;
    }

    inline __m256i build_mask(size_t word_index, size_t bit_index){
        assert(bit_index < 64);
        __m256i temp;
        uint64_t data[4] = {0,0,0,0};
        data[word_index] = (1ULL << bit_index);
        memcpy(&temp, data, 32);
        return temp;
    }

    inline void keep_only_one_bit(__m256i *mask, const __m256i *bucket) {
        uint64_t mask_arr[4];
        memcpy(mask_arr, mask, 32);
        uint64_t bucket_arr[4];
        memcpy(mask_arr, bucket, 32);

        size_t i = 0;
        size_t bit_index = 0;
        for (; i < 4; i++) {
            uint64_t temp_val = _mm_popcnt_u64(mask_arr[i] & bucket_arr[i]);
            if (temp_val){
                size_t pre_pop_mask = popcount_MaskMask(mask);
                bit_index = _tzcnt_u64(mask_arr[i] & bucket_arr[i]);
                assert(bit_index < 64);
                __m256i temp = build_mask(i, bit_index);
                *mask = temp;
                size_t post_pop_mask = popcount_MaskMask(mask);
                assert(post_pop_mask == 1);
                auto test_not_zero = _mm256_and_si256(*mask, *bucket);
                auto test_zero = _mm256_andnot_si256(*bucket,*mask);
                auto pop_res1 = popcount_MaskMask(&test_not_zero);
                auto pop_res0 = popcount_MaskMask(&test_zero);
                assert(pop_res1 == 1);
                assert(pop_res0 == 0);
                // assert(popcount_MaskMask(&test_zero) == 0);
                // assert()
            }
        }
        return;



        // uint64_t *temp_arr = (const uint64_t *) mask;
        // uint64_t db_vals[4] = {temp_arr[0], temp_arr[1], temp_arr[2], temp_arr[3]};
        // size_t res = 0;
        // size_t val1 = _mm_popcnt_u64(temp_arr[0]);
        // size_t val2 = _mm_popcnt_u64(temp_arr[1]);
        // size_t val3 = _mm_popcnt_u64(temp_arr[2]);
        // size_t val4 = _mm_popcnt_u64(temp_arr[3]);
        // assert(val1 <= 64);
        // assert(val2 <= 64);
        // assert(val3 <= 64);
        // assert(val4 <= 64);
        // return val1 + val2 + val3 + val4;
    }

    template<typename T>
    void print_array(T *a, size_t size) {
        assert(size);
        std::cout << "[" << a[0];
        for (size_t i = 1; i < size; i++) {
            std::cout << ", " << a[i];
        }
        std::cout << "]" << std::endl;
    }

    // void print_array(uint8_t *a, size_t size);
    inline void print_array(uint8_t *a, size_t size) {
        assert(size);
        std::cout << "[" << ((uint16_t) a[0]);
        ;
        for (size_t i = 1; i < size; i++) {
            std::cout << ", " << ((uint16_t) a[i]);
        }
        std::cout << "]" << std::endl;
    }


    template<typename vecSize>
    void print_vec(size_t size, const vecSize *v) {
        const size_t bit_size = sizeof(*v) * CHAR_BIT;
        const size_t a_size = bit_size / size;
        if (size == 64) {
            uint64_t val[a_size];
            memcpy(val, v, sizeof(*v));
            print_array(val, a_size);
            // printf("[%zu, %zu, %zu, %zu] \n", val[0], val[1], val[2], val[3]);
        } else if (size == 32) {
            uint32_t val[a_size];
            memcpy(val, v, sizeof(*v));
            print_array(val, a_size);
            // printf("[%u, %u, %u, %u, %u, %u, %u, %u] \n",
            //        val[0], val[1], val[2], val[3],
            //        val[4 + 0], val[4 + 1], val[4 + 2], val[4 + 3]);
        } else if (size == 16) {
            // assert(a_size == 16);
            uint16_t val[a_size];
            memcpy(val, v, sizeof(*v));
            print_array(val, a_size);
        } else if (size == 8) {
            // assert(a_size == 32);
            uint8_t val[a_size];
            memcpy(val, v, sizeof(*v));
            print_array(val, a_size);
        } else {
            std::cout << "size is: " << size << std::endl;
            assert(0);
        }
    }
}// namespace Helpers


template<typename HashFamily = ::hashing::TwoIndependentMultiplyShift>
class TS_SimdBlockFilter {
private:
    // The filter is divided up into Buckets:
    using Bucket = uint32_t[8];

    // log2(number of bytes in a bucket):
    static constexpr int LOG_BUCKET_BYTE_SIZE = 5;

    static_assert(
            (1 << LOG_BUCKET_BYTE_SIZE) == sizeof(Bucket) && sizeof(Bucket) == sizeof(__m256i),
            "Bucket sizing has gone awry.");

    // log_num_buckets_ is the log (base 2) of the number of buckets in the directory:
    const int log_num_buckets_;

    // directory_mask_ is (1 << log_num_buckets_) - 1. It is precomputed in the contructor
    // for efficiency reasons:
    const uint32_t directory_mask_;

    Bucket *directory_;

    HashFamily hasher_;

    size_t total_del_counter;
    size_t failed_del_counter;

    size_t freq_pop_count_mask[257];
    size_t freq_pop_count_block[257];

public:
    // Consumes at most (1 << log_heap_space) bytes on the heap:
    explicit TS_SimdBlockFilter(const int log_heap_space);

    TS_SimdBlockFilter(TS_SimdBlockFilter &&that)
        : log_num_buckets_(that.log_num_buckets_),
          directory_mask_(that.directory_mask_),
          directory_(that.directory_),
          hasher_(that.hasher_),
          total_del_counter(0),
          failed_del_counter(0) {
        for (size_t i = 0; i < 257; i++) {
            freq_pop_count_mask[i] = 0;
            freq_pop_count_block[i] = 0;
        }
    }

    ~TS_SimdBlockFilter() noexcept;

    void Add(const uint64_t key) noexcept;


    void Stupid_Delete(const uint64_t key) noexcept;
    void naiveDelete(const uint64_t key) noexcept;
    void Delete(const uint64_t key) noexcept;

    bool Find(const uint64_t key) const noexcept;

    uint64_t SizeInBytes() const { return sizeof(Bucket) * (1ull << log_num_buckets_); }

private:
    // A helper function for Insert()/Find(). Turns a 32-bit hash into a 256-bit Bucket
    // with 1 single 1-bit set in each 32-bit lane.
    static __m256i MakeMask(const uint32_t hash) noexcept;

    TS_SimdBlockFilter(const TS_SimdBlockFilter &) = delete;

    void operator=(const TS_SimdBlockFilter &) = delete;
};

template<typename HashFamily>
TS_SimdBlockFilter<HashFamily>::TS_SimdBlockFilter(const int log_heap_space)
    :// Since log_heap_space is in bytes, we need to convert it to the number of Buckets
      // we will use.
      log_num_buckets_(::std::max(1, log_heap_space - LOG_BUCKET_BYTE_SIZE)),
      // Don't use log_num_buckets_ if it will lead to undefined behavior by a shift that is
      // too large.
      directory_mask_((1ull << ::std::min(63, log_num_buckets_)) - 1),
      directory_(nullptr),
      hasher_() {
    if (!__builtin_cpu_supports("avx2")) {
        throw ::std::runtime_error("SimdBlockFilter does not work without AVX2 instructions");
    }
    const size_t alloc_size = 1ull << (log_num_buckets_ + LOG_BUCKET_BYTE_SIZE);
    const int malloc_failed =
            posix_memalign(reinterpret_cast<void **>(&directory_), 64, alloc_size);
    if (malloc_failed) throw ::std::bad_alloc();
    memset(directory_, 0, alloc_size);

    for (size_t i = 0; i < 257; i++) {
        freq_pop_count_mask[i] = 0;
        freq_pop_count_block[i] = 0;
    }
}

template<typename HashFamily>
TS_SimdBlockFilter<HashFamily>::~TS_SimdBlockFilter() noexcept {
    double remove_ratio = 1.0 * failed_del_counter / total_del_counter;
    std::cout << "Failed-remove-counter-ratio: " << remove_ratio << std::endl;


    size_t sum = 0;
    for (size_t i = 0; i < 257; i++) {
        sum += freq_pop_count_block[i];
    }

    std::cout << "sum: " << sum << std::endl;
    for (size_t i = 0; i < 257; i++) {
        // double val = 1.0 * freq_pop_count_mask[i] / sum;
        std::cout << i << ": \t" << freq_pop_count_block[i] << std::endl;
    }


    free(directory_);
    directory_ = nullptr;
}

// The SIMD reinterpret_casts technically violate C++'s strict aliasing rules. However, we
// compile with -fno-strict-aliasing.
template<typename HashFamily>
[[gnu::always_inline]] inline __m256i
TS_SimdBlockFilter<HashFamily>::MakeMask(const uint32_t hash) noexcept {
    const __m256i ones = _mm256_set1_epi32(1);
    // Odd contants for hashing:
    const __m256i rehash = _mm256_setr_epi32(0x47b6137bU, 0x44974d91U, 0x8824ad5bU,
                                             0xa2b7289dU, 0x705495c7U, 0x2df1424bU, 0x9efc4947U, 0x5c6bfb31U);
    // Load hash into a YMM register, repeated eight times
    __m256i hash_data = _mm256_set1_epi32(hash);
    // Multiply-shift hashing ala Dietzfelbinger et al.: multiply 'hash' by eight different
    // odd constants, then keep the 5 most significant bits from each product.
    hash_data = _mm256_mullo_epi32(rehash, hash_data);
    hash_data = _mm256_srli_epi32(hash_data, 27);
    // Use these 5 bits to shift a single bit to a location in each 32-bit lane
    return _mm256_sllv_epi32(ones, hash_data);
}

template<typename HashFamily>
[[gnu::always_inline]] inline void
TS_SimdBlockFilter<HashFamily>::Add(const uint64_t key) noexcept {
    static int counter = 0;
    counter++;
    const auto hash = hasher_(key);
    const uint32_t bucket_idx = hash & directory_mask_;
    const __m256i mask = MakeMask(hash >> log_num_buckets_);

    // if ((counter & 63) == 63) {
    //     Helpers::print_vec(8, &mask);
    // }
    size_t mask_index = Helpers::popcount_MaskMask(&mask);
    assert(mask_index <= 256);
    freq_pop_count_mask[mask_index]++;
    // }
    __m256i *const bucket = &reinterpret_cast<__m256i *>(directory_)[bucket_idx];
    _mm256_store_si256(bucket, _mm256_or_si256(*bucket, mask));
    size_t block_index = Helpers::popcount_MaskMask(bucket);
    assert(block_index <= 256);
    freq_pop_count_block[block_index]++;
}

template<typename HashFamily>
[[gnu::always_inline]] inline void
TS_SimdBlockFilter<HashFamily>::Stupid_Delete(const uint64_t key) noexcept {
    // constexpr unsigned
    return;
    total_del_counter++;
    if ((key & 3) != 3) {
        failed_del_counter++;
        return;
    }
    const auto hash = hasher_(key);
    const uint32_t bucket_idx = hash & directory_mask_;
    const __m256i mask = MakeMask(hash >> log_num_buckets_);
    __m256i *const bucket = &reinterpret_cast<__m256i *>(directory_)[bucket_idx];
    // __m256i temp = _mm256_or_si256(*bucket, mask);
    // __m256i res = _mm256_xor_si256(temp, mask);
    // __m256i att = _mm256_andnot_si256(mask, *bucket);
    // assert(_mm256_cmpeq_epi64_mask(res, res) == 15);
    // assert(_mm256_cmpeq_epi64_mask(att, att) == 15);
    // bool compare_res = (_mm256_cmpeq_epi64_mask(res, att) == 15);
    // assert(compare_res);
    _mm256_store_si256(bucket, _mm256_andnot_si256(mask, *bucket));
}

template<typename HashFamily>
[[gnu::always_inline]] inline void
TS_SimdBlockFilter<HashFamily>::naiveDelete(const uint64_t key) noexcept {
    total_del_counter++;
    const auto hash = hasher_(key);
    const uint32_t bucket_idx = hash & directory_mask_;
    __m256i mask = MakeMask(hash >> log_num_buckets_);
    __m256i *const bucket = &reinterpret_cast<__m256i *>(directory_)[bucket_idx];
    Helpers::keep_only_one_bit(&mask, bucket);
    _mm256_store_si256(bucket, _mm256_andnot_si256(mask, *bucket));
}


// [[gnu::always_inline]] inline void
// keep_only_the_first_bit(const uint64_t key) noexcept {

// }

template<typename HashFamily>
[[gnu::always_inline]] inline void
TS_SimdBlockFilter<HashFamily>::Delete(const uint64_t key) noexcept {
    total_del_counter++;
    return naiveDelete(key);
    return;

    const auto hash = hasher_(key);
    const uint32_t bucket_idx = hash & directory_mask_;
    const __m256i mask = MakeMask(hash >> log_num_buckets_);
    
    __m256i *const bucket = &reinterpret_cast<__m256i *>(directory_)[bucket_idx];
    _mm256_store_si256(bucket, _mm256_andnot_si256(mask, *bucket));
}


template<typename HashFamily>
[[gnu::always_inline]] inline bool
TS_SimdBlockFilter<HashFamily>::Find(const uint64_t key) const noexcept {
    const auto hash = hasher_(key);
    const uint32_t bucket_idx = hash & directory_mask_;
    const __m256i mask = MakeMask(hash >> log_num_buckets_);
    const __m256i bucket = reinterpret_cast<__m256i *>(directory_)[bucket_idx];
    // We should return true if 'bucket' has a one wherever 'mask' does. _mm256_testc_si256
    // takes the negation of its first argument and ands that with its second argument. In
    // our case, the result is zero everywhere iff there is a one in 'bucket' wherever
    // 'mask' is one. testc returns 1 if the result is 0 everywhere and returns 0 otherwise.
    return _mm256_testc_si256(bucket, mask);
}
