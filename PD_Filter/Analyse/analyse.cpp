//
// Created by tomer on 31/07/2020.
//

#include "analyse.hpp"

auto lp_average(pd512_wrapper *pd_arr, size_t size, size_t p)  ->double{
    size_t sum = 0;
    for (int i = 0; i < size; ++i) {
        auto temp_pd = pd_arr[i];
        size_t temp_cap = temp_pd.get_capacity();
        sum += (p)? pow(temp_cap, p) : bool(temp_cap);
        // sum += pow(temp_cap, p);
    }
    return sum / (double) size;
}

auto m512i_lp_average(__m512i* pd_arr, size_t size, size_t p)  ->double{
    size_t sum = 0;
    for (int i = 0; i < size; ++i) {
        auto temp_pd = pd_arr[i];
        size_t temp_cap = pd512::get_capacity(&temp_pd);
        // if(p){
        //     sum += pow(temp_cap, p);
        // }
        // else{
        //     sum +=    bool(temp_cap);
        // }
        sum += (p)? pow(temp_cap, p) : bool(temp_cap);
    }
    return (p) ? sum / (double) size : sum;
    // return sum / (double) size;
}


