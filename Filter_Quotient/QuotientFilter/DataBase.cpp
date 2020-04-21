//
// Created by tomer on 11/7/18.
//

#include "DataBase.h"

DataBase::DataBase(size_t q, size_t r, bool isAdaptive) : isAdaptive(isAdaptive) {
    if (!isAdaptive) {
        depth = 1;
        filtersVec.push_back(new Filter_QF(q, r, 3));
    } else {
        depth = calcDepthByQ(q, r);
        filtersVec.resize(depth);
        size_t qArr[depth];
        size_t rArr[depth];
        qArr[0] = q;
        rArr[0] = r;
        getArraysSizes(qArr, rArr, depth);
        for (int i = 0; i < depth; ++i)
            filtersVec[i] = new Filter_QF(qArr[i], rArr[i], 4);
    }
}


DataBase::~DataBase() {
    for (auto vec: filtersVec)
        delete vec;
}

void DataBase::add(string *s, size_t level) {
    if (!isAdaptive) {
        nonAdaptiveAdd(s);
        return;
    }

    for (size_t i = level; i < depth; ++i) {
        KEY_TYPE key = filtersVec[i]->insert(s);
        if (get<1>(key).capacity() != 0)
            return memberMap[key].push_back(*s);
    }
    assert(false);
}

bool DataBase::nonAdaptiveAdd(string *s) {
    KEY_TYPE key = filtersVec[0]->insert(s);
    assert(get<1>(key).capacity() != 0);
    memberMap[key].push_back(*s);
}

int DataBase::lookup(string *s) {
    if (!isAdaptive)
        return nonAdaptiveLookup(s);

    size_t fpIndex = 0;
    for (size_t i = 0; i < depth; ++i) {
        int res = filtersVec[i]->lookup(s, &fpIndex);
        if (res == 0)
            return 0;
        if (res == 1) {
            KEY_TYPE key = filtersVec[i]->strToKey(s);
            assert(memberMap.count(key));
            vector<string> tempVec = memberMap[key];
            if (vectorFind(&tempVec, s))
                return 2;
            adapt(&tempVec, fpIndex, i);
            return 1;
        }
    }
    assert(false);
}

int DataBase::nonAdaptiveLookup(string *s) {
    int res = filtersVec[0]->lookup(s, nullptr);
    if (res == 0)
        return 0;
    if (res == 1)
    {
        KEY_TYPE key = filtersVec[0]->strToKey(s);
        assert(memberMap.count(key));
        vector<string> tempVec = memberMap[key];
        if (vectorFind(&tempVec, s))
            return 2;
        return 1;
    }
    assert(false);
}

void DataBase::adapt(vector<string> *elementVec, size_t fpIndex, size_t level) {
    filtersVec[level]->setFP(fpIndex);
    ++level;
    for (auto s: *elementVec)
        add(&s, level);
}
