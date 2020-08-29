#ifndef CF_MURMURHASH3_H
#define CF_MURMURHASH3_H
#include <stdint.h>

    //-----------------------------------------------------------------------------
    // MurmurHash3 was written by Austin Appleby, and is placed in the public
    // domain. The author hereby disclaims copyright to this source code.
    //-----------------------------------------------------------------------------
    //    namespace MurmurHash {
    void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, void *out);

    void MurmurHash3_x86_128(const void *key, int len, uint32_t seed, void *out);

    void MurmurHash3_x64_128(const void *key, int len, uint32_t seed, void *out);
    //    }
    //-----------------------------------------------------------------------------

#endif // CF_MURMURHASH3_H