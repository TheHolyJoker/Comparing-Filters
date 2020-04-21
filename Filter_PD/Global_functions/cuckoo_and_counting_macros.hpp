//
// Created by tomer on 4/11/20.
//

#ifndef CLION_CODE_CUCKOO_AND_COUNTING_MACROS_HPP
#define CLION_CODE_CUCKOO_AND_COUNTING_MACROS_HPP

#define CPD_TYPE uint32_t
#define CPD_TYPE_SIZE (sizeof(CPD_TYPE) * (CHAR_BIT))
/*
#define DEFAULT_PD_CAPACITY (64u)
#define DEFAULT_QUOTIENT_LENGTH (6u)
#define DEFAULT_QUOTIENT_RANGE (1u<<DEFAULT_QUOTIENT_LENGTH)
*/

#define DEFAULT_CPD_CAPACITY (32u)
#define DEFAULT_CPD_QUOTIENT_LENGTH (5u)
#define DEFAULT_CPD_QUOTIENT_RANGE (32u)

#define DEFAULT_COUNTER_SIZE (3u)


#define EMPTY (0x80000000)
#define DELETED (0xc0000000)

#define MAX_CUCKOO_LOOP (32)
#define DEFAULT_BUCKET_SIZE (4)
#define DEFAULT_MAX_LOAD_FACTOR (.5)


#define MAX_CUCKOO_LOOP_MULT (128)
#define DEFAULT_BUCKET_SIZE_MULT (8)
#define DEFAULT_MAX_LOAD_FACTOR_MULT (.5)

#define S_TYPE uint32_t
#define MS_TYPE uint32_t

enum counter_status {
    inc_overflow,
    dec_underflow,
    OK,
    not_a_member,
    not_a_member_can_be_inserted,
    not_a_member_can_NOT_be_inserted,
    succeed
};

#endif //CLION_CODE_CUCKOO_AND_COUNTING_MACROS_HPP
