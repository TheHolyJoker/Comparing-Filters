//
// Created by tomer on 11/9/19.
//

#ifndef CLION_CODE_MACROS_H
#define CLION_CODE_MACROS_H

#define HEADER_BLOCK_SIZE (32)
#define HEADER_BLOCK_TYPE uint32_t

#define BODY_BLOCK_SIZE (32)
#define BODY_BLOCK_TYPE uint32_t
#define FP_TYPE_SIZE (32)
#define FP_TYPE uint32_t

#define SL(p) (1ULL <<(p))
#define MASK(perm)  ( (1ULL <<(perm)) - 1ULL)
#define MASK32 (4294967295ul)
#define ON(a, b) (MASK(a) ^ MASK(b))
#define OFF(a, b) (~(MASK(a) ^ MASK(b)))
#define MOD_INVERSE(n) (HEADER_BLOCK_SIZE - (n) - 1)
#define INTEGER_ROUND(a, b) (((a)/(b)) + ((a)%(b) != 0))
#define MB_BIT(n) (1ULL << (MB - (n)))

////hash_util macros
#define DEFAULT_SEED 2
#define SECOND_SEED 42
#define HASH_BLOCK_TYPE uint32_t
#define HASH_BLOCK_SIZE (32)
#define MAX_MULTI (1ULL<<10)
#define D_TYPE uint32_t
#define D_TYPE_SIZE (32)
#define HASH_SEED (127)

////Debug macros
#define DB (0)
#define DB_PRINT (0)


#define S_TYPE uint32_t
#define MAX_CUCKOO_LOOP (256)
#define EMPTY (0x80000000)
#define DEFAULT_BUCKET_SIZE (4)
//#define MAX_CUCKOO_LOOP_MULT (128)


#endif //CLION_CODE_MACROS_H
