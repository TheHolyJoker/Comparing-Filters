//
// Created by tomer on 4/14/19.
//

#include "Wrapper.h"

Wrapper::Wrapper(size_t n, double eps, bool isAdaptive) : isAdaptive(isAdaptive), r(), adapt_counter(0),
                                                          adapt_duration(0) {
    if (isAdaptive) {
        this->depth = getDepth(n, eps);

        size_t mList[depth], kList[depth];

        get_bitArrays_size_list(n, eps, mList, depth);
        get_hashFunctions_num_list(n, eps, kList, mList, depth);

        for (int i = 0; i < depth; ++i)
            BF_Vec.emplace_back(mList[i], kList[i], true);
    } else {
        this->depth = 1;
        BF_Vec.emplace_back(n, eps, false);
    }
}


//// Basic functions.
void Wrapper::add(string *s, size_t starting_level) {
    if (!this->isAdaptive) {
        this->BF_Vec[0].insert(s);
        this->remote_add(s, 0);
        return;
    }

    for (int i = starting_level; i < depth; ++i) {
        this->remote_add(s, i);
        if (this->BF_Vec[i].insert(s))
            return;
    }
    //todo Rebuilt here.
    assert(false);

}

void Wrapper::add(string *s, size_t starting_level, bool remote_trick) {
    if (!this->isAdaptive) {
        bool cell_cond[BF_Vec[0].get_hash_vec_size()];
        this->BF_Vec[0].insert(s, cell_cond);
        this->remote_add(s, 0, cell_cond);
        return;
    }

    for (int i = starting_level; i < depth; ++i) {
        bool cell_cond[this->get_hash_vec_size_by_level(i)];
        bool res = this->BF_Vec[i].insert(s, cell_cond);
        this->remote_add(s, i, cell_cond);
        if (res)
            return;
    }
    //todo Rebuilt here.
    assert(false);
}


void Wrapper::naive_add(string *s, size_t starting_level) {
    if (!this->isAdaptive) {
        this->BF_Vec[0].insert(s);
        return;
    }

    for (int i = starting_level; i < depth; ++i) {
        if (this->BF_Vec[i].insert(s))
            return;
    }
    //todo Rebuilt here.
    assert(false);
}

void Wrapper::remote_add(string *s, size_t level) {
    size_t keys_num = get_hash_vec_size_by_level(level);
    size_t keys[keys_num];
    for (int i = 0; i < keys_num; ++i) {
        keys[i] = get_key(s, 0, i);
    }
    this->r.add(s, keys, keys_num);

}

void Wrapper::remote_add(string *s, size_t level, const bool *cell_cond) {
    size_t keys_num = get_hash_vec_size_by_level(level);
    size_t keys[keys_num];
    for (int i = 0; i < keys_num; ++i) {
        keys[i] = get_key(s, 0, i);
    }
    this->r.add(s, keys, keys_num, cell_cond);
}


bool Wrapper::naive_lookup(string *s) {
    if (!this->isAdaptive)
        return this->BF_Vec[0].lookup(s);

    for (int i = 0; i < depth; ++i) {
        filter_lookup_res temp_res = this->BF_Vec[i].lookup(s);
        if (temp_res == IDK_check_next_level) // Maybe s is in the next level Filter_QF.
            continue;

        return temp_res;
    }
    assert(false);
}

actual_lookup_res Wrapper::lookup_verifier(string *s, bool call_adapt) {
    if (!this->isAdaptive) {
        filter_lookup_res temp_res = this->BF_Vec[0].lookup(s);
        if (temp_res == definitely_not_in_filter)
            return true_negative; // True Negative.

        size_t key = get_key(s, 0, 0);
        return this->remote_lookup(s, key);
    }

    for (int i = 0; i < depth; ++i) {
        filter_lookup_res temp_res = this->BF_Vec[i].lookup(s);
        if (temp_res == definitely_not_in_filter)
            return true_negative;
        if (temp_res == IDK_check_next_level)
            continue;

        size_t key = get_key(s, i, 0);

        actual_lookup_res res = this->remote_lookup(s, key);
        if (call_adapt & (res == false_positive))
            this->adapt_time_it(s, i);
//            this->adapt(s, i);
        return res;
    }
    assert(false);

}

actual_lookup_res Wrapper::remote_lookup(string *s, size_t key) {
    return (r.lookup(s, key)) ? true_positive : false_positive;
}


//// Adapt.

void Wrapper::adapt(string *s, size_t level) {
    size_t index = adapt_choose_which_index_to_stale(s, level);
    return adapt_by_index_level(level, index);

}

void Wrapper::adapt_time_it(string *s, size_t level) {
    clock_t t0 = clock();
    adapt(s, level);
    double end = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    this->adapt_counter++;
//    cout << "adapt duration was: " << end << endl;
    this->adapt_duration += end;

}

size_t Wrapper::adapt_choose_which_index_to_stale(string *s, size_t level) {
    auto h = this->get_hashFunc(level, 0); // todo
    return h(s);
}

void Wrapper::adapt_by_index_level(size_t level, size_t index) {
    this->BF_Vec[level].set_stale_on(index);
    uint64_t key = (index << 4) + level; // NOLINT(hicpp-signed-bitwise)
    string elements_to_move;

    elements_to_move = this->r.retrieve(key);
    return adapt_insertion(&elements_to_move, level + 1);

}

void Wrapper::adapt_insertion(string *all_elements, size_t level_to_insert_in) {
    assert(level_to_insert_in <= depth);

    istringstream iss(*all_elements);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};

    for (auto s: tokens)
        this->add(&s, level_to_insert_in);
}


////Getters Setters
double Wrapper::get_adapt_ratio() const {
    return (this->adapt_counter / this->adapt_duration);
}

ostream &operator<<(ostream &os, const Wrapper &wrapper) {
    for (int i = 0; i < wrapper.depth; ++i) {
        os << endl << "level " << i << endl;
        os << wrapper.BF_Vec[i] << endl;
    }
    if (wrapper.getAdaptCounter()) {
        os << "adapt_time: " << wrapper.getAdaptDuration() << "\tnumber: " << wrapper.getAdaptCounter() << "\t"
           << wrapper.get_adapt_ratio() << " el/sec" << endl;
        cout << "adapt_time: " << wrapper.getAdaptDuration() << "\tnumber: " << wrapper.getAdaptCounter() << "\t"
           << wrapper.get_adapt_ratio() << " el/sec" << endl;
    }
    return os;
}

const vector<GeneralBF> &Wrapper::getBfVec() const {
    return BF_Vec;
}

size_t Wrapper::get_depth() const {
    return depth;
}

bool Wrapper::is_adaptive() const {
    return isAdaptive;
}

size_t Wrapper::get_hash_vec_size_by_level(size_t level) {
    return this->BF_Vec[level].get_hash_vec_size();
}

uint64_t Wrapper::get_key(string *s, size_t level, size_t hashFunc_index) {
    return get_key(s, level, this->get_hashFunc(level, hashFunc_index));
}

uint64_t Wrapper::get_key(string *s, size_t level, Hash h) {
    return (h(s) << 4) + level; // NOLINT(hicpp-signed-bitwise)
}

Hash Wrapper::get_hashFunc(size_t level, size_t index) {
//    assert(level < depth);
    return this->BF_Vec[level].get_hash_vec()[index];
}

size_t Wrapper::getAdaptCounter() const {
    return adapt_counter;
}

double Wrapper::getAdaptDuration() const {
    return adapt_duration;
}


