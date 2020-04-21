//
// Created by tomer on 4/14/19.
//

#ifndef ABF_REMOTE_WRAPPER_H
#define ABF_REMOTE_WRAPPER_H

#include <sstream>
#include <iterator>
#include <ostream>

#include "../General_BF/GeneralBF.h"
//#include "../Remote/Remote.h"

enum actual_lookup_res {
    true_negative, false_positive, true_positive
};


class Wrapper {
    vector<GeneralBF> BF_Vec;
    Remote r;
    size_t depth;
    bool isAdaptive;
    size_t adapt_counter;
    double adapt_duration;

public:
    Wrapper(size_t n, double eps, bool isAdaptive);

    Wrapper(size_t n, double eps, bool isAdaptive, string *path);

    /*

//    Wrapper(size_t n, double eps, bool is_adaptive) : Wrapper(n, eps, is_adaptive, create_new_path()) {}
*/

    void add(string *s, size_t starting_level);

    void naive_add(string *s, size_t starting_level);

    void add(string *s, size_t starting_level, bool remote_trick);

    bool naive_lookup(string *s);

    actual_lookup_res lookup_verifier(string *s, bool call_adapt);

    /*actual_lookup_res lookup_verifier(string *s, bool call_adapt, bool remote_trick);*/

    const vector<GeneralBF> &getBfVec() const;

    size_t get_depth() const;

    bool is_adaptive() const;

    friend ostream &operator<<(ostream &os, const Wrapper &wrapper);

    size_t get_hash_vec_size_by_level(size_t level);

    double get_adapt_ratio() const;

    size_t getAdaptCounter() const;

    double getAdaptDuration() const;


private:

    void remote_add(string *s, size_t level);

    void remote_add(string *s, size_t level, const bool *cell_cond);

    actual_lookup_res remote_lookup(string *s, size_t key);

    /*actual_lookup_res remote_lookup(string *s, size_t key, bool remote_trick);*/

    void adapt(string *s, size_t level);

    void adapt_time_it(string *s, size_t level);

    size_t adapt_choose_which_index_to_stale(string *s, size_t level);

    void adapt_by_index_level(size_t level, size_t index);

    void adapt_insertion(string *all_elements, size_t level_to_insert_in);

    Hash get_hashFunc(size_t level, size_t index);

    uint64_t get_key(string *s, size_t level, size_t hashFunc_index = 0);

    static uint64_t get_key(string *s, size_t level, Hash h);

};

#endif //ABF_REMOTE_WRAPPER_H

