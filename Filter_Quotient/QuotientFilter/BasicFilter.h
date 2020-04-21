//
// Created by tomer on 11/2/18.
//

#ifndef INHERITANCE_BASICFILTER_H
#define INHERITANCE_BASICFILTER_H


#include "BitArray.h"

class BasicFilter : public BitArray {

public:

    size_t defaultSlotsPerElement;

    BasicFilter(size_t q, size_t r, size_t MBSize);

    bool insert(size_t qIndex, BLOCK_TYPE *dataArray);

    int lookup(size_t qIndex, BLOCK_TYPE *dataArray, size_t *fpIndex);

    void compareHelper(BLOCK_TYPE *arrayToFix, size_t arraySize);


protected:

    void push(size_t qIndex, bool toPushAgain = false);

    size_t runShift(size_t emptyQIndex, size_t qIndex);

    size_t getCompatibleRun(size_t qIndex);

    size_t getClusterStart(size_t qIndex, size_t *occupiedCounter);

    size_t getRunStart(size_t qIndex);

    size_t getNextRunStart(size_t qIndex);

    void getNextEmpty(size_t qIndex, size_t *emptyIndex, size_t *runCounter);

    bool runAdd(size_t runIndex, BLOCK_TYPE *dataArray);

    int compare(size_t qIndex, BLOCK_TYPE *dataArray);

    virtual bool addFPCondition(size_t qIndex);

    virtual int lookupFPCondition(size_t qIndex, size_t *fpIndex);

private:
//    bool insert(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize);

    void inc(size_t *index);

    size_t inc(size_t index);

    void dec(size_t *index);

    size_t dec(size_t index);

};


#endif //INHERITANCE_BASICFILTER_H
