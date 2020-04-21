//
// Created by tomer on 9/16/18.
//

#ifndef ADAPTIVEBF_HASH_H
#define ADAPTIVEBF_HASH_H

#include "../Grey_Box_Function_Dir/GreyBoxFunction.h"
#include "../../../Shared_files/MurmurHash3.h"

//using namespace std;

namespace BF_Hash {
    class Hash {

        size_t multiConst, size;

    public:
        Hash() : multiConst(0), size(0) {}

        Hash(size_t size, bool isAdaptive);

//    explicit Hash(size_t size);

        /**
         * Calculate the fingerprint on an element (elementP) using double hashing.
         * @param elementP
         * @param size
         * @return
         */
        size_t calc(const char *elementP, size_t size) const;

        size_t calc(const string *elementP, size_t size) const;

        size_t getMultiConst() const;

        size_t getMultiConst();

        size_t operator()(const string *s);

        size_t operator()(const string *s, size_t arraySize);

        size_t operator()(const string *s, size_t arraySize, bool isAdaptive);

    };

    ostream &operator<<(ostream &os, const Hash &h);
}

#endif //ADAPTIVEBF_HASH_H
