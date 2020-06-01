//
// Created by root on 01/06/2020.
//

#ifndef FILTERS_PRINTUTIL_HPP
#define FILTERS_PRINTUTIL_HPP

#include <climits>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <vector>
#include <set>
#include <random>
#include <stdio.h>
#include <iostream>
#include <cassert>


#define UNIVERSE_SIZE (0xffffffffffffffff)
#define DEFAULT_WIDTH (24)


using namespace std;

//static const size_t default_line_width = 116;
static const size_t default_line_width = 160;

void print_name(const std::string& filter_name, size_t line_width = default_line_width);

void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors);

void print_line();

void print_round_header();

void print_single_round(size_t var_num, const size_t *values, const size_t *divisors, std::size_t width = DEFAULT_WIDTH);

void
table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);

void
print_single_round_false_positive_rates(size_t lookups_repetitions, size_t expected_false_positive,
                                        size_t true_positive_counter, size_t false_positive_counter);


#endif //FILTERS_PRINTUTIL_HPP
