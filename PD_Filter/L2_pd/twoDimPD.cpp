
#include "twoDimPD.hpp"


int compare_vector_and_tpd(b_vec *v, const uint64_t *arr, size_t bits_to_compare) {
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
            bool res = word & b;
            bool v_res = v->at(v_index_offset + j);
            if (res != v_res) {
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
            assert(((int )i - one_count) == zero_count);
            if (one_count == (int )k) {
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
    assert(false);
    return -1;
}

auto count_zeros_up_to_the_kth_one(u128 word, size_t k) -> size_t {
    int one_count = -1;
    int zero_count = 0;
    u128 b = u128(1);
    for (size_t i = 0; i < 128; i++) {
        if (b & word) {
            one_count++;
            assert((int)(i - one_count) == zero_count);
            if (one_count == ((int) k)) {
                // auto res = i - k;
                // assert(res == zero_count);
                return i - k;
            }
        } else {
            zero_count++;
        }
        b <<= 1ull;
    }
    std::cout << one_count << std::endl;
    std::cout << zero_count << std::endl;
    assert(false);
    return -1;
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
            assert((int )(i - zero_count) == one_count);
            if (zero_count == ((int )k)) {
                // auto res = i - k;
                // assert(res == zero_count);
                return i - k;
            }
        }
        b <<= 1ul;
    }
    std::cout << one_count << std::endl;
    std::cout << zero_count << std::endl;
    assert(false);
    return -1;
}


auto count_zeros_up_to_the_kth_one(const uint64_t *pd, size_t k) -> size_t {
    uint64_t pop1 = _mm_popcnt_u64(pd[0]);
    if (pop1 > k) {
        return count_zeros_up_to_the_kth_one(pd[0], k);
    }
    uint64_t pop2 = _mm_popcnt_u64(pd[1]);
    assert(pop1 + pop2 > k);// starting from zero.
    auto w1_zeros = 64 - pop1;
    auto w2_zeros = count_zeros_up_to_the_kth_one(pd[1], k - pop1);
    return w1_zeros + w2_zeros;
}

auto count_ones_up_to_the_kth_zero(const uint64_t *pd, size_t k) -> size_t {
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

bool test_bit(const uint64_t *a, size_t bit_index) {
    constexpr uint64_t slot_size = sizeof(a[0]) * CHAR_BIT;
    const size_t word_index = bit_index / slot_size;
    const size_t rel_index = bit_index % slot_size;
    return a[word_index] & (1ULL << rel_index);
}

namespace packing_helper {

//    template<typename T>
    void unpack_array(uint64_t *unpacked_array, size_t unpack_size, const uint64_t *pack_array, size_t pack_size, size_t el_length) {
        static constexpr unsigned slot_size = sizeof(uint64_t) * CHAR_BIT;
        uint64_t word_capacity = slot_size / el_length;
        // uint64_t words_num = pack_size;
        assert((unpack_size % word_capacity) == 0);

        uint64_t mask = MASK(el_length);

        // std::cout << std::string(80, '*') << std::endl;
        // std::cout << "Before:  " << "\t\t";
        // print_memory::print_array(unpacked_array, unpack_size);

        for (size_t i = 0; i < pack_size; i++) {
            uint64_t temp = pack_array[i];
            for (size_t j = 0; j < word_capacity; j++) {
                size_t index = i * word_capacity + j;
                unpacked_array[index] = temp & mask;
                // std::cout << "index:\t" << index << "\t\t";
                // print_memory::print_array(unpacked_array, unpack_size);
                temp >>= el_length;
            }
        }
        // std::cout << "After:   " << "\t\t";
        // print_memory::print_array(unpacked_array, unpack_size);
        // std::cout << std::string(80, '*') << std::endl;
    }

    void pack_array(const uint64_t *unpacked_array, size_t unpack_size, uint64_t *pack_array, size_t pack_size, size_t el_length) {
        uint64_t slot_size = sizeof(uint64_t) * CHAR_BIT;
        uint64_t quots_in_word = slot_size / el_length;
        unsigned words_num = unpack_size / quots_in_word;
        assert(words_num == pack_size);
        assert((unpack_size % quots_in_word) == 0);

        for (size_t i = 0; i < words_num; i++) {
            //            uint64_t temp_mask = MASK(el_length);
            for (size_t j = 0; j < quots_in_word; j++) {
                pack_array[i] |= (unpacked_array[i * quots_in_word + j] << (el_length * j));
                //                temp_mask <<= el_length;
            }
        }
    }

    bool validate_pack_unpack(uint64_t *unpacked_array, size_t unpack_size, size_t el_length) {
        uint64_t slot_size = sizeof(uint64_t) * CHAR_BIT;
        uint64_t quots_in_word = slot_size / el_length;
        size_t pack_size = quots_in_word;
        uint64_t temp_pack_array[pack_size] = {0};
        uint64_t temp_unpack_array[unpack_size] = {0};
        pack_array(unpacked_array, unpack_size, temp_pack_array, pack_size, el_length);
        unpack_array(temp_unpack_array, unpack_size, temp_pack_array, pack_size, el_length);

        bool temp = cmp_array(temp_unpack_array, unpacked_array, unpack_size);
        assert(temp);
        return true;
    }

    bool validate_unpack_pack(uint64_t *packed_array,const size_t packed_size, size_t el_length) {
        constexpr uint64_t slot_size = sizeof(uint64_t) * CHAR_BIT;
        const uint64_t quots_in_word = slot_size / el_length;
        const size_t unpack_size = quots_in_word * packed_size;
        uint64_t temp_unpack_array[unpack_size] = {0};
        uint64_t temp_packed_array[packed_size] = {0};
        // std::cout << std::string(80, '=') << std::endl;
        // std::cout << "packed a:       \t";
        unpack_array(temp_unpack_array, unpack_size, packed_array, packed_size, el_length);
        // print_memory::print_array(packed_array, packed_size);
        // std::cout << "temp_unpacked a:\t";

        pack_array(temp_unpack_array, unpack_size, temp_packed_array, packed_size, el_length);
        // print_memory::print_array(temp_unpack_array, packed_size);
        // std::cout << "temp_packed a:  \t";

        // print_memory::print_array(temp_packed_array, packed_size);
        // std::cout << "packed a:       \t";

        // print_memory::print_array(packed_array, packed_size);
        // std::cout << std::string(80, '=') << std::endl;

        bool temp = cmp_array(temp_packed_array, packed_array, packed_size);
        if (!temp) {
            std::cout << std::string(80, '~') << std::endl;
            print_memory::print_array(packed_array, packed_size);
            print_memory::print_array(temp_packed_array, packed_size);
            std::cout << std::string(80, '~') << std::endl;
            // print_memory::print_array_LE(packed_array, packed_size, el_length);
            // print_memory::print_array_LE(temp_packed_array, packed_size, el_length);
            std::cout << std::string(80, '~') << std::endl;
            assert(0);
        }
        assert(temp);
        return true;
    }
}// namespace packing_helper


// namespace bits_memcpy {
//     template<>
//     uint64_t my_cmp_epu<4, 3>(uint64_t rem, const uint64_t *a) {
//         constexpr unsigned length = 4ull;
//         constexpr uint64_t mask = 15ull;
//         uint64_t b = 1ULL;
//         uint64_t res = 0;
//         for (size_t i = 0; i < 3; i++) {
//             uint64_t temp = a[i];
//             if ((((temp >> (length * 0ul)) & mask) == rem))
//                 res |= (1ull << 0);
//             res |=  << 0ul;
//             res |= (((temp >> (length * 1ul)) & mask) == rem) << 1ul;
//             res |= (((temp >> (length * 2ul)) & mask) == rem) << 2ul;
//             res |= (((temp >> (length * 3ul)) & mask) == rem) << 3ul;
//             res |= (((temp >> (length * 4ul)) & mask) == rem) << 4ul;
//             res |= (((temp >> (length * 5ul)) & mask) == rem) << 5ul;
//             res |= (((temp >> (length * 6ul)) & mask) == rem) << 6ul;
//             res |= (((temp >> (length * 7ul)) & mask) == rem) << 7ul;
//             res |= (((temp >> (length * 8ul)) & mask) == rem) << 8ul;
//             res |= (((temp >> (length * 9ul)) & mask) == rem) << 9ul;
//             res |= (((temp >> (length * 10ul)) & mask) == rem) << 10ul;
//             res |= (((temp >> (length * 11ul)) & mask) == rem) << 11ul;
//             res |= (((temp >> (length * 12ul)) & mask) == rem) << 12ul;
//             res |= (((temp >> (length * 13ul)) & mask) == rem) << 13ul;
//             res |= (((temp >> (length * 14ul)) & mask) == rem) << 14ul;
//             res |= (((temp >> (length * 15ul)) & mask) == rem) << 15ul;
//         }
//         return res;
//     }

// }// namespace bits_memcpy
// uint64_t get_select_mask_naive(uint64_t x, int64_t j){
//     assert(j >= 0);
//     assert(_mm_popcnt_u64(x) > j);
//     uint64_t second_set_bit_index = select64(x, j);
//     uint64_t second_set_bit_index = select64(x, j - 1);
//     // return _pdep_u64(3ul << (j), x);
// }


// template<>
// auto str_word_LE(u128 word, size_t gap) -> std::string{
//     constexpr unsigned slot_size = 128;
//     u128 b = 1ULL;
//     std::string res = "";
//     for (size_t i = 0; i < slot_size; i++) {
//         if ((i > 0) && (is_divisible(i, gap * gap))) {
//             res += "|";
//         } else if ((i > 0) && (is_divisible(i, gap))) {
//             res += ".";
//         }

//         res += (word & b) ? "1" : "0";
//         b <<= 1ul;
//     }
//     return res;
// }

// template<>
// void print_word_LE(u128 word, size_t gap = 4) {
//     std::cout << str_word_LE(word, gap) << std::endl;
// }

/* Old attempt. 
void memcpy_bits_naive(uint64_t *src, uint64_t *dest, size_t a, size_t b, size_t x, size_t y);

void memcpy_bits_naive_with_vectors(uint64_t *src, uint64_t *dest, size_t a, size_t b, size_t x, size_t y);

void memcpy_bits_naive(uint64_t *src, uint64_t *dest, size_t a, size_t b, size_t x, size_t y) {
    size_t length = b - a;
    assert(length == y - x);
    constexpr unsigned slot_size = 64;
    size_t align_dest_size = (b - a + slot_size - 1) / slot_size;
    uint64_t align_dest[align_dest_size] = {0};
    memcpy_bits(src, align_dest, a, b);

    size_t new_a = ((a + slot_size - 1) / slot_size) * slot_size;
    size_t new_x = ((x + slot_size - 1) / slot_size) * slot_size;
    assert((new_a % slot_size) == 0);
    assert((new_x % slot_size) == 0);

    size_t src_offset = a % slot_size;
    size_t dest_offset = x % slot_size;

    size_t diff_a = slot_size - (a % slot_size);
    size_t diff_x = slot_size - (x % slot_size);
    size_t diff = (diff_a <= diff_x) ? diff_x : diff_a;

    size_t new_length = length - diff;
    size_t bytes2Copy = new_length / CHAR_BIT;


    assert((new_a % slot_size) == 0);
    assert((new_x % slot_size) == 0);
    uint8_t *src_start = ((uint8_t *) src) + (new_a / CHAR_BIT);
    uint8_t *dest_start = ((uint8_t *) dest) + (new_x / CHAR_BIT);
    memcpy(dest_start, src_start, bytes2Copy);


    uint8_t *src_8_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);
    size_t word_i = a / slot_size;
    size_t rel_i = a & slot_size;

    size_t word_j = x / slot_size;
    size_t rel_j = x & slot_size;
}


void memcpy_bits_easy_case(const uint64_t *src, uint64_t *dest, size_t start, size_t end) {
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

void right_shift_array(uint64_t *a, size_t a_size, uint64_t shift) {
    for (size_t i = 0; i < a_size - 1; i++) {
        a[i] >>= shift;
        uint64_t temp = (a[i + 1] << (64 - shift));
        a[i] |= temp;
    }
    a[a_size - 1] >>= shift;
}

void memcpy_bits_dest_is_word_aligned(const uint64_t *src, uint64_t *dest, size_t start, size_t end) {
    bool start_is_x8 = (start & 7 == 0);

    if (start_is_x8) {
        memcpy_bits_easy_case(src, dest, start, end);
        return;
    }

    size_t new_start = (start / 8) * 8;
    assert(new_start < start);
    memcpy_bits_easy_case(src, dest, new_start, end);

    uint64_t shift = (start & 7);

    // start or new_start?
    size_t dest_size = (end - new_start + 63) / 64;
    right_shift_array(dest, dest_size, shift);
}

void memcpy_bits_dest_is_byte_aligned(const uint64_t *src, uint8_t *dest, size_t start, size_t length){
    bool start_is_x8 = (start & 7 == 0);

    if (start_is_x8) {
        memcpy_bits_easy_case(src, dest, start, end);
        return;
    }

    size_t new_start = (start / 8) * 8;
    assert(new_start < start);
    memcpy_bits_easy_case(src, dest, new_start, end);

    uint64_t shift = (start & 7);

    // start or new_start?
    size_t dest_size = (end - new_start + 63) / 64;
    right_shift_array(dest, dest_size, shift);
}


void memcpy_bits_dest_is_byte_aligned(const uint64_t *src, uint64_t *dest, size_t src_start, size_t dest_start, size_t length) {
    assert((dest_start & 7) == 0);

    uint8_t *dest_start_pointer = ((uint8_t *) dest) + (dest_start / CHAR_BIT);



    memcpy_bits_dest_is_word_aligned()
    
    size_t new_start = (start / 8) * 8;
    assert(new_start < start);
    memcpy_bits_easy_case(src, dest, new_start, end);

    uint64_t shift = (start & 7);

    // start or new_start?
    size_t dest_size = (end - new_start + 63) / 64;
    right_shift_array(dest, dest_size, shift);
}

void memcpy_bits(uint64_t *src, uint64_t *dest, size_t src_start, size_t dest_start, size_t length) { 
    if ((dest_start & 7) == 0)
    {

    }
    size_t dest_byte_off_set = dest_start & 7;
    size_t start_diff = 8 - dest_byte_off_set;

    
    bool src_start_is_x8 = (src_start & 7 == 0);
    bool dest_start_is_x8 = (dest_start & 7 == 0);

    assert(src_start_is_x8 && dest_start_is_x8);

    uint8_t *src_8_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);
    uint8_t *src_8_pointer = ((uint8_t *) src) + (src_start / CHAR_BIT);
}
*/