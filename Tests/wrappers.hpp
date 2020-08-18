/* Taken from
 * https://github.com/FastFilter/fastfilter_cpp
 * */

#ifndef FILTERS_WRAPPERS_HPP
#define FILTERS_WRAPPERS_HPP

#include <climits>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <stdio.h>
#include <vector>

#include "../Bloom_Filter/bloom.hpp"
#include "../PD_Filter/dict.hpp"
#include "TPD_Filter/T_dict.hpp"
 //#include "../TPD_Filter/pd512_wrapper.hpp"
 //#include "dict512.hpp"
#include "TPD_Filter/dict512.hpp"
#include "d512/att_d512.hpp"
#include "d512/twoChoicer.hpp"
// #include "../cuckoo/cuckoofilter.h"
#include "../cuckoofilter/src/cuckoofilter.h"
//#include "../morton/compressed_cuckoo_filter.h"
#include "../Bloom_Filter/simd-block-fixed-fpp.h"
#include "../Bloom_Filter/simd-block.h"
#include "../morton/morton_sample_configs.h"

//#include "xorfilter.h"
//#include "../xorfilter/xorfilter_2.h"
//#include "../xorfilter/xorfilter_2n.h"
//#include "../xorfilter/xorfilter_10bit.h"
//#include "../xorfilter/xorfilter_10_666bit.h"
//#include "../xorfilter/xorfilter_13bit.h"
//#include "../xorfilter/xorfilter_plus.h"
//#include "../xorfilter/xorfilter_singleheader.h"
//#include "../xorfilter/xor_fuse_filter.h"

#define CONTAIN_ATTRIBUTES __attribute__((noinline))

enum filter_id
{
    BF,
    CF,
    CF_ss,
    MF,
    SIMD,
    pd_id,
    tpd_id,
    d512,
    att_d512_id,
    twoChoicer_id
};

template <typename Table>
struct FilterAPI
{
};


template <typename ItemType, size_t bits_per_item, template <size_t> class TableType, typename HashFamily>
struct FilterAPI<cuckoofilter::CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>>
{
    using Table = cuckoofilter::CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count)
    {
        return Table(add_count);
    }

    static void Add(uint64_t key, Table *table)
    {
        if (table->Add(key) != cuckoofilter::Ok)
        {
            std::cerr << "Cuckoo filter is too full. Inertion of the element (" << key << ") failed.\n";
            get_info(table);

            throw logic_error("The filter is too small to hold all of the elements");
        }
    }

    static void AddAll(const vector<ItemType> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            if (table->Add(keys[i]) != cuckoofilter::Ok)
            {
                std::cerr << "Cuckoo filter is too full. Inertion of the element (" << keys[i] << ") failed.\n";
                get_info(table);

                throw logic_error("The filter is too small to hold all of the elements");
            }
        }
    }

    static void AddAll(const std::vector<ItemType> keys, Table *table)
    {
        for (int i = 0; i < keys.size(); ++i)
        {
            if (table->Add(keys[i]) != cuckoofilter::Ok) {
                std::cerr << "Cuckoo filter is too full. Inertion of the element (" << keys[i] << ") failed.\n";
                // std::cerr << "Load before insertion is: " << ;
                get_info(table);
                
                throw logic_error("The filter is too small to hold all of the elements");

            }
        }
        //        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table)
    {
        table->Delete(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table)
    {
        return (0 == table->Contain(key));
    }

    static string get_name(Table *table)
    {
        auto ss = table->Info();
        std::string temp = "PackedHashtable";
        if (ss.find(temp)!= std::string::npos){
            return "CF-ss";
        }
        return "Cuckoo";
    }

    static auto get_info(const Table *table) ->std::stringstream
    {
        std::string state =  table->Info();
        std::stringstream ss;
        ss << state;
        return ss;
        // std::cout << state << std::endl;
    }

    static auto get_ID(Table *table) -> filter_id
    {
        return CF;
    }
};

template <
        class TableType, typename spareItemType,
        typename itemType>
struct FilterAPI<att_d512<TableType, spareItemType, itemType>>
{
    using Table = att_d512<TableType, spareItemType, itemType, 8, 51, 50>;
    //    using Table = dict512<TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count)
    {
        return Table(add_count, .955, .5);
    }

    static void Add(itemType key, Table *table)
    {
        // assert(table->case_validate());
        table->insert(key);
        // assert(table->case_validate());
    }

    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table)
    {
        for (int i = 0; i < keys.size(); ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void Remove(itemType key, Table *table)
    {
        // std::cout << "Remove in Wrapper!" << std::endl;
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table)
    {
        return table->lookup(key);
    }

    static string get_name(Table *table)
    {
        return table->get_name();
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        return table->get_extended_info();
    }

    static auto get_ID(Table *table) -> filter_id
    {
        return att_d512_id;
    }
};


template <typename itemType>
struct FilterAPI<twoChoicer<itemType>>
{
    using Table = twoChoicer<itemType, 8, 51, 50>;
    //    using Table = dict512<TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count)
    {
        return Table(add_count, .9, .5);
    }

    static void Add(itemType key, Table *table)
    {
        // assert(table->case_validate());
        table->insert(key);
        // assert(table->case_validate());
    }

    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table)
    {
        for (int i = 0; i < keys.size(); ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void Remove(itemType key, Table *table)
    {
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table)
    {
        return table->lookup(key);
    }

    static string get_name(Table *table)
    {
        return table->get_name();
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        return table->get_extended_info();
    }

    static auto get_ID(Table *table) -> filter_id
    {
        return twoChoicer_id;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename ItemType, size_t bits_per_item, bool branchless, typename HashFamily>
struct FilterAPI<bloomfilter::bloom<ItemType, bits_per_item, branchless, HashFamily>>
{
    using Table = bloomfilter::bloom<ItemType, bits_per_item, branchless, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count);
    }

    static void Add(uint64_t key, Table *table)
    {
        table->Add(key);
    }

    static void AddAll(const std::vector<ItemType> keys, const size_t start, const size_t end, Table *table)
    {
        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<ItemType> keys, Table *table)
    {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table)
    {
        throw std::runtime_error("Unsupported");
    }

    static string get_name(Table *table)
    {
        return "Bloom";
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        assert(false);
        std::stringstream ss;
        return ss;
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table)
    {
        return (0 == table->Contain(key));
    }

    static auto get_ID(Table *table) -> filter_id
    {
        return BF;
    }
};



template <>
struct FilterAPI<SimdBlockFilter<>>
{
    using Table = SimdBlockFilter<>;

    static Table ConstructFromAddCount(size_t add_count)
    {
        Table ans(ceil(log2(add_count * 8.0 / CHAR_BIT)));
        return ans;
    }

    static void Add(uint64_t key, Table *table)
    {
        table->Add(key);
    }

    static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->Add(keys[i]);
        }
    }

    static void AddAll(const std::vector<uint64_t> keys, Table *table)
    {
        AddAll(keys, 0, keys.size(), table);
        /*  for (int i = 0; i < keys.size(); ++i) {
              table->Add(keys[i]);
          }*/
    }

    static bool Contain(uint64_t key, const Table *table)
    {
        return table->Find(key);
    }

    static void Remove(uint64_t key, Table *table)
    {
        throw std::runtime_error("Unsupported");
    }

    static string get_name(Table *table)
    {
        return "SimdBlockFilter";
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        assert(false);
        std::stringstream ss;
        return ss;

    }

    static auto get_ID(Table *table) -> filter_id
    {
        return SIMD;
    }
};

class MortonFilter
{
    using mf7_6 = CompressedCuckoo::Morton7_6;
    mf7_6 *filter;
    size_t size;

public:
    MortonFilter(const size_t size)
    {
        //        filter = new CompressedCuckoo::Morton3_8((size_t) (size / 0.95) + 64);
        //        filter = new CompressedCuckoo::Morton3_8((size_t) (2.1 * size) + 64);
        filter = new mf7_6((size_t)(size / 0.95) + 64);
        this->size = size;
    }

    ~MortonFilter()
    {
        delete filter;
    }

    void Add(uint64_t key)
    {
        filter->insert(key);
    }

    void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end)
    {
        size_t size = end - start;
        ::std::vector<uint64_t> k(size);
        ::std::vector<bool> status(size);
        for (size_t i = start; i < end; i++)
        {
            k[i - start] = keys[i];
        }
        // TODO return value and status is ignored currently
        filter->insert_many(k, status, size);
    }

    void AddAll(const std::vector<uint64_t> keys)
    {
        AddAll(keys, 0, keys.size());
    }

    inline bool Contain(uint64_t &item)
    {
        return filter->likely_contains(item);
    };

    size_t SizeInBytes() const
    {
        // according to morton_sample_configs.h:
        // Morton3_8 - 3-slot buckets with 8-bit fingerprints: 11.7 bits/item
        // (load factor = 0.95)
        // so in theory we could just hardcode the size here,
        // and don't measure it
        // return (size_t)((size * 11.7) / 8);

        return filter->SizeInBytes();
    }
};

template <>
struct FilterAPI<MortonFilter>
{
    using Table = MortonFilter;

    static Table ConstructFromAddCount(size_t add_count)
    {
        return Table(add_count);
    }

    static void Add(uint64_t key, Table *table)
    {
        table->Add(key);
    }

    static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->Add(keys[i]);
        }
        //        table->AddAll(keys, start, end);
    }

    static void AddAll(const std::vector<uint64_t> keys, Table *table)
    {
        for (unsigned long key : keys)
        {
            table->Add(key);
        }
    }

    static void Remove(uint64_t key, Table *table)
    {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, Table *table)
    {
        return table->Contain(key);
    }

    static string get_name(Table *table)
    {
        return "Morton";
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        assert(false);
        std::stringstream ss;
        return ss;

    }

    static auto get_ID(Table *table) -> filter_id
    {
        return MF;
    }
};

//template<typename itemType, size_t bits_per_item,brancless, Hashfam>
//template<typename itemType, size_t bits_per_item, bool branchless, typename HashFamily>
//template<typename itemType, template<typename> class TableType>

//template<template<typename> class TableType, typename itemType, size_t bits_per_item>
//struct FilterAPI<dict<PD, TableType, itemType, bits_per_item>> {

template <template <typename> class TableType, typename itemType, typename spareItemType>
struct FilterAPI<dict<PD, TableType, itemType, spareItemType>>
{
    //    using Table = dict<PD, hash_table<uint32_t>, itemType, bits_per_item, branchless, HashFamily>;
    using Table = dict<PD, TableType, itemType, spareItemType>;

    static Table ConstructFromAddCount(size_t add_count, size_t bits_per_item)
    {
        return Table(add_count, bits_per_item, .95, .5);
    }

    static void Add(itemType key, Table *table)
    {
        table->insert(key);
    }

    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table)
    {
        for (int i = 0; i < keys.size(); ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void Remove(itemType key, Table *table)
    {
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table)
    {
        return table->lookup(key);
    }

    static string get_name(Table *table)
    {
        return "PD";
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        assert(false);
        std::stringstream ss;
        return ss;

    }

    static auto get_ID(Table *table) -> filter_id
    {
        return pd_id;
    }
};

/*

template<class temp_PD, template<typename> class TableType, typename itemType, typename spareItemType>
struct FilterAPI<dict<temp_PD, TableType, itemType, spareItemType>> {
    using Table = dict<temp_PD, TableType, itemType, spareItemType>;

    static Table ConstructFromAddCount(size_t add_count, size_t bits_per_item) {
        return Table(add_count, bits_per_item, .95, .5);
    }

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
//        string res = "TPD";
//        res += sizeof()
        return "TPD";
    }
};
*/
/*

template<template<typename, size_t, size_t> class temp_PD, typename slot_type, size_t bits_per_item, size_t max_capacity,
        typename itemType,
        template<typename> class TableType, typename spareItemType>
struct FilterAPI<dict<temp_PD<slot_type, bits_per_item, max_capacity>, TableType, itemType, spareItemType>> {
//    using Table = dict<PD, hash_table<uint32_t>, itemType, bits_per_item, branchless, HashFamily>;
    using Table = dict<TPD_name::TPD<slot_type, bits_per_item, max_capacity>, TableType, itemType, spareItemType>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, bits_per_item, .95, .5, max_capacity);
    }

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
//        string res = "TPD";
//        res += sizeof()
        return "TPD";
    }
};
*/

//<slot_type, bits_per_item, max_capacity>

template <
    class temp_PD,
    typename slot_type, size_t bits_per_item, size_t max_capacity,
    typename itemType,
    class TableType, typename spareItemType>
    struct FilterAPI<
    T_dict<temp_PD,
    slot_type, bits_per_item, max_capacity,
    TableType, spareItemType,
    itemType>>
{
    //    using Table = T_dict<TPD_name::TPD<slot_type,bits_per_item, max_capacity>, slot_type, bits_per_item, max_capacity, TableType, spareItemType, itemType>;
    using Table = T_dict<temp_PD, slot_type, bits_per_item, max_capacity, TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count)
    {
        return Table(add_count, .95, .5);
    }

    static void Add(itemType key, Table *table)
    {
        table->insert(key);
    }

    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table)
    {
        for (int i = 0; i < keys.size(); ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void Remove(itemType key, Table *table)
    {
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table)
    {
        return table->lookup(key);
    }

    static string get_name(Table *table)
    {
        return table->get_name();
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        table->get_dynamic_info();
        std::stringstream ss;
        return ss;
    }

    static auto get_ID(Table *table) -> filter_id
    {
        return tpd_id;
    }
};

template <
    class TableType, typename spareItemType,
    typename itemType>
    struct FilterAPI<
    dict512<TableType, spareItemType,
    itemType>>
{
    using Table = dict512<TableType, spareItemType, itemType, 8, 51, 50>;
    //    using Table = dict512<TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count)
    {
        return Table(add_count, 1, .5);
    }

    static void Add(itemType key, Table *table)
    {
        table->insert(key);
    }

    static void AddAll(const std::vector<itemType> keys, const size_t start, const size_t end, Table *table)
    {
        for (int i = start; i < end; ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void AddAll(const std::vector<itemType> keys, Table *table)
    {
        for (int i = 0; i < keys.size(); ++i)
        {
            table->insert(keys[i]);
        }
    }

    static void Remove(itemType key, Table *table)
    {
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table)
    {
        return table->lookup(key);
    }

    static string get_name(Table *table)
    {
        return table->get_name();
    }

    static auto get_info(Table *table) ->std::stringstream
    {
        table->get_dynamic_info();
        std::stringstream ss;
        return ss;
    }

    static auto get_ID(Table *table) -> filter_id
    {
        return d512;
    }
};


/**Before changing first argument in T_dict template argument*/
/*
template<
        template<typename, size_t, size_t> class temp_PD,
        typename slot_type, size_t bits_per_item, size_t max_capacity,
        typename itemType,
        template<typename> class TableType, typename spareItemType
>
struct FilterAPI<
        T_dict<temp_PD,
                slot_type, bits_per_item, max_capacity,
                TableType, spareItemType,
                itemType>
> {
    using Table = T_dict<TPD_name::TPD, slot_type, bits_per_item, max_capacity, TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .95, .5);
    }

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
//        string res = "TPD";
//        res += sizeof()
        return "T_dict";
    }
};
*/

/*
#ifdef __AVX2__

template<typename HashFamily>
struct FilterAPI<SimdBlockFilter<HashFamily>> {
    using Table = SimdBlockFilter<HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        Table ans(ceil(log2(add_count * 8.0 / CHAR_BIT)));
        return ans;
    }

    static void Add(uint64_t key, Table *table) {
        table->Add(key);
    }

    static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void AddAll(const vector<uint64_t> keys, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return table->Find(key);
    }

    static string get_name() {
        return "SimdBlockFilter";
    }

};

template<typename HashFamily>
struct FilterAPI<SimdBlockFilterFixed64<HashFamily>> {
    using Table = SimdBlockFilterFixed64<HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        Table ans(ceil(add_count * 8.0 / CHAR_BIT));
        return ans;
    }

    static void Add(uint64_t key, Table *table) {
        table->Add(key);
    }

    static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table *table) {
        throw std::runtime_error("Unsupported");
    }
    static void AddAll(const vector<uint64_t> keys, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return table->Find(key);
    }

    static string get_name() {
        return "SimdBlockFilterFixed64";
    }
};

*/
/*

template <typename HashFamily>
struct FilterAPI<SimdBlockFilterFixed16<HashFamily>> {
  using Table = SimdBlockFilterFixed16<HashFamily>;
  static Table ConstructFromAddCount(size_t add_count) {
    Table ans(ceil(add_count * 8.0 / CHAR_BIT));
    return ans;
  }
  static void Add(uint64_t key, Table* table) {
    table->Add(key);
  }
  static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table* table) {
    throw std::runtime_error("Unsupported");
  }
  static void Remove(uint64_t key, Table * table) {
    throw std::runtime_error("Unsupported");
  }
  CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table * table) {
    return table->Find(key);
  }
};
*/
/*


template<typename HashFamily>
struct FilterAPI<SimdBlockFilterFixed<HashFamily>> {
    using Table = SimdBlockFilterFixed<HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        Table ans(ceil(add_count * 8.0 / CHAR_BIT));
        return ans;
    }

    static void Add(uint64_t key, Table *table) {
        table->Add(key);
    }

    static void AddAll(const vector<uint64_t> keys, const size_t start, const size_t end, Table *table) {
        table->AddAll(keys, start, end);
    }
    static void AddAll(const vector<uint64_t> keys, Table *table) {
        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
        throw std::runtime_error("Unsupported");
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return table->Find(key);
    }

    static string get_name() {
        return "SimdBlockFilterFixed";
    }


};

#endif
*/

/*
template<typename itemType>
struct FilterAPI<set<itemType>> {
//    using Table = dict<PD, hash_table<uint32_t>, itemType, bits_per_item, branchless, HashFamily>;
//    using Table = set<itemType>;
    static set<itemType> ConstructFromAddCount(size_t add_count) { return set<itemType>(); }

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
};

*/

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
