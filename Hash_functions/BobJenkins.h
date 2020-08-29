#ifndef CF_BOBJENKINGS_H
#define CF_BOBJENKINGS_H
#include <stdint.h>
#include <stdlib.h>
#include <string>

// using namespace std;
// Bob Jenkins Hash
namespace BobJenkins {
    uint32_t BobHash(const void *buf, size_t length, uint32_t seed = 0);
    
    void BobHash(const void *buf, size_t length, uint32_t *idx1, uint32_t *idx2);
    // static uint64_t BobHash(const void *buf, size_t length, uint32_t seed = 0);
}// namespace BobJenkins

#endif// CF_BOBJENKINGS_H
