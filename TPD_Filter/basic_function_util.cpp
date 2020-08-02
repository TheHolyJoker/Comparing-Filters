//
// Created by tomereven on 18/07/2020.
//

#include "basic_function_util.h"


auto compute_number_of_PD(size_t max_number_of_elements, size_t max_capacity, double l1_load) -> size_t {
    // cout << "here" << endl;
    double b = max_capacity * l1_load;
    // std::cout << "b is: "<< b << std::endl;
    auto res = (std::size_t) ceil(max_number_of_elements / b);
    // std::cout << "res: "<< res << std::endl;
//    std::cout << "res is: " << res << std::endl;
    return (std::size_t) ceil(max_number_of_elements / b);
}


