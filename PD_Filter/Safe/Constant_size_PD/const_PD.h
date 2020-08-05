//
// Created by tomer on 11/9/19.
//

#ifndef CLION_CODE_CONST_PD_H
#define CLION_CODE_CONST_PD_H

#include "const_Header.h"
#include "const_Body.h"

class const_PD {
    const_Header header;
    const_Body body;
    size_t capacity;
    //Max capacity is 32.

public:
    explicit const_PD(bool preventing_empty_constructor);

    bool lookup(size_t quotient, uint8_t remainder);

    void insert(size_t quotient, uint8_t remainder);

    /**
     *
     * @param quotient
     * @param remainder
     * Error will occur if the element is not in the body.
     */
    void remove(size_t quotient, uint8_t remainder);


    /**
     * Removes the element only if it is in the PD.
     * @param quotient
     * @param remainder
     * @return true if the element was removed, false otherwise.
     */
    bool conditional_remove(size_t quotient, uint8_t remainder);

    void header_pp();

    void print();

    bool is_full();

    size_t get_capacity() const;

};


#endif //CLION_CODE_CONST_PD_H
