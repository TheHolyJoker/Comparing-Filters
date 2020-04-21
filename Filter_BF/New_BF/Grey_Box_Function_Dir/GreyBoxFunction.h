//
// Created by tomer on 9/20/18.
//

#ifndef ADAPTIVEBF_GLOBALFUNCTION_H
#define ADAPTIVEBF_GLOBALFUNCTION_H


#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <vector>
#include <math.h>
#include <assert.h>
#include <set>


#define DEFAULT_SEED 2
#define SECOND_SEED 42
#define DEFAULT_REL_OUTPUT_PATH "../New_BF/Outputs/"
#define DEFAULT_OUTPUT_FILE_SUFFIX ".out"


using namespace std;


double get_bitArray_size(size_t n, double eps);

size_t get_bitArray_size_ceil(size_t n, double eps);

/**
 *
 * @param a
 * @param b
 * @return natural random number in the range [a,b]
 */
size_t rangedUniformDistribution(size_t a, size_t b);

/**
 * Calculate the best size (m) for a BF with n elements and eps error rate.
 * @param n
 * @param eps
 * @return
 */
double calcM(size_t n, double eps);


/**
 * Calculate the best number (k) of hash function a BF with n elements and eps error rate,
 * should have.
 * @param n
 * @param eps
 * @return round up answer.
 */
size_t get_hashFunction_num(size_t n, size_t m);


/**
 * Using the description in the grey box.
 * @param n
 * @param previousEps
 * @return
 */
double get_next_level_bit_array_size(size_t n, double previousEps);


/**
 * How many levels should an adaptive BF with parameters n,eps have.
 * @param n
 * @param eps
 * @return
 */
size_t getDepth(size_t n, double eps);

/**
 * Set the elements in mList to have the optimal size for the adaptive BF in the i'th level.
 * @param n number of elements
 * @param eps error rate
 * @param mList a list with depth elements
 * @param depth the optimal number of levels an adaptive BF should have
 */
void get_bitArrays_size_list(size_t n, double eps, size_t *mList, size_t depth);

/**
 * Set the elements in kList to have the optimal number of hash functions an adaptive BF in the
 * i'th level should have, using mList, and an upper bound of the number of elements in the i'th level.
 * @param n number of elements
 * @param eps error rate
 * @param mList a list with depth elements
 * @param depth the optimal number of levels an adaptive BF should have
 */
void get_hashFunctions_num_list(size_t n, double eps, size_t *kList, size_t *mList, size_t depth);


ostream &print_array(size_t *a, size_t size, ostream &os = cout);

ostream &print_array(int *a, size_t size, ostream &os = cout);


#endif //ADAPTIVEBF_GLOBALFUNCTION_H
