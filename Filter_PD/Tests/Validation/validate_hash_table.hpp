
#ifndef CLION_CODE_VALIDATE_HASH_TABLE_HPP
#define CLION_CODE_VALIDATE_HASH_TABLE_HPP

#include <set>
#include "../../Hash_Table/hash_table.hpp"

auto
v_hash_table_rand(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                  double load_factor) -> bool;

auto
v_hash_table_rand_gen_load(size_t reps, size_t max_capacity, size_t element_length, size_t bucket_size,
                           double max_load_factor, double working_load_factor, double variance = 1) -> bool;

template<typename T>
void single_op(hash_table<T> *ht, set<T> *my_set, set<uint32_t> *out_set, set<uint32_t> *in_set, bool to_insert);


auto
v_hash_table(size_t reps, size_t table_size, size_t max_capacity, size_t element_length, double load_factor) -> bool;

template<typename T>
auto v_HT_insert(T x, hash_table<T> *ht, set<T> *my_set) -> bool;

template<typename T>
auto v_HT_remove(T x, hash_table<T> *ht, set<T> *my_set) -> bool;

template<typename T>
auto v_HT_find(T x, hash_table<T> *ht, set<T> *my_set) -> bool;


auto get_prob(size_t x, size_t y) -> bool;

//auto

#endif //CLION_CODE_VALIDATE_HASH_TABLE_HPP
