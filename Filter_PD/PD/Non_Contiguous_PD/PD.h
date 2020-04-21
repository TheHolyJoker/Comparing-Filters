//
// Created by tomer on 10/24/19.
//

#ifndef CLION_CODE_PD_H
#define CLION_CODE_PD_H

#include <ostream>
#include "Header.h"
#include "Body.h"


class PD {
    Header header;
    Body body;
    size_t capacity;
    const size_t max_capacity, max_quotient;

public:
    PD(size_t m, size_t f, size_t l);

    virtual ~PD();
//    virtual ~PD();

    auto lookup(size_t quotient, FP_TYPE remainder) -> bool;

    void insert(size_t quotient, FP_TYPE remainder);

    /**
     *
     * @param quotient
     * @param remainder
     * Error will occur if the element is not in the body.
     */
    void remove(size_t quotient, FP_TYPE remainder);


    /**
     * Removes the element only if it is in the PD.
     * @param quotient
     * @param remainder
     * @return true if the element was removed, false otherwise.
     */
    auto conditional_remove(size_t quotient, FP_TYPE remainder) -> bool;

    void header_pp();


    void print();

    auto is_full() -> bool;

    auto get_capacity() const -> size_t;

    friend auto operator<<(ostream &os, const PD &pd) -> ostream &;

private:

    uint8_t
    get_body_abstract_start_index(size_t header_interval_start_index, size_t header_interval_end_index,
                                  size_t quotient);

    uint8_t
    get_body_abstract_end_index(size_t header_interval_start_index, size_t header_interval_end_index, size_t quotient);

};


#endif //CLION_CODE_PD_H
