//
// Created by tomer on 11/7/18.
//

#ifndef INHERITANCE_FILTER_H
#define INHERITANCE_FILTER_H

#include "../Hash/Hash.h"
#include "BasicFilter.h"

#include <tuple>


typedef tuple<size_t, vector<BLOCK_TYPE>> KEY_TYPE;

class Filter_QF {
    BasicFilter bf;
    size_t q, r;
    QF_Hash::Hash indexHash;
    vector<QF_Hash::Hash> hashVec;

public:
    Filter_QF(size_t q, size_t r, size_t MBSize);

    auto insert(const string *s) -> KEY_TYPE;

    auto lookup(const string *s, size_t *fpIndex) -> int;

    auto lookup(const string *s) -> int;

    auto strToKey(const string *s) -> KEY_TYPE;

    void setFP(size_t qIndex);

private:
    void strToData(const string *s, size_t *qIndex, BLOCK_TYPE *dataArray);

};


#endif //INHERITANCE_FILTER_H
