//
// Created by root on 01/06/2020.
//

#include "printutil.hpp"

auto print_name(const std::string &filter_name, size_t line_width) -> std::stringstream {
    size_t temp_width = line_width - 5;
    bool cond = (filter_name.find('\t') != std::string::npos);
    cond |= (filter_name.find('\t') != std::string::npos);
    std::size_t fixed_width = (cond) ? temp_width - 2u : temp_width;
    // if (filter_name.size() >= 10) {}
    std::string line = " |" + std::string(temp_width, '-') + '|';
    std::stringstream ss;
    ss << line << "\n |" << std::left << std::setw(fixed_width) << filter_name << "|\n"
       << line << std::endl;
    return ss;
    //              << " |" << std::left << std::setw(temp_width) << filter_name.size() << "|\n"
}

void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors) {
    size_t max_length = 0;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }


    // values for controlling format
    const uint32_t name_width = int(max_length);
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;

    const std::string line = sep + std::string(4 * total_width - 1, '-') + '|';
    std::cout << line << "\n"
              << sep << left;

    const std::string spacing = std::string(name_width, ' ');

    const size_t column_num = 4;
    string columns[column_num] = {"var_name", "ratio", "actual value", "divider"};
    size_t counter = 0;
    while (counter < column_num - 1) {
        cout << std::setw(name_width) << columns[counter++] << sep;
    }
    cout << std::setw(name_width) << columns[counter] << std::string(4, ' ') << sep;
    cout << "\n"
         << line << "\n";

    //    assert(var_num % column_num == 0);
    for (int i = 0; i < var_num; ++i) {
        std::cout << sep << std::setw(name_width) << var_names[i];
        double rate = divisors[i] / (values[i] / 1e9);
        std::cout << sep << std::setw(name_width) << rate;
        std::cout << sep << std::setw(name_width) << divisors[i];
        std::cout << sep << std::setw(name_width) << values[i];
        auto temp_length = total_width - name_width * column_num;
        assert(temp_length > 0);
        std::cout << std::string(4, ' ') << sep << "\n";
    }
    std::cout << line << endl;
}

void table_print(size_t var_num, string *var_names, size_t *values, std::stringstream *os) {
    size_t max_length = 0;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    *os << line << "\n"
        << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1) {
        cout << std::setw(name_width) << var_names[counter++] << sep;
    }
    *os << std::setw(name_width - 1) << var_names[counter] << sep;
    *os << "\n"
        << line << "\n" + sep;


    counter = 0;
    while (counter < var_num - 1) {
        *os << std::setw(name_width) << values[counter++] << sep;
    }
    *os << std::setw(name_width - 1) << values[counter] << sep;
    *os << "\n"
        << line << "\n";
}


void print_seperating_line() {
    const uint32_t name_width = 24;
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << "\n";
}

auto print_round_header() -> std::stringstream {
    std::stringstream ss;
    size_t max_length = 24;
    const size_t var_num = 5;
    string names[var_num] = {"Round", "add rate (op/sec)", "uni-lookup (op/sec)",
                             "True-lookup (op/sec)", "deletion rate (op/sec)"};
    //    size_t max_length = 0;
    //    for (auto &name : names) {
    //        max_length = max(name.length(), max_length);
    //    }


    // values for controlling format
    const uint32_t name_width = max_length;
    const std::string sep = " |";
    const uint32_t total_width = (name_width + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << "\n";

    for (auto &name : names) {
        ss << sep << std::setw(name_width) << name;
    }
    ss << sep << "\n"
       << line << endl;
    return ss;
    //    ss << sep << std::setw(name_width) << "Round";
    //    ss << "  " << sep << std::setw(name_width) << "Insertion rate (op/sec)";
    //    ss << sep << std::setw(name_width) << "uniform lookup rate (op/sec)";
    //    ss << sep << std::setw(name_width) << "True lookup rate (op/sec)";
    //    ss << sep << std::setw(name_width) << "deletion rate (op/sec)";
    //    ss << std::string(6, ' ') << sep << "\n" << line << endl;
}

auto print_single_round(size_t var_num, const size_t *values, const size_t *divisors, size_t width) -> std::stringstream {
    std::stringstream ss;
    bool is_last_round = values[0] == values[1];
    bool is_round_contain_two_digits = (values[0] > 9);
    /*size_t max_length = width;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }*/

    // values for controlling format
    const uint32_t name_width = width;
    const std::string sep = " |";
    const uint32_t total_width = (name_width + sep.size()) * (var_num - 1);
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    //    std::cout << line << "\n" << sep << left;
    ss << sep << left;

    std::size_t first_arg_length = 2 + is_round_contain_two_digits;
    //Round Number
    ss << values[0] << "/" << std::setw(name_width - first_arg_length) << values[1];
    double rate;
    //Insertion rate (op/sec)
    rate = divisors[0] / (values[2] / 1e9);
    ss << sep << std::setw(name_width) << rate;
    //uniform_lookup rate (op/sec)
    rate = divisors[1] / ( values[3] / 1e9);
    ss << sep << std::setw(name_width) << rate;
    //true_lookup rate (op/sec)
    rate = divisors[2] / (values[4] / 1e9);
    ss << sep << std::setw(name_width) << rate;
    //deletion rate (op/sec)
    rate = divisors[3] / (values[5] / 1e9);
    ss << sep << std::setw(name_width) << rate << sep << std::endl;

    if (is_last_round)
        ss << line << "\n";
    return ss;
    //    auto temp_length = total_width - name_width * var_num;
    //    assert(temp_length > 0);
    //    std::cout  << sep << "\n";
    //    std::cout << std::string(6, ' ') << sep << "\n";

    //    << line << endl;
}

// auto table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count)->std::stringstream {
//     size_t var_num = 3;
//     string names[3] = {"expected_FP_count", "high_load_FP_count", "mid_load_FP_count"};
//     size_t values[3] = {expected_FP_count, high_load_FP_count, mid_load_FP_count};
//     table_print(var_num, names, values);
//
//     /*size_t max_length = 0;
//     for (int i = 0; i < var_num; ++i) {
//         max_length = max(names[i].length(), max_length);
//     }
//
//     // values for controlling format
//     const uint32_t name_width = int(max_length);
//     const std::string sep = " |";
//     const int total_width = default_line_width;
//     const std::string line = sep + std::string(total_width - 1, '-') + '|';
//     std::cout << line << "\n" << sep << left;
//
//     size_t counter = 0;
//     while (counter < var_num - 1) {
//         cout << std::setw(name_width) << names[counter++] << sep;
//     }
//     cout << std::setw(name_width - 1) << names[counter] << sep;
//     cout << "\n" << line << "\n" + sep;
//
//
//     counter = 0;
//     while (counter < var_num - 1) {
//         cout << std::setw(name_width) << values[counter++] << sep;
//     }
//     cout << std::setw(name_width - 1) << values[counter] << sep;
//     cout << "\n" << line << "\n";*/
// }

auto att_print_single_round_false_positive_rates(size_t lookups_repetitions, size_t bits_per_item,
                                                 size_t false_positive_counter, size_t true_positive_counter) -> std::stringstream {
    std::stringstream ss;
    const size_t var_num = 6;
    string names[var_num] = {"#reps", "#expected FP", "#Actual FP", "#TP", "expected FP prob", "actual FP prob"};
    size_t num_of_expected_fp = (lookups_repetitions - true_positive_counter) >> bits_per_item;
    size_t values[var_num - 2] = {lookups_repetitions, num_of_expected_fp, false_positive_counter,
                                  true_positive_counter};
    size_t max_length = 24;
    for (auto &name : names) {
        max_length = max(name.length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const uint32_t total_width = (name_width + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << "\n"
       << sep << left;

    size_t counter = 0;
    while (counter < var_num) {
        ss << std::setw(name_width) << names[counter++] << sep;
    }
    ss << "\n"
       << line << "\n" + sep;


    counter = 0;
    while (counter < var_num - 2) {
        ss << std::setw(name_width) << values[counter++] << sep;
    }
    //    std::os << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    double expected_fp_prob = 1 / ((double) (1u << bits_per_item));
    double actual_fp_prob = false_positive_counter / ((double) (lookups_repetitions - true_positive_counter));
    assert(expected_fp_prob > 0);
    assert(expected_fp_prob < 1);
    assert(actual_fp_prob > 0);
    assert(actual_fp_prob < 1);

    ss << std::setw(name_width) << expected_fp_prob << sep;

    ss << std::setw(name_width) << actual_fp_prob << sep;
    ss << "\n"
       << line << "\n";
    return ss;
}


auto print_false_positive_rates_header(size_t width) -> std::stringstream {
    std::stringstream ss;
    const size_t var_num = 7;
    string names[var_num] = {"Filter", "#reps", "#expected FP", "#FP", "#TP", "expected FP ratio", "actual FP ratio"};
    size_t max_length = max(12ul, width);
    for (auto &name : names) {
        max_length = max(name.length(), max_length);
    }
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const uint32_t total_width = max((name_width + sep.size()) * var_num, 138ul);
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << "\n"
       << sep << left;

    size_t i = 0;
    size_t w = 18u;
    ss << std::setw(16) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << "\n"
       << line << "\n";
    return ss;

    // for (size_t i = 0; i < var_num; i++)
    // {
    //     cout << std::setw(name_width) << names[i] << sep;
    // }
}


auto print_single_round_false_positive_rates(std::string filter_name, size_t lookups_repetitions, size_t expected_false_positive,
                                             size_t true_positive_counter, size_t false_positive_counter, bool is_last_line) -> std::stringstream {
    /**every lookup result is one of the following:
     * FP - False Positive
     * TP - True Positive
     * TN - True Negative.
      */
    std::stringstream ss;
    const size_t var_num = 6;
    string names[var_num] = {"#reps", "#expected FP", "#FP", "#TP", "expected FP ratio", "actual FP ratio"};
    size_t bits_per_item = expected_false_positive;
    expected_false_positive = lookups_repetitions >> bits_per_item;
    size_t values[var_num - 2] = {lookups_repetitions, expected_false_positive, false_positive_counter,
                                  true_positive_counter};

    // values for controlling format
    const uint32_t name_width = 12;
    const std::string sep = " |";
    const uint32_t total_width = (24 + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';

    ss << sep << left;
    size_t width = 18;
    ss << std::setw(16) << filter_name << sep;
    size_t counter = 0;
    while (counter < var_num - 2) {
        ss << std::setw(width) << values[counter++] << sep;
    }
    //    std::ss << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    ss << std::setw(width) << 1ULL / ((double) (1ULL << bits_per_item)) << sep;
    ss << std::setw(width) << values[2] / ((double) values[0]) << sep << std::endl;

    if (is_last_line) {
        string closing_line = sep + std::string(137, '-') + '|';
        ss << closing_line << std::endl;
    }
    return ss;
}

auto temp_temp(std::string filter_name, size_t lookups_repetitions, size_t expected_false_positive,
               size_t true_counter, size_t false_counter) -> void {
    const size_t var_num = 6;
    string names[var_num] = {"#reps", "#expected FP", "#False", "#True", "expected FP ratio", "actual True ratio"};
    size_t bits_per_item = expected_false_positive;
    expected_false_positive = lookups_repetitions >> bits_per_item;
    size_t values[var_num - 2] = {lookups_repetitions, expected_false_positive, false_counter, true_counter};

    // std::cout
    // values for controlling format
    const uint32_t name_width = 12;
    const std::string sep = " |";
    const uint32_t total_width = (24 + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << "\n"
              << sep << left;

    size_t i = 0;
    size_t w = 18u;
    std::cout << std::setw(16) << names[0] << sep;
    std::cout << std::setw(w) << names[1] << sep;
    std::cout << std::setw(w) << names[2] << sep;
    std::cout << std::setw(w) << names[3] << sep;
    std::cout << std::setw(w) << names[4] << sep;
    std::cout << std::setw(w) << names[5] << sep;
    // std::cout << std::setw(w) << names[2] << sep;
    std::cout << "\n"
              << line << "\n";

    size_t width = 18;
    std::cout << std::setw(16) << filter_name << sep;
    size_t counter = 0;
    while (counter < var_num - 2) {
        std::cout << std::setw(width) << values[counter++] << sep;
    }
    //    std::std::cout << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    std::cout << std::setw(width) << 1ULL / ((double) (1ULL << bits_per_item)) << sep;
    std::cout << std::setw(width) << values[2] / ((double) values[0]) << sep << std::endl;

    // if (is_last_line) {
    string closing_line = sep + std::string(137, '-') + '|';
    std::cout << closing_line << std::endl;
    // }
}

auto print_single_round_false_positive_rates_probabilistic(std::string filter_name, size_t lookups_repetitions, size_t expected_false_positive,
                                                           size_t true_counter, size_t false_counter, bool is_last_line) -> std::stringstream {
    /**every lookup result is one of the following:
     * FP - False Positive
     * TP - True Positive
     * TN - True Negative.
      */
    // temp_temp(filter_name, lookups_repetitions, expected_false_positive, true_counter, false_counter);
    std::stringstream ss;
    const size_t var_num = 6;
    string names[var_num] = {"#reps", "#expected FP", "#False", "#True", "expected FP ratio", "actual True ratio"};
    size_t bits_per_item = expected_false_positive;
    expected_false_positive = lookups_repetitions >> bits_per_item;
    size_t values[var_num - 2] = {lookups_repetitions, expected_false_positive, false_counter, true_counter};

    // values for controlling format
    const uint32_t name_width = 12;
    const std::string sep = " |";
    const uint32_t total_width = (24 + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << "\n"
       << sep << left;

    size_t space_left = line.size() - 20 * 7 - 2;
    std::string close = std::string(space_left,' ') + sep;
    size_t i = 0;
    size_t w = 18u;
    ss << std::setw(w) << "Name" << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << sep;
    ss << std::setw(w) << names[i++] << close;
    // ss << std::setw(w) << names[i++] << sep;
    ss << "\n"
       << line << "\n";

    size_t width = 18;
    ss << sep << left << std::setw(w) << filter_name << sep;
    size_t counter = 0;
    while (counter < var_num - 2) {
        ss << std::setw(width) << values[counter++] << sep;
    }
    //    std::ss << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    ss << std::setw(width) << 1.0 / ((double) (1ULL << bits_per_item)) << sep;
    ss << std::setw(width) << 1.0 * true_counter / lookups_repetitions << close << std::endl;

    if (is_last_line) {
        string closing_line = sep + std::string(155, '-') + '|';
        ss << closing_line << std::endl;
    }
    return ss;
}


auto print_single_round_false_positive_rates(size_t lookups_repetitions, size_t expected_false_positive,
                                             size_t true_positive_counter, size_t false_positive_counter) -> std::stringstream {
    /**every lookup result is one of the following:
     * FP - False Positive
     * TP - True Positive
     * TN - True Negative.
      */
    std::stringstream ss;

    const size_t var_num = 6;
    string names[var_num] = {"#reps", "#expected FP", "#FP", "#TP", "expected FP ratio", "actual FP ratio"};
    size_t values[var_num - 2] = {lookups_repetitions, expected_false_positive, false_positive_counter,
                                  true_positive_counter};
    size_t max_length = 24;
    for (auto &name : names) {
        max_length = max(name.length(), max_length);
    }

    // values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const uint32_t total_width = (name_width + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    ss << line << "\n"
       << sep << left;

    size_t counter = 0;
    // while (counter < var_num) {
    //     cout << std::setw(name_width) << names[counter++] << sep;
    // }
    // cout << "\n" << line << "\n" + sep;


    // counter = 0;
    while (counter < var_num - 2) {
        ss << std::setw(name_width) << values[counter++] << sep;
    }
    //    os << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    ss << std::setw(name_width) << values[1] / ((double) values[0]) << sep;
    ss << std::setw(name_width) << values[2] / ((double) values[0]) << sep;
    ss << "\n"
       << line << "\n";
    return ss;
}

