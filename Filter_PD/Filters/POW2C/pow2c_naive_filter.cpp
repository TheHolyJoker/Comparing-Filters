//
// Created by tomer on 11/5/19.
//

#include "pow2c_naive_filter.h"

pow2c_naive_filter::pow2c_naive_filter(size_t size, size_t interval_length, size_t single_pd_capacity, size_t fp_size)
        : size(size), capacity(0), h(P31), perm(P31),
          interval_length(interval_length), single_pd_capacity(single_pd_capacity), fp_size(fp_size) {
    for (size_t i = 0; i < size; ++i) {
        pd_vec.emplace_back(naive_PD(interval_length, single_pd_capacity, fp_size));
    }
    quotient_length = ceil(log2(single_pd_capacity + 1));
    pd_index_length = ceil(log2(size));

    //otherwise need to use other hash function.
    if (quotient_length + pd_index_length + fp_size > 32) {
        cout << "need to use bigger hash function." << endl;
        assert(false);
    }

    perm_naive_vec.resize(pd_vec.size());
    for (size_t i = 0; i < pd_vec.size(); ++i) { perm_naive_vec[i] = i; }
    auto rng = default_random_engine{};
    shuffle(perm_naive_vec.begin(), perm_naive_vec.end(), rng);

    while (count_perm_vec_fixed_point()) {
        shuffle(perm_naive_vec.begin(), perm_naive_vec.end(), rng);
    }

}

bool pow2c_naive_filter::lookup(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split_string_att(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    return pd_vec[pd_index1].lookup(q1, r1) || pd_vec[pd_index2].lookup(q2, r2);

}

void pow2c_naive_filter::insert(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split_string_att(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        pd_vec[pd_index1].insert(q1, r1);
    }
    capacity++;
//    get_capacity();
}

void pow2c_naive_filter::remove(string *s) {
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;
    wrap_split_string_att(s, &pd_index1, &pd_index2, &q1, &q2, &r1, &r2);

    if (pd_vec[pd_index1].conditional_remove(q1, r1))
        return;

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter." << endl;
    }
    capacity--;
}

void pow2c_naive_filter::split(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
    *remainder = hash_index & MASK(fp_size);
    hash_index >>= fp_size;
    *quotient = hash_index & MASK(quotient_length);
    hash_index >>= quotient_length;
    *pd_index = hash_index & MASK(pd_index_length);
}

void pow2c_naive_filter::wrap_split_string_att(string *s, size_t *pd_index1, size_t *pd_index2, D_TYPE *q1, D_TYPE *q2,
                                               D_TYPE *r1, D_TYPE *r2) {
    ulong h1 = h.hash(s) << 1ul;
    *r1 = h1 & MASK(fp_size);
    *r2 = *r1 | 1ul;
    h1 >>= fp_size;
    *q1 = h1 % (interval_length);
    *q2 = *q1;
    h1 >>= quotient_length;
    *pd_index1 = h1 % pd_vec.size();
    *pd_index2 = index_perm(*pd_index1);
    if (*pd_index1 == *pd_index2) {
        cout << "same pd indexes" << endl;
        cout << *pd_index1 << endl;
    }
    assert(*pd_index1 < pd_vec.size());
    assert(*pd_index2 < pd_vec.size());
}

void pow2c_naive_filter::split_print(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
    printf("hash_index: %zu, pd_index: %zu, quotient: %u, remainder: %u", hash_index, *pd_index, *quotient, *remainder);
}

size_t pow2c_naive_filter::get_capacity() {
    size_t s = 0;
    for (auto d: pd_vec) {
        s += d.get_capacity();
    }
    assert(s == capacity);
    return capacity;
}


static size_t count_perm_vec_fixed_point() {
    size_t counter = 0;
    for (size_t i = 0; i < perm_naive_vec.size(); ++i) {
        if (perm_naive_vec[i] == i) {
            counter++;
        };
    }
    return counter;
}
