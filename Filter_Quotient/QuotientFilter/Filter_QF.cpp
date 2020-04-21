//
// Created by tomer on 11/7/18.

#include "Filter_QF.h"

Filter_QF::Filter_QF(size_t q, size_t r, size_t MBSize) : q(q), r(r), bf(q, r, MBSize), indexHash(),
                                                          hashVec(bf.defaultSlotsPerElement, QF_Hash::Hash()) {
}

KEY_TYPE Filter_QF::insert(const string *s) {
    size_t qIndex;
    BLOCK_TYPE dataArray[bf.defaultSlotsPerElement];
    strToData(s, &qIndex, dataArray);

    if (!bf.insert(qIndex, dataArray))
        return KEY_TYPE();
    vector<BLOCK_TYPE> tempVec(dataArray, dataArray + bf.defaultSlotsPerElement);
    return KEY_TYPE(qIndex, tempVec);
}

int Filter_QF::lookup(const string *s, size_t *fpIndex) {
    size_t qIndex;
    BLOCK_TYPE dataArray[bf.defaultSlotsPerElement];
    strToData(s, &qIndex, dataArray);
    return bf.lookup(qIndex, dataArray, fpIndex);
}

auto Filter_QF::lookup(const string *s) -> int {
    std::size_t fpIndex = -1;
    return lookup(s, &fpIndex);
}


KEY_TYPE Filter_QF::strToKey(const string *s) {
    size_t qIndex;
    BLOCK_TYPE dataArray[bf.defaultSlotsPerElement];
    strToData(s, &qIndex, dataArray);
    vector<BLOCK_TYPE> tempVec(dataArray, dataArray + bf.defaultSlotsPerElement);
    return KEY_TYPE(qIndex, tempVec);
}

void Filter_QF::strToData(const string *s, size_t *qIndex, BLOCK_TYPE *dataArray) {
    if (bf.defaultSlotsPerElement == 1) {
        auto tempVal = uint32_t(indexHash.hash(s) % SL(q + r));
        *qIndex = tempVal >> r;
        *dataArray = BLOCK_TYPE(tempVal % SL(r));
        return;
    }
    *qIndex = indexHash.hash(s) >> (BLOCK_SIZE - q);

    for (int i = 0; i < bf.defaultSlotsPerElement; ++i)
        dataArray[i] = hashVec[i].hash(s);

    bf.compareHelper(dataArray, bf.defaultSlotsPerElement);
}

void Filter_QF::setFP(size_t qIndex) {
    bf.setFP(qIndex);
}



