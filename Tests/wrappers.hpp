/* Taken from
 * https://github.com/FastFilter/fastfilter_cpp
 * */

#ifndef FILTERS_WRAPPERS_HPP
#define FILTERS_WRAPPERS_HPP

#include <climits>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <vector>
#include <set>
#include <random>
#include <stdio.h>
#include <iostream>


#include "../Bloom_Filter/bloom.hpp"
#include "../PD_Filter/dict.hpp"
#include "../cuckoo/cuckoofilter.h"
#include "../morton/compressed_cuckoo_filter.h"
#include "../morton/morton_sample_configs.h"
#include "xorfilter.h"
#include "../xorfilter/xorfilter_2.h"
#include "../xorfilter/xorfilter_2n.h"
#include "../xorfilter/xorfilter_10bit.h"
#include "../xorfilter/xorfilter_10_666bit.h"
#include "../xorfilter/xorfilter_13bit.h"
#include "../xorfilter/xorfilter_plus.h"
#include "../xorfilter/xorfilter_singleheader.h"
#include "../xorfilter/xor_fuse_filter.h"


#define CONTAIN_ATTRIBUTES  __attribute__ ((noinline))


template<typename Table>
struct FilterAPI {
};


template<typename ItemType, size_t bits_per_item, bool branchless, typename HashFamily>
struct FilterAPI<bloomfilter::bloom<ItemType, bits_per_item, branchless, HashFamily>> {
    using Table = bloomfilter::bloom<ItemType, bits_per_item, branchless, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        table->Add(key);
    }

    static void AddAll(const std::vector<ItemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<ItemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
//        return;
        throw std::runtime_error("Unsupported");
    }

    static string get_name() {
        return "Bloom";
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};


template<typename ItemType, size_t bits_per_item, template<size_t> class TableType, typename HashFamily>
struct FilterAPI<cuckoofilter::CuckooFilter<ItemType, bits_per_item, TableType, HashFamily> > {
    using Table = cuckoofilter::CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count);
    }

    static void Add(uint64_t key, Table *table) {
        if (0 != table->Add(key)) {
            throw logic_error("The filter is too small to hold all of the elements");
        }
    }

    static void AddAll(const vector<ItemType> keys, const size_t start, const size_t end, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const std::vector<ItemType> keys, Table *table) {
        throw std::runtime_error("Unsupported");
//        table->AddAll(keys, 0, keys.size());
    }


    static void Remove(uint64_t key, Table *table) {
        table->Delete(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }

    static string get_name() {
        return "Cuckoo";
    }
};


/**Morton filter does not pass validation tests. (Probably due to wrong usage)*/
class MortonFilter {
    CompressedCuckoo::Morton3_8 *filter;
    size_t size;
public:
    MortonFilter(const size_t size) {
//        filter = new CompressedCuckoo::Morton3_8((size_t) (size / 0.95) + 64);
        filter = new CompressedCuckoo::Morton3_8((size_t) (2.1 * size) + 64);
        this->size = size;
    }

    ~MortonFilter() {
        delete filter;
    }

    void Add(uint64_t key) {
        filter->insert(key);
    }

    void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end) {
        size_t size = end - start;
        ::std::vector<uint64_t> k(size);
        ::std::vector<bool> status(size);
        for (size_t i = start; i < end; i++) {
            k[i - start] = keys[i];
        }
        // TODO return value and status is ignored currently
        filter->insert_many(k, status, size);
    }

    void AddAll(const std::vector<uint64_t> keys) {
        AddAll(keys, 0, keys.size());
    }


    inline bool Contain(uint64_t &item) {
        return filter->likely_contains(item);
    };

    size_t SizeInBytes() const {
        // according to morton_sample_configs.h:
        // Morton3_8 - 3-slot buckets with 8-bit fingerprints: 11.7 bits/item
        // (load factor = 0.95)
        // so in theory we could just hardcode the size here,
        // and don't measure it
        // return (size_t)((size * 11.7) / 8);

        return filter->SizeInBytes();
    }
};

template<>
struct FilterAPI<MortonFilter> {
    using Table = MortonFilter;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count);
    }

    static void Add(uint64_t key, Table *table) {
        table->Add(key);
    }

    static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<uint64_t> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, Table *table) {
        return table->Contain(key);
    }

    static string get_name() {
        return "Morton";
    }
};

//template<typename itemType, size_t bits_per_item,brancless, Hashfam>
//template<typename itemType, size_t bits_per_item, bool branchless, typename HashFamily>
//template<typename itemType, template<typename> class TableType>


//template<template<typename> class TableType, typename itemType, size_t bits_per_item>
//struct FilterAPI<dict<PD, TableType, itemType, bits_per_item>> {

template<template<typename> class TableType, typename itemType>
struct FilterAPI<dict<PD, TableType, itemType, uint32_t>> {
//    using Table = dict<PD, hash_table<uint32_t>, itemType, bits_per_item, branchless, HashFamily>;
    using Table = dict<PD, TableType, itemType, uint32_t>;

    static Table ConstructFromAddCount(size_t add_count, size_t bits_per_item) { return Table(add_count, bits_per_item, .95, .5); }

    static void Add(itemType key, Table *table) {
        table->insert(key);
    }


    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        for (int i = start; i < end; ++i) {
            table->insert(keys[i]);
        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        for (int i = 0; i < keys.size(); ++i) {
            table->insert(keys[i]);
        }
    }

    static void Remove(itemType key, Table *table) {
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {
        return table->lookup(key);
    }
    static string get_name() {
        return "PD";
    }
};

/*
template<typename itemType>
struct FilterAPI<set<itemType>> {
//    using Table = dict<PD, hash_table<uint32_t>, itemType, bits_per_item, branchless, HashFamily>;
//    using Table = set<itemType>;
    static set<itemType> ConstructFromAddCount(size_t add_count) { return set<itemType>(add_count); }

    static void Add(itemType key, set<itemType> *table) {
        table->insert(key);
    }


    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, set<itemType> *table) {
        table->insert(keys);
//        for (int i = start; i < end; ++i) {
//            table->insert(keys[i]);
//        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->insert(keys);
//        for (int i = 0; i < keys.size(); ++i) {
//            table->insert(keys[i]);
//        }
    }

    static void Remove(itemType key, Table *table) {
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {
        return table->lookup(key);
    }
    static string get_name() {
        return "std::set";
    }
};*/



//typedef struct FilterAPI<bloomfilter::bloom<uint64_t, 8, false, HashUtil>> filter_api_bloom;

/*
template<typename itemType, typename FingerprintType, typename HashFamily>
struct FilterAPI<xorfilter::XorFilter<itemType, FingerprintType, HashFamily>> {
    using Table = xorfilter::XorFilter<itemType, FingerprintType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }


    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};

template<typename itemType, typename FingerprintType, typename FingerprintStorageType, typename HashFamily>
struct FilterAPI<xorfilter2::XorFilter2<itemType, FingerprintType, FingerprintStorageType, HashFamily>> {
    using Table = xorfilter2::XorFilter2<itemType, FingerprintType, FingerprintStorageType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }


    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};

template<typename itemType, typename HashFamily>
struct FilterAPI<xorfilter::XorFilter10<itemType, HashFamily>> {
    using Table = xorfilter::XorFilter10<itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }


    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};

template<typename itemType, typename HashFamily>
struct FilterAPI<xorfilter::XorFilter13<itemType, HashFamily>> {
    using Table = xorfilter::XorFilter13<itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};

template<typename itemType, typename HashFamily>
struct FilterAPI<xorfilter::XorFilter10_666<itemType, HashFamily>> {
    using Table = xorfilter::XorFilter10_666<itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};

template<typename itemType, typename FingerprintType, typename FingerprintStorageType, typename HashFamily>
struct FilterAPI<xorfilter2n::XorFilter2n<itemType, FingerprintType, FingerprintStorageType, HashFamily>> {
    using Table = xorfilter2n::XorFilter2n<itemType, FingerprintType, FingerprintStorageType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};

template<typename itemType, typename FingerprintType, typename HashFamily>
struct FilterAPI<xorfilter_plus::XorFilterPlus<itemType, FingerprintType, HashFamily>> {
    using Table = xorfilter_plus::XorFilterPlus<itemType, FingerprintType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) { return Table(add_count); }

    static void Add(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<itemType> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<itemType> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }


    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }
};
*/

#endif //FILTERS_WRAPPERS_HPP
