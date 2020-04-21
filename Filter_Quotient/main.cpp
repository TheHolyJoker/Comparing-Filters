#include <iostream>
#include "Hash/Hash.h"
#include "QuotientFilter/DataBase.h"
#include "Tests/DBtests.h"

int main() {
    cout << "Hello, World!" << endl;

//    srand(time(NULL));
    double loadFactor = 0.75;
    size_t q = 15, r = 6, numberOfElements = size_t(loadFactor * SL(q)), reps = SL(13ul);
//    t1(true);
//    t1(false);
    stringstream s;

    for (int i = 0; i < 32; ++i) {
        only_quotient_rates(q, r, loadFactor, reps, false, s);
    }
//    only_quotient_rates(q, r, loadFactor, reps, false, s);
//    only_quotient_rates(q, r, loadFactor, reps, false, s);
//    only_quotient_rates(q, r, loadFactor, reps, false, s);
//    only_quotient_rates(q, r, loadFactor, reps, false, s);
    /*DB_rates(q, r, loadFactor, reps, false, s);
    DB_rates(q, r, loadFactor, reps, false, s);
    DB_rates(q, r, loadFactor, reps, false, s);
    DB_rates(q, r, loadFactor, reps, true, s);
*/
//    cout << endl << endl << s.str();
//    DB_AdaptivenessTest(q, r, numberOfElements, reps);

    //    DB_TPLookup(q, r, numberOfElements, true);
//    for (int i = 0; i < 1000; ++i) {
//        DB_TPLookup(q, r, numberOfElements, false);
//    }
    cout << "End!" << endl;
}