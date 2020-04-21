//
// Created by tomer on 11/9/19.
//

#include "const_filter.h"

const_filter::const_filter(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size)
        : size(size), capacity(0), h(P31), perm(P31),
          interval_length(interval_length), single_pd_capacity(32), fp_size(8) {
    for (size_t i = 0; i < size; ++i) {
        pd_vec.emplace_back(const_PD(true));
    }
    quotient_length = 5;
    pd_index_length = ceil(log2(size));

    //otherwise need to use other hash function.
    if (quotient_length + pd_index_length + fp_size > 32) {
        cout << "need to use bigger hash function." << endl;
        assert(false);
    }

}


bool const_filter::lookup(string *s) {
    size_t hash1 = h(s) << 1ul;
    size_t hash2 = (perm.get_perm(h(s)) << 1ul) | 1ul;
//    printf("hash1 is %zu, hash2 is: %zu\n", hash1, hash2);
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    split(hash1, &pd_index1, &q1, &r1);
    split(hash2, &pd_index2, &q2, &r2);

    return pd_vec[pd_index1].lookup(q1, r1) || pd_vec[pd_index2].lookup(q2, r2);
}

void const_filter::insert(string *s) {
    size_t hash1 = h(s) << 1ul;
    size_t hash2 = (perm.get_perm(h(s)) << 1ul) | 1ul;

    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    split(hash1, &pd_index1, &q1, &r1);
    split(hash2, &pd_index2, &q2, &r2);

    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        pd_vec[pd_index1].insert(q1, r1);
    }
    capacity++;
}

void const_filter::remove(string *s) {
    size_t hash1 = h(s) << 1ul;
    size_t hash2 = (perm.get_perm(h(s)) << 1ul) | 1ul;

    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    split(hash1, &pd_index1, &q1, &r1);
    split(hash2, &pd_index2, &q2, &r2);

    if (pd_vec[pd_index1].conditional_remove(q1, r1))
        return;

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter." << endl;
    }
    capacity--;
}

bool const_filter::lookup(uint32_t s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split32(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    return pd_vec[pd_index1].lookup(q1, r1) || pd_vec[pd_index2].lookup(q2, r2);
}

void const_filter::insert(uint32_t s) {
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

void const_filter::remove(uint32_t s) {
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

size_t const_filter::find_min_occupancy() {
    size_t min_occupancy = 32;
    for (auto d :pd_vec) min_occupancy = min(d.get_capacity(), min_occupancy);
    return min_occupancy;
}

size_t const_filter::find_max_occupancy() {
    size_t max_occupancy = 0;
    for (auto d :pd_vec) max_occupancy = max(d.get_capacity(), max_occupancy);
    return max_occupancy;
}

size_t const_filter::count_min_occupancy() {
    size_t counter = 0, min_occupancy = find_min_occupancy();
    for (auto d :pd_vec) counter += (d.get_capacity() == min_occupancy);
    return counter;
}

size_t const_filter::count_max_occupancy() {
    size_t counter = 0, max_occupancy = find_max_occupancy();
    for (auto d :pd_vec) counter += (d.get_capacity() == max_occupancy);
    return counter;
}

double const_filter::absolute_mean_deviation() {
    double average_occupancy = get_occupancy() / (double) pd_vec.size();
    double deviance = 0;
    for (auto d :pd_vec) {
        deviance += abs(d.get_capacity() - average_occupancy);
    }
    return deviance / (double) (pd_vec.size() - 1);
}

double const_filter::squared_deviation() {
//    size_t total_occupancy = get_occupancy();
    double average_occupancy = get_occupancy() / (double) pd_vec.size();
    double deviance = 0;
    for (auto d :pd_vec) {
        auto temp = d.get_capacity() - average_occupancy;
        deviance += temp * temp;
    }
    return deviance / (double) (pd_vec.size() - 1);
}

size_t const_filter::get_occupancy() {
    size_t counter = 0;
    for (auto d :pd_vec) counter += d.get_capacity();
    return counter;
}

void const_filter::print_stats() {
    cout << "Min occupancy is " << find_min_occupancy();
    cout << ". Min occupancy counter is " << count_min_occupancy() << endl;
    cout << "Max occupancy is " << find_max_occupancy();
    cout << ". Max occupancy counter is " << count_max_occupancy() << endl;
    cout << "Absolute mean deviation is " << absolute_mean_deviation() << endl;
    cout << "Squared deviation is " << squared_deviation() << endl;
}


/*

void const_filter::split(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
    *remainder = hash_index & MASK(fp_size);
    hash_index >>= fp_size;
    *quotient = hash_index & MASK(quotient_length);
    hash_index >>= quotient_length;
    *pd_index = hash_index & MASK(pd_index_length);

}
*/
/*
void const_filter::wrap_split32(uint32_t s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2, D_TYPE *r1,
                                D_TYPE *r2) {
    size_t hash1 = h.hash32(s) << 1ul;
    size_t hash2 = (perm.get_perm(h.hash32(s)) << 1ul) | 1ul;
    split(hash1, pd_index1, q1, r1);
    split(hash2, pd_index2, q2, r2);
}*/
