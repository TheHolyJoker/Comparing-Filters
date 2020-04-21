//
// Created by tomer on 11/2/18.
//

#include "BasicFilter.h"

BasicFilter::BasicFilter(size_t q, size_t r, size_t MBSize) : BitArray(q, r, MBSize),
                                                              defaultSlotsPerElement(
                                                                      (INTEGER_ROUND((r + MBSize), BLOCK_SIZE))) {}

int BasicFilter::lookup(size_t qIndex, BLOCK_TYPE *dataArray, size_t *fpIndex) {
    if (!isOccupied(getMB(qIndex)))
        return 0;
    size_t runIndex = getCompatibleRun(qIndex);

    int res = compare(runIndex, dataArray);
    if (res == 0)
        return lookupFPCondition(runIndex, fpIndex);

    inc(&runIndex);
    while (isContinuation(getMB(runIndex))) {
        res = compare(runIndex, dataArray);
        if (res == 0)
            return lookupFPCondition(runIndex, fpIndex);
        inc(&runIndex);
    }
    return 0;
}

bool BasicFilter::insert(size_t qIndex, BLOCK_TYPE *dataArray) {
    size_t runIndex = getCompatibleRun(qIndex);

    /*Inserting element to an empty slot, could be shifted*/
    if (isEmpty(getMB(runIndex))) {
        auto tempMB = (qIndex != runIndex) ? BLOCK_TYPE(MB_BIT(3)) : BLOCK_TYPE(MB_BIT(1));
        setCell(runIndex, dataArray, defaultSlotsPerElement, tempMB);
        setOccupied(qIndex, true);
        return true;
    }

    /*Inserting element to a new run, must be shifted. (require push)*/
    if (!isOccupied(getMB(qIndex))) {
        setOccupied(qIndex, true);

        push(runIndex);

        BLOCK_TYPE tempMB = MB_BIT(3); // Set is_shifted on.
        if (isOccupied(getMB(runIndex)))
            tempMB |= MB_BIT(1);
        setCell(runIndex, dataArray, defaultSlotsPerElement, tempMB);
        return true;
    }

    /*If element shouldn't be first in it's run. (isn't smallest in its run). */
    return runAdd(runIndex, dataArray);
}

bool BasicFilter::runAdd(size_t runIndex, BLOCK_TYPE *dataArray) {
    int res = compare(runIndex, dataArray);
    if (res == -1) {
        inc(&runIndex);
        while (isContinuation(getMB(runIndex))) {
            res = compare(runIndex, dataArray);
            if (res != -1)
                break;
            else
                inc(&runIndex);
        }
        if (res == 0 and addFPCondition(runIndex))
            return false;

        push(runIndex, true);
        BLOCK_TYPE tempMB = MB_BIT(2) | MB_BIT(3); // Continuation and Shifted.
        if (isOccupied(getMB(runIndex)))
            tempMB |= MB_BIT(1);
        setCell(runIndex, dataArray, defaultSlotsPerElement, tempMB);

    } else {
        if (res == 0 and addFPCondition(runIndex))
            return false;

        BLOCK_TYPE tempMB = getMB(runIndex);
        if (isFP(tempMB)) {
            cout << "in runAdd, copied mb with fp when shouldn't, then fixed it;" << endl;
            --tempMB;
        }
        push(runIndex);
        setCell(runIndex, dataArray, defaultSlotsPerElement, tempMB);
        inc(&runIndex);
        setShifted(runIndex);
        setContinuation(runIndex);
    }

    return true;
}

void BasicFilter::push(size_t qIndex, bool toPushAgain) {
    if (isEmpty(getMB(qIndex))) return;
    size_t emptyQIndex, runCounter = 0;
    getNextEmpty(qIndex, &emptyQIndex, &runCounter);
    for (int i = 0; i < runCounter; ++i) {
        size_t tempIndex = dec(emptyQIndex);
        size_t tempRunStart = getRunStart(tempIndex);
        emptyQIndex = runShift(emptyQIndex, tempRunStart);
    }
    if (toPushAgain)
        runShift(emptyQIndex, qIndex);
}

size_t BasicFilter::runShift(size_t emptyQIndex, size_t qIndex) {
    BLOCK_TYPE dataArray[defaultSlotsPerElement];
    while (emptyQIndex != qIndex) {
        size_t temp = dec(emptyQIndex);
        getCell(temp, dataArray, defaultSlotsPerElement);
        (isOccupied(getMB(emptyQIndex))) ?
                dataArray[defaultSlotsPerElement - 1] |= MB_BIT(1) : dataArray[defaultSlotsPerElement - 1] &= ~(MB_BIT(
                1));
        (isFP(getMB(emptyQIndex))) ?
                dataArray[defaultSlotsPerElement - 1] |= MB_BIT(4) : dataArray[defaultSlotsPerElement - 1] &= (~(MB_BIT(
                4)));
        setCell(emptyQIndex, dataArray, defaultSlotsPerElement);
        emptyQIndex = temp;
    }

    if (isRunStart(getMB(qIndex)))
        setShifted(inc(qIndex), true);

    return emptyQIndex;
}

size_t BasicFilter::getClusterStart(size_t qIndex, size_t *occupiedCounter) {
    if (isEmpty(getMB(qIndex)))
        return qIndex;
    while (!isClusterStart(getMB(qIndex))) {
        dec(&qIndex);
        if (isOccupied(getMB(qIndex)))
            ++*occupiedCounter;
    }
    return qIndex;
}

size_t BasicFilter::getRunStart(size_t qIndex) {
    while (!isRunStart(getMB(qIndex))) {
        dec(&qIndex);
    }
    return qIndex;
}

size_t BasicFilter::getNextRunStart(size_t qIndex) {
    if (isRunStart(getMB(qIndex)))
        inc(&qIndex);
    while (isContinuation(getMB(qIndex)))
        inc(&qIndex);
    return qIndex;
}

size_t BasicFilter::getCompatibleRun(size_t qIndex) {
    size_t occupiedCounter = 0;
    size_t tempRunStart = getClusterStart(qIndex, &occupiedCounter);
    for (int i = 0; i < occupiedCounter; ++i)
        tempRunStart = getNextRunStart(tempRunStart);
    return tempRunStart;
}

void BasicFilter::getNextEmpty(size_t qIndex, size_t *emptyIndex, size_t *runCounter) {
    while (!isEmpty(getMB(qIndex))) {
        *runCounter += isRunStart(getMB(qIndex));
        inc(&qIndex);
    }
    *emptyIndex = qIndex;
}

int BasicFilter::compare(size_t qIndex, BLOCK_TYPE *dataArray) {
    BLOCK_TYPE qIndexData[defaultSlotsPerElement];
    getCell(qIndex, qIndexData, defaultSlotsPerElement);
    compareHelper(qIndexData, defaultSlotsPerElement);
    for (int i = 0; i < defaultSlotsPerElement; ++i) {
        if (qIndexData[i] != dataArray[i])
            return (qIndexData[i] < dataArray[i]) ? -1 : 1;
    }
    return 0;
}

void BasicFilter::compareHelper(BLOCK_TYPE *arrayToFix, size_t arraySize) {
    auto bitsLeft = (dataSize + MB) % BLOCK_SIZE;
    if (bitsLeft > MB)
        arrayToFix[arraySize - 1] >>= MB;
    else {
        arrayToFix[arraySize - 1] = 0;
        if (MB < bitsLeft) {
            auto shiftRight = MB - bitsLeft;
            arrayToFix[arraySize - 2] >>= shiftRight;
        }
    }
}

bool BasicFilter::addFPCondition(size_t qIndex) {
    return isFP(getMB(qIndex));
}

int BasicFilter::lookupFPCondition(size_t qIndex, size_t *fpIndex) {
    if(MB != 4) // TODO
        return 1; // TODO
    if (isFP(this->getMB(qIndex)))
        return -1;
    *fpIndex = qIndex;
    return 1;
}

inline void BasicFilter::inc(size_t *index) {
    if (*index == capacity - 1)
        *index = 0;
    else
        ++*index;
}

inline size_t BasicFilter::inc(size_t index) {
    if (index == capacity - 1)
        return 0;
    return ++index;
}

inline void BasicFilter::dec(size_t *index) {
    if (*index == 0)
        *index = capacity - 1;
    else
        --*index;
}

inline size_t BasicFilter::dec(size_t index) {
    if (index == 0)
        return capacity - 1;
    return --index;
}
