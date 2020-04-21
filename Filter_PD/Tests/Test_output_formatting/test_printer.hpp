//
// Created by tomer on 4/12/20.
//

#ifndef CLION_CODE_TEST_PRINTER_HPP
#define CLION_CODE_TEST_PRINTER_HPP

#include "../../Global_functions/basic_functions.h"

namespace name_compare {

    void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors);


    void
    table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count);

}
#endif //CLION_CODE_TEST_PRINTER_HPP
