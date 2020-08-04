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

void print_single_round(size_t var_num, const size_t *values, const size_t *divisors, std::size_t width = DEFAULT_WIDTH);

void table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);

void att_print_single_round_false_positive_rates(size_t lookups_repetitions, size_t bits_per_item,
                                                 size_t false_positive_counter, size_t true_positive_counter);

void print_single_round_false_positive_rates(size_t lookups_repetitions, size_t expected_false_positive,
                                             size_t true_positive_counter, size_t false_positive_counter);

template <typename val_type>
void table_print(size_t var_num, string *var_names, val_type *values)
{
    size_t max_length = 16;
    for (int i = 0; i < var_num; ++i)
    {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << '\n'
              << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1)
    {
        cout << std::setw(name_width) << var_names[counter++] << sep;
    }
    cout << std::setw(name_width - 1) << var_names[counter] << sep;
    cout << '\n'
         << line << '\n' + sep;

    counter = 0;
    while (counter < var_num - 1)
    {
        cout << std::setw(name_width) << values[counter++] << sep;
    }
    cout << std::setw(name_width - 1) << values[counter] << sep;
    cout << '\n'
         << line << '\n';
}

template <typename T>
auto bin_print(T x, std::ostream &os = std::cout) -> std::ostream &
{
    if (x == 0)
    {
        os << '0' << std::endl;
        return os;
    }
    size_t slot_size = sizeof(T) * CHAR_BIT;
    uint64_t b = 1ULL << (slot_size - 1u);
    while ((!(b & x)) and (b))
    {
        b >>= 1ul;
    }
    assert(b);
    while (b)
    {
        std::string temp = (b & x) ? "1" : "0";
        os << temp;
        b >>= 1ul;
    }
    os << std::endl;
    return os;
}

template <typename T>
auto print_array(T *a, size_t a_size, std::ostream &os = std::cout) -> std::ostream &
{
    os << "[" << a[0];
    for (size_t i = 1; i < a_size; i++)
    {
        os << ", " << a[i];
    }
    os << "]" << std::endl;
    return os;
}

template <typename T>
auto print_8array(T *val, std::ostream &os = std::cout) -> void
{
    printf("%zu %zu %zu %zu %zu %zu %zu %zu \n",
           val[0], val[1], val[2], val[3], val[4], val[5],
           val[6], val[7]);
}

template <typename T>
auto str_format(T value) ->std::string
{
    std::string numWithCommas = to_string(value);
    int insertPosition = numWithCommas.length() - 3;
    while (insertPosition > 0)
    {
        numWithCommas.insert(insertPosition, ",");
        insertPosition -= 3;
    }
    return numWithCommas;
}
#endif //FILTERS_PRINTUTIL_HPP
