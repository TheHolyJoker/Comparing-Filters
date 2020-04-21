//
// Created by tomer on 10/29/19.
//

#ifndef CLION_CODE_BENCHMARK_TESTS_H
#define CLION_CODE_BENCHMARK_TESTS_H


#include <set>
#include <fstream>
#include <tuple>
#include <random>
#include <chrono>
#include <iomanip>


#include "../../PD/Safe_PD/safe_PD.h"
#include "../../Filters/POW2C/pow2c_filter.h"
#include "../../Filters/POW2C/gen_2Power.h"
#include "../../Filters/POW2C/pow2c_naive_filter.h"
#include "../../Filters/POW2C/const_filter.h"
#include "../Validation/validation_tests.h"
#include "/usr/local/include/cuckoofilter/cuckoofilter.h"
#include "../Validation/validate_filter.hpp"


#define MIN_ELEMENT_LENGTH 8
#define MIN_CHAR_RANGE 32
#define MIN_LENGTH_RANGE 8
#define WIDTH 24
#define QR_TUPLE tuple<uint32_t ,uint32_t>

using cuckoofilter::CuckooFilter;

typedef chrono::microseconds ms;
typedef chrono::nanoseconds ns;

static bool to_print_header = true;

vector<uint64_t> random_vector(size_t size);

string rand_string(int minLength, int charsNum, int numOfDiffLength = MIN_LENGTH_RANGE);

void set_init(size_t size, set<string> *mySet, int minLength = MIN_ELEMENT_LENGTH, int charsNum = MIN_CHAR_RANGE);

void vec_init(size_t size, vector<uint32_t> *my_vec);
//void vec_init(size_t size, vector<string> *my_vec);

void vector_lexicographic_init(size_t size, vector<string> *vec, int minLength = MIN_ELEMENT_LENGTH,
                               int charsNum = MIN_CHAR_RANGE);

void set_init(size_t size, set<QR_TUPLE > *mySet, uint32_t max_q, uint32_t max_r);

ostream &b0(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps, ostream &os);

ostream &
b0_naive(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps, ostream &os);
//
//ostream &filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
//                      ostream &os);

ostream &
const_filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                   ostream &os);

ostream &
const_filter_rates32(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                     ostream &os);

ostream &
cuckoo_filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, const size_t l, size_t lookup_reps,
                    ostream &os);

template<class T>
ostream &
filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps, ostream &os);

ostream &
filter_rates_simple_pd(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                       ostream &os);


template<class T>
ostream &
template_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
               ostream &os);


void b1(size_t f, size_t m, size_t l, size_t lookup_reps, ostream &os);

void test_table_header_print();

ostream &
test_table(size_t n, double eps, size_t lookups_num, double set_ratio, int *counter, ulong member_set_init_time,
           ulong nom_set_init_time, ulong init_time, ulong insertion_time, ulong lookup_time, ulong total_run_time,
           ostream &os);

auto test_table(size_t n, size_t lookups_num, double set_ratio, ulong insertion_time, ulong lookup_time,
                ulong total_run_time, ostream &os) -> ostream &;


ostream &
test_printer(size_t n, double eps, size_t lookups_num, bool is_adaptive, double set_ratio, int *counter,
             double member_set_init_time,
             double nom_set_init_time, double init_time, double insertion_time, double lookup_time,
             double total_run_time, ostream &os);


ostream &lookup_result_array_printer(int *counter, size_t lookup_reps, size_t fp_size, double load_factor);


bool filter_r1(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l);

bool filter_naive_r1(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l);

bool filter_r0(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l);

bool filter_naive_r0(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l);

bool const_filter_r0(size_t number_of_pds, float load_factor, size_t l);

template<class T>
void filter_fp_rates(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l, size_t lookup_reps,
                     ostream &os);

//template<class T>
//void filter_rates_core(D *filter, size_t max_capacity, size_t lookup_reps, ostream &os) -> ostream &;


#endif //CLION_CODE_BENCHMARK_TESTS_H
