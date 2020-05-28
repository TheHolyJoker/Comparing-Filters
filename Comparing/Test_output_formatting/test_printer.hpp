//
// Created by tomer on 4/12/20.
//

#ifndef CLION_CODE_TEST_PRINTER_HPP
#define CLION_CODE_TEST_PRINTER_HPP

//#include "../../Global_functions/basic_functions.h"

#include <cstddef>
#include <zconf.h>
#include <iostream>
#include <set>

#include <fstream>
#include <tuple>
#include <random>
#include <iomanip>
#include <cassert>


namespace name_compare {
    static const size_t default_line_width = 116;

    void print_name(std::string filter_name);

    void table_print_rates(size_t var_num, std::string *var_names, size_t *values, size_t *divisors);


    void
    table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);

}
#endif //CLION_CODE_TEST_PRINTER_HPP
