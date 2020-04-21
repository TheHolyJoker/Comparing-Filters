#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <fstream>
#include <functional>
//#include "New_BF/V_tests/tests.h"

using namespace std;


int main() {

    cout << "Start" << endl;

    size_t shift = 15;
    size_t exp_el = 1ULL << shift;
    double eps = 0.01;
    size_t insertions = 1ULL << shift;
    size_t lookups = 1ULL << (shift - 2);
    bool is_adaptive = false;
    bool call_adapt = false;
    stringstream s;


    /*bf_only_rates(exp_el, eps, insertions, lookups, false, false, s);
    bf_only_rates(exp_el, eps, insertions, lookups, false, false, s);
    bf_only_rates(exp_el, eps, insertions, lookups, false, false, s);
    bf_only_rates(exp_el, eps, insertions, lookups, false, false, s);
    bf_only_rates(exp_el, eps, insertions, lookups, false, false, s);*/
/*

    ////Without using the remote
    auto_rates(exp_el, eps, insertions, lookups, false, false, false, true, false); //not adaptive.
    auto_rates(exp_el, eps, insertions, lookups, true, false, false, true, false); //Adaptive.

    ////Using the remote.
    auto_rates(exp_el, eps, insertions, lookups, false, false, true, false, false); //not adaptive
    auto_rates(exp_el, eps, insertions, lookups, true, false, true, false, false); //Adaptive but doesn't call adapt.
    auto_rates(exp_el, eps, insertions, lookups, true, true, true, false, false); //Adaptive.
*/

    cout << "End" << endl;
    return 0;
}


