
#include "tpd.hpp"


int compare_vector_and_tpd(b_vec *v, uint64_t *arr, size_t bits_to_compare) {
    constexpr size_t slot_size = sizeof(uint64_t) * CHAR_BIT;
    size_t word_count = (bits_to_compare + slot_size - 1) / slot_size;
    size_t bit_count = 0;
    for (size_t i = 0; i < word_count; i++) {
        uint64_t word = arr[i];
        size_t v_index_offset = slot_size * i;
        uint64_t b = 1ULL;
        for (size_t j = 0; j < slot_size; j++) {
            if (bit_count == bits_to_compare)
                return -1;
            bool a = word & b;
            bool b = v->at(v_index_offset + j);
            if (a != b) {
                // std::cout << "/* message */" << std::endl;
                return i * slot_size + j;
            }
            b <<= 1ull;
            bit_count++;
        }
    }
    return -1;
}


auto count_zeros_up_to_the_kth_one(uint64_t word, size_t k) -> size_t {
    int one_count = -1;
    int zero_count = 0;
    uint64_t b = 1ULL;
    for (size_t i = 0; i < 64; i++) {
        if (b & word) {
            one_count++;
            assert(i - one_count == zero_count);
            if (one_count == k) {
                // auto res = i - k;
                // assert(res == zero_count);
                return i - k;
            }
        } else {
            zero_count++;
        }
        b <<= 1ul;
    }
    std::cout << one_count << std::endl;
    std::cout << zero_count << std::endl;
    return -1;
    assert(false);
}

auto count_ones_up_to_the_kth_zero(uint64_t word, size_t k) -> size_t {
    int zero_count = -1;
    int one_count = 0;
    uint64_t b = 1ULL;
    for (size_t i = 0; i < 64; i++) {
        if (b & word) {
            one_count++;
        } else {
            zero_count++;
            assert(i - zero_count == one_count);
            if (zero_count == k) {
                // auto res = i - k;
                // assert(res == zero_count);
                return i - k;
            }
        }
        b <<= 1ul;
    }
    std::cout << one_count << std::endl;
    std::cout << zero_count << std::endl;
    return -1;
    assert(false);
}


auto count_zeros_up_to_the_kth_one(const uint64_t *pd, size_t k) -> size_t {
    uint64_t pop1 = _mm_popcnt_u64(pd[0]);
    if (pop1 >= k) {
        return count_zeros_up_to_the_kth_one(pd[0], k);
    }
    uint64_t pop2 = _mm_popcnt_u64(pd[1]);
    assert(pop1 + pop2 >= k);
    auto w1_zeros = 64 - pop1;
    auto w2_zeros = count_zeros_up_to_the_kth_one(pd[1], k - pop1);
    return w1_zeros + w2_zeros;
}

auto count_ones_up_to_the_kth_zero(const uint64_t *arr, size_t k) -> size_t {
    uint64_t pop1 = _mm_popcnt_u64(pd[0]);
    size_t z1 = 64 - pop1;
    if (z1 >= k) {
        return count_ones_up_to_the_kth_zero(pd[0], k);
    }
    uint64_t pop2 = _mm_popcnt_u64(pd[1]);
    size_t z2 = 64 - pop2;
    assert(z1 + z2 >= k);
    auto o2 = count_ones_up_to_the_kth_zero(pd[1], k - z1);
    return pop1 + o2;
}

void read_bits_easy_case(const uint64_t *src, uint64_t *dest, size_t start, size_t end) {
    bool start_is_x8 = (start & 7 == 0);
    assert(start_is_x8);
    const uint64_t offset = start / CHAR_BIT;
    const uint64_t kBytes2copy = (end - start + CHAR_BIT - 1) / CHAR_BIT;
    memcpy(dest, ((uint8_t *) src) + offset, kBytes2copy);

    size_t last_index = (end - start + 63) / 64;
    uint64_t last_word_bits = (end - start) & 63;
    uint64_t mask = MASK(last_word_bits);
    dest[last_index] &= mask;
}

/**
 * @brief Clear the first shift bits in the array, and move the data shift places to the right.
 * 
 * @param a 
 * @param a_size 
 * @param shift 
 */
void right_shift_array(uint64_t *a, size_t a_size, uint64_t shift) {
    for (size_t i = 0; i < a_size - 1; i++) {
        a[i] >>= shift;
        uint64_t temp = (a[i + 1] << (64 - shift));
        a[i] |= temp;
    }
    a[a_size - 1] >>= shift;
}

void read_bits(const uint64_t *src, uint64_t *dest, size_t start, size_t end) {
    bool start_is_x8 = (start & 7 == 0);

    if (start_is_x8) {
        read_bits_easy_case(src, dest, start, end);
        return;
    }

    size_t new_start = (start / 8) * 8;
    assert(new_start < start);
    read_bits_easy_case(src, dest, new_start, end);
    
    uint64_t shift = (start & 7);
    
    // start or new_start?
    size_t dest_size = (end - new_start + 63) / 64;
    right_shift_array(dest, dest_size, shift);
}

bool test_bit(const uint64_t *a, size_t bit_index){
    constexpr uint64_t slot_size = sizeof(a[0]) * CHAR_BIT;
    const size_t word_index =  bit_index / slot_size;
    const size_t rel_index =  bit_index % slot_size;
    return a[word_index] & (1ULL << rel_index);
}
