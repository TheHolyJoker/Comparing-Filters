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
    void set_init(size_t size, std::set<T> *mySet) {
        /*const size_t range_from = 0;
        ulong shift = (sizeof(T) * CHAR_BIT) << 1u;
        const unsigned long long range_to = (1ULL << shift) - 1ull;
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_int_distribution<size_t> distr(range_from, range_to);*/
        size_t counter = 0;
        while (mySet->size() < size) {
//            T temp = distr(generator);
            mySet->insert((uint32_t) random());
            if (++counter > 2 * size) {
                std::cout << "set could not reach the right size!" << std::endl;
                std::cout << "set size: " << mySet->size() << std::endl;
                std::cout << "expected size: " << size << std::endl;
                std::cout << "counter: " << counter << std::endl;
                break;
            }
        }

    }


    template<typename T>
    void multiset_init(size_t size, std::multiset<T> *mySet, size_t universe_size) {
        const size_t range_from = 0;
        const size_t range_to = universe_size;
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_int_distribution<size_t> distr(range_from, range_to);

        size_t counter = 0;
        while (mySet->size() < size) {
            mySet->insert((size_t) distr(generator));
            if (++counter > 2 * size) {
                std::cout << "set could not reach the right size!" << std::endl;
                break;
            }
        }

    }


}

template void name_compare::set_init<uint32_t>(size_t size, std::set<uint32_t> *mySet);

template void name_compare::set_init<uint64_t>(size_t size, std::set<uint64_t> *mySet);


template void name_compare::multiset_init<uint32_t>(size_t size, std::multiset<uint32_t> *mySet, size_t universe_size);

template void name_compare::multiset_init<uint64_t>(size_t size, std::multiset<uint64_t> *mySet, size_t universe_size);
