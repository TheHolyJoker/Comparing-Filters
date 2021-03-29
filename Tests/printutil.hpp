//
// Created by root on 01/06/2020.
//

#ifndef FILTERS_PRINTUTIL_HPP
#define FILTERS_PRINTUTIL_HPP

#include <algorithm>// std::max
#include <cassert>
#include <climits>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <stdio.h>
#include <vector>

#define UNIVERSE_SIZE (0xffffffffffffffff)
#define DEFAULT_WIDTH (24)

using namespace std;

//static const size_t default_line_width = 116;
static const size_t default_line_width = 160;

auto print_name(const std::string &filter_name, size_t line_width = default_line_width) -> std::stringstream;

// void table_print(size_t var_num, string *var_names, size_t *values, std::stringstream* os);

// void table_print(size_t var_num, string *var_names, size_t *values);

void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors);

void print_seperating_line();

auto print_round_header() -> std::stringstream;

auto print_single_round(size_t var_num, const size_t *values, const size_t *divisors, size_t width = DEFAULT_WIDTH) -> std::stringstream;
// void print_single_round(size_t var_num, const size_t *values, const size_t *divisors, std::stringstream *ss, std::size_t width = DEFAULT_WIDTH);

void table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);

auto att_print_single_round_false_positive_rates(size_t lookups_repetitions, size_t bits_per_item,
                                                 size_t false_positive_counter, size_t true_positive_counter) -> std::stringstream;

auto print_false_positive_rates_header(size_t width = 12) -> std::stringstream;


auto print_single_round_false_positive_rates(size_t lookups_repetitions, size_t expected_false_positive,
                                             size_t true_positive_counter, size_t false_positive_counter) -> std::stringstream;

auto print_single_round_false_positive_rates_probabilistic(std::string filter_name, size_t lookups_repetitions, size_t expected_false_positive,
                                                           size_t true_counter, size_t false_counter, bool is_last_line) -> std::stringstream;


auto print_single_round_false_positive_rates(std::string filter_name, size_t lookups_repetitions, size_t expected_false_positive,
                                             size_t true_positive_counter, size_t false_positive_counter, bool is_last_line = false) -> std::stringstream;

// template<typename val_type>
// void table_print(size_t var_num, string *var_names, val_type *values) {
//     size_t max_length = 16;
//     for (int i = 0; i < var_num; ++i) {
//         max_length = max(var_names[i].length(), max_length);
//     }
//
//     // values for controlling format
//     const uint32_t name_width = int(max_length);
//     const std::string sep = " |";
//     const int total_width = default_line_width;
//     const std::string line = sep + std::string(total_width - 1, '-') + '|';
//     std::cout << line << '\n'
//               << sep << left;
//
//     size_t counter = 0;
//     while (counter < var_num - 1) {
//         cout << std::setw(name_width) << var_names[counter++] << sep;
//     }
//     cout << std::setw(name_width - 1) << var_names[counter] << sep;
//     cout << '\n'
//          << line << '\n' + sep;
//
//     counter = 0;
//     while (counter < var_num - 1) {
//         cout << std::setw(name_width) << values[counter++] << sep;
//     }
//     cout << std::setw(name_width - 1) << values[counter] << sep;
//     cout << '\n'
//          << line << '\n';
// }

template<typename val_type>
void table_print(size_t var_num, string *var_names, val_type *values, std::stringstream *os) {
    size_t max_length = 16;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    *os << line << '\n'
        << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1) {
        *os << std::setw(name_width) << var_names[counter++] << sep;
    }
    *os << std::setw(name_width - 1) << var_names[counter] << sep;
    *os << '\n'
        << line << '\n' + sep;

    counter = 0;
    while (counter < var_num - 1) {
        *os << std::setw(name_width) << values[counter++] << sep;
    }
    *os << std::setw(name_width - 1) << values[counter] << sep;
    *os << '\n'
        << line << '\n';
}

template<typename val_type>
auto table_print(size_t var_num, string *var_names, val_type *values) -> std::stringstream {
    std::stringstream ss;
    size_t max_length = 16;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << '\n'
       << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1) {
        ss << std::setw(name_width) << var_names[counter++] << sep;
    }
    ss << std::setw(name_width - 1) << var_names[counter] << sep;
    ss << '\n'
       << line << '\n' + sep;

    counter = 0;
    while (counter < var_num - 1) {
        ss << std::setw(name_width) << values[counter++] << sep;
    }
    ss << std::setw(name_width - 1) << values[counter] << sep;
    ss << '\n'
       << line << '\n';
    return ss;
}

template<typename val_type>
auto table_print_normalized(size_t var_num, string *var_names, val_type *values, size_t *normalizer_array) -> std::stringstream {
    std::stringstream ss;
    size_t max_length = 16;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << '\n'
       << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1) {
        ss << std::setw(name_width) << var_names[counter++] << sep;
    }
    ss << std::setw(name_width - 1) << var_names[counter] << sep;
    ss << '\n'
       << line << '\n' + sep;

    counter = 0;
    while (counter < var_num - 1) {
        auto temp_norm_val = values[counter] / normalizer_array[counter];
        ss << std::setw(name_width) << temp_norm_val << sep;
        counter++;
    }
    auto temp_norm_val = values[counter] / normalizer_array[counter];
    ss << std::setw(name_width - 1) << temp_norm_val << sep;
    ss << '\n'
       << line << '\n';
    return ss;
}


template<typename T>
auto bin_print(T x, std::ostream &os = std::cout) -> std::ostream & {
    if (x == 0) {
        os << '0' << std::endl;
        return os;
    }
    size_t slot_size = sizeof(T) * CHAR_BIT;
    uint64_t b = 1ULL << (slot_size - 1u);
    while ((!(b & x)) and (b)) {
        b >>= 1ul;
    }
    assert(b);
    while (b) {
        std::string temp = (b & x) ? "1" : "0";
        os << temp;
        b >>= 1ul;
    }
    os << std::endl;
    return os;
}

template<typename T>
auto print_array(T *a, size_t a_size, std::ostream &os = std::cout) -> std::ostream & {
    os << "[" << a[0];
    for (size_t i = 1; i < a_size; i++) {
        os << ", " << a[i];
    }
    os << "]" << std::endl;
    return os;
}

template<typename T>
auto print_8array(T *val, std::ostream &os = std::cout) -> void {
    printf("%zu %zu %zu %zu %zu %zu %zu %zu \n",
           val[0], val[1], val[2], val[3], val[4], val[5],
           val[6], val[7]);
}

template<typename T>
auto str_format(T value) -> std::string {
    std::string numWithCommas = to_string(value);
    int insertPosition = numWithCommas.length() - 3;
    while (insertPosition > 0) {
        numWithCommas.insert(insertPosition, ",");
        insertPosition -= 3;
    }
    return numWithCommas;
}

template<typename T>
void print_line(T *vals, size_t num, size_t width, bool is_last_line) {
    /*bool is_last_round = values[0] == values[1];
    bool is_round_contain_two_digits = (values[0] > 9);
    */
    /*size_t max_length = width;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }*/
    /*

// values for controlling format
    const uint32_t name_width = width;
    const std::string sep = " |";
    const uint32_t total_width = (name_width + sep.size()) * (var_num - 1);
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
//    std::cout << line << '\n' << sep << left;
    std::cout << sep << left;

    std::size_t first_arg_length = 2 + is_round_contain_two_digits;
    //Round Number
    std::cout << values[0] << "/" << std::setw(name_width - first_arg_length) << values[1];
    double rate;
    //Insertion rate (op/sec)
    rate = divisors[0] / (values[2] / 1e9);
    std::cout << sep << std::setw(name_width) << rate;
    //uniform_lookup rate (op/sec)
    rate = divisors[1] / (values[3] / 1e9);
    std::cout << sep << std::setw(name_width) << rate;
    //true_lookup rate (op/sec)
    rate = divisors[0] / (values[4] / 1e9);
    std::cout << sep << std::setw(name_width) << rate;
    //deletion rate (op/sec)
    rate = divisors[2] / (values[4] / 1e9);
    std::cout << sep << std::setw(name_width) << rate << sep << std::endl;

    if (is_last_round)
        std::cout << line << '\n';
*/
}

template<typename T>
auto print_array_normalized_vertical(T *a, size_t a_size, size_t normalizer_val, std::string header = "", std::ostream &os = std::cout) -> std::ostream & {
    auto line = std::string(40, '=');
    os << line << std::endl;
    auto mid_line = std::string(40, '-');
    if (header.size()) {
        os << header << std::endl;
        os << mid_line << std::endl;
    }
    for (size_t i = 0; i < a_size; i++) {
        os << i << ":\t" << (1.0 * a[i] / normalizer_val) << std::endl;
    }
    os << line << std::endl;
    return os;
}


template<typename T>
auto print_two_arrays_normalized_vertical(T *a, T *b, size_t a_size, size_t normalizer_val_a, size_t normalizer_val_b, std::ostream &os = std::cout) -> std::ostream & {
    auto line = std::string(80, '=');
    auto sep = "  |  ";
    // os << line << std::endl;
    auto mid_line = std::string(80, '-');
    // if (header.size()) {
    //     os << header << std::endl;
    //     os << mid_line << std::endl;
    // }
    os << setw(6) << "Round" << sep;
    os << setw(12) << "    FPP      " << " |";
    // os << setw(12) << "    FPP      " << sep;
    os << setw(12) << "    FNP      " << std::endl;
    os << mid_line << std::endl;
    for (size_t i = 0; i < a_size; i++) {
        os << left;
        os << setw(6) << i << sep;
        os << setw(12) << (1.0 * a[i] / normalizer_val_a) << sep;
        os << setw(12) << (1.0 * b[i] / normalizer_val_b) << std::endl;
        //  << (1.0 * a[i] / normalizer_val_a);
        // os << "\t| \t" <<   (1.0 * b[i] / normalizer_val_b) << std::endl;
    }
    os << line << std::endl;
    return os;
}

#endif//FILTERS_PRINTUTIL_HPP
