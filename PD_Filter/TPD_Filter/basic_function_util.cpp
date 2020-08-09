//
// Created by tomereven on 18/07/2020.
//

#include "basic_function_util.h"

auto compute_number_of_PD(size_t max_number_of_elements, size_t max_capacity, double l1_load) -> size_t
{
    // cout << "here" << endl;
    double b = max_capacity * l1_load;
    // std::cout << "b is: "<< b << std::endl;
    // auto res = (std::size_t)ceil(max_number_of_elements / b);
    // std::cout << "res: "<< res << std::endl;
    //    std::cout << "res is: " << res << std::endl;
    return (std::size_t)ceil(max_number_of_elements / ((double) b));
}

auto compute_spare_element_size(size_t max_number_of_elements, float level1_load_factor,
    size_t pd_max_capacity, size_t quot_range, size_t rem_length) -> size_t
{
    size_t number_of_pd = compute_number_of_PD(max_number_of_elements, pd_max_capacity, level1_load_factor);
    size_t pd_index_length = ceil_log2(number_of_pd);
    size_t quot_range_length = ceil_log2(quot_range);
    return rem_length + pd_index_length + quot_range_length;

}

auto pd_filter_total_byte_size(size_t max_number_of_elements, size_t max_capacity, double l1_load, double l2_load) ->size_t {
    size_t number_of_pd = compute_number_of_PD(max_number_of_elements, max_capacity, l1_load);
    size_t l1_size = number_of_pd * (64 + 2);
    // size_t pd_index_length = ceil_log2(number_of_pd);
    // size_t spare_element_length = (6 + 6) + pd_index_length; 
    size_t log2_size = ceil_log2(max_number_of_elements);
    // size_t temp = ceil(max_number_of_elements *1);
    auto number_of_buckets = ceil(max_number_of_elements / log2_size);
    size_t bucket_size = 4u;
    size_t spare_element_byte_size = 8u;
    size_t spare_size = number_of_buckets * bucket_size * spare_element_byte_size;
    return l1_size + spare_size;
}
