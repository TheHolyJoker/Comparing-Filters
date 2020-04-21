//
// Created by tomer on 9/16/18.
//

#include "Hash.h"

namespace BF_Hash {
    Hash::Hash(size_t size, bool isAdaptive) {
        if (isAdaptive)
            size >>= 1;
        this->size = size;
        if (size > 16)
            this->multiConst = rangedUniformDistribution(7, size - 1);
        else
            this->multiConst = rangedUniformDistribution(1, size - 1);
    }

    size_t Hash::calc(const char *elementP, size_t size) const {
        uint32_t a, b;
        MurmurHash3_x86_32(elementP, (int) (strlen(elementP)), DEFAULT_SEED, &a);
        MurmurHash3_x86_32(elementP, (int) (strlen(elementP)), SECOND_SEED, &b);
        return (a % size + this->multiConst * (b % size)) % size;
    }

    size_t Hash::calc(const string *elementP, size_t size) const {
        char const *cp = elementP->c_str();
        return calc(cp, size);
    }

//
//Hash::Hash(size_t size) {
//    this->multiConst = rangedUniformDistribution(1, size - 1);
//}

    size_t Hash::getMultiConst() const {
        return multiConst;
    }

    size_t Hash::getMultiConst() {
        return multiConst;
    }

    size_t Hash::operator()(const string *s) {
        return this->calc(s, this->size);
    }

    size_t Hash::operator()(const string *s, size_t arraySize) {
        return this->calc(s, arraySize);
    }

    size_t Hash::operator()(const string *s, size_t arraySize, bool isAdaptive) {
        if (isAdaptive)
            arraySize >>= 1;
        return this->calc(s, arraySize);
    }

    ostream &operator<<(ostream &os, const Hash &h) {
        os << h.getMultiConst();
        return os;
    }
}

//
//unsigned int Hash::calc(char const *elementP, size_t size) const{
//    uint32_t a, b;
//    MurmurHash3_x86_32(elementP, (int) (strlen(elementP)), DEFAULT_SEED, &a);
//    MurmurHash3_x86_32(elementP, (int) (strlen(elementP)), SECOND_SEED, &b);
//    unsigned int index = (a % size + this->multiConst * (b % size)) % size;
//    return index;
//}
//
//
//unsigned int Hash::calc(string *elementP, size_t size) const {
//    char const *cp = elementP->c_str();
//    return Hash::calc(cp,size);
//}

//unsigned int Hash::calc(string *elementP, size_t size) {
//    char const *cp = elementP->c_str();
//    return Hash::calc(cp,size);
//}

//

//unsigned int Hash::multipliHash(uint32_t index) {
//    return (this->a * index + )
//}

