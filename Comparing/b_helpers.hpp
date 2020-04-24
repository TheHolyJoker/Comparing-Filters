//
// Created by tomer on 4/18/20.
//

#ifndef FILTER_COMPARISON_B_HELPERS_HPP
#define FILTER_COMPARISON_B_HELPERS_HPP


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
    auto rand_string(int minLength, int charsNum, int numOfDiffLength = 8) -> std::string;

    void set_init(size_t size, std::set<std::string> *mySet, int minLength = 8, int charsNum = 32);

    template<typename T>
    void set_init(size_t size, std::set<T> *mySet);

    template<typename T>
    void multiset_init(size_t size, std::multiset<T> *mySet, size_t universe_size);


}
#endif //FILTER_COMPARISON_B_HELPERS_HPP
