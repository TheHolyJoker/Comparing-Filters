//
// Created by tomer on 4/18/20.
//

#include "b_helpers.hpp"


namespace name_compare {

    auto rand_string(int minLength, int charsNum, int numOfDiffLength) -> std::string {
        int len = minLength bitor (rand() % numOfDiffLength);
        assert(4 < len);
        char s[len];
        for (int i = 0; i < len; ++i) s[i] = 64 bitor (rand() % charsNum);
        s[len] = 0;
        std::string res(s);
        return res;
    }

    void set_init(size_t size, std::set<std::string> *mySet, int minLength, int charsNum) {
        for (int i = 0; i < size; ++i) mySet->insert(rand_string(minLength, charsNum));
    }

    template<typename T>
    void number_set_init(size_t size, std::set<T> *mySet) {
        for (int i = 0; i < size; ++i) {
            mySet->insert(random());
        }
    }

    template void number_set_init<uint32_t>(size_t size, std::set<uint32_t> *mySet);

    template void number_set_init<uint64_t>(size_t size, std::set<uint64_t> *mySet);

}
