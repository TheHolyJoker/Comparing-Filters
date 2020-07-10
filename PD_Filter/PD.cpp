//
// Created by tomer on 11/24/19.
//

#include "PD.h"

PD::PD(size_t m, size_t f, size_t l) : max_capacity(f), fp_size(l), size(get_a_size(m, f, l)),
                                       deal_with_joined_slot(should_deal_with_joined_slot(m, f, l)) {
    static_assert(CG_TYPE_SIZE == 32, "");

//    auto temp = posix_memalign((void**)&a, 32, size*sizeof(CG_TYPE));
//    assert(temp != 0);
    a = new CG_TYPE[size]();

    for (size_t i = 0; i < size; ++i) {
        a[i] = 0;
    }

}

auto PD::lookup(CG_TYPE q, CG_TYPE r) -> bool {
    size_t start_index = -1, end_index = -1;
    if (not header_lookup(q, &start_index, &end_index))
        return false;

    if (DB) assert(q <= start_index <= end_index);

    size_t unpacked_start_index = start_index - q;
    size_t unpacked_end_index = end_index - q;

    size_t p_array_index = -1, p_bit_index = -1;
    return body_find(r, unpacked_start_index, unpacked_end_index, &p_array_index, &p_bit_index);

}

void PD::insert(CG_TYPE q, CG_TYPE r) {
    if (DB) assert(get_capacity() < max_capacity);
    size_t start_index = -1, end_index = -1;
    header_insert(q, &start_index, &end_index);

    if (DB) assert((q <= start_index) and (start_index <= end_index));

    size_t unpacked_start_index = start_index - q;
    size_t unpacked_end_index = end_index - q;

    body_insert(r, unpacked_start_index, unpacked_end_index);
}

void PD::remove(CG_TYPE q, CG_TYPE r) {
    size_t start_index = -1, end_index = -1;
    header_remove(q, &start_index, &end_index);

    if (DB) assert(q <= start_index <= end_index);

    size_t unpacked_start_index = start_index - q;
    size_t unpacked_end_index = end_index - q;

//    size_t p_array_index = -1, p_bit_index = -1;
//    assert (body_find(r, unpacked_start_index, unpacked_end_index, &p_array_index, &p_bit_index));
    body_remove(r, unpacked_start_index, unpacked_end_index);
}

auto PD::conditional_remove(CG_TYPE q, CG_TYPE r) -> bool {
//    return naive_conditional_remove(q, r);
    size_t start_index = -1, end_index = -1;
    if (not header_lookup(q, &start_index, &end_index))
        return false;

    assert(q <= start_index);
    assert(start_index <= end_index);

    size_t unpacked_start_index = start_index - q;
    size_t unpacked_end_index = end_index - q;
    if (body_conditional_remove(r, unpacked_start_index, unpacked_end_index)) {
        header_pull(end_index);
        return true;
    }
    return false;
}

auto PD::naive_conditional_remove(CG_TYPE q, CG_TYPE r) -> bool {
    bool res = lookup(q, r);
    if (res) { remove(q, r); }
    return res;
}


////Header functions
auto PD::header_lookup(CG_TYPE q, size_t *start_index, size_t *end_index) -> bool {
    header_find(q, start_index, end_index);
    if (DB) assert(*start_index <= *end_index);
    return (*start_index != *end_index);
}

void PD::header_find(CG_TYPE q, size_t *start, size_t *end) {
    if (q == 0) {
        *start = 0;
        size_t j = 0;
        while (a[j] == MASK32) j++;
        *end = (j) * CG_TYPE_SIZE + __builtin_clz(~a[j]);
//        uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
//        *end_index = (j) * CG_TYPE_SIZE + select_r(~slot2, 1);
//        cout << "h0" << endl;
        return;
    }
    for (size_t i = 0; i <= get_last_a_index_containing_the_header(); ++i) {
        auto cz = __builtin_popcount(~a[i]);
        if (cz < q) q -= cz;
        else if (cz == q) {
            uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, q);
            if (DB) assert(bit_pos < CG_TYPE_SIZE);
            *start = (i + (bit_pos + 1 == CG_TYPE_SIZE)) * CG_TYPE_SIZE + (bit_pos + 1) % CG_TYPE_SIZE;
            size_t j = i + 1;
            while (a[j] == MASK32) j++;
            *end = (j) * CG_TYPE_SIZE + __builtin_clz(~a[j]);
//            uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
//            *end_index = (j) * CG_TYPE_SIZE + select_r(~slot2, 1);
//            cout << "h5" << endl;
            return;
            /*if (bit_pos == CG_TYPE_SIZE - 1) {
                *start = (i + 1) * CG_TYPE_SIZE;
                size_t j = i + 1;
                while (a[j] == MASK32) j++;
                uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
                *end_index = (j) * CG_TYPE_SIZE + bit_rank(~slot2, 1);
                cout << "h1" << endl;
            } else {
                *start = (i) * CG_TYPE_SIZE + bit_pos + 1;
                size_t j = i + 1;
                while (a[j] == MASK32) j++;
                uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
                *end_index = (j) * CG_TYPE_SIZE + select_r(~slot2, 1);
                cout << "h2" << endl;
            }
            return;*/
        } else {
            if (DB) assert(q < CG_TYPE_SIZE);
            uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, q);
            if (DB) assert(bit_pos < CG_TYPE_SIZE);
            *start = i * CG_TYPE_SIZE + select_r(~slot, q) + 1;
            *end = i * CG_TYPE_SIZE + select_r(~slot, q + 1);
//            cout << "h3" << endl;
            return;

        }
    }
    assert(false);

}

void PD::header_insert(CG_TYPE q, size_t *start_index, size_t *end_index) {
    size_t temp_index;
    CG_TYPE temp_slot;
    if (deal_with_joined_slot) {
        temp_index = get_joined_slot_index();
        temp_slot = a[temp_index];
    }

    header_find(q, start_index, end_index);
    header_push(*end_index);

    /*Restore body's part in the joined cell*/
    if (deal_with_joined_slot) {
        auto mask_bit = CG_TYPE_SIZE - get_header_bit_index();
        a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
    }

}

void PD::header_push(size_t end_index) {
    size_t index, last_header_a_slot = get_last_a_index_containing_the_header();
    index = end_index / CG_TYPE_SIZE;

    ulong to_shift = CG_TYPE_SIZE - 1ul;
    for (uint_fast16_t i = last_header_a_slot; i > index; --i) {
        a[i] = ((ulong) (a[i]) >> 1ul) | ((a[i - 1] & 1ul) << to_shift);
    }


    uint_fast16_t bit_index = end_index % CG_TYPE_SIZE;
    uint_fast16_t shift = CG_TYPE_SIZE - bit_index;
    ulong mask = MASK(shift);
    ulong upper = a[index] & (~mask);
    CG_TYPE lower = (a[index] >> 1ul) & mask;
    a[index] = (upper | lower | SL(shift - 1ul));

}

void PD::header_remove(CG_TYPE q, size_t *start_index, size_t *end_index) {
    size_t temp_index;
    CG_TYPE temp_slot;
    if (deal_with_joined_slot) {
        temp_index = get_joined_slot_index();
        temp_slot = a[temp_index];
    }

    header_find(q, start_index, end_index);
    if (DB) {
        if (*start_index == *end_index) {
            cout << "trying to remove element not in the PD. Specifically not in the header. " << endl;
            return;
        }
    }
    header_pull(*end_index);

    /*Restore body's part in the joined cell*/
    if (deal_with_joined_slot) {
        //Plus one to insure that body's first bit did not effect header last bit.
        //This bit (header last bit) should always be zero.
        auto mask_bit = CG_TYPE_SIZE - get_header_bit_index() + 1;
        a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
    }

}

void PD::header_pull(size_t end_index) {
    size_t index = (end_index - 1) / CG_TYPE_SIZE, last_header_a_slot = get_last_a_index_containing_the_header();

    if (index == last_header_a_slot) {
        uint_fast16_t bit_index = (end_index - 1) % CG_TYPE_SIZE;
        uint_fast16_t shift = CG_TYPE_SIZE - bit_index;
        ulong mask = MASK(shift);
        ulong upper = a[index] & (~mask);
        CG_TYPE mid = ((ulong) a[index] << 1ul) & mask;
        a[index] = upper | mid;
        return;
    }
    CG_TYPE lower = ((ulong) a[index + 1]) >> ((ulong) (CG_TYPE_SIZE - 1));

    for (size_t i = index + 1; i < last_header_a_slot; ++i) {
        a[i] = ((ulong) a[i] << 1ul) | ((ulong) a[i + 1] >> ((ulong) (CG_TYPE_SIZE - 1)));
    }
    a[last_header_a_slot] <<= 1ul;

    uint_fast16_t bit_index = (end_index - 1) % CG_TYPE_SIZE;
    uint_fast16_t shift = CG_TYPE_SIZE - bit_index;
    ulong mask = MASK(shift);
    ulong upper = a[index] & (~mask);
    CG_TYPE mid = ((ulong) a[index] << 1ul) & mask;
    a[index] = upper | mid | lower;// | SL(shift);

}

////Body functions
auto PD::body_find(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index, size_t *p_array_index,
                   size_t *p_bit_index) -> bool {
    size_t total_bit_counter = (unpacked_start_index * fp_size) + get_header_size_in_bits();
//    size_t total_bit_counter = abstract_body_start_index * fp_size;

    /*Empty case. */
    if (unpacked_start_index == unpacked_end_index) {
        *p_array_index = total_bit_counter / BODY_BLOCK_SIZE;
        *p_bit_index = total_bit_counter % BODY_BLOCK_SIZE;
//        *p_array_index = (unpacked_start_index * fp_size) / BODY_BLOCK_SIZE;
//        *p_bit_index = (unpacked_start_index * fp_size) % BODY_BLOCK_SIZE + get_header_size_in_bits();
        if (DB_PRINT) cout << "a1" << endl;
        return false;
    }

    size_t comparing_counter = 0, comparing_lim = unpacked_end_index - unpacked_start_index;

    for (; comparing_counter < comparing_lim; ++comparing_counter) {
        size_t B_index = total_bit_counter / BODY_BLOCK_SIZE;
        size_t bit_index_inside_slot = total_bit_counter % BODY_BLOCK_SIZE;
        size_t bits_left_to_read_inside_slot = BODY_BLOCK_SIZE - bit_index_inside_slot;

        BODY_BLOCK_TYPE current_cell = a[B_index];
        /*More than fp_size bits remain in B[B_index].*/
        if (bits_left_to_read_inside_slot > fp_size) {
            ulong shift = bits_left_to_read_inside_slot - fp_size;
            if (DB) assert(shift < BODY_BLOCK_SIZE);
            BODY_BLOCK_TYPE current_remainder = (current_cell >> (shift)) & MASK(fp_size);
            if (r <= current_remainder) {
                if (DB_PRINT) cout << "a2" << endl;
                body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                return r == current_remainder;
            }
            /*Exactly fp_size bits remain in B[B_index].*/
        } else if (bits_left_to_read_inside_slot == fp_size) {
            BODY_BLOCK_TYPE current_remainder = current_cell & MASK(fp_size);
            if (r <= current_remainder) {
                if (DB_PRINT) cout << "a3" << endl;
                body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                return r == current_remainder;
            }
            /*Less than fp_size bits remain in B[B_index].*/
        } else {
            size_t number_of_bits_to_read_from_next_slot = fp_size - bits_left_to_read_inside_slot;
            ulong upper_shift = fp_size - bits_left_to_read_inside_slot;
            if (DB) assert(upper_shift >= 0 and upper_shift < BODY_BLOCK_SIZE);
            ulong upper = (current_cell & MASK(bits_left_to_read_inside_slot)) << (upper_shift);
            if (DB) assert(size > B_index + 1);
            ulong lower_shift = BODY_BLOCK_SIZE - number_of_bits_to_read_from_next_slot;
            if (DB) assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
            ulong lower = (a[B_index + 1] >> lower_shift) & MASK(number_of_bits_to_read_from_next_slot);
            BODY_BLOCK_TYPE current_remainder = upper | lower;
            if (r <= current_remainder) {
                if (DB_PRINT) cout << "a4" << endl;
                body_find_helper(B_index, bits_left_to_read_inside_slot, p_array_index, p_bit_index);
                return r == current_remainder;
            }
        }
        total_bit_counter += fp_size;
    }
    *p_array_index = total_bit_counter / BODY_BLOCK_SIZE;
    *p_bit_index = total_bit_counter % BODY_BLOCK_SIZE;
    if (DB_PRINT) cout << "a5" << endl;
    return false;


}

void PD::body_insert(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) {
    /*size_t temp_index;
    CG_TYPE temp_slot;
    if (deal_with_joined_slot) {
        temp_index = get_joined_slot_index();
        temp_slot = a[temp_index];
    }*/

    size_t B_index = -1, bit_index = -1;
    auto res = body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
    body_push(r, B_index, bit_index);

    /*if (deal_with_joined_slot) {
        auto mask_bit = CG_TYPE_SIZE - get_header_bit_index();
        a[temp_index] = (a[temp_index] & (~MASK(mask_bit))) | (temp_slot & MASK(mask_bit));
    }*/
}

void PD::body_push(CG_TYPE r, size_t B_index, size_t bit_index) {
//    size_t B_index = -1, bit_index = -1;
//    auto res = body_find(unpacked_start_index, unpacked_end_index, r, &B_index, &bit_index);
    size_t left_bit_index = BODY_BLOCK_SIZE - bit_index;
    for (size_t i = size - 1; i > B_index; --i) {
        a[i] = (a[i] >> fp_size) | ((a[i - 1]) << (BODY_BLOCK_SIZE - fp_size));
    }

    if (BODY_BLOCK_SIZE >= fp_size + bit_index) {
        ulong mask = MASK(left_bit_index);
        ulong upper = a[B_index] & (~mask);
        BODY_BLOCK_TYPE mid = r << (left_bit_index - fp_size);
        BODY_BLOCK_TYPE lower = (a[B_index] >> fp_size) & (MASK(left_bit_index - fp_size));
        a[B_index] = (upper | lower | mid);
        if (DB) assert(left_bit_index - fp_size >= 0);
    } else { // Prevents negative shifting.
        size_t shift = left_bit_index;
        ulong mask = MASK(left_bit_index);
        ulong upper = a[B_index] & (~mask);
        BODY_BLOCK_TYPE lower = r >> (fp_size - left_bit_index);
        a[B_index] = (upper | lower);

        if (DB) assert(fp_size - shift > 0);

        //same amount that r was shifted right by. (fp_size - shift)
        size_t bits_left = fp_size - shift; // bits_left =  fp_size + bit_index - BODY_BLOCK_SIZE
        upper = (r & MASK(bits_left)) << (BODY_BLOCK_SIZE - bits_left); // todo Check this.
        lower = (a[B_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits. todo try mask instead.
        a[B_index + 1] = (upper | lower);
    }
//    capacity++;
}

void PD::body_remove(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) {
    size_t temp_index;
    CG_TYPE temp_slot;
    if (deal_with_joined_slot) {
        temp_index = get_joined_slot_index();
        temp_slot = a[temp_index];
    }

    size_t B_index = -1, bit_index = -1;
    auto res = body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
    if (not res) {
        body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
        cout << "trying to remove element not in the PD. "
                "Specifically not in the Body (might be in the header). " << endl;
        return;
    }
    body_pull(B_index, bit_index);

    if (deal_with_joined_slot) {
        auto mask_bit = CG_TYPE_SIZE - get_header_bit_index();
        a[temp_index] = (a[temp_index] & (MASK(mask_bit))) | (temp_slot & (~MASK(mask_bit)));
    }
}

auto PD::body_conditional_remove(CG_TYPE r, size_t unpacked_start_index, size_t unpacked_end_index) -> bool {
    size_t temp_index;
    CG_TYPE temp_slot;
    if (deal_with_joined_slot) {
        temp_index = get_joined_slot_index();
        temp_slot = a[temp_index];
    }

    size_t B_index = -1, bit_index = -1;
    auto res = body_find(r, unpacked_start_index, unpacked_end_index, &B_index, &bit_index);
    if (not res) {
        return false;
    }
    body_pull(B_index, bit_index);

    if (deal_with_joined_slot) {
        auto mask_bit = CG_TYPE_SIZE - get_header_bit_index();
        a[temp_index] = (a[temp_index] & (MASK(mask_bit))) | (temp_slot & (~MASK(mask_bit)));
    }
    return true;
}

void PD::body_pull(size_t B_index, size_t bit_index) {
    size_t left_fp_start_index = BODY_BLOCK_SIZE - bit_index;

    if (B_index == size - 1) {
        assert(bit_index + fp_size <= BODY_BLOCK_SIZE);

//        ulong shift = left_fp_start_index;
        if (DB) assert(left_fp_start_index >= 0);
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[B_index] & (~mask);
        BODY_BLOCK_TYPE mid = (a[B_index] & MASK(left_fp_start_index - fp_size)) << fp_size;
        if (DB) assert(left_fp_start_index >= fp_size);
        a[B_index] = (upper | mid);
        return;
    }

    if (BODY_BLOCK_SIZE >= fp_size + bit_index) {
        ulong shift = left_fp_start_index;
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[B_index] & (~mask);
        BODY_BLOCK_TYPE mid = (a[B_index] & MASK(left_fp_start_index - fp_size)) << fp_size;
        BODY_BLOCK_TYPE lower = (a[B_index + 1]) >> (BODY_BLOCK_SIZE - fp_size);
        a[B_index] = (upper | lower | mid);

        if (DB) assert(shift >= fp_size);
        if (DB) assert(BODY_BLOCK_SIZE - fp_size >= 0);

        for (size_t i = B_index + 1; i < size - 1; ++i) {
            a[i] = (a[i] << fp_size) | (a[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
        }
        a[size - 1] <<= fp_size;

    } else {
        ulong shift = left_fp_start_index;
        ulong mask = MASK(left_fp_start_index);
        ulong upper = a[B_index] & (~mask);
        ulong lower_shift = BODY_BLOCK_SIZE - fp_size;
        BODY_BLOCK_TYPE lower = a[B_index + 1] >> lower_shift;
        lower &= MASK(left_fp_start_index);
        a[B_index] = upper | lower;

        for (size_t i = B_index + 1; i < size - 1; ++i) {
            a[i] = (a[i] << fp_size) | (a[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
        }
        a[size - 1] <<= fp_size;

        if (DB) assert(0 <= shift and shift < fp_size);
        if (DB) assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
    }

}

auto PD::get_last_a_index_containing_the_header() -> size_t {
    return (max_capacity << 1u) / CG_TYPE_SIZE - int(!deal_with_joined_slot);
//    return INTEGER_ROUND(temp, CG_TYPE_SIZE);
}

auto PD::get_header_bit_index() -> size_t {
    auto temp = (max_capacity << 1u);
    return temp % CG_TYPE_SIZE;
}

auto PD::get_joined_slot_index() -> size_t {
    return get_last_a_index_containing_the_header();
}

//size_t PD::get_header_size_in_bits() {
//    return max_capacity << 1u;
//}

auto get_a_size(size_t m, size_t f, size_t l) -> size_t {
    return INTEGER_ROUND(((l + 2) * f), (CG_TYPE_SIZE));
}

size_t PD::get_number_of_bits_in_a(size_t m, size_t f, size_t l) {
    return (l + 2) * f;
}

auto PD::get_a() const -> uint32_t * {
    return a;
}

auto PD::get_fp_size() const -> const uint_fast16_t {
    return fp_size;
}

auto PD::get_max_capacity() const -> const uint_fast16_t {
    return max_capacity;
}

auto PD::get_size() const -> const uint_fast16_t {
    return size;
}

auto PD::get_deal_with_joined_slot() const -> const bool {
    return deal_with_joined_slot;
}
/*
void PD::print_as_array() const {
    print_array_as_integers<uint32_t>(a, size);
}

void PD::print_as_consecutive_memory() {
    print_array_as_consecutive_memory<CG_TYPE>(a, size, cout);
}

auto operator<<(ostream &os, const PD &pd) -> ostream & {
    print_array_as_consecutive_memory<CG_TYPE>(pd.a, pd.size, cout);
    return os;
}
*/

auto PD::get_capacity() -> size_t {
//    size_t total_bits = get_header_size_in_bits();
    size_t last_index = get_last_a_index_containing_the_header();
    size_t bit_index = get_header_bit_index();
    size_t res = 0;
    for (size_t i = 0; i < last_index; ++i) {
        res += __builtin_popcount(a[i]);
    }

    if (bit_index == 0)
        return res + __builtin_popcount(a[last_index]);
    auto temp = a[last_index] & (~MASK(CG_TYPE_SIZE - bit_index));
    return res + __builtin_popcount(temp);
}

auto PD::is_full() -> bool {
    return get_capacity() == max_capacity;
}


PD::~PD() {
    delete[] a;
}


uint32_t select_r(uint64_t slot, uint32_t rank) {
//    uint64_t slot;          // Input value to find position with rank rank.
//    unsigned int rank;      // Input: bit's desired rank [1-64].
    unsigned int s;      // Output: Resulting position of bit with the desired rank.[1-64]
    uint64_t a, b, c, d; // Intermediate temporaries for bit count.
    unsigned int t;      // Bit count temporary.

    // Do a normal parallel bit count for a 64-bit integer,
    // but store all intermediate steps.
    // a = (slot & 0x5555...) + ((slot >> 1) & 0x5555...);
    a = slot - ((slot >> 1) & ~0UL / 3);
    // b = (a & 0x3333...) + ((a >> 2) & 0x3333...);
    b = (a & ~0UL / 5) + ((a >> 2) & ~0UL / 5);
    // c = (b & 0x0f0f...) + ((b >> 4) & 0x0f0f...);
    c = (b + (b >> 4)) & ~0UL / 0x11;
    // d = (c & 0x00ff...) + ((c >> 8) & 0x00ff...);
    d = (c + (c >> 8)) & ~0UL / 0x101;
    t = (d >> 32) + (d >> 48);
    // Now do branchless select!
    s = 64;
    // if (rank > t) {s -= 32; rank -= t;}
    s -= ((t - rank) & 256) >> 3;
    rank -= (t & ((t - rank) >> 8));
    t = (d >> (s - 16)) & 0xff;
    // if (rank > t) {s -= 16; rank -= t;}
    s -= ((t - rank) & 256) >> 4;
    rank -= (t & ((t - rank) >> 8));
    t = (c >> (s - 8)) & 0xf;
    // if (rank > t) {s -= 8; rank -= t;}
    s -= ((t - rank) & 256) >> 5;
    rank -= (t & ((t - rank) >> 8));
    t = (b >> (s - 4)) & 0x7;
    // if (rank > t) {s -= 4; rank -= t;}
    s -= ((t - rank) & 256) >> 6;
    rank -= (t & ((t - rank) >> 8));
    t = (a >> (s - 2)) & 0x3;
    // if (rank > t) {s -= 2; rank -= t;}
    s -= ((t - rank) & 256) >> 7;
    rank -= (t & ((t - rank) >> 8));
    t = (slot >> (s - 1)) & 0x1;
    // if (rank > t) s--;
    s -= ((t - rank) & 256) >> 8;
    //current res between [0,63]
    return 64 - s;
}

