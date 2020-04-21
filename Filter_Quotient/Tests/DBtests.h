//
// Created by tomer on 11/7/18.
//

#ifndef NEWQF_DBTESTS_H
#define NEWQF_DBTESTS_H

#include "../QuotientFilter/DataBase.h"
#include <set>

#define TIME_PASSED_FROM(t) (double(clock()-(t))/CLOCKS_PER_SEC)

string randString(int minWordLength = 8, int charsNum = 32, int numOfDiffLength = 8);

void setInit(size_t size, set<string> *mySet, int minWordLength = 8, int charsNum = 32);

void t1(bool isAdaptive);

bool DB_TPLookup(size_t q, size_t r, size_t elNum, bool isAdaptive);

void
DB_AdaptivenessTest(size_t q, size_t r, size_t elementNum, size_t lookupReps);

ostream &
only_quotient_rates(size_t q, size_t r, double loadFactor, size_t lookupReps, bool isAdaptive, ostream &os = cout);

ostream &
DB_rates(size_t q, size_t r, double loadFactor, size_t lookupReps, bool isAdaptive, ostream &os = cout);


ostream &
getStats(double qfInitTime, double memberSetInitTime, double lookupSetInitTime, double insertionTime, double lookupTime,
         size_t memberSetSize, size_t lookupReps, int *counter, ostream &os);


void counterPrinter(int *counter, ostream &os);



#endif //NEWQF_DBTESTS_H
