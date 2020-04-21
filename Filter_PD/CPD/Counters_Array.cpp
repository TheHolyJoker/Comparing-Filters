//
// Created by tomer on 3/14/20.
//

#include "Counters_Array.hpp"

Counters_Array::Counters_Array(size_t max_capacity, size_t max_rep) : max_capacity(max_capacity), max_rep(max_rep),
                                                                      a_size(get_array_size(
                                                                              get_bit_size(max_capacity, max_rep))) {
    /* COUNTER_TYPE empty_cell = 0;
    if (COUNTER_TYPE_SIZE == 8)
        empty_cell = COUNTER_CELL_C8;
    else if (COUNTER_TYPE_SIZE == 16)
        empty_cell = COUNTER_CELL_C16;
    else if (COUNTER_TYPE_SIZE == 32)
        empty_cell = COUNTER_CELL_C32;
    else
        assert(false);*/

    A = new COUNTER_TYPE[a_size]();
    for (int i = 0; i < a_size; ++i) { A[i] = 0; }
}

void Counters_Array::update_counter(size_t bit_start_index, size_t bit_end_index, size_t update_val) {
//    auto symbol = read_word(bit_start_index,bit_end_index);
//    auto new_symbol = update<COUNTER_TYPE>(symbol, update_val);
//    u32 shift = get_numbers_bit_length_difference<COUNTER_TYPE>(symbol, new_symbol);


//    if (shift == 0)

}

size_t Counters_Array::get_ith_counter(size_t ith) {


    return 0;
}

/*void Counters_Array::find_counter_interval(size_t index, size_t *start, size_t *end) {
    ////cg_PD header_find rewriting.
    //todo: start over.
    if (index == 0) {
        *start = 0;
        *end = __builtin_clz(DECODE(A[0]));
        return;
    }

    for (int i = 0; i < a_size; ++i) {
        COUNTER_TYPE decoded_slot = DECODE(A[i]);
        auto cz = __builtin_popcount(decoded_slot);
        if (cz < index) index -= cz;
        else if (cz == index) {
            uint32_t bit_pos = __builtin_ctz(decoded_slot) - 2;
//            uint64_t slot = ((ulong) (slot) << 32ul) | 4294967295ul;
//            uint32_t bit_pos = select_r(~slot, index);
//            if (DB) assert(bit_pos < COUNTER_TYPE_SIZE);
            *start = (i + (bit_pos + 1 == COUNTER_TYPE_SIZE)) * COUNTER_TYPE_SIZE + (bit_pos + 1) % COUNTER_TYPE_SIZE;
            size_t j = i + 1;
            while (A[j] == MASK32) j++;
            *end = (j) * COUNTER_TYPE_SIZE + __builtin_clz(~A[j]);
            return;

        } else {
            if (DB) assert(index < COUNTER_TYPE_SIZE);
            uint64_t slot = ((ulong) (A[i]) << 32ul) | 4294967295ul;
            uint32_t bit_pos = select_r(~slot, index);
            if (DB) assert(bit_pos < COUNTER_TYPE_SIZE);
            *start = i * COUNTER_TYPE_SIZE + select_r(~slot, index) + 1;
            *end = i * COUNTER_TYPE_SIZE + select_r(~slot, index + 1);
//            cout << "h3" << endl;
            return;

            auto temp_count = __builtin_popcount(slot);
            if (temp_count + 1 <= index)
                break;
            index -= temp_count;
        }


        if (index == 0) {
            *start = 0;
            size_t j = 0;
            while (A[j] == MASK32) j++;
            *end = (j) * COUNTER_TYPE_SIZE + __builtin_clz(~A[j]);
//        uint64_t slot2 = ((ulong) (A[j]) << 32ul) | 4294967295ul;
//        *end_index = (j) * CG_TYPE_SIZE + select_r(~slot2, 1);
//        cout << "h0" << endl;
            return;
        }
        for (size_t i = 0; i <= 42; ++i) {
            auto cz = __builtin_popcount(~A[i]);
            if (cz < index) index -= cz;
            else if (cz == index) {
                uint64_t slot = ((ulong) (A[i]) << 32ul) | 4294967295ul;
                uint32_t bit_pos = select_r(~slot, index);
                if (DB) assert(bit_pos < COUNTER_TYPE_SIZE);
                *start = (i + (bit_pos + 1 == COUNTER_TYPE_SIZE)) * COUNTER_TYPE_SIZE +
                         (bit_pos + 1) % COUNTER_TYPE_SIZE;
                size_t j = i + 1;
                while (A[j] == MASK32) j++;
                *end = (j) * COUNTER_TYPE_SIZE + __builtin_clz(~A[j]);
                return;

            } else {
                if (DB) assert(index < COUNTER_TYPE_SIZE);
                uint64_t slot = ((ulong) (A[i]) << 32ul) | 4294967295ul;
                uint32_t bit_pos = select_r(~slot, index);
                if (DB) assert(bit_pos < COUNTER_TYPE_SIZE);
                *start = i * COUNTER_TYPE_SIZE + select_r(~slot, index) + 1;
                *end = i * COUNTER_TYPE_SIZE + select_r(~slot, index + 1);
//            cout << "h3" << endl;
                return;

            }
        }
        assert(false);

    }


}*/

auto Counters_Array::count_capacity() const -> size_t {
    size_t counter = 0;
    for (int i = 0; i < a_size - 1; ++i) {
        auto slot = SPLITTED_COMMA_DECODER(A[i], A[i + 1]);
        counter += __builtin_popcount(slot);
    };
    size_t res = counter + __builtin_popcount(COMMA_DECODE(A[a_size - 1]));
    assert(res == get_capacity());
    return res;
}

auto Counters_Array::get_capacity() const -> size_t {
    return capacity;
}


auto Counters_Array::extract_symbol(size_t bit_start_index, size_t bit_end_index) -> COUNTER_TYPE {
    if (bit_start_index == bit_end_index) {
        return 0;
    }
    auto bit_start_rem = bit_start_index % COUNTER_TYPE_SIZE;
    auto bit_end_rem = bit_end_index % COUNTER_TYPE_SIZE;
    size_t shift_power = COUNTER_TYPE_SIZE - bit_start_rem;
    uint32_t mask = SL(shift_power) - 1u;
    uint32_t shift_left = COUNTER_TYPE_SIZE - (bit_end_rem);

    if (bit_end_rem == 0) {
        size_t array_start_index = bit_start_index / COUNTER_TYPE_SIZE;
        auto temp = A[array_start_index] & mask;
        return temp;
    }
    size_t array_start_index = bit_start_index / COUNTER_TYPE_SIZE, array_end_index = bit_end_index / COUNTER_TYPE_SIZE;

    if (array_start_index == array_end_index) {
        auto temp = A[array_start_index] & mask;
        assert(shift_left > 0);
        return temp >> shift_left;

    } else {
        auto i = array_start_index;
        assert(i + 1 == array_end_index);
        assert(i + 1 < a_size);
        return ((A[i] & mask) << bit_end_rem) | (A[i + 1] >> shift_left);
    }
}

auto get_bit_size(size_t max_capacity, size_t max_rep) -> size_t {
    return max_capacity << 1ul;
}

auto get_array_size(size_t bit_size) -> size_t {
    return INTEGER_ROUND(bit_size, COUNTER_TYPE_SIZE);
}

//template auto static_extract_symbol<COUNTER_TYPE>(const COUNTER_TYPE *A, size_t a_size, size_t bit_start_index,
//                                                  size_t bit_end_index) -> COUNTER_TYPE;
/*

template auto static_extract_symbol<unsigned int>(const unsigned int *A, size_t a_size, size_t bit_start_index,
                                                  size_t bit_end_index) -> unsigned int;*/
