
#ifndef FILTERS_BLOOM_HPP
#define FILTERS_BLOOM_HPP

#include <utility>
#include <ostream>
#include <vector>
#include "../hashutil.h"


namespace bloomfilter {

//    static const s

// status returned by a Bloom filter operation
    enum Status {
        Ok = 0,
        NotFound = 1,
        NotEnoughSpace = 2,
        NotSupported = 3,
    };

    static size_t getBestK(size_t bitsPerItem) {
        return bitsPerItem;
    }

    static size_t getBestM(size_t bitsPerItem, size_t number_of_elements) {
        return std::max(64, (int) round((double) number_of_elements * bitsPerItem / log(2)));
    }

    template<typename ItemType, size_t bits_per_item, bool branchless, typename HashFamily = hashing::TwoIndependentMultiplyShift>
    class bloom {

        std::vector<bool> bit_array;
        uint32_t *seed_array;
        const size_t k{bits_per_item};
        size_t size;
//        HashFamily hasher;
    public:
        explicit bloom(size_t numberOfElements) :
                size(bloomfilter::getBestM(bits_per_item, numberOfElements)),
                bit_array(bloomfilter::getBestM(bits_per_item, numberOfElements), false) {
//                                                  ,k(bits_per_item)
//                                                  {
            seed_array = new uint32_t[k];
            for (int i = 0; i < k; ++i) {
                seed_array[i] = random();
            }
        }

        ~bloom() {
            delete[] seed_array;
        }

        // Add an item to the filter.
        bloomfilter::Status Add(const ItemType &item) {
            for (int i = 0; i < k; ++i) {
                size_t index = wrap_hash(item, seed_array[i]);
                bit_array[index] = true;
            }
            return bloomfilter::Ok;
        }

        // Add multiple items to the filter.
        bloomfilter::Status AddAll(const std::vector<ItemType> data, const size_t start,
                                   const size_t end) {
            for (int i = start; i < end; ++i) {
                Add(data[i]);
            }
            return Ok;

        }

        bloomfilter::Status AddAll(const ItemType *data, const size_t start,
                                   const size_t end) {
            for (int i = start; i < end; ++i) {
                Add(data[i]);
            }
            return Ok;
        }

        // Report if the item is inserted, with false positive rate.
        bloomfilter::Status Contain(const ItemType &item) const {
            for (int i = 0; i < k; ++i) {
                size_t index = wrap_hash(item, seed_array[i]);
                if (!bit_array[index])
                    return bloomfilter::NotFound;
            }
            return bloomfilter::Ok;

        }

        /* methods for providing stats  */
        // summary infomation
        std::string Info() const;

        // number of current inserted items;
        size_t Size() const { return size; }

        inline size_t wrap_hash(const ItemType &item, uint32_t seed) const {
            return hashing::hashint(item + seed) % size;
        }

    };
}

#endif //FILTERS_BLOOM_HPP
