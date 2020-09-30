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

#include "../cuckoofilter/src/cuckoofilter.h"
#include "Dict320/Dict320.hpp"
// #include "Dict320/Dict320_v2.hpp"
#include "Dict320/Dict256_Ver4.hpp"
#include "Dict320/twoChoicer320.hpp"
#include "Dict512/Dict512.hpp"
#include "Dict512/Dict512_SparseSpare.hpp"
#include "Dict512/Dict512_Ver2.hpp"
#include "Dict512/Dict512_Ver3.hpp"
#include "Dict512/Dict512_Ver4.hpp"
#include "Dict512/Dict512_With_CF.hpp"
#include "Dict512/twoChoicer.hpp"

// using Dict512_SS = Dict512_SparseSpare<>;
// #include "../Bloom_Filter/simd-block-fixed-fpp.h"
// #include "../Bloom_Filter/simd-block.h"
// #include "../morton/morton_sample_configs.h"

//#include "../morton/compressed_cuckoo_filter.h"
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

enum filter_id {
    BF,
    CF,
    CF_ss,
    MF,
    SIMD,
    pd_id,
    tpd_id,
    Dict512_id,
    Dict512_SS_id,
    d512_WCF,
    d512_ver2,
    d512_ver3,
    d512_ver4,
    d256_ver4,
    att_d512_id,
    twoChoicer_id,
    twoChoicer320_id,
    att_d320,
    att_d320_v2
};

template<typename Table>
struct FilterAPI {
};


template<typename ItemType, size_t bits_per_item, template<size_t> class TableType, typename HashFamily>
struct FilterAPI<cuckoofilter::CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>> {
    using Table = cuckoofilter::CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count);
    }

    static void Add(uint64_t key, Table *table) {
        if (table->Add(key) != cuckoofilter::Ok) {
            std::cerr << "Cuckoo filter is too full. Inertion of the element (" << key << ") failed.\n";
            get_info(table);

            throw logic_error("The filter is too small to hold all of the elements");
        }
    }

    static void AddAll(const vector<ItemType> keys, const size_t start, const size_t end, Table *table) {
        for (int i = start; i < end; ++i) {
            if (table->Add(keys[i]) != cuckoofilter::Ok) {
                std::cerr << "Cuckoo filter is too full. Inertion of the element (" << keys[i] << ") failed.\n";
                get_info(table);

                throw logic_error("The filter is too small to hold all of the elements");
            }
        }
    }

    static void AddAll(const std::vector<ItemType> keys, Table *table) {
        for (int i = 0; i < keys.size(); ++i) {
            if (table->Add(keys[i]) != cuckoofilter::Ok) {
                std::cerr << "Cuckoo filter is too full. Inertion of the element (" << keys[i] << ") failed.\n";
                // std::cerr << "Load before insertion is: " << ;
                get_info(table);

                throw logic_error("The filter is too small to hold all of the elements");
            }
        }
        //        table->AddAll(keys, 0, keys.size());
    }

    static void Remove(uint64_t key, Table *table) {
        table->Delete(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(uint64_t key, const Table *table) {
        return (0 == table->Contain(key));
    }

    static string get_name(Table *table) {
        auto ss = table->Info();
        std::string temp = "PackedHashtable";
        if (ss.find(temp) != std::string::npos) {
            return "CF-ss";
        }
        return "Cuckoo";
    }

    static auto get_info(const Table *table) -> std::stringstream {
        std::string state = table->Info();
        std::stringstream ss;
        ss << state;
        return ss;
        // std::cout << state << std::endl;
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return CF;
    }
};


template<typename itemType>
struct FilterAPI<Dict512_SparseSpare<itemType>> {
    using Table = Dict512_SparseSpare<itemType>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .64, .5);
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

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return Dict512_SS_id;
    }
};


template<
        class TableType, typename spareItemType,
        typename itemType,
        typename HashFamily>
struct FilterAPI<Dict512<TableType, spareItemType, itemType, HashFamily>> {
    using Table = Dict512<TableType, spareItemType, itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .66, .5);
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
        // std::cout << "Remove in Wrapper!" << std::endl;
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {
        // minimal_body_lookup
        // return table->minimal_body_lookup(key);
        // return table->minimal_lookup(key);
        // return table->lookup_low_load(key);

        return table->lookup(key);
        // #ifdef NDEBUG
        //         return table->lookup_low_load(key);
        //         // return table->minimal_lookup(key);
        // #else
        //         return table->lookup(key);
        // #endif
        // return table->minimal_lookup(key);

        // return table->bitwise_lookup(key);
        // std::cout << "tomer!" << std::endl;
        // return table->minimal_lookup(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return Dict512_id;
    }
};


template<
        class TableType, typename spareItemType,
        typename itemType,
        typename HashFamily>
struct FilterAPI<Dict320<TableType, spareItemType, itemType, HashFamily>> {
    using Table = Dict320<TableType, spareItemType, itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        // return Table(add_count, 21.0/32.0, .5);
        return Table(add_count, .955, .5);
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
        // std::cout << "Remove in Wrapper!" << std::endl;
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {

        // return table->lookup(key);
#ifdef NDEBUG
        return table->minimal_lookup(key);
#else
        return table->lookup(key);
#endif

        // return table->minimal_lookup(key);
        // return table->lookup(key);
        // return table->bitwise_lookup(key);
        // std::cout << "tomer!" << std::endl;
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return att_d320;
    }
};


template<
        class TableType, typename spareItemType,
        typename itemType,
        typename HashFamily>
struct FilterAPI<Dict512_Ver2<TableType, spareItemType, itemType, HashFamily>> {
    using Table = Dict512_Ver2<TableType, spareItemType, itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .75, .5);
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
        // std::cout << "Remove in Wrapper!" << std::endl;
        table->remove(key);
    }

    // Todo return const here:
    // CONTAIN_ATTRIBUTES static bool Contain(itemType key,const Table *table) {
    CONTAIN_ATTRIBUTES static bool Contain(itemType key, Table *table) {
#ifdef NDEBUG
        return table->lookup_low_load(key);
        // return table->minimal_lookup(key);
#else
        return table->lookup(key);
#endif

        // return table->lookup_count(key);
        return table->lookup(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return d512_ver2;
    }
};


template<
        class TableType, typename spareItemType,
        typename itemType,
        typename HashFamily>
struct FilterAPI<Dict512_Ver3<TableType, spareItemType, itemType, HashFamily>> {
    using Table = Dict512_Ver3<TableType, spareItemType, itemType, HashFamily>;

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
        throw std::runtime_error("Unsupported");
        // std::cout << "Remove in Wrapper!" << std::endl;
        // table->remove(key);
    }

    // Todo return const here:
    // CONTAIN_ATTRIBUTES static bool Contain(itemType key,const Table *table) {
    CONTAIN_ATTRIBUTES static bool Contain(itemType key, Table *table) {
#ifdef COUNT
        return table->lookup_count(key);
#endif// COUNT

        return table->lookup(key);
        // return table->lookup_count(key);
        // return table->lookup_minimal(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
#ifdef COUNT
        table->lookup_count(0, 2);
        table->lookup_count(0, 1);
#endif// COUNT
        return 3;
        // table->lookup_count(0, 2); \
        // table->lookup_count(0, 1);
    }
    static auto get_ID(Table *table) -> filter_id {
        return d512_ver3;
    }
};


template<
        class TableType, typename spareItemType,
        typename itemType,
        typename HashFamily>
struct FilterAPI<Dict512_Ver4<TableType, spareItemType, itemType, HashFamily>> {
    using Table = Dict512_Ver4<TableType, spareItemType, itemType, HashFamily>;

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
        throw std::runtime_error("Unsupported");
        // std::cout << "Remove in Wrapper!" << std::endl;
        // table->remove(key);
    }

    // Todo return const here:
    // CONTAIN_ATTRIBUTES static bool Contain(itemType key,const Table *table) {
    CONTAIN_ATTRIBUTES static bool Contain(itemType key, Table *table) {
        return table->lookup(key);
        // return table->lookup_count(key);
        // return table->lookup_minimal(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 3;
        // table->lookup_count(0, 2); \
        // table->lookup_count(0, 1);
    }
    static auto get_ID(Table *table) -> filter_id {
        return d512_ver4;
    }
};

template<
        typename spareItemType,
        typename itemType>
struct FilterAPI<Dict256_Ver4<spareItemType, itemType>> {
    using Table = Dict256_Ver4<spareItemType, itemType>;

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
        throw std::runtime_error("Unsupported");
        // std::cout << "Remove in Wrapper!" << std::endl;
        // table->remove(key);
    }

    // Todo return const here:
    // CONTAIN_ATTRIBUTES static bool Contain(itemType key,const Table *table) {
    CONTAIN_ATTRIBUTES static bool Contain(itemType key, Table *table) {
        return table->lookup(key);
        // return table->lookup_count(key);
        // return table->lookup_minimal(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 3;
        // table->lookup_count(0, 2); \
        // table->lookup_count(0, 1);
    }
    static auto get_ID(Table *table) -> filter_id {
        return d256_ver4;
    }
};


template<typename itemType, typename HashFamily>
struct FilterAPI<Dict512_With_CF<itemType, HashFamily>> {
    using Table = Dict512_With_CF<itemType, HashFamily>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .955, .5);
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
        // std::cout << "Remove in Wrapper!" << std::endl;
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {

        return table->lookup(key);
        // return table->bitwise_lookup(key);
        // std::cout << "tomer!" << std::endl;
        // return table->minimal_lookup(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return d512_WCF;
    }
};

template<typename itemType>
struct FilterAPI<twoChoicer320<itemType>> {
    using Table = twoChoicer320<itemType>;
    //    using Table = dict512<TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .955, .5);
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
        // throw std::runtime_error("Unsupported");
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {
        return table->lookup(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return twoChoicer320_id;
    }
};


template<typename itemType>
struct FilterAPI<twoChoicer<itemType>> {
    using Table = twoChoicer<itemType, 8, 51, 50>;
    //    using Table = dict512<TableType, spareItemType, itemType>;

    static Table ConstructFromAddCount(size_t add_count) {
        return Table(add_count, .955, .5);
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
        // throw std::runtime_error("Unsupported");
        table->remove(key);
    }

    CONTAIN_ATTRIBUTES static bool Contain(itemType key, const Table *table) {
        // std::cout << "here!!!" << std::endl;
        return table->lookup_consecutive_only_body(key);
        // return table->lookup_consecutive(key);
        // return table->lookup(key);
    }

    static string get_name(Table *table) {
        return table->get_name();
    }

    static auto get_info(Table *table) -> std::stringstream {
        return table->get_extended_info();
    }
    /**
     * Returns int indciating which function can the filter do.
     * 1 is for lookups.
     * 2 is for adds.
     * 4 is for deletions.
     */
    static auto get_functionality(Table *table) -> uint32_t {
        return 7;
    }
    static auto get_ID(Table *table) -> filter_id {
        return twoChoicer_id;
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif//FILTERS_WRAPPERS_HPP
