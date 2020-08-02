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

void print_name(const std::string &filter_name, size_t line_width = default_line_width);


void table_print(size_t var_num, string *var_names, size_t *values);

void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors);

void print_line();

void print_round_header();

void
print_single_round(size_t var_num, const size_t *values, const size_t *divisors, std::size_t width = DEFAULT_WIDTH);

void
table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);


void att_print_single_round_false_positive_rates(size_t lookups_repetitions, size_t bits_per_item,
                                                 size_t false_positive_counter, size_t true_positive_counter);

void
print_single_round_false_positive_rates(size_t lookups_repetitions, size_t expected_false_positive,
                                        size_t true_positive_counter, size_t false_positive_counter);


template<typename val_type>
void table_print(size_t var_num, string *var_names, val_type *values){
    size_t max_length = 16;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << '\n' << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1) {
        cout << std::setw(name_width) << var_names[counter++] << sep;
    }
    cout << std::setw(name_width - 1) << var_names[counter] << sep;
    cout << '\n' << line << '\n' + sep;


    counter = 0;
    while (counter < var_num - 1) {
        cout << std::setw(name_width) << values[counter++] << sep;
    }
    cout << std::setw(name_width - 1) << values[counter] << sep;
    cout << '\n' << line << '\n';

}

#endif //FILTERS_PRINTUTIL_HPP
