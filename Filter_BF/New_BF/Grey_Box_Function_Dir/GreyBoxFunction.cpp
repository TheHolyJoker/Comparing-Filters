//
// Created by tomer on 9/20/18.
//

#include "GreyBoxFunction.h"


double get_bitArray_size(size_t n, double eps) {
    return (n * log(eps) / (M_LN2 * M_LN2)) * (-1);
}

size_t  get_bitArray_size_ceil(size_t n, double eps) {
    double temp = get_bitArray_size(n, eps);
    return size_t(temp + 1);
}


size_t rangedUniformDistribution(size_t a, size_t b) {
    const int range_from = a;
    const int range_to = b;
    random_device rand_dev;
    mt19937 generator(rand_dev());
    uniform_int_distribution<int> distr(range_from, range_to);
    return (size_t) distr(generator);
}

double get_next_level_bit_array_size(size_t n, double previousEps) {
    return (n * (previousEps * M_LOG2E) * (1 + 1.0 / log2(n)) * (log2(1.0 / previousEps)));
}

//double calcM(size_t n, double eps) {
//    return (n * log(eps) / (M_LN2 * M_LN2)) * (-1);
//}

size_t get_hashFunction_num(size_t n, size_t m) {
    auto temp = size_t(round(M_LN2 * ((double) m) / n));
    if (temp <= 1)
        return 1;
    return temp;
//    return (temp) ? temp : 1;
//    return (unsigned int) (M_LN2 * ((float) m) / n);
}

size_t getDepth(size_t n, double eps) {
    size_t depth = 0;
    const double WHP_Const = 1 + 1 / log2(n);

    auto tempN = (double) n;
    double tempM = get_bitArray_size(n, eps);

    while (tempM > 1 && tempN > 1) {
        ++depth;
        tempM = get_next_level_bit_array_size(n, eps);
        tempN = WHP_Const * (tempN * eps);
        eps *= eps;
    }
    return depth;
}

void get_bitArrays_size_list(size_t n, double eps, size_t *mList, size_t depth) {
    mList[0] = (size_t) get_bitArray_size(n, eps);
    for (int i = 1; i < depth; ++i) {
        mList[i] = (size_t) get_next_level_bit_array_size(n, eps);
        eps *= eps;
    }
//    mList[depth - 1] = mList[depth - 2];
}

void get_hashFunctions_num_list(size_t n, double eps, size_t *kList, size_t *mList, size_t depth) {
    kList[0] = get_hashFunction_num(n, mList[0]);
    size_t n_i = n;
    double upperConst = 1 + 1 / log2(n);
    for (int i = 1; i < depth; ++i) {
        double exp_i = n_i * eps;
        n_i = (size_t) (exp_i * upperConst + 1);
        kList[i] = get_hashFunction_num(n_i, mList[i]);
    }
//    kList[depth - 1] = kList[depth - 2];

}

ostream &print_array(size_t *a, size_t size, ostream &os) {
    os << "[" << a[0];
    for (int i = 1; i < size; ++i)
        os << ", " << a[i];

    os << "]" << endl;
    return os;

}

ostream &print_array(int *a, size_t size, ostream &os) {
    os << "[" << a[0];
    for (int i = 1; i < size; ++i)
        os << ", " << a[i];

    os << "]" << endl;
    return os;
}