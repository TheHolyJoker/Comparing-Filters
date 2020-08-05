//
// Created by tomer on 31/07/2020.
//

#ifndef COMPARING_FILTERS_SIMPLER_ANALYSE_HPP
#define COMPARING_FILTERS_SIMPLER_ANALYSE_HPP

#include "../TPD_Filter/pd512_wrapper.hpp"
#include <cmath>

auto lp_average(pd512_wrapper* pd_arr, size_t size, size_t p) ->double;

auto m512i_lp_average(__m512i* pd_arr, size_t size, size_t p) ->double;
#endif //COMPARING_FILTERS_SIMPLER_ANALYSE_HPP
