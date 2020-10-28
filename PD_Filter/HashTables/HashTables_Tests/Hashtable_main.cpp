//
// Created by tomer on 22/10/2020.
//

#include "validate_hash_table.hpp"


/*
void t_HT() {
    const size_t max_capacity = 1u << 10u, element_length = 28, bucket_size = 4;
    size_t reps = 1u << 14u;
    auto load_factor = .75;
    double working_LF = .72;
    double variance = .1;

    v_hash_table_rand_gen_load(reps, max_capacity, element_length, 2, load_factor, working_LF, variance);
    cout << "\n\n\n" << endl;
    v_hash_table_rand_gen_load(reps, max_capacity, element_length, 4, load_factor, working_LF, variance);
    cout << "\n\n\n" << endl;
    v_hash_table_rand_gen_load(reps, max_capacity, element_length, 8, load_factor, working_LF, variance);
    cout << "\n\n\n" << endl;
    v_hash_table_rand_gen_load(reps, max_capacity, element_length, 12, load_factor, working_LF, variance);
}
*/

int main() {
    // simpler_HTA ht(1<<20,32,.9);
    // return 0;

    constexpr uint64_t temp = 0x11DCE5;
    using temp_HT = hashTable_Aligned<uint64_t, 4>;
    auto res = v_hash_table_rand_gen_load<temp_HT, uint64_t>(1 << 13, 1 << 13, 32, 4, .9, .5);
    assert(res);
    std::cout << "end" << std::endl;
    return 0;
    // bool res = packed_spare_wrapper_multi(1 << 10, 1 << 24, .95, .95);
    // bool res = packed_spare_wrapper(1 << 10, 1 << 24, .95, .95);
    // assert(res);
    // res = packed_spare_wrapper(1 << 18, 1 << 20, .9, .75);
    // res = packed_spare_wrapper(1 << 18, 1 << 20, .9, .75);
    // assert(res);
    // res = packed_spare_wrapper(1 << 18, 1 << 20, .9, .75);
    // assert(res);
//    bool res = packed_spare_wrapper(1 << 10, 1 << 18, 42, .9, .9);
//    v_hash_table_rand(1u << 14u, max_capacity, element_length, 4, load_factor);
    // return 0;
}
