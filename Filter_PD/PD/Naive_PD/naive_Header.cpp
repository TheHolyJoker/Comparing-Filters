//
// Created by tomer on 11/5/19.
//

#include "naive_Header.h"

naive_Header::naive_Header(size_t m, size_t f, size_t l) : max_capacity(f), capacity(0) {
    size_t number_of_bits = ((m + f) << 1ULL) + 1;
    this->vec.resize(number_of_bits);
}

naive_Header::naive_Header(vector<bool> *vector) : max_capacity((vector->size() - 1) / 2), capacity(-42),
                                                   vec(*vector) {}

bool naive_Header::lookup(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    get_interval(quotient, start_index, end_index);
    return *start_index != *end_index;
}

void naive_Header::insert(size_t quotient, size_t *start_index, size_t *end_index) {
    if (capacity == max_capacity) {
        cout << "Tyring to insert to full vector!!!" << endl;
    }
    this->get_interval(quotient, start_index, end_index);
    vec.insert(vec.begin() + *start_index, true);
    vec.pop_back();
    capacity++;
}

void naive_Header::insert_att(size_t quotient, size_t *start_index, size_t *end_index) {
    if (capacity == max_capacity) {
        cout << "Tyring to insert to full vector!!!" << endl;
    }
    /*naive_Header temp(&this->vec);
    temp.insert(quotient, start_index, end_index);
*/
    this->get_interval(quotient, start_index, end_index);
    rotate(vec.rbegin(), vec.rbegin() + 1, vec.rend() - *end_index);
    vec[*end_index] = true;
    vec[vec.size() - 1] = false;

    /*  if (vec != temp.vec){
          cout << "Bad insertion" << endl;
          print_vector_as_words(&vec);
          print_bit_vector_as_words(&temp.vec);
          print_vector(&vec);
          print_vector(&temp.vec);

          assert(false);
      }*/
}

void naive_Header::remove(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {
    this->get_interval(quotient, start_index, end_index);
    assert(start_index < end_index);
    vec.erase(vec.begin() + *start_index);
    vec.push_back(false);
    capacity--;
}

void naive_Header::pull(size_t start_index) {
    vec.erase(vec.begin() + start_index);
    vec.push_back(false);
    capacity--;
}

void naive_Header::get_interval(size_t quotient, size_t *start_index, size_t *end_index) {
    size_t zero_counter = -1, continue_from_index = 0;

    for (size_t i = 0; i < vec.size(); ++i) {
        if (zero_counter == quotient - 1) {
            *start_index = i;
            continue_from_index = i;
            break;
        }
        if (vec[i] == 0)
            zero_counter++;
    }
    assert(*start_index < vec.size());

    for (size_t i = continue_from_index; i < vec.size(); ++i) {
        if (vec[i] == 0) {
            *end_index = i;
            return;
        }
    }
    assert(false);
}

vector<bool> *naive_Header::get_vec() {
    return &vec;
}

void naive_Header::remove_att(uint_fast16_t quotient, size_t *start_index, size_t *end_index) {

}

