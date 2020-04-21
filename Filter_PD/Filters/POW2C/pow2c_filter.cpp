//
// Created by tomer on 11/4/19.
//

#include "pow2c_filter.h"


pow2c_filter::pow2c_filter(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size)
        : size(size), capacity(0), h(M64), perm(P31, 1),
          interval_length(interval_length), single_pd_capacity(single_pd_capacity),
          fp_size(fp_size), pd_index_length(ceil(log2(size))), quotient_length(ceil(log2(single_pd_capacity + 1))) {
    for (size_t i = 0; i < size; ++i) {
        pd_vec.emplace_back(PD(interval_length, single_pd_capacity, fp_size));
    }
    perm_vec.resize(pd_vec.size());
    for (size_t i = 0; i < pd_vec.size(); ++i) { perm_vec[i] = i; }
    auto rng = default_random_engine{42};
    shuffle(perm_vec.begin(), perm_vec.end(), rng);

//    auto temp = perm_vec[176];
//    perm_vec[176] = 0;
//    perm_vec[0] = temp;
//    cout << perm_vec[0] << endl;
//    auto fixed_point_counter = count_perm_vec_fixed_point();
    while (count_perm_vec_fixed_point()) {
        shuffle(perm_vec.begin(), perm_vec.end(), rng);
    }
}
//    if (fixed_point_counter) cout << "number of fixed point is " << fixed_point_counter << endl;

//    pd_index_length = ceil(log2(size));
//    quotient_length = ceil(log2(single_pd_capacity + 1));
//
//pow2c_filter::~pow2c_filter(){
//    pd_vec.erase(pd_vec.begin(), pd_vec.end());
//}


bool pow2c_filter::lookup(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split_string_att_wrong(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    return pd_vec[pd_index1].lookup(q1, r1) || pd_vec[pd_index2].lookup(q2, r2);


}

void pow2c_filter::insert(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split_string_att_wrong(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        pd_vec[pd_index1].insert(q1, r1);
    }
    capacity++;
}

void pow2c_filter::remove(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split_string_att_wrong(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_vec[pd_index1].conditional_remove(q1, r1))
        return;

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter." << endl;
    }
    capacity--;
}

bool pow2c_filter::lookup(uint32_t s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split32(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    return pd_vec[pd_index1].lookup(q1, r1) || pd_vec[pd_index2].lookup(q2, r2);
}

void pow2c_filter::insert(uint32_t s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split32(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        pd_vec[pd_index1].insert(q1, r1);
    }
    capacity++;

}

void pow2c_filter::remove(uint32_t s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split32(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);
    if (pd_vec[pd_index1].conditional_remove(q1, r1))
        return;

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter." << endl;
    }
    capacity--;

}

//void pow2c_filter::split(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
//    *remainder = hash_index & MASK(fp_size);
//    hash_index >>= fp_size;
//    *quotient = hash_index & MASK(quotient_length);
//    hash_index >>= quotient_length;
//    *pd_index = hash_index & MASK(pd_index_length);
//
//}


