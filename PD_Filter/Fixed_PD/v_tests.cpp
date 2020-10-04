


#include "v_tests.hpp"
namespace fpd_tests {
    // using namespace Wrap_Fixed_pd;
    // void pd_init(__m512i *pd) {
    //     *pd = __m512i{(INT64_C(1) << 50) - 1, 0, 0, 0, 0, 0, 0, 0};
    // }

    auto insert_find_single(int64_t quot, uint8_t rem, Wrap_Fixed_pd::packed_fpd *pd) -> bool {
        Wrap_Fixed_pd::add(quot, rem, pd);
        auto find_res = Wrap_Fixed_pd::find(quot, rem, pd);
        assert(find_res);
        return true;
    }

    auto insert_find_single_with_capacity(int64_t quot, uint8_t rem, Wrap_Fixed_pd::packed_fpd *pd) -> bool {
        auto old_capacity = Wrap_Fixed_pd::get_capacity(pd);
        Wrap_Fixed_pd::add(quot, rem, pd);
        auto find_res = Wrap_Fixed_pd::find(quot, rem, pd);
        assert(find_res);
        auto new_capacity = Wrap_Fixed_pd::get_capacity(pd);
        assert(new_capacity == old_capacity + 1);
        return true;
    }


    auto insert_find_all(Wrap_Fixed_pd::packed_fpd *pd) -> bool {
        for (size_t q = 0; q < fixed_pd::QUOT_RANGE; q++) {
            for (size_t r = 0; r < 256; r++) {
                insert_find_single(q, r, pd);
            }
        }
        return true;
    }


    auto insert_find_all() -> bool {
        Wrap_Fixed_pd::packed_fpd pd = Wrap_Fixed_pd::packed_fpd();
        return insert_find_all(&pd);
    }

    auto rand_test1() -> bool {
        size_t max_capacity = fixed_pd::CAPACITY;
        // uint64_t valid_max_quot = 0;
        Wrap_Fixed_pd::packed_fpd pd = Wrap_Fixed_pd::packed_fpd();
        for (size_t i = 0; i < max_capacity; i++) {
            uint64_t q = rand() % fixed_pd::QUOT_RANGE;
            uint64_t r = rand() & 255;
            Wrap_Fixed_pd::add(q, r, &pd);

            assert(Wrap_Fixed_pd::find(q, r, &pd));
            auto temp_capacity = Wrap_Fixed_pd::get_capacity(&pd);
            assert(Wrap_Fixed_pd::get_capacity(&pd) == i + 1);
        }
        return true;
    }

    auto rand_test2() -> bool {
        size_t max_capacity = fixed_pd::CAPACITY + (fixed_pd::CAPACITY / 4);
        // uint64_t valid_max_quot = 0;
        Wrap_Fixed_pd::packed_fpd pd = Wrap_Fixed_pd::packed_fpd();
        for (size_t i = 0; i < max_capacity; i++) {
            uint64_t q = rand() % fixed_pd::QUOT_RANGE;
            uint64_t r = rand() & 255;
            Wrap_Fixed_pd::add(q, r, &pd);

            assert(Wrap_Fixed_pd::find(q, r, &pd));
            auto temp_capacity = Wrap_Fixed_pd::get_capacity(&pd);
            assert(Wrap_Fixed_pd::get_capacity(&pd) == i + 1);
        }
        return true;
    }

    auto determ_no_false_positive() -> bool {
        size_t max_capacity = fixed_pd::CAPACITY + (fixed_pd::CAPACITY / 4);
        // std::vector<std::tuple<uint64_t, uint64_t>> vals;
        std::vector<Wrap_Fixed_pd::vec_key> vals;
        // uint64_t valid_max_quot = 0;
        Wrap_Fixed_pd::packed_fpd pd = Wrap_Fixed_pd::packed_fpd();
        for (size_t i = 0; i < max_capacity; i++) {
            uint64_t q = rand() % fixed_pd::QUOT_RANGE;
            uint64_t r = rand() & 255;
            Wrap_Fixed_pd::vec_key temp_key = std::make_tuple(q, r);
            vals.push_back(temp_key);
            Wrap_Fixed_pd::add(q, r, &pd);

            assert(Wrap_Fixed_pd::find(q, r, &pd));
            auto temp_capacity = Wrap_Fixed_pd::get_capacity(&pd);
            assert(Wrap_Fixed_pd::get_capacity(&pd) == i + 1);
        }

        for (size_t i = 0; i < (1<<10); i++)  
        {
            uint64_t q = rand() % fixed_pd::QUOT_RANGE;
            uint64_t r = rand() & 255;
            Wrap_Fixed_pd::vec_key temp_key = std::make_tuple(q, r);
            if (std::find(vals.begin(), vals.end(), temp_key) != vals.end()){
                continue;
            }
            
            assert(!Wrap_Fixed_pd::find(q, r, &pd));
        }
        return true;
        
    }


}// namespace fpd_tests
