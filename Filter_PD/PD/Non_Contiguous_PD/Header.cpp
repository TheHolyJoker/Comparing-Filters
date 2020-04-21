//
// Created by tomer on 10/24/19.
//

#include "Header.h"

Header::Header(size_t m, size_t f, size_t l) : capacity(0), max_capacity(f), size(get_initialize_size(m, f)) {
    if (HEADER_BLOCK_SIZE != (8 * sizeof(HEADER_BLOCK_TYPE))) {
        assert(false);
    }
    H = new HEADER_BLOCK_TYPE[size]();
}

Header::Header(size_t f) : capacity(0), max_capacity(f), size(get_initialize_size(f, f)) {
    size_t number_of_bits = f << 1ul;
    if (HEADER_BLOCK_SIZE != (8 * sizeof(HEADER_BLOCK_TYPE))) {
        assert(false);
    }
    H = new HEADER_BLOCK_TYPE[size]();
}

//Header::~Header() {
//    delete[] H;
//}

bool Header::lookup(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    this->get_quotient_start_and_end_index(quotient, start_index, end_index);
    return *start_index < *end_index;
}

void Header::insert(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    if (DB) assert(capacity < max_capacity);
    //Todo: no need to find the start_index, only the end_index.
    get_quotient_start_and_end_index(quotient, start_index, end_index);
    push(quotient, *start_index, *end_index);

    capacity++;
}

void Header::remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    get_quotient_start_and_end_index(quotient, start_index, end_index);
    if (DB) assert (*end_index > *start_index); //deleting from empty interval.
    pull(quotient, *start_index, *end_index);
    capacity--;
}


void Header::push(uint_fast16_t quotient, size_t start, size_t end) {
    size_t index;
    index = end / HEADER_BLOCK_SIZE;

    for (uint_fast16_t i = size - 1; i > index; --i) {
        this->H[i] = ((ulong) (H[i]) >> 1ul) | (H[i - 1] & 1ul) << ((ulong) (HEADER_BLOCK_SIZE - 1));
    }

    uint_fast16_t bit_index = end % HEADER_BLOCK_SIZE;
    uint_fast16_t shift = HEADER_BLOCK_SIZE - bit_index;
    ulong mask = MASK(shift);
    ulong upper = H[index] & (~mask);
    HEADER_BLOCK_TYPE lower = ((ulong) H[index] >> 1ul) & mask;
    this->H[index] = (upper | lower | SL(shift - 1ul));

}

void Header::pull(uint_fast16_t quotient, size_t start, size_t end) {
    size_t index = (end - 1) / HEADER_BLOCK_SIZE;
    if (index == size - 1) {
        uint_fast16_t bit_index = (end - 1) % HEADER_BLOCK_SIZE;
        uint_fast16_t shift = HEADER_BLOCK_SIZE - bit_index;
        ulong mask = MASK(shift);
        ulong upper = H[index] & (~mask);
        HEADER_BLOCK_TYPE mid = ((ulong) H[index] << 1ul) & mask;
        this->H[index] = upper | mid;
        return;
    }
    HEADER_BLOCK_TYPE lower = ((ulong) H[index + 1]) >> ((ulong) (HEADER_BLOCK_SIZE - 1));

    for (size_t i = index + 1; i < this->size - 1; ++i) {
        this->H[i] = ((ulong) H[i] << 1ul) | ((ulong) H[i + 1] >> ((ulong) (HEADER_BLOCK_SIZE - 1)));
    }
    this->H[this->size - 1] <<= 1ul;

    uint_fast16_t bit_index = (end - 1) % HEADER_BLOCK_SIZE;
    uint_fast16_t shift = HEADER_BLOCK_SIZE - bit_index;
    ulong mask = MASK(shift);
    ulong upper = H[index] & (~mask);
    HEADER_BLOCK_TYPE mid = ((ulong) H[index] << 1ul) & mask;
    H[index] = upper | mid | lower;// | SL(shift);
}

/**
 * Used as a wrapper here, for different implementation of get_interval function.
 * @param quotient
 * @param start_index
 * @param end_index
 */
void Header::get_quotient_start_and_end_index(size_t quotient, size_t *start_index, size_t *end_index) {
//    get_interval_attempt(H, size, quotient, start_index, end_index);
    get_interval_by_rank(H, size, quotient, start_index, end_index);
//    get_interval_by_rank2(H, size, quotient, start_index, end_index);
//    validate_get_interval(quotient);

}

void
get_interval_attempt(const HEADER_BLOCK_TYPE *a, size_t a_size, size_t quotient, size_t *start_index,
                     size_t *end_index) {
    size_t zero_counter = -1, continue_from_a_index = 0, continue_from_bit_index = -1;
    bool to_break = false;
    for (size_t i = 0; i < a_size; ++i) {
        ulong b = 1ULL << (ulong) (HEADER_BLOCK_SIZE - 1);
        for (int j = 0; j < HEADER_BLOCK_SIZE; ++j) {
            if (DB) assert(b > 0);
            if (zero_counter == quotient - 1) {
                *start_index = i * HEADER_BLOCK_SIZE + j;
                continue_from_a_index = i;
                continue_from_bit_index = j;
                to_break = true;
                break;
            }
            if (not(a[i] & b))
                zero_counter++;

            b >>= 1ul;
        }
        if (to_break) break;
    }
    if (DB) assert(continue_from_bit_index >= 0);

    size_t j = continue_from_bit_index;
    ulong b = 1ULL << (ulong) (HEADER_BLOCK_SIZE - 1 - j);
    for (size_t i = continue_from_a_index; i < a_size; ++i) {
        for (; j < HEADER_BLOCK_SIZE; ++j) {
            if (not(a[i] & b)) {
                *end_index = i * HEADER_BLOCK_SIZE + j;
                return;
            }
            b >>= 1ul;
        }
        b = 1ULL << (ulong) (HEADER_BLOCK_SIZE - 1);
        j = 0;
    }
    assert(false);
}

void get_interval_by_rank(const HEADER_BLOCK_TYPE *a, size_t a_size, size_t quotient, size_t *start_index,
                          size_t *end_index) {
    if (quotient == 0) {
        *start_index = 0;
        size_t j = 0;
        while (a[j] == MASK32) j++;
        *end_index = (j) * HEADER_BLOCK_SIZE + __builtin_clz(~a[j]);
//        uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
//        *end_index = (j) * HEADER_BLOCK_SIZE + select_r(~slot2, 1);
//        cout << "h0" << endl;
        return;
    }
    for (size_t i = 0; i < a_size; ++i) {
        auto cz = __builtin_popcount(~a[i]);
        if (cz < quotient) quotient -= cz;
        else if (cz == quotient) {
            uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, quotient);
            if (DB) assert(bit_pos < HEADER_BLOCK_SIZE);
            *start_index =
                    (i + (bit_pos + 1 == HEADER_BLOCK_SIZE)) * HEADER_BLOCK_SIZE + (bit_pos + 1) % HEADER_BLOCK_SIZE;
            size_t j = i + 1;
            while (a[j] == MASK32) j++;
            *end_index = (j) * HEADER_BLOCK_SIZE + __builtin_clz(~a[j]);
//            uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
//            *end_index = (j) * HEADER_BLOCK_SIZE + select_r(~slot2, 1);
//            cout << "h5" << endl;
            return;
            /*if (bit_pos == HEADER_BLOCK_SIZE - 1) {
                *start_index = (i + 1) * HEADER_BLOCK_SIZE;
                size_t j = i + 1;
                while (a[j] == MASK32) j++;
                uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
                *end_index = (j) * HEADER_BLOCK_SIZE + bit_rank(~slot2, 1);
                cout << "h1" << endl;
            } else {
                *start_index = (i) * HEADER_BLOCK_SIZE + bit_pos + 1;
                size_t j = i + 1;
                while (a[j] == MASK32) j++;
                uint64_t slot2 = ((ulong) (a[j]) << 32ul) | 4294967295ul;
                *end_index = (j) * HEADER_BLOCK_SIZE + select_r(~slot2, 1);
                cout << "h2" << endl;
            }
            return;*/
        } else {
            if (DB) assert(quotient < HEADER_BLOCK_SIZE);
            uint64_t slot = ((ulong) (a[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, quotient);
            if (DB) assert(bit_pos < HEADER_BLOCK_SIZE);
            *start_index = i * HEADER_BLOCK_SIZE + select_r(~slot, quotient) + 1;
            *end_index = i * HEADER_BLOCK_SIZE + select_r(~slot, quotient + 1);
//            cout << "h3" << endl;
            return;

        }
    }
    assert(false);
}

void get_interval_by_rank2(const HEADER_BLOCK_TYPE *a, size_t a_size, size_t quotient, size_t *start_index,
                           size_t *end_index) {
    if (quotient == 0) {
        *start_index = 0;
//        *end_index = select_r(~slot, 1);
        *end_index = __builtin_clz(~a[0]);
        if (*end_index == 32) *end_index += __builtin_clz(~a[1]);
        return;
    }
    uint64_t slot = ((ulong) (a[0]) << 32ul) | a[1];
    *start_index = select_r(~slot, quotient) + 1;
    if (*start_index == 64) {
        *end_index = 64;
        return;
    }
//    size_t i = *start_index / HEADER_BLOCK_SIZE, mask_bit = HEADER_BLOCK_SIZE - (*start_index % HEADER_BLOCK_SIZE);
//    *end_index = __builtin_clz(~a[i] & MASK(mask_bit));
    *end_index = select_r(~slot, quotient + 1);

}


bool validate_get_interval_functions(const HEADER_BLOCK_TYPE *arr, size_t a_size, size_t quotient) {
    size_t va = -1, vb = -1, c = -1, d = -1;
    get_interval_attempt(arr, a_size, quotient, &c, &d);
    get_interval_by_rank(arr, a_size, quotient, &va, &vb);
    bool cond2 = (c == va) && (d == vb);
    if (not cond2) {
        print_array_as_consecutive_memory(arr, a_size);

        size_t t1 = -1, t2 = -1;
        get_interval_by_rank(arr, a_size, quotient, &t1, &t2);
        assert(false);
    }
    assert(c == va);
    assert(d == vb);
    return true;
}

void Header::print() {
    cout << "[" << H[0];
    for (size_t i = 1; i < size; ++i) {
        cout << ", " << H[i];
    }
    cout << "]" << endl;

}

void Header::print_as_bool_array() {
    size_t a_size = size * HEADER_BLOCK_SIZE;
    bool a[a_size];
    header_to_bool_array(a);
    print_bool_array(a, a_size);
}

void Header::header_to_bool_array(bool *a) {
    size_t counter = 0;
    for (size_t i = 0; i < size; ++i) {
        size_t b = 1ULL << (ulong) (HEADER_BLOCK_SIZE - 1);
        for (int j = 0; j < HEADER_BLOCK_SIZE; ++j) {
            a[counter] = H[i] & b;
            counter++;
            b >>= 1ul;
        }
    }
}

void Header::pretty_print() {
    size_t a_size = size * HEADER_BLOCK_SIZE;
    bool a[a_size];
    header_to_bool_array(a);

    int run_length_array[max_capacity];
    size_t run_index = 0;
    size_t index = 0;

    while ((index < a_size) and (run_index < max_capacity)) {
        size_t counter = 0;
        while (a[index] != 0) {
            counter++;
            index++;
        }
        run_length_array[run_index] = counter;
        index++;
        run_index++;
    }

    assert(run_index == max_capacity);
    cout << "[(0:" << run_length_array[0] << ")";
    for (size_t i = 1; i < max_capacity; ++i) {
        cout << ", (" << i << ":" << run_length_array[i] << ")";
    }
    cout << "]" << endl;

}

void Header::print_as_word() {
    size_t const_factor = (32 / HEADER_BLOCK_SIZE);
    size_t a_size = INTEGER_ROUND(size, const_factor);
    uint32_t a[a_size];
    header_to_word_array(a, a_size);
    cout << "[" << a[0];
    for (size_t i = 1; i < a_size; ++i)
        cout << ", " << a[i];
    cout << "]" << endl;
    /*size_t lim = size / const_factor;
    for (size_t i = 0; i < lim; ++i) {
        ulong x = H[i * 4];
        for (int j = 1; j < const_factor; ++j) {
            x <<= 1ul;
            x = H[i * 4 + j];
        }
    }*/
}

void Header::header_to_word_array(uint32_t *a, size_t a_size) {
    size_t const_factor = (32 / HEADER_BLOCK_SIZE);
    size_t lim = size / const_factor;
    size_t a_index = 0;
    for (size_t i = 0; i < lim; ++i) {
        a[a_index] = ((uint32_t) H[(i * 4)]) << 24ul | ((uint32_t) H[(i * 4 + 1)]) << 16ul |
                     ((uint32_t) H[(i * 4) + 2]) << 8ul | ((uint32_t) H[(i * 4) + 3]);
        /*
//        ulong x = H[i * 4];
//        for (size_t j = 1; j < const_factor; ++j) {
//            x <<= 1ul;
//            x = H[i * 4 + j];
//        }
//        a[a_index] = x;
*/
        a_index++;
    }
    if (size % const_factor == 0)
        return;

    size_t bits_left_counter = size % const_factor;
    size_t i = lim;

    if (bits_left_counter == 1)
        a[a_index] = (ulong) H[(i * 4)] << 24ul | (ulong) H[(i * 4 + 1)] << 16ul | (ulong) H[(i * 4) + 2] << 8ul;
    else if (bits_left_counter == 2)
        a[a_index] = (ulong) H[(i * 4)] << 24ul | (ulong) H[(i * 4 + 1)] << 16ul;
    else if (bits_left_counter == 3)
        a[a_index] = (ulong) H[(i * 4)] << 24ul;

    else {
        assert(const_factor == 4);
        assert(false);
    }
}

ostream &operator<<(ostream &os, const Header &header) {
    os << "H: " << header.H;
    return os;
}

size_t Header::get_size() const {
    return size;
}

HEADER_BLOCK_TYPE *Header::get_h() const {
    return H;
}

/*

void Header::vector_get_interval(size_t quotient, size_t *start_index, size_t *end_index) {
    size_t zero_counter = -1, continue_from_index = 0;

    for (size_t i = 0; i < vec.size(); ++i) {
        if (zero_counter == quotient - 1) {
            *start_index = i;
            continue_from_index = i;
            break;
        }
        if (vec[i] == 0)
            zero_counter++;
    }

    for (size_t i = continue_from_index; i < vec.size(); ++i) {
        if (vec[i] == 0) {
            *end_index = i;
            return;
        }
    }
    assert(false);
}

void Header::insert(size_t quotient) {
    size_t a = -1, b = -1;
    this->vector_get_interval(quotient, &a, &b);
    vec.insert(vec.begin() + a, true);
    vec.pop_back();
}

void Header::remove(uint_fast16_t quotient) {
    size_t a = -1, b = -1;
    this->vector_get_interval(quotient, &a, &b);
    assert(a < b);
    vec.erase(vec.begin() + a);
    vec.push_back(false);
}

uint32_t Header::sum_vector() {
    size_t sum = 0;
    for (auto b  : vec)
        if (b) sum++;
    return sum;
}

void Header::validate_get_interval(size_t quotient) {
    size_t a = -1, b = -1, va = -1, vb = -1, c = -1, d = -1;
    get_interval(quotient, &va, &vb);
    get_interval_attempt(H, size, quotient, &c, &d);
    bool cond2 = (c == va) && (d == vb);
    if (not cond2) {
        cout << "header as word is: ";
        this->print_as_word();
        cout << "vector as word is: ";
        print_bit_vector_as_words(&vec);

    }
    assert(c == va);
    assert(d == vb);
*/

void
get_interval_old(const HEADER_BLOCK_TYPE *a, size_t a_size, size_t quotient, size_t *start_index, size_t *end_index) {
    if (quotient == 0) {
        *start_index = 0;
        size_t b = 1 << (HEADER_BLOCK_SIZE - 1);

        if (a[0] < b) {
            *end_index = 0;
            return;
        }

        size_t one_counter = 1;
        b >>= 1ULL;
        for (size_t a_index = 0; a_index < a_size; ++a_index) {
            for (int i = 0; i < HEADER_BLOCK_SIZE; ++i) {
                if (a[a_index] & b)
                    one_counter++;
                else {
                    *end_index = one_counter;
                    return;
                }
                b >>= 1ULL;
            }
            b = 1 << (HEADER_BLOCK_SIZE - 1);
        }
        assert(false);
/*
        for (int i = 0; i <; ++i) {

        }

        bool cond = true;
        size_t i = 0;
        uint b;

        size_t zero_counter = 1;
        size_t array_index = -1;
        size_t bit_index = 0;

        while (cond) {
            for (i; i < HEADER_BLOCK_SIZE; i++) {
                if (b & (a[array_index]))
                    bit_index++;
                else {
                    cond = false;
                    break;
                }
                b >>= 1;
            }
            b = SL((HEADER_BLOCK_SIZE - 1));
            i = 0;
            array_index++;
        }
        *end_index = bit_index + 1;
        for (int i = 0; i < a_size; ++i) {
            auto reversed = (a[i]) ^((MASK(HEADER_BLOCK_SIZE)));
            u32 floor_log = floor_log2(reversed);
            *end_index = HEADER_BLOCK_SIZE - floor_log - 1;

        }
        auto reversed = (a[index]) ^((MASK(HEADER_BLOCK_SIZE)));
        u32 floor_log = floor_log2(reversed);
        *end_index = HEADER_BLOCK_SIZE - floor_log - 1;
//        if ()
        size_t index = 0;
        while (true) {
            if (a[index] == 0) {
                index++;
                continue;
            }
            auto reversed = (a[index]) ^((MASK(HEADER_BLOCK_SIZE)));
            u32 floor_log = floor_log2(reversed);
            *end_index = HEADER_BLOCK_SIZE - floor_log - 1;
            return;*/
        /*
//            cout << "reversed is: " << reversed << endl;

//            cout << "floor_log: " << floor_log << endl;
//            cout << "res: " << HEADER_BLOCK_SIZE - floor_log - 1 << endl;
        return;
        if (floor_log) {
            cout << "msb is: " << floor_log << endl;
            cout << "res: " << HEADER_BLOCK_SIZE - floor_log << endl;
            *end_index = HEADER_BLOCK_SIZE - floor_log;
            return;
        }*/
    }
    uint b;
    size_t zero_counter = 1;
    size_t array_index = -1;
    size_t bit_index = 0;

    bool cond = true;
    while (cond) {
        assert(zero_counter <= quotient);
        array_index++;
        b = SL((HEADER_BLOCK_SIZE - 1));
        for (int i = 0; i < HEADER_BLOCK_SIZE; ++i) {
            if (not(b & (a[array_index]))) {
                zero_counter++;
                if (zero_counter > quotient) {
                    cond = false;
                    break;
                }
            }
            bit_index++;
            b >>= 1;
        }
    }
    *start_index = bit_index + 1;
    b >>= 1;
//    assert(b);
    size_t i = (*start_index) % HEADER_BLOCK_SIZE;


    /*else {
       array_index++;
       *start_index = 0;
       b = (1ULL << (HEADER_BLOCK_SIZE - 1));
   }*/
    cond = true;
    while (cond) {
        for (i; i < HEADER_BLOCK_SIZE; i++) {
            if (b & (a[array_index]))
                bit_index++;
            else {
                cond = false;
                break;
            }
            b >>= 1;
        }
        b = SL((HEADER_BLOCK_SIZE - 1));
        i = 0;
        array_index++;
    }
    *end_index = bit_index + 1;

}

void static_push(HEADER_BLOCK_TYPE *H, size_t size, size_t end) {
    size_t index;
    index = end / HEADER_BLOCK_SIZE;

    for (uint_fast16_t i = size - 1; i > index; --i) {
        H[i] = ((ulong) (H[i]) >> 1ul) | (H[i - 1] & 1ul) << ((ulong) (HEADER_BLOCK_SIZE - 1));
    }

    uint_fast16_t bit_index = end % HEADER_BLOCK_SIZE;
    uint_fast16_t shift = HEADER_BLOCK_SIZE - bit_index;
    HEADER_BLOCK_TYPE upper = (shift < HEADER_BLOCK_SIZE) ? (((ulong) H[index]) >> shift) << shift : 0;
    HEADER_BLOCK_TYPE lower = ((ulong) H[index] >> 1ul) & (MASK(shift));
    H[index] = ((ulong) upper | lower | SL(shift - 1ul));

}

void static_pull(HEADER_BLOCK_TYPE *H, size_t size, size_t end) {
    size_t index;

    index = (end - 1) / HEADER_BLOCK_SIZE;
    if (index == size - 1) {
        uint_fast16_t bit_index = (end - 1) % HEADER_BLOCK_SIZE;
        uint_fast16_t shift = HEADER_BLOCK_SIZE - bit_index;
        HEADER_BLOCK_TYPE upper = (shift < HEADER_BLOCK_SIZE) ? ((ulong) H[index] >> shift) << shift : 0;
//        HEADER_BLOCK_TYPE upper = (this->H[index] >> shift) << shift;
        HEADER_BLOCK_TYPE mid = ((ulong) H[index] << 1ul) & (MASK(shift));
//        HEADER_BLOCK_TYPE lower = (this->H[index + 1]) >> (HEADER_BLOCK_SIZE - 1);
        H[index] = upper | mid;

        //Making sure the run's end, marked by zero, is not deleted.
        if (DB) assert(H[index] ^ SL((ulong) bit_index - 2));

        cout << "here" << endl;
        return;
    }
    HEADER_BLOCK_TYPE lower = ((ulong) H[index + 1]) >> ((ulong) (HEADER_BLOCK_SIZE - 1));

    for (size_t i = index + 1; i < size - 1; ++i) {
        H[i] = ((ulong) H[i] << 1ul) | ((ulong) H[i + 1] >> ((ulong) (HEADER_BLOCK_SIZE - 1)));
    }
    H[size - 1] <<= 1ul;

    uint_fast16_t bit_index = (end - 1) % HEADER_BLOCK_SIZE;
    uint_fast16_t shift = HEADER_BLOCK_SIZE - bit_index;
    HEADER_BLOCK_TYPE upper = (shift < HEADER_BLOCK_SIZE) ? ((ulong) H[index] >> shift) << shift : 0;
//    HEADER_BLOCK_TYPE upper = (this->H[index] >> shift) << shift;
    HEADER_BLOCK_TYPE mid = ((ulong) H[index] << 1ul) & (MASK(shift));

    H[index] = (ulong) upper | mid | lower;// | SL(shift);
//todo: why here?    assert(H[index] ^ SL((ulong)bit_index - 2)); //Making sure the run's end, marked by zero, is not deleted.
}

//bool Header::conditional_remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
//    //todo
//    /*get_quotient_start_and_end_index(quotient, start_index, end_index);
//    assert (*end_index > *start_index); //deleting from empty run.
//
//    pull(quotient, *start_index, *end_index);
//
//    capacity--;*/
//}
