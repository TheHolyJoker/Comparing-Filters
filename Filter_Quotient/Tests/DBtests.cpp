//
// Created by tomer on 11/7/18.
//

#include "DBtests.h"

string randString(int minWordLength, int charsNum, int numOfDiffLength) {
    int len = minWordLength bitor (rand() % numOfDiffLength);
    assert(4 < len);
    char s[len];
    for (int i = 0; i < len; ++i) s[i] = 64 bitor (rand() % charsNum);
    s[len] = 0;
    string res(s);
    return res;
}

void setInit(size_t size, set<string> *mySet, int minWordLength, int charsNum) {
    for (int i = 0; i < size; ++i) mySet->insert(randString(minWordLength, charsNum));
    if (mySet->size() / double(size) < 1)
        cout << "setInit ratio is: " << (mySet->size() / double(size) < 1) << endl;
}

void t1(bool isAdaptive) {
    DataBase d(8, 5, isAdaptive);
    string s1 = "Tomer";
    d.add(&s1);
    assert(d.lookup(&s1) == 2);
}

bool DB_TPLookup(size_t q, size_t r, size_t elNum, bool isAdaptive) {
    assert(elNum <= SL(q));
    set<string> memberSet;
    setInit(elNum, &memberSet);
    DataBase db(q, r, isAdaptive);
    size_t counter = 0;
    for (auto s: memberSet) {
        db.add(&s);
        int res = db.lookup(&s);
        if (res == 0)
            db.lookup(&s);
        ++counter;
    }

    counter = 0;
    for (auto s: memberSet) {
        if (!db.lookup(&s)) {
            db.lookup(&s);
            assert(false);
        }
        ++counter;
    }
    cout << "Pass tp_lookup" << endl;
}

void DB_AdaptivenessTest(size_t q, size_t r, size_t elementNum, size_t lookupReps) {
    DataBase db(q, r, true);
    set<string> memberSet;
    setInit(elementNum, &memberSet);

    for (auto s: memberSet)
        db.add(&s);

//    db.statisticsPrint();

    set<string> lookupSet;
    setInit(lookupReps, &lookupSet);
    int counter[3] = {0};
    for (auto s: lookupSet)
        ++counter[db.lookup(&s)];
    counterPrinter(counter, cout);
    cout << 1.0 / (1ULL << r) << endl;
//    db.statisticsPrint();


    counter[0] = counter[1] = counter[2] = 0;
    for (auto s: lookupSet)
        ++counter[db.lookup(&s)];
    counterPrinter(counter, cout);
    cout << 0 << endl;

    for (auto s: memberSet)
        assert(db.lookup(&s) == 2);
}


ostream &only_quotient_rates(size_t q, size_t r, double loadFactor, size_t lookupReps, bool isAdaptive, ostream &os) {
    string name =  "Not adaptive:";
//    os << name << endl;
//    cout << name << endl;
    clock_t t0 = clock(), temp;
    DataBase db(q, r, false);
    double qfInitTime = TIME_PASSED_FROM(t0);

    size_t setSize = size_t(ceil(SL(q) * loadFactor));
    set<string> memberSet, lookupSet;
    temp = clock();
    setInit(setSize, &memberSet);
    double memberSetInitTime = TIME_PASSED_FROM((temp));

    temp = clock();
    setInit(lookupReps, &lookupSet);
    double lookupSetInitTime = TIME_PASSED_FROM((temp));

    temp = clock();
    for (auto s: memberSet) db.nonAdaptiveAdd(&s);
    double insertionTime = TIME_PASSED_FROM((temp));

//    db.statusCorrectness(true, os);

    /*temp = clock();
    for (auto s: lookupSet)
        db.lookup(&s, vLength);
    cout << "First vlLookup time is: " << TIME_PASSED_FROM((temp)) << endl;*/


    // [TN, FP, TP]
    int counter[3] = {0};
    temp = clock();
    for (auto s: lookupSet) db.nonAdaptiveLookup(&s);

    double lookupTime = TIME_PASSED_FROM((temp));


    getStats(qfInitTime, memberSetInitTime, lookupSetInitTime, insertionTime, lookupTime,
             setSize, lookupReps, counter, os);
    os << 1.0 / SL(r + q) << endl;
    os << "Total run time: " << TIME_PASSED_FROM(t0) << endl;

//    db.statisticsPrint(os);

    return os;
}

ostream &DB_rates(size_t q, size_t r, double loadFactor, size_t lookupReps, bool isAdaptive, ostream &os) {
    string name = (isAdaptive) ? "Adaptive:" : "Not adaptive:";
    os << name << endl;
    cout << name << endl;
    clock_t t0 = clock(), temp;
    DataBase db(q, r, isAdaptive);
    double qfInitTime = TIME_PASSED_FROM(t0);

    size_t setSize = size_t(ceil(SL(q) * loadFactor));
    set<string> memberSet, lookupSet;
    temp = clock();
    setInit(setSize, &memberSet);
    double memberSetInitTime = TIME_PASSED_FROM((temp));

    temp = clock();
    setInit(lookupReps, &lookupSet);
    double lookupSetInitTime = TIME_PASSED_FROM((temp));

    temp = clock();
    for (auto s: memberSet) {
        db.add(&s);
    }
    double insertionTime = TIME_PASSED_FROM((temp));

//    db.statusCorrectness(true, os);

    /*temp = clock();
    for (auto s: lookupSet)
        db.lookup(&s, vLength);
    cout << "First vlLookup time is: " << TIME_PASSED_FROM((temp)) << endl;*/


    // [TN, FP, TP]
    int counter[3] = {0};
    temp = clock();
    for (auto s: lookupSet) {
        ++counter[db.lookup(&s)];
//        db.statusCorrectness(true, os);
    }
    double lookupTime = TIME_PASSED_FROM((temp));


    getStats(qfInitTime, memberSetInitTime, lookupSetInitTime, insertionTime, lookupTime,
             setSize, lookupReps, counter, os);
    os << 1.0 / SL(r + q) << endl;
    os << "Total run time: " << TIME_PASSED_FROM(t0) << endl;

//    db.statisticsPrint(os);

    return os;
}

ostream &
getStats(double qfInitTime, double memberSetInitTime, double lookupSetInitTime, double insertionTime, double lookupTime,
         size_t memberSetSize, size_t lookupReps, int *counter, ostream &os) {
    os << "qfInitTime: " << qfInitTime << endl;
    os << "memberSetInitTime: " << memberSetInitTime << endl;
    os << "lookupSetInitTime: " << lookupSetInitTime << endl;
    double insertionRatio = memberSetSize / insertionTime;
    os << "insertionTime: " << insertionTime << "\tSpeed: " << insertionRatio << " el/s" << endl;
    cout << "insertionTime: " << insertionTime << "\tSpeed: " << insertionRatio << " el/s.\t";
    double lookupRatio = lookupReps / lookupTime;
    os << "lookupTime: " << lookupTime << "\tSpeed: " << lookupRatio << " el/s" << endl;
    cout << "lookupTime: " << lookupTime << "\tSpeed: " << lookupRatio << " el/s" << endl;
    counterPrinter(counter, os);
    return os;
}


void counterPrinter(int *counter, ostream &os) {
    os << "[TN, FP, TP]" << "\t";
    os << "[" << counter[0] << ", " << counter[1] << ", " << counter[2] << "]";
    double sum = counter[0] + counter[1] + counter[2];
    os << "\t FP ratio:" << (counter[1] / sum) << "\t expected: ";
}
