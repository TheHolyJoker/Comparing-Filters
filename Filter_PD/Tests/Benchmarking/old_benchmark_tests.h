/*
//
// Created by tomer on 10/29/19.
//

#ifndef CLION_CODE_OLD_BENCHMARK_TESTS_H
#define CLION_CODE_OLD_BENCHMARK_TESTS_H

//
// Created by tomer on 4/15/19.
//

#include <set>
#include <fstream>

#include "../Global_functions/basic_functions.h"
#include "../PD/PD.h"
#include "../Safe_PD/safe_PD.h"


#define MIN_ELEMENT_LENGTH 8
#define MIN_CHAR_RANGE 32
#define MIN_LENGTH_RANGE 8


string rand_string(int minLength, int charsNum, int numOfDiffLength = MIN_LENGTH_RANGE);

void set_init(size_t size, set<string> *mySet, int minLength = MIN_ELEMENT_LENGTH, int charsNum = MIN_CHAR_RANGE);

bool naive_true_positive_validation(GeneralBF *vbp, size_t reps);

bool empty_true_negative_validation(GeneralBF *vbp, size_t reps);


ostream &validate(size_t expected_el_num, double eps, size_t insertion_num, size_t lookup_reps, bool is_adaptive,
                  bool call_adapt, ostream &os);


ostream &
validate_adaptiveness(size_t expected_el_num, double eps, size_t insertion_num, size_t lookup_reps, size_t fp_reps,
                      ostream &os);


*/
/**
 *
 * @param n maximal number of elements expected to be in the filter.
 * @param eps expected false positive rate
 * @param insertion_reps Actual number of elements that will be inserted.
 * @param lookup_reps Number of lookups that will be performed.
 * @param is_adaptive Is filter adaptive.
 * @param call_adapt Determine whether adapt operation will be called or not (only on adaptive filter)
 * @param remote_opt An attempt to improve Remote use.
 * @param naive_check Performs test without using the Remote.
 * @param to_cout Print the full test results to cout or to files in Output directory.
 *//*

void auto_rates(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
                bool remote_opt, bool naive_check, Remote_name remote_name, bool to_cout);

ostream &
rates(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
      Remote_name remote_name, ostream &os);

ostream &
rates_attempt(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
              Remote_name remote_name, ostream &os);


ostream &
naive_rates(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
            Remote_name remote_name, ostream &os);


ostream &
test_printer(size_t n, double eps, size_t lookups_num, bool is_adaptive, double set_ratio, int *counter,
             double member_set_init_time,
             double nom_set_init_time, double init_time, double insertion_time, double lookup_time,
             double total_run_time, ostream &os);


ostream &counter_printer(ostream &os, int *counter);

double get_relative_deviation(double act_res, double exp_res);

string test_get_output_file_path(bool is_adaptive, bool call_adapt);

size_t test_count_files(const string &path);



#endif //CLION_CODE_OLD_BENCHMARK_TESTS_H
*/
