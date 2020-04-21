//
// Created by tomer on 4/14/19.
//

#ifndef ABF_REMOTE_GENERALBF_H
#define ABF_REMOTE_GENERALBF_H

#include <utility>
#include <ostream>
#include "../Hash_files/Hash.h"

#define PRINT_HASH true

//using namespace std;

enum filter_lookup_res {
    maybe_in_filter, definitely_not_in_filter, IDK_check_next_level
};

class GeneralBF {

    vector<bool> bit_array;
    vector<BF_Hash::Hash> hashVec;
    size_t size;
    bool is_adaptive;

public:
    GeneralBF(size_t n, double eps, bool is_adaptive);

    GeneralBF(size_t n, double eps, bool isAdaptive, const string &path)
            : GeneralBF(n, (size_t) (get_bitArray_size(n, eps) + 1), isAdaptive, path) {};


    GeneralBF(size_t size, size_t k_hashFunc, bool is_adaptive);

    void set_stale_on(size_t index);
    /*

//    GeneralBF(size_t n, double eps, string path)
//            : GeneralBF(n, (size_t) get_bitArray_size(n, eps) + 1, std::move(path)) {}
*/

    /**
     *
     * @param s
     * @param hashIndex
     * @return False when s was mapped to turned on stale bit. Therefore element should be added to the next level.
     */
    bool insert(const string *s);

    bool insert(const string *s, bool *cell_cond);

    /**
     *
     * @param s
     * @return
     * 1  Yes. Element is in this filter.
     * 0  No. Element is NOT in this filter (or any next level).
     * -1 Maybe. Look in the next level to determine.
     */
    filter_lookup_res lookup(string *s);

    auto lookup(const string *s) ->bool;

    const vector<bool> &getBitArray() const;

    const vector<BF_Hash::Hash> &get_hash_vec() const;

    size_t get_hash_vec_size() const;

    size_t getSize() const;

    size_t get_filter_on_bits() const;

    size_t get_stale_on_bits() const;

    /*bool* operator[](size_t index);
    bool* operator[](size_t index) const;
*/
    friend ostream &operator<<(ostream &os, const GeneralBF &bf);

    void non_adaptive_add(const string *s);

    filter_lookup_res non_adaptive_lookup(const string *s);

private:

    GeneralBF(size_t n, size_t m, bool isAdaptive, const string &path) : GeneralBF(n, m, isAdaptive) {}

    void non_adaptive_add(const string *s, bool *cell_cond);

};

/*

*/
/**
 * Prints the BF
 * @param os
 * @param bfr
 * @return
 *//*

ostream &operator<<(ostream &os, const GeneralBF &generalBf);
*/

/**
 * Print the BF's hash functions as in array.
 * @param os
 * @param bfr
 * @return
 */
ostream &print_hashFunctions(ostream &os, const GeneralBF &generalBf);



//
// Created by tomer on 9/15/18.
//




#endif //ABF_REMOTE_GENERALBF_H
