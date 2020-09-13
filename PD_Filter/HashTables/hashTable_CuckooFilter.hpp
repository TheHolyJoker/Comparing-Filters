//
// Created by tomer on 9/2/20.
//


/**
 * This class is a wrapper for Cuckoofilter, in order to use it as a hashtable. Specifically, to be used a second level for a PD base filter.
 * 
 * 
*/
#ifndef FILTERS_HASHTABLE_CUCKOOFILTER_HPP
#define FILTERS_HASHTABLE_CUCKOOFILTER_HPP


// #include "../../cuckoofilter/ "
#include "../../cuckoofilter/src/cuckoofilter.h"
// #include "../basic_function_util.h"

// #include "../../hashutil.hpp"
// #include "../macros.h"
// #include "TPD_Filter/basic_function_util.h"
// #include "../../Tests/printutil.hpp"
// #include "printutil.hpp"

#include "../macros.h"
#include "../basic_function_util.h"
#include "../../Tests/printutil.hpp"
#include "../../hashutil.h"
#include <vector>


namespace cf_wrapper_ns {
    // status returned by a cuckoo filter operation
    enum Status {
        Ok = 0,
        NotFound = 1,
        NotEnoughSpace = 2,
        NotSupported = 3,
    };

    // maximum number of cuckoo kicks before claiming failure
    const size_t kMaxCuckooCount = 500;

    // A cuckoo filter class exposes a Bloomier filter interface,
    // providing methods of Add, Delete, Contain. It takes three
    // template parameters:
    //   ItemType:  the type of item you want to insert
    //   bits_per_item: how many bits each item is hashed into
    //   TableType: the storage of table, SingleTable by default, and
    // PackedTable to enable semi-sorting
    template<typename ItemType, size_t bits_per_item,
             template<size_t> class TableType = cuckoofilter::SingleTable,
             typename HashFamily = cuckoofilter::TwoIndependentMultiplyShift>
    class my_CuckooFilter {
        // Storage of items
        TableType<bits_per_item> *table_;

        // Number of items stored
        size_t num_items_;

        typedef struct {
            size_t index;
            uint32_t tag;
            bool used;
        } VictimCache;

        VictimCache victim_;

        HashFamily hasher_;

        inline size_t IndexHash(uint32_t hv) const {
            // table_->num_buckets is always a power of two, so modulo can be replaced
            // with
            // bitwise-and:
            return hv & (table_->NumBuckets() - 1);
        }

        inline uint32_t TagHash(uint32_t hv) const {
            uint32_t tag;
            tag = hv & ((1ULL << bits_per_item) - 1);
            tag += (tag == 0);
            return tag;
        }

        inline void GenerateIndexTagHash(const ItemType &item, size_t *index,
                                         uint32_t *tag) const {
            const uint64_t hash = hasher_(item);
            *index = IndexHash(hash >> 32);
            *tag = (item == 0) ? 1 : item;
        }

        inline size_t AltIndex(const size_t index, const uint32_t tag) const {
            // NOTE(binfan): originally we use:
            // index ^ HashUtil::BobHash((const void*) (&tag), 4)) & table_->INDEXMASK;
            // now doing a quick-n-dirty way:
            // 0x5bd1e995 is the hash constant from MurmurHash2
            return IndexHash((uint32_t)(index ^ (tag * 0x5bd1e995)));
        }

        Status AddImpl(const size_t i, const uint32_t tag);

        // load factor is the fraction of occupancy
        double LoadFactor() const { return 1.0 * Size() / table_->SizeInTags(); }

        double BitsPerItem() const { return 8.0 * table_->SizeInBytes() / Size(); }

    public:
        explicit my_CuckooFilter(const size_t max_num_keys)
            : num_items_(0), victim_(), hasher_() {
            size_t assoc = 4;
            size_t num_buckets = upperpower2(std::max<uint64_t>(1, max_num_keys / assoc));
            double frac = (double) max_num_keys / num_buckets / assoc;
            if (frac > 0.96) {
                std::cout << "CF might fail." << std::endl;
                //       num_buckets <<= 1;
            }
            victim_.used = false;
            table_ = new TableType<bits_per_item>(num_buckets);
        }

        ~my_CuckooFilter() { delete table_; }

        // Add an item to the filter.
        Status Add(const ItemType &item);

        // Report if the item is inserted, with false positive rate.
        Status Contain(const ItemType &item) const;

        // Delete an key from the filter
        Status Delete(const ItemType &item);

        /* methods for providing stats  */
        // summary infomation
        std::string Info() const;

        // number of current inserted items;
        size_t Size() const { return num_items_; }

        // size of the filter in bytes.
        size_t SizeInBytes() const { return table_->SizeInBytes(); }
    };

    template<typename ItemType, size_t bits_per_item,
             template<size_t> class TableType, typename HashFamily>
    Status my_CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>::Add(
            const ItemType &item) {
        size_t i;
        uint32_t tag;

        if (victim_.used) {
            return NotEnoughSpace;
        }

        GenerateIndexTagHash(item, &i, &tag);
        return AddImpl(i, tag);
    }

    template<typename ItemType, size_t bits_per_item,
             template<size_t> class TableType, typename HashFamily>
    Status my_CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>::AddImpl(
            const size_t i, const uint32_t tag) {
        size_t curindex = i;
        uint32_t curtag = tag;
        uint32_t oldtag;

        for (uint32_t count = 0; count < kMaxCuckooCount; count++) {
            bool kickout = count > 0;
            oldtag = 0;
            if (table_->InsertTagToBucket(curindex, curtag, kickout, oldtag)) {
                num_items_++;
                return Ok;
            }
            if (kickout) {
                curtag = oldtag;
            }
            curindex = AltIndex(curindex, curtag);
        }

        victim_.index = curindex;
        victim_.tag = curtag;
        victim_.used = true;
        return Ok;
    }

    template<typename ItemType, size_t bits_per_item,
             template<size_t> class TableType, typename HashFamily>
    Status my_CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>::Contain(
            const ItemType &key) const {
        bool found = false;
        size_t i1, i2;
        uint32_t tag;

        GenerateIndexTagHash(key, &i1, &tag);
        i2 = AltIndex(i1, tag);

        assert(i1 == AltIndex(i2, tag));

        found = victim_.used && (tag == victim_.tag) &&
                (i1 == victim_.index || i2 == victim_.index);

        if (found || table_->FindTagInBuckets(i1, i2, tag)) {
            return Ok;
        } else {
            return NotFound;
        }
    }

    template<typename ItemType, size_t bits_per_item,
             template<size_t> class TableType, typename HashFamily>
    Status my_CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>::Delete(
            const ItemType &key) {
        size_t i1, i2;
        uint32_t tag;

        GenerateIndexTagHash(key, &i1, &tag);
        i2 = AltIndex(i1, tag);

        if (table_->DeleteTagFromBucket(i1, tag)) {
            num_items_--;
            goto TryEliminateVictim;
        } else if (table_->DeleteTagFromBucket(i2, tag)) {
            num_items_--;
            goto TryEliminateVictim;
        } else if (victim_.used && tag == victim_.tag &&
                   (i1 == victim_.index || i2 == victim_.index)) {
            // num_items_--;
            victim_.used = false;
            return Ok;
        } else {
            return NotFound;
        }
    TryEliminateVictim:
        if (victim_.used) {
            victim_.used = false;
            size_t i = victim_.index;
            uint32_t tag = victim_.tag;
            AddImpl(i, tag);
        }
        return Ok;
    }

    template<typename ItemType, size_t bits_per_item,
             template<size_t> class TableType, typename HashFamily>
    std::string my_CuckooFilter<ItemType, bits_per_item, TableType, HashFamily>::Info()
            const {
        std::stringstream ss;
        ss << "my_CuckooFilter Status:\n"
           << "\t\t" << table_->Info() << "\n"
           << "\t\tKeys stored: " << Size() << "\n"
           << "\t\tLoad factor: " << LoadFactor() << "\n"
           << "\t\tHashtable size: " << (table_->SizeInBytes() >> 10) << " KB\n";
        if (Size() > 0) {
            ss << "\t\tbit/key:   " << BitsPerItem() << "\n";
        } else {
            ss << "\t\tbit/key:   N/A\n";
        }
        return ss.str();
    }
}// namespace cf_wrapper_ns


// template<typename bucket_type, size_t bucket_size, typename HashFamily = hashing::TwoIndependentMultiplyShift>

class hashTable_CuckooFilter {


    const size_t max_capacity, element_length;
    size_t capacity{0};
    const double max_load_factor;
    cf_wrapper_ns::my_CuckooFilter<uint32_t, 32> table;
    // const bucket_type empty_slot{(bucket_type) -1};
    // size_t insert_existing_counter = 0;
    // HashFamily hasher;

public:
    hashTable_CuckooFilter(size_t max_capacity, size_t element_length, double max_load_factor)
        : max_capacity(std::ceil(max_capacity / (max_load_factor))),
          element_length(element_length),
          max_load_factor(max_load_factor),
          table(std::ceil(max_capacity / (max_load_factor))) {
        if(element_length > 32){
            auto line = std::string(80,'!');
            std::cout << line << std::endl;
            std::cout << "In hashTable_CuckooFilter:" << std::endl;
            std::cout << "element length is: " << element_length <<".\t Which is too big to hold. This might cause problems." << std::endl;
            std::cout << line <<"\n" << std::endl;

        }
        // assert(num_of_buckets <= MASK32);

        /* Todo: test changes to second argument */
        // int ok = posix_memalign((void **) &Table, sizeof(Bucket), sizeof(Bucket) * num_of_buckets);
        // if (ok != 0) {
        //     cout << "Failed!!!" << endl;
        //     return;
        // }
        // //        Table = new Bucket[num_of_buckets];
        // assert(element_length < sizeof(bucket_type) * CHAR_BIT);
        // for (int i = 0; i < num_of_buckets; ++i) {
        //     auto bp = Table[i].bits_;
        //     for (int j = 0; j < bucket_size; ++j) {
        //         bp[j] = empty_slot;
        //     }
        // }
    }

    virtual ~hashTable_CuckooFilter() {
        // free(Table);
        // delete[] Table;
    }

    inline auto find(uint32_t x) const -> bool {
        assert((x & MASK(element_length)) == x);
        return (table.Contain(x) == cf_wrapper_ns::Ok);
    }

    void insert(uint32_t x) {
        assert((x & MASK(element_length)) == x);
        cf_wrapper_ns::Status res = table.Add(x);
        if (res != cf_wrapper_ns::Ok)
        {
            std::cout << "Status res: " << res << std::endl;
            assert(0);
        }
        
        // return table.cf_wrapper_ns::my_Add(x);
    }

    void remove(uint32_t x) {
        table.Delete(x);
    }

    void get_info(std::stringstream *os) {

        auto temp = table.Info();
        *os << temp;
        /* auto line = std::string(64, '-');
        *os << line << std::endl;
        *os << "Spare max_capacity is: " << str_format(get_max_capacity()) << std::endl;
        *os << "Spare capacity is: " << str_format(get_capacity()) << std::endl;
        *os << "Byte size is: " << str_format(get_byte_size()) << std::endl;
        *os << "Spare load factor is: " << get_load_factor() << std::endl;

        if (insert_existing_counter) {
            *os << "insert_existing_counter: " << insert_existing_counter << std::endl;
            double ratio = insert_existing_counter / (double) max_capacity;
            *os << "ratio to max capacity: " << ratio << std::endl;
        }
        double waste_ratio = (sizeof(bucket_size) * CHAR_BIT) / ((double) element_length);
        *os << "Waste ratio (by not packing): " << waste_ratio << std::endl;
        *os << "element bit size: " << element_length << std::endl;

        size_t empty_buckets = count_empty_buckets();
        *os << "Number of empty buckets " << empty_buckets << "/" << num_of_buckets << "\t";
        *os << "ratio is: " << (empty_buckets / (double) num_of_buckets) << std::endl;

        auto tp = find_empty_bucket_interval();
        size_t start = std::get<0>(tp), end = std::get<1>(tp);
        *os << "longest_empty_interval length is: " << end - start << std::endl;
        *os << "start: " << start << std::endl;
        *os << "end: " << end << std::endl;
        if (start + 10 < end) {
            *os << "longest_empty_interval length is: " << end - start << std::endl;
            *os << "start: " << start << std::endl;
            *os << "end: " << end << std::endl;
        }
        *os << line << std::endl;
         */
        // return *os;
    }

    auto get_name() -> std::string {
        return "hashTable_CuckooFilter";
    }
};

// private:
//     /**
//          * Picks random element in the bucket. denote by "temp_val".
//          * Insert "hold" in "temp_val" position.
//          * Set *hold = "temp_val".
//          * Store in "bucket_index" the OTHER bucket "temp_val" can be stored in.
//          * @param hold
//          * @param bucket_index
//          */
//     inline void cuckoo_swap(uint32_t *hold, size_t *bucket_index) {
//         uint32_t old_val = *hold;

//         bucket_type junk = swap_elements_from_bucket(*bucket_index, *hold);
//         assert(junk != empty_slot);
//         *hold = junk;

//         // using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
//         // uint32_t b1 = HTA_seed1, b2 = HTA_seed2;
//         // Hash_ns::BobHash(hold, sizeof(*hold), &b1, &b2);
//         // b1 = reduce32((uint32_t) b1, (uint32_t) num_of_buckets);
//         // b2 = reduce32((uint32_t) b2, (uint32_t) num_of_buckets);

//         const uint64_t out = hasher(*hold);
//         const uint32_t b1 = reduce32((uint32_t)(out >> 32ul), (uint32_t) num_of_buckets);
//         const uint32_t b2 = reduce32((uint32_t) out & MASK32, (uint32_t) num_of_buckets);

//         *bucket_index = (b1 == *bucket_index) ? b2 : b1;
//         return;
//     }

//     inline auto swap_elements_from_bucket(size_t bucket_index, bucket_type x) -> bucket_type {
//         assert(is_bucket_full_by_index(bucket_index));
//         auto *bp = Table[bucket_index].bits_;

//         auto temp_index = rand() & (bucket_size - 1);
//         bucket_type temp = bp[temp_index];
//         bp[temp_index] = x;
//         return temp;
//     }

//     /**
//          * Tries to insert "x" to bucket in "bucket_index".
//          * If the bucket is not full, x will be inserted, and true will be returned.
//          * Otherwise, x was not inserted, and false will be returned.
//          * @param x
//          * @param bucket_index
//          * @return
//          */
//     inline auto insert_if_bucket_not_full(bucket_type x, size_t bucket_index) -> bool {
//         assert((x & MASK(element_length)) == x);
//         auto *bp = Table[bucket_index].bits_;

//         if (bp[0] == empty_slot) {
//             bp[0] = x;
//             capacity++;
//             return true;
//         } else if (bp[1] == empty_slot) {
//             bp[1] = x;
//             capacity++;
//             return true;
//         } else if (bp[2] == empty_slot) {
//             bp[2] = x;
//             capacity++;
//             return true;
//         } else if (bp[3] == empty_slot) {
//             bp[3] = x;
//             capacity++;
//             return true;
//         }
//         return false;
//         // for (int i = 0; i < bucket_size; ++i) {
//         //     if (is_empty_by_bucket_index_and_location(bucket_index, i)) {
//         //         bp[i] = x;
//         //         capacity++;
//         //         return true;
//         //     }
//         //     /*Add pop attempt*/
//         // }
//         // return false;
//     }


//     /**
//          *
//          * @param x
//          * @param bucket_index
//          * @param location
//          * table[bucket_index*bucket_size + location] = x;
//          */
//     inline void insert_by_bucket_index_and_location(bucket_type x, size_t bucket_index, size_t location) {
//         auto *bp = Table[bucket_index].bits_;
//         bp[location] = x;
//         capacity++;
//     }

//     auto print_bucket(uint32_t bucket_index, ostream &os = std::cout) -> ostream & {
//         auto line = std::string(80, '-') + "\n";
//         os << line;
//         os << "bucket number " << bucket_index << " content:" << std::endl;
//         auto *bp = Table[bucket_index].bits_;
//         os << "[";
//         bool is_slot_empty = is_empty_by_bucket_index_and_location(bucket_index, 0);
//         if (!is_slot_empty)
//             os << bp[0];
//         else
//             os << "Empty";

//         for (int i = 1; i < bucket_size; ++i) {
//             if (is_empty_by_bucket_index_and_location(bucket_index, i)) {
//                 os << ", Empty!";
//             } else {
//                 os << ", " << bp[i];
//             }
//         }
//         os << "]" << std::endl;
//         os << line;
//         return os;
//     }

//     inline auto get_element_buckets(bucket_type x, uint32_t *b1, uint32_t *b2) {
//         using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
//         *b1 = HTA_seed1;
//         *b2 = HTA_seed2;
//         Hash_ns::BobHash(&x, sizeof(x), b1, b2);
//         *b1 = reduce32((uint32_t) *b1, (uint32_t) num_of_buckets);
//         *b2 = reduce32((uint32_t) *b2, (uint32_t) num_of_buckets);
//         assert(does_bucket_contain_valid_elements(*b1));
//         assert(does_bucket_contain_valid_elements(*b2));
//     }

//     ////Getters
//     /*
//         auto get_max_cuckoo_insert() const -> size_t {
//             return max_cuckoo_insert;
//         }

//         auto get_cuckoo_HT_insert_counter() const -> size_t {
//             return cuckoo_HT_insert_counter;
//         }

//         auto get_max_capacity_reached() const -> size_t {
//             return max_capacity_reached;
//         }

//         void get_data() {
//             std::cout << "max_cuckoo_insert " << get_max_cuckoo_insert() << std::endl;
//             std::cout << "cuckoo_HT_insert_counter " << get_cuckoo_HT_insert_counter() << std::endl;
//             std::cout << "get_max_capacity_reached " << get_max_capacity_reached() << std::endl;
//         }
//     */
//     inline auto get_bucket(size_t bucket_index) const -> Bucket * {
//         return Table[bucket_index];
//     }


//     auto get_byte_size() {
//         return sizeof(bucket_type) * bucket_size * num_of_buckets;
//     }

//     auto get_table_size() const -> const size_t {
//         return num_of_buckets;
//     }

//     auto get_max_capacity() const -> const size_t {
//         return max_capacity;
//     }

//     auto get_element_length() const -> const size_t {
//         return element_length;
//     }

//     auto get_bucket_size() const -> const size_t {
//         return bucket_size;
//     }

//     auto get_capacity() const -> size_t {
//         return capacity;
//     }

//     auto get_max_load_factor() const -> const double {
//         return max_load_factor;
//     }

//     auto get_load_factor() const -> double {
//         return capacity / (double) max_capacity;
//     }

//     ////Setters

//     void increase_capacity() {
//         capacity++;
//     }

//     void decrease_capacity() {
//         capacity--;
//     }

//     /*
//         void clear_slot_bucket_index_and_location(size_t bucket_index, size_t location) {
//             table[bucket_index * bucket_size + location] = EMPTY;
//         }
//     */

//     ////validation

//     void get_all_elements(vector<bucket_type> *vec) {
//         auto resize_val = get_capacity();
//         vec->resize(resize_val);
//         for (size_t i = 0; i < num_of_buckets; i++) {
//             auto *bp = Table[i].bits_;

//             for (size_t j = 0; j < bucket_size; j++) {
//                 if (bp[j] == empty_slot)
//                     continue;
//                 vec->push_back(bp[j]);
//             }
//         }
//     }

//     auto count_empty_buckets() -> size_t {
//         size_t res = 0;
//         for (size_t i = 0; i < num_of_buckets; i++) {
//             if (is_bucket_empty(i))
//                 res++;
//         }
//         return res;
//     }

//     auto is_state_valid() -> bool;

//     auto is_bucket_elements_unique(size_t bucket_index) -> bool {
//         auto *bp = Table[bucket_index].bits_;
//         for (int j = 0; j < bucket_size; ++j) {
//             if (is_empty_by_bucket_index_and_location(bucket_index, j))
//                 continue;
//             for (int i = j + 1; i < bucket_size; ++i) {
//                 if (bp[j] == bp[i])
//                     return false;
//             }
//         }
//         return true;
//     }

//     /*auto find_table_location(slot_type x) -> size_t {
//             uint32_t b1 = -1, b2 = -1;
//             my_hash(x, &b1, &b2);

//             auto res = find_helper_table_location(x, b1);
//             if (res != -1)
//                 return res;
//             return find_helper_table_location(x, b2);


//         }*/

//     /**
//          * @param table_index the index of table, in which the element is stored.
//          * @return the element without the counter.
//          */
//     auto get_element_by_index(size_t table_index) -> bucket_type {
//         size_t bucket_index = table_index / bucket_size;
//         size_t in_bucket_index = table_index % bucket_size;
//         return get_element_by_bucket_index_and_location(bucket_index, in_bucket_index);
//         //        assert(false);
//         //        return table[table_index];
//     }

//     auto get_element_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bucket_type {
//         auto *bp = Table[bucket_index].bits_;
//         return bp[location];
//     }

//     /*
//         void update_max_cuckoo_insert(size_t i) {
//             max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
//         }

//         /**
//          * Increase "cuckoo_HT_insert_counter" by "cuckoo_chain_length" * "bucket_size".
//          * @param cuckoo_chain_length
//          *
//         void update_cuckoo_HT_insert_counter(size_t cuckoo_chain_length) {
//             cuckoo_HT_insert_counter += cuckoo_chain_length * bucket_size;
//         }
//         */

//     auto is_empty_by_index(size_t table_index) -> bool {
//         size_t bucket_index = table_index / bucket_size;
//         size_t in_bucket_index = table_index % bucket_size;
//         return is_empty_by_bucket_index_and_location(bucket_index, in_bucket_index);
//         //        assert(false);
//         //        return (table[table_index] == EMPTY);
//     }

//     auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location) -> bool {
//         auto *bp = Table[bucket_index].bits_;
//         return bp[location] == empty_slot;
//     }

//     auto clear_slot_bucket_index_and_location(size_t bucket_index, size_t location) -> void {
//         auto *bp = Table[bucket_index].bits_;
//         bp[location] = empty_slot;
//     }

//     inline auto is_bucket_full_by_index(uint32_t bucket_index) const -> bool {
//         return get_bucket_capacity(bucket_index) == bucket_size;
//     }

//     auto does_bucket_contain_valid_elements(uint32_t bucket_index) const -> bool {
//         auto *bp = Table[bucket_index].bits_;
//         for (int i = 0; i < bucket_size; ++i) {
//             auto temp = bp[i];
//             if (temp == empty_slot)
//                 continue;

//             uint32_t b1, b2;
//             my_hash(temp, &b1, &b2);
//             bool cond = (bucket_index == b1) or (bucket_index == b2);
//             if (!cond) {
//                 std::cout << "h5" << std::endl;
//                 std::cout << "bucket_index is: " << bucket_index << std::endl;
//                 std::cout << "temp: " << temp;
//                 std::cout << "\tbuckets: (" << b1 << ", " << b2 << ")" << std::endl;
//                 assert(false);
//             }
//         }
//         return true;
//     }

//     inline void my_hash(bucket_type x, uint32_t *b1, uint32_t *b2) const {
//         assert(x == (x & MASK(element_length)));
//         const uint64_t out = hasher(x);
//         *b1 = reduce32((uint32_t)(out >> 32ul), (uint32_t) num_of_buckets);
//         *b2 = reduce32((uint32_t) out & MASK32, (uint32_t) num_of_buckets);
//         // *b1 = HTA_seed1;
//         // *b2 = HTA_seed2;
//         // using Hash_ns = s_pd_filter::cuckoofilter::HashUtil;
//         assert(x == (x & MASK(element_length)));
//         // Hash_ns::BobHash(&x, sizeof(x), b1, b2);
//         // *b1 = reduce32((uint32_t) *b1, (uint32_t) num_of_buckets);
//         // *b2 = reduce32((uint32_t) *b2, (uint32_t) num_of_buckets);

//         // *b1 %= num_of_buckets;
//         // *b2 %= num_of_buckets;

//         // size_t number_of_buckets_in_each_table = num_of_buckets / 2;
//         // *b1 = (*b1)% number_of_buckets_in_each_table;
//         // *b2 = ((*b2) % number_of_buckets_in_each_table) + number_of_buckets_in_each_table;
//     }


//     auto in_which_bucket_is_the_element(bucket_type x) -> int {
//         uint32_t b1 = -1, b2 = -1;
//         my_hash(x, &b1, &b2);

//         // if (db_cond)
//         // {
//         //     std::cout << "b1: " << b1 << std::endl;
//         //     std::cout << "b2: " << b2 << std::endl;
//         //     std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
//         // }
//         if (find_helper(x, b1))
//             return 1;
//         if (find_helper(x, b2))
//             return 2;
//         return -1;
//         // return ((find_helper(x, b1)) || find_helper(x, b2));
//     }

// private:
//     inline auto find_helper(bucket_type x, size_t bucket_index) const -> bool {
//         //        auto table_index = bucket_index * bucket_size;
//         auto *bp = Table[bucket_index].bits_;
//         return ((bp[0] == x) | (bp[1] == x) | (bp[2] == x) | (bp[3] == x));
//         // for (int i = 0; i < bucket_size; ++i) {
//         //     if (bp[i] == x)
//         //         //            if (is_equal(table[table_index + i], x))
//         //         return true;
//         // }
//         // return false;
//     }

//     /*
//         auto find_helper_table_location(bucket_type x, size_t bucket_index) -> int {
//             auto table_index = bucket_index * bucket_size;
//             for (int i = 0; i < bucket_size; ++i) {
//                 if (is_equal(table[table_index + i], x))
//                     return table_index + i;
//             }
//             return -1;
//         }*/

//     auto remove_helper(bucket_type x, size_t bucket_index) -> bool {
//         //        auto table_index = bucket_index * bucket_size;
//         auto *bp = Table[bucket_index].bits_;
//         for (int i = 0; i < bucket_size; ++i) {
//             if (bp[i] == x) {
//                 bp[i] = empty_slot;
//                 capacity--;
//                 return true;
//             }
//             /*if (is_equal(table[table_index + i], x)) {
//                 table[table_index + i] = EMPTY;
//     //            auto prev_val = table[table_index + i];
//                             if (find(x)) {
//         //                auto res = find_table_location(x);
//         //                find(x);
//         //                assert(false);
//         //            }
//                     capacity--;
//                     return true;*/
//         }
//         return false;
//     }

//     /**
//          *
//          * @param with_counter
//          * @param without_counter
//          * @return compares x,y first "element length" bits.
//          */
//     auto is_equal(bucket_type with_counter, bucket_type without_counter) const -> bool {
//         bucket_type after_mask = without_counter & MASK(element_length);
//         //        assert((without_counter & MASK(element_length)) == without_counter);
//         return (with_counter & MASK(element_length)) == without_counter;
//     }

//     auto get_bucket_capacity(size_t bucket_index) const -> size_t {
//         size_t res = 0;
//         auto *bp = Table[bucket_index].bits_;
//         for (int i = 0; i < bucket_size; ++i) {
//             if (bp[i] != empty_slot) {
//                 res++;
//             }
//         }
//         return res;
//     }

//     auto is_bucket_empty(size_t bucket_index) const -> bool {
//         auto *bp = Table[bucket_index].bits_;
//         for (int i = 0; i < bucket_size; ++i) {
//             if (bp[i] != empty_slot) {
//                 return false;
//             }
//         }
//         return true;
//     }

//     auto find_empty_bucket_interval() -> std::tuple<size_t, size_t> {
//         size_t max_length = 0;
//         size_t start = 0, end = 0;
//         size_t temp_start = 0, temp_end = 0;
//         size_t index = 0;
//         // __m512i *ppd = &pd_array[0];
//         while (index < num_of_buckets) {
//             //            size_t temp_length = 0;
//             if (is_bucket_empty(index)) {
//                 size_t temp_length = 1;
//                 size_t temp_index = index + 1;
//                 while ((temp_index < num_of_buckets) and (is_bucket_empty(temp_index))) {
//                     temp_index++;
//                     temp_length++;
//                 }
//                 if (temp_index == num_of_buckets) {
//                     std::cout << "h8!" << std::endl;
//                 }
//                 temp_length = temp_index - index;
//                 if (temp_length > max_length) {
//                     start = index;
//                     end = temp_index;
//                     max_length = temp_length;
//                 }
//                 index = temp_index + 1;
//             } else
//                 index++;
//         }
//         return {start, end};
//     }
// };


#endif//FILTERS_HASHTABLE_CUCKOOFILTER_HPP


//Old functions

// auto find_old(bucket_type x) const -> bool {
//         // HT_lookup_counter++;
//         // bool db_cond = f(x);
//         assert((x & MASK(element_length)) == x);
//         // bool printer = f(x);

//         uint32_t b1 = -1, b2 = -1;
//         my_hash(x, &b1, &b2);
//         assert(does_bucket_contain_valid_elements(b1));
//         assert(does_bucket_contain_valid_elements(b2));
//         // if (printer) {
//         //     // std::cout << "***spare lookup***\nb1: " << b1 << std::endl;
//         //     std::cout << "***spare lookup***\nb1: " << b1 << std::endl;
//         //     std::cout << "b2: " << b2 << std::endl;
//         //     bool find1 = (find_helper(x, b1));
//         //     bool find2 = (find_helper(x, b2));
//         //     std::cout << "find1: " << find1 << std::endl;
//         //     std::cout << "find2: " << find2 << std::endl;
//         //     std::cout << std::endl;
//         // }

//         return ((find_helper(x, b1)) || find_helper(x, b2));
//         // if (db_cond) {
//         //     std::cout << "b1: " << b1 << std::endl;
//         //     std::cout << "b2: " << b2 << std::endl;
//         //     std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
//         // }
//         // found_something |= ((find_helper(x, b1)) || find_helper(x, b2));
//     }

// void insert_old(bucket_type x) {
//     // if (find(x)) {
//     //     insert_existing_counter++;
//     // }
//     // bool printer = f(x);
//     assert((x & MASK(element_length)) == x);

//     if (capacity >= max_capacity) {
//         std::cout << "Trying to insert into fully loaded hash table" << std::endl;
//         assert(false);
//     }
//     // capacity++;
//     /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

//     uint32_t b1 = -1, b2 = -1;

//     my_hash(x, &b1, &b2);
//     /* if (printer) {
//             std::cout << "***spare insert***\nb1: " << b1 << std::endl;
//             std::cout << "b2: " << b2 << std::endl;
//             bool find1 = (find_helper(x, b1));
//             bool find2 = (find_helper(x, b2));
//             std::cout << "find1: " << find1 << std::endl;
//             std::cout << "find2: " << find2 << std::endl;
//             std::cout << std::endl;

//         } */
//     assert(does_bucket_contain_valid_elements(b1));
//     assert(does_bucket_contain_valid_elements(b2));

//     if (insert_if_bucket_not_full(x, b2)) {
//         /* if (printer) {
//                 std::cout << "was inserted into b1" << std::endl;
//             } */
//         return;
//     }

//     auto hold = x;
//     size_t bucket_index = b1;
//     for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
//         // assert(does_bucket_contain_valid_elements(bucket_index));

//         if (insert_if_bucket_not_full(hold, bucket_index)) {

//             // assert(does_bucket_contain_valid_elements(bucket_index));
//             // std::cout << "/* message */" << std::endl;
//             return;
//         }

//         bucket_type old_hold = hold;
//         uint32_t old_bucket_index = bucket_index;

//         cuckoo_swap(&hold, &bucket_index);
//         // line_counter++;
//         // assert(find_helper(old_hold, old_bucket_index));
//         // assert(does_bucket_contain_valid_elements(bucket_index));
//         //
//         // if (printer) {
//         //     uint32_t j1, j2;
//         //     my_hash(hold, &j1, &j2);
//         //     std::cout << "was swapped to " << hold << std::endl;
//         //     std::cout << "b1: " << j1 << std::endl;
//         //     std::cout << "b2: " << j2 << std::endl;
//         //     std::cout << "i is: " << i << std::endl;
//         // }
//         // if (old_hold == hold) {
//         //     std::cout << "swap counter" << swap_counter << std::endl;
//         //     assert(false);
//         // }
//     }
//     std::stringstream ss;
//     get_info(&ss);
//     std::cout << ss.str();
//     assert(false);
// }

// void very_old_insert(bucket_type x) {
//     /* bool was_flipped_now = false;
//         // HT_insert_counter++;
// //        bool ic_cond = (HT_insert_counter == 1118697);
//         // bool ic_cond = false;
//         //if (ic_cond) {
//             std::cout << std::string(128, '@') << std::endl;
//         }
//         // if (el3002_was_inserted) {
//         //     if (!h()) {
//         //         std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
//         //         std::cout << "x: " << x << std::endl;
//         //         assert(false);
//         //     }
//         // }
//         // bool printer = false;
//         //bool skip_assert = false;
//         if (f(x) or ic_cond) {
//             if (!el3002_was_inserted) {
//                 std::cout << std::string(80, '*') << std::endl;
//                 std::cout << "insert index: " << HT_insert_counter << std::endl;
//                 std::cout << "el3002 was flipped! " << std::endl;
//                 std::cout << std::string(80, '*') << std::endl;
//                 skip_assert = true;
//                 was_flipped_now = true;
//             }
//             printer = true;
// //            el3002_was_inserted = true;
//         }*/
//     assert((x & MASK(element_length)) == x);
//     bool printer = f(x);

//     if (capacity >= max_capacity) {
//         std::cout << "Trying to insert into fully loaded hash table" << std::endl;
//         assert(false);
//     }
//     /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

//     uint32_t b1 = -1, b2 = -1;

//     my_hash(x, &b1, &b2);
//     if (printer) {
//         std::cout << "***spare insert***\nb1: " << b1 << std::endl;
//         std::cout << "b2: " << b2 << std::endl;
//         bool find1 = (find_helper(x, b1));
//         bool find2 = (find_helper(x, b2));
//         std::cout << "find1: " << find1 << std::endl;
//         std::cout << "find2: " << find2 << std::endl;
//         std::cout << std::endl;
//     }
//     assert(does_bucket_contain_valid_elements(b1));
//     assert(does_bucket_contain_valid_elements(b2));

//     if (insert_if_bucket_not_full(x, b2)) {
//         if (printer) {
//             std::cout << "was inserted into b1" << std::endl;
//         }
//         return;
//         /*if (was_flipped_now) {
//                 std::cout << "Trying to insert (" << x << ") into the second bucket, (" << b2 << ").\n";
//                 std::cout << "Bucket contents After insertion attempt is:\n";
//                 print_bucket(b2);
//             }
//             if (f(x))
//                 assert(h());

//             bool will_not_assert = (!el3002_was_inserted) || h();
//             if (!will_not_assert) {
//                 std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
//                 std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
//                 std::cout << "swap counter: " << swap_counter << std::endl;
//                 std::cout << "x: " << x << std::endl; //2499577067
//                 assert(false);
//             }

//             if (printer)
//                 std::cout << "was inserted to b2. bucket number " << b2 << std::endl;
//             assert(does_bucket_contain_valid_elements(b2));
//             */
//     }

//     auto hold = x;
//     size_t bucket_index = b1;
//     for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
//         assert(does_bucket_contain_valid_elements(bucket_index));
//         if (insert_if_bucket_not_full(hold, bucket_index)) {

//             assert(does_bucket_contain_valid_elements(bucket_index));
//             std::cout << "/* message */" << std::endl;
//             /*if (p2_cond) {
//                     std::cout << "T_After_T" << std::endl;
//                     std::cout << "h() := " << h() << std::endl;
//                     print_bucket(b1);
//                 }
//                 bool going_to_assert = ((!el3002_was_inserted) || h());
//                 if (!going_to_assert) {
//                     std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
//                     std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
//                     std::cout << "i: " << i << std::endl;
//                     std::cout << "sleeping: ";
//                     usleep(500000);
//                     std::cout << ".";
//                     usleep(300000);
//                     std::cout << ".";
//                     usleep(200000);
//                     std::cout << ".";
//                     usleep(100000);
//                     std::cout << ".";
//                     std::cout << std::endl;
//                 }
//                 assert(going_to_assert);
//                 */
//             return;
//         }

//         bucket_type old_hold = hold;
//         uint32_t old_bucket_index = bucket_index;

//         assert(does_bucket_contain_valid_elements(bucket_index));
//         cuckoo_swap(&hold, &bucket_index);
//         line_counter++;
//         assert(find_helper(old_hold, old_bucket_index));

//         assert(does_bucket_contain_valid_elements(bucket_index));

//         if (printer) {
//             std::cout << "was swapped to " << hold << std::endl;
//             std::cout << "i is: " << i << std::endl;
//         }
//         if (old_hold == hold) {
//             std::cout << "swap counter" << swap_counter << std::endl;
//             assert(false);
//         }
//     }
//     assert(false);
// }

// void cuckoo_swap_old(bucket_type *hold, size_t *bucket_index) {
//     //        assert(false);
//     // assert(!el3002_was_inserted || h());
//     assert(does_bucket_contain_valid_elements(*bucket_index));
//     swap_counter++;
//     bucket_type old_val = *hold;
//     bucket_type junk = swap_elements_from_bucket(*bucket_index, *hold);
//     assert(junk != empty_slot);
//     *hold = junk;

//     uint32_t temp_b1 = -1, temp_b2 = -1;
//     my_hash(*hold, &temp_b1, &temp_b2);
//     assert(does_bucket_contain_valid_elements(*bucket_index));
//     // assert(temp_b2 != temp_b1);

//     if (temp_b1 == *bucket_index)
//         *bucket_index = temp_b2;
//     else if (temp_b2 == *bucket_index)
//         *bucket_index = temp_b1;
//     else {
//         std::cout << "h4" << std::endl;
//         std::cout << "*hold: " << *hold;
//         std::cout << "\tbuckets: (" << temp_b1 << ", " << temp_b2 << ")" << std::endl;
//         //            my_hash(old_val, &old_b2, &old_b1);
//         //            std::cout << "\tRevered: (" << old_b1<<", " << old_b2 << ")" << std::endl;


//         std::cout << "*bucket_index: " << *bucket_index << std::endl;
//         std::cout << "num_of_buckets: " << num_of_buckets << std::endl;

//         uint32_t old_b1, old_b2;
//         my_hash(old_val, &old_b1, &old_b2);

//         std::cout << "old_val: " << old_val;
//         std::cout << "\tbuckets: (" << old_b1 << ", " << old_b2 << ")";
//         my_hash(old_val, &old_b2, &old_b1);
//         std::cout << "\tRevered: (" << old_b1 << ", " << old_b2 << ")" << std::endl;


//         std::cout << "*hold: " << *hold;
//         my_hash(*hold, &old_b1, &old_b2);
//         std::cout << "\tbuckets: (" << old_b1 << ", " << old_b2 << ")";
//         my_hash(*hold, &old_b2, &old_b1);
//         std::cout << "\tRevered: (" << old_b1 << ", " << old_b2 << ")" << std::endl;

//         std::cout << "empty_slot: " << empty_slot << std::endl;
//         assert(false);
//     }
//     assert(does_bucket_contain_valid_elements(*bucket_index));

//     // assert(!el3002_was_inserted || h());
// }
// auto swap_elements_from_bucket_old(size_t bucket_index, bucket_type x) -> bucket_type {
//     assert(is_bucket_full_by_index(bucket_index));
//     auto *bp = Table[bucket_index].bits_;

//     auto rand_bucket_index = random() % bucket_size;
//     auto correct_index = rand_bucket_index;
//     bucket_type temp = bp[correct_index];
//     if (temp != x) {
//         bp[correct_index] = x;
//         return temp;
//     }
//     if (temp == x) {
//         // std::cout << "G2" << std::endl;
//         bool did_fail = true;
//         for (int i = 1; i < bucket_size; ++i) {
//             correct_index = (rand_bucket_index + i) % bucket_size;
//             temp = bp[correct_index];
//             if (temp != x) {
//                 did_fail = false;
//                 break;
//             }
//         }
//         if (did_fail) {
//             std::cout << "insertion failed. The bucket contains is full with the same element. (" << x << ")"
//                       << std::endl;
//             print_bucket(bucket_index);
//             assert(false);
//         }
//     }
//     assert(temp != x);
//     bp[correct_index] = x;
//     if (f(x)) {
//         std::cout << "x bucket is: " << bucket_index;
//         std::cout << "\t in location :" << correct_index << std::endl;
//     }
//     return temp;
// };
