//
// Created by tomer on 11/7/18.
//

#ifndef INHERITANCE_BITARRAY_H
#define INHERITANCE_BITARRAY_H

#include "../GlobalFunctions/globalFunction.h"

#define BLOCK_SIZE (32)
#define BLOCK_TYPE uint32_t

//#define SL(p) (1ULL <<(p))
//#define MASK(p)  ( (1ULL <<(p))  - 1ULL)
#define MB_BIT(n) (1ULL << (MB - (n)))
#define ON(a, b) (MASK(a) ^ MASK(b))
#define OFF(a, b) (~(MASK(a) ^ MASK(b)))
//#define MOD_INVERSE(n) (BLOCK_SIZE - (n) - 1)


using namespace std;

class BitArray {
protected:
    size_t size;
    size_t capacity;
    size_t dataSize;
    size_t MB;
    BLOCK_TYPE *A;

public:
    BitArray(size_t q, size_t r, size_t MBSize);

    ~BitArray();

    void setFP(size_t qIndex, bool setOn = true);

    size_t getMBSize() const;

protected:

    void getCell(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize);

//    void getData(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize);

    BLOCK_TYPE getMB(size_t qIndex);

    void setCell(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize);

    void setCell(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize, BLOCK_TYPE tempMB);

    bool isEmpty(BLOCK_TYPE tempMB);

    bool isOccupied(BLOCK_TYPE tempMB);

    bool isContinuation(BLOCK_TYPE tempMB);

    bool isShifted(BLOCK_TYPE tempMB);

    bool isFP(BLOCK_TYPE tempMB);

    bool isPartOfRemainder(BLOCK_TYPE tempMB);

    bool isRunStart(BLOCK_TYPE tempMB);

    bool isClusterStart(BLOCK_TYPE tempMB);

    void setOccupied(size_t qIndex, bool setOn = true);

    void setContinuation(size_t qIndex, bool setOn = true);

    void setShifted(size_t qIndex, bool setOn = true);

private:
    BLOCK_TYPE get_bits(size_t address, size_t length);

    void set_bits(size_t address, size_t bitsToWriteCounter, size_t value);

    void set_bit(size_t address, bool setOn);

    void setHelp(size_t aIndex, size_t value, size_t borderStart, size_t borderEnd);

};


#endif //INHERITANCE_BITARRAY_H
