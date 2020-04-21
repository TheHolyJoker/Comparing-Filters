//
// Created by tomer on 11/25/19.
//

#include "gen_2Power.h"

/*
template<class D>
gen_2Power<D>::gen_2Power(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size)
        :size(size), capacity(0), interval_length(interval_length),
         single_pd_capacity(single_pd_capacity), fp_size(fp_size), pd_index_length(ceil(log2(size))),
         quotient_length(ceil(log2(single_pd_capacity + 1))), is_const_size_indicator(false) {
    for (size_t i = 0; i < size; ++i) {
        pd_vec.emplace_back(D(interval_length, single_pd_capacity, fp_size));
    }
}
*/


template<class D>
gen_2Power<D>::gen_2Power(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size,
                          bool const_size) :size(size), capacity(0), interval_length(interval_length),
                                            single_pd_capacity(single_pd_capacity), fp_size(fp_size),
                                            pd_index_length(ceil(log2(size))),
                                            quotient_length(ceil(log2(single_pd_capacity + 1))),
                                            is_const_size_indicator(false) {
//    pd_vec.resize(this->size);
    pd_capacity_vec.resize(size);
//    for (int j = 0; j < size; ++j) {
//        assert(pd_capacity_vec[j] == 0);
//    }

    for (size_t i = 0; i < size; ++i) {
        pd_vec.emplace_back(D(interval_length, single_pd_capacity, fp_size));
    }

}

//template<class D>
//gen_2Power<D>::gen_2Power(size_t max_number_of_elements, size_t error_power_inv, size_t level1_counter_size,
//                          size_t level2_counter_size, double level1_load_factor, double level2_load_factor) {
//
//
//}


template<class D>
bool gen_2Power<D>::lookup(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    wrap_split(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);
    return pd_vec[pd_index1].lookup(q1, r1) or pd_vec[pd_index2].lookup(q2, r2);

}

template<class D>
void gen_2Power<D>::insert(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    wrap_split(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_capacity_vec[pd_index2] <= pd_capacity_vec[pd_index1]) {
        pd_vec[pd_index2].insert(q2, r2);
        pd_capacity_vec[pd_index2]++;
    } else {
        pd_vec[pd_index1].insert(q1, r1);
        pd_capacity_vec[pd_index1]++;
    }
    /*    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        pd_vec[pd_index1].insert(q1, r1);
    }*/
    capacity++;
}

template<class D>
void gen_2Power<D>::remove(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    wrap_split(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);


    if (pd_vec[pd_index1].conditional_remove(q1, r1)) {
        pd_capacity_vec[pd_index1]--;
        return;
    }

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter." << endl;
        pd_capacity_vec[pd_index2]++;
    }
    pd_capacity_vec[pd_index2]--;
    capacity--;
}

template<class D>
bool gen_2Power<D>::lookup(uint32_t s) {
    assert(false);
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    printf("pd_index1:%zu, q1:%u, r1:%u\n", pd_index1, q1, r1);
    printf("pd_index2:%zu, q2:%u, r2:%u\n", pd_index2, q2, r2);

    return pd_vec[pd_index2].lookup(q2, r2) || pd_vec[pd_index1].lookup(q1, r1);
    return false;
}

template<class D>
void gen_2Power<D>::insert(uint32_t s) {
    assert(false);
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    printf("pd_index1:%zu, q1:%u, r1:%u\n", pd_index1, q1, r1);
    printf("pd_index2:%zu, q2:%u, r2:%u\n", pd_index2, q2, r2);

    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        cout << "was inserted to second index." << endl;
        printf("pd_index2:%zu, q2:%u, r2:%u\n", pd_index2, q2, r2);
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        cout << "was inserted to first index." << endl;
        printf("pd_index1:%zu, q1:%u, r1:%u\n", pd_index1, q1, r1);
        pd_vec[pd_index1].insert(q1, r1);
    }
    capacity++;
}

template<class D>
void gen_2Power<D>::remove(uint32_t s) {
    assert(false);
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_vec[pd_index1].conditional_remove(q1, r1))
        return;

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter." << endl;
    }
    capacity--;
}

//template<class D>
//gen_2Power<D>::~gen_2Power() {
////    for (int i = 0; i < pd_vec.size(); ++i) {
////         pd_vec[i];
////    }
//    pd_vec.clear();
//}


template
class gen_2Power<cg_PD>;

template
class gen_2Power<PD>;