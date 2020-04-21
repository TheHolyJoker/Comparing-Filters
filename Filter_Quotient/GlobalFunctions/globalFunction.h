//
// Created by tomer on 10/26/18.
//

#ifndef INHERITANCE_GLOBALFUNCTION_H
#define INHERITANCE_GLOBALFUNCTION_H

#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <tgmath.h>

#include "../../Filter_PD/Global_functions/macros.h"
//#define INTEGER_ROUND(a, b) (((a)/(b)) + ((a)%(b) != 0))


using namespace std;

void arrayPrinter(uint8_t *a, size_t length);

void arrayPrinter(uint16_t *a, size_t length);

void arrayPrinterNoEndl(uint16_t *a, size_t length);

void arrayPrinterNoEndl(uint32_t *a, size_t length);

void arrayPrinterNoEndl(uint64_t *a, size_t length);

void arrayPrinter(size_t *a, size_t length);

void arrayPrinter(int *a, size_t length);

void arrayPrinter(uint *a, size_t length);

bool vectorFind(vector<string> *vec, string *s);

//bool iteratorFind(vector<string>::iterator *iter, string *s);

void vectorPrint(vector<uint32_t> vec, ostream &os);


size_t calcDepthByQ(size_t q, size_t r);

void getArraysSizes(size_t *qArr, size_t *rArr, size_t length);




#endif //INHERITANCE_GLOBALFUNCTION_H
