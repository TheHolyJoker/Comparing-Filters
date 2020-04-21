//
// Created by tomer on 11/7/18.
//

#include "BitArray.h"

BitArray::BitArray(size_t q, size_t r, size_t MBSize) : capacity(SL(q)), dataSize(r), MB(MBSize) {
    if (BLOCK_SIZE != (8 * sizeof(BLOCK_TYPE)))
        assert(false);
    size = INTEGER_ROUND((capacity * (dataSize + MB)), (BLOCK_SIZE));
    size_t temp = (capacity * (dataSize + MB)) / BLOCK_SIZE;
    temp += ((capacity * (dataSize + MB)) % BLOCK_SIZE != 0);
    assert(temp == size);
    A = new BLOCK_TYPE[size]();
}

BitArray::~BitArray() {
    delete[] A;
}

void BitArray::getCell(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize) {
    size_t address = qIndex * (this->dataSize + MB);
    for (int i = 0; i < arraySize - 1; ++i) {
        dataArray[i] = get_bits(address, BLOCK_SIZE);
        address += BLOCK_SIZE;
    }
    auto bitsLeft = (dataSize + MB) % BLOCK_SIZE;
    dataArray[arraySize - 1] = get_bits(address, bitsLeft);
}

BLOCK_TYPE BitArray::getMB(size_t qIndex) {
    size_t address = qIndex * (dataSize + MB) + dataSize;
    return uint32_t(get_bits(address, MB));
}

void BitArray::setCell(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize) {
    size_t address = qIndex * (this->dataSize + MB);
    for (int i = 0; i < arraySize - 1; ++i) {
        set_bits(address, BLOCK_SIZE, dataArray[i]);
        address += BLOCK_SIZE;
    }
    auto bitsLeft = (dataSize + MB) % BLOCK_SIZE;
    set_bits(address, bitsLeft, dataArray[arraySize - 1]);
}

void BitArray::setCell(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize, BLOCK_TYPE tempMB) {
    size_t address = qIndex * (this->dataSize + MB);
    for (int i = 0; i < arraySize - 1; ++i) {
        set_bits(address, BLOCK_SIZE, dataArray[i]);
        address += BLOCK_SIZE;
    }
    auto bitsLeft = (dataSize + MB) % BLOCK_SIZE;
    set_bits(address, bitsLeft, (dataArray[arraySize - 1] << MB) | tempMB);
}

bool BitArray::isEmpty(BLOCK_TYPE tempMB) {
    return tempMB == 0;
}

bool BitArray::isOccupied(BLOCK_TYPE tempMB) {
    return tempMB >= MB_BIT(1);
}

bool BitArray::isContinuation(BLOCK_TYPE tempMB) {
    return bool(tempMB & MB_BIT(2));
}

bool BitArray::isShifted(BLOCK_TYPE tempMB) {
    return bool(tempMB & MB_BIT(3));
}

bool BitArray::isFP(BLOCK_TYPE tempMB) {
    return bool(tempMB & MB_BIT(4));
}

bool BitArray::isPartOfRemainder(BLOCK_TYPE tempMB) {
    return ((tempMB & MB_BIT(2)) and !(tempMB & MB_BIT(3)));
}

bool BitArray::isRunStart(BLOCK_TYPE tempMB) {
    return bool((tempMB & MASK(MB)) and (!(tempMB & MB_BIT(2))));
}

bool BitArray::isClusterStart(BLOCK_TYPE tempMB) {
    return ((tempMB != 0) and !((MB_BIT(3) | MB_BIT(2)) & tempMB));
}

void BitArray::setOccupied(size_t qIndex, bool setOn) {
    size_t address = qIndex * (this->dataSize + MB) + dataSize;
    set_bit(address, setOn);
}

void BitArray::setContinuation(size_t qIndex, bool setOn) {
    size_t address = qIndex * (this->dataSize + MB) + dataSize + 1;
    set_bit(address, setOn);
}

void BitArray::setShifted(size_t qIndex, bool setOn) {
    size_t address = qIndex * (this->dataSize + MB) + dataSize + 2;
    set_bit(address, setOn);
}

void BitArray::setFP(size_t qIndex, bool setOn) {
    size_t address = qIndex * (this->dataSize + MB) + dataSize + 3;
    set_bit(address, setOn);
}

BLOCK_TYPE BitArray::get_bits(size_t address, size_t length) {
    size_t aIndex = address / BLOCK_SIZE, bitIndex = address % BLOCK_SIZE;
    size_t bitsFromRight = MOD_INVERSE(bitIndex);
    BLOCK_TYPE res = A[aIndex];
    if (bitIndex + length < BLOCK_SIZE) {
        BLOCK_TYPE temp = ON((bitsFromRight + 1), (bitsFromRight + 1 - length));
        res &= temp;
        size_t shift_right = ++bitsFromRight - length;
        res >>= shift_right;
    } else if (bitIndex + length == BLOCK_SIZE) {
        res &= MASK(bitsFromRight + 1);
    } else {
        size_t temp_length = bitIndex + length - BLOCK_SIZE;
        res &= get_bits(address, length - temp_length);
        res <<= temp_length;
        res += get_bits((aIndex + 1) * BLOCK_SIZE, temp_length);
    }
    return res;
}

void BitArray::set_bits(size_t address, size_t bitsToWriteCounter, size_t value) {
    value &= MASK(bitsToWriteCounter);
    size_t aIndex = address / BLOCK_SIZE, bitIndex = address % BLOCK_SIZE;
    size_t rBitsCounter = MOD_INVERSE(bitIndex) + 1;

    if (bitIndex + bitsToWriteCounter < BLOCK_SIZE) {
        value <<= (BLOCK_SIZE - (bitIndex + bitsToWriteCounter));
        setHelp(aIndex, value, rBitsCounter, rBitsCounter - bitsToWriteCounter);
    } else if (bitIndex + bitsToWriteCounter == BLOCK_SIZE) {
        setHelp(aIndex, value, rBitsCounter, 0);
    } else {
        size_t exceedingBits = bitIndex + bitsToWriteCounter - BLOCK_SIZE;
        uint64_t firstVal = value >> exceedingBits;
        uint64_t secondVal = value & MASK(exceedingBits);
        secondVal <<= (BLOCK_SIZE - exceedingBits);
        setHelp(aIndex, firstVal, rBitsCounter, 0);
        if (exceedingBits > BLOCK_SIZE)
            set_bits((++aIndex) * BLOCK_SIZE, exceedingBits, secondVal);
        else
            setHelp(++aIndex, secondVal, BLOCK_SIZE, BLOCK_SIZE - exceedingBits);
    }
}

void BitArray::set_bit(size_t address, bool setOn) {
    size_t aIndex, bitIndex, shift;
    bitIndex = address % BLOCK_SIZE;
    aIndex = address / BLOCK_SIZE;
    shift = MOD_INVERSE(bitIndex);
    size_t temp = (1ULL << shift);
    (setOn) ? A[aIndex] |= temp : A[aIndex] ^= temp;
}

void BitArray::setHelp(size_t aIndex, size_t value, size_t borderStart, size_t borderEnd) {
    A[aIndex] &= OFF(borderStart, borderEnd);
    value &= ON(borderStart, borderEnd);
    A[aIndex] |= value;
}

size_t BitArray::getMBSize() const {
    return MB;
}



/*
void BitArray::getData(size_t qIndex, BLOCK_TYPE *dataArray, size_t arraySize) {
    size_t address = qIndex * (this->dataSize + MB);
    for (int i = 0; i < arraySize - 1; ++i) {
        dataArray[i] = get_bits(address, BLOCK_SIZE);
        address += BLOCK_SIZE;
    }
    auto bitsLeft = (dataSize + MB) % BLOCK_SIZE;
    if (bitsLeft > MB)
        dataArray[arraySize  - 1] = get_bits(address, bitsLeft - MB);
    else {
        if (MB == bitsLeft) return;
        auto shiftRight = MB - bitsLeft;
        dataArray[arraySize  - 2] >>= shiftRight;
    }
}
*/
