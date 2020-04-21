//
// Created by tomer on 11/9/19.
//

#include "const_Body.h"

const_Body::const_Body() : B() {
    for (unsigned char &i : B) i = 0;
}

/*
const_Body::const_Body(size_t max_capacity) {
    B = new uint8_t[max_capacity]();
}
*/

bool const_Body::lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, uint8_t remainder) {
    size_t B_index = 0;
    return find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index) == 1;
}

void const_Body::insert(size_t abstract_body_start_index, size_t abstract_body_end_index, uint8_t remainder) {
    size_t B_index = 0, size = sizeof(B) / sizeof(B[0]);
    find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index);
    rotate(&B[B_index], &B[size - 1], &B[size]);
    B[B_index] = remainder;


}

void const_Body::remove(size_t abstract_body_start_index, size_t abstract_body_end_index, uint8_t remainder) {
    size_t B_index = 0, size = sizeof(B) / sizeof(B[0]);
    auto res = find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index);
    if (res == 2) {
        cout << "Trying to delete element with remainder that is not in the Body." << endl;
        return;
    }
    if (B_index + 1 == size) {
        B[B_index] = 0;
        return;
    }
    rotate(&B[B_index], &B[B_index + 1], &B[size]);
    B[size - 1] = 0;
}

bool
const_Body::conditional_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, uint8_t remainder) {
    size_t B_index = 0, size = sizeof(B) / sizeof(B[0]);
    auto res = find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index);
    if (res == 2) { return false; }
    if (B_index + 1 == size) { B[B_index] = 0; }
    else {
        rotate(&B[B_index], &B[B_index + 1], &B[size]);
        B[size - 1] = 0;
    }
    return true;
}

/*
int
const_Body::find(size_t abstract_body_start_index, size_t abstract_body_end_index, uint8_t remainder,
                 size_t *p_B_index) {
    for (size_t i = abstract_body_start_index; i < abstract_body_end_index; ++i) {
        if (remainder == B[i]) {
            *p_B_index = i;
            return 1;
        } else if (remainder < B[i]) {
            *p_B_index = i;
            return 2;
        }
    }
    *p_B_index = abstract_body_end_index;
    return 2;
}
*/

void const_Body::print() {
//    cout << "body is: ";
    print_array(B, sizeof(B) / sizeof(B[0]));

}

