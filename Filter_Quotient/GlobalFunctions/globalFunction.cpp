//
// Created by tomer on 10/26/18.
//

#include "globalFunction.h"

void arrayPrinter(uint8_t *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;
}

void arrayPrinter(uint16_t *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;
}

void arrayPrinterNoEndl(uint16_t *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]";
}

void arrayPrinterNoEndl(uint32_t *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]";
}

void arrayPrinterNoEndl(uint64_t *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]";
}

void arrayPrinter(size_t *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;
}

void arrayPrinter(int *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;
}

void arrayPrinter(uint *a, size_t length) {
    cout << "[" << a[0];
    for (int i = 1; i < length; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;
}

bool vectorFind(vector<string> *vec, string *s) {
    return find(vec->begin(), vec->end(), *s) != vec->end();
}

//bool iteratorFind(vector<string>::iterator *iter, string *s) {
//    return find(iter, iter.end(), *s) != vec->end();
//}

void vectorPrint(vector<uint32_t> vec, ostream &os) {
    os << "[" << vec[0];
    for (int i = 1; i < vec.size(); ++i)
        os << ", " << vec[i];
    os << "]";
}


size_t calcDepthByQ(size_t q, size_t r) {
//    cout << "calcDepthByQ:" << endl;
    double doubleQ = double(q);
    size_t counter = 0;
//    cout << "(" << doubleQ << ", " << r << ")" << endl;
    while (doubleQ > 1) {
        double exp = pow(2, doubleQ) / (1ULL << r);
        double whpConst = (1 + 1.0 / doubleQ);
        doubleQ = log2(exp * whpConst);
        r *= r;
        ++counter;
//        cout << "(" << doubleQ << ", " << r << ")" << endl;
    }
//    cout << counter << endl;
    return counter;
}

void getArraysSizes(size_t *qArr, size_t *rArr, size_t length) {
    double doubleQ = double(qArr[0]);
    size_t counter = 0;
    for (int i = 1; i < length; ++i) {
        double exp = (1ULL << (qArr[i - 1])) / (1ULL << (rArr[i - 1]));
        double whpConst = (1 + 1.0 / doubleQ);
        doubleQ = log2(exp * whpConst);
//        cout << doubleQ;
        qArr[i] = int(round(++doubleQ));
//        cout << ", " << qArr[i] << endl;
        rArr[i] = rArr[i - 1] * rArr[i - 1];
    }
}
