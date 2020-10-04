
#include "wrap_fpd.hpp"

namespace Wrap_Fixed_pd {
    // using namespace fixed_pd;


    // auto vector_find(uint64_t quot, uint8_t rem, packed_fpd *pd) -> bool {
    //     auto my_key = std::make_tuple(quot, rem);
    //     return std::find(pd->vec.begin(), pd->vec.end(), my_key) != pd->vec.end();
    // }

    // void vector_add(uint64_t quot, uint8_t rem, packed_fpd *pd) {
    //     auto my_key = std::make_tuple(quot, rem);
    //     pd->vec.push_back(my_key);
    // }
    /* 
    void promote_quot_and_remove_from_body(uint64_t quot, size_t counter_size, uint64_t *header, __m512i *body) {
        // auto before_cap = get_capacity(pd);
        assert(false);
        auto safe_counter = fixed_pd::Header::read_counter(quot, header);
        size_t begin_fingerprint = fixed_pd::Header::get_start(quot, header);
        auto t1 = fixed_pd::Header::read_counter(quot, header);
        assert(t1 == safe_counter);
        for (size_t i = 0; i < counter_size; i++) {
            uint8_t temp_rem = ((uint8_t *) (&body))[begin_fingerprint + i];
            vector_add(quot, temp_rem, pd);
        }
        auto t2 = fixed_pd::Header::read_counter(quot, header);
        assert(t2 == safe_counter);
        fixed_pd::Body::remove_quot(begin_fingerprint, begin_fingerprint + counter_size, &body);
        auto t3 = fixed_pd::Header::read_counter(quot, header);
        assert(t3 == safe_counter);
        // auto after_cap = get_capacity(pd);
        // assert(before_cap == after_cap);
    }


    void promote_quot_and_remove_from_body(uint64_t quot, uint64_t *header, __m512i *body) {
        assert(false);
        size_t counter_size = fixed_pd::Header::read_counter(quot, pd->header);
        assert(counter_size < fixed_pd::counter_overflowed_val);
        assert(0 < counter_size);

        promote_quot_and_remove_from_body(quot, counter_size, pd);
    }
 */
    auto validate_init_packed_fpd(packed_fpd *pd) -> bool{
        uint64_t* h1 = pd->header1;
        uint64_t* h2 = pd->header2;
        for (size_t i = 0; i < 4; i++)
        {
            bool cond = (h1[0] == 0) && (h2[0] == 0);
            if (!cond){
                return false;
            }
        }

        __m512i all_zeros = __m512i{0,0,0,0,0,0,0,0}; 
        __m512i *b1 = &pd->body1;
        __m512i *b2 = &pd->body2;

        int c1 = memcmp(b1, &all_zeros, 64);
        int c2 = memcmp(b2, &all_zeros, 64);
        return (c1 == 0) && (c2 == 0);
        // auto cond = (*b1 == all_zeros) ;
        // bool cond = b1 == __m512i{0,0,0,0,0,0,0,0};
    }

    auto find(uint64_t quot, uint8_t rem, const uint64_t *header, const __m512i *body) -> int {
        auto counter = fixed_pd::Header::read_counter(quot, header);
        if (counter == 0) {
            // #ifdef PRINT
            // std::cout << "Find 1" << std::endl;
            // #endif// PRINT
            return 0;
        }

        else if (counter == fixed_pd::counter_overflowed_val) {
            // #ifdef PRINT
            // std::cout << "Find 2" << std::endl;
            // #endif// PRINT

            // return vector_find(quot, rem, pd);
            return -1;
        }

        uint64_t v = fixed_pd::Body::get_v(rem, body);
        if (!v) {
            // #ifdef PRINT
            // std::cout << "Find 3" << std::endl;
            // #endif// PRINT
            return 0;
        }

        uint64_t start = fixed_pd::Header::get_start(quot, header);
        // #ifdef PRINT
        // std::cout << "Find 4" << std::endl;
        // #endif// PRINT
        return (v >> start) & MASK(counter);
    }


    auto add(uint64_t quot, uint8_t rem, uint64_t *header, __m512i *body) -> int {
        // auto before_cap = get_capacity(pd);
        assert(quot < fixed_pd::QUOT_RANGE);
        int header_add_res = fixed_pd::Header::add(quot, header);
        if (header_add_res == -1) {
            uint64_t start = fixed_pd::Header::get_start(quot, header);
            assert(start <= fixed_pd::CAPACITY);
            uint64_t end = fixed_pd::Header::read_counter(quot, header);
            fixed_pd::Body::add(start, start + end - 1, rem, body);
            // #ifdef PRINT
            // std::cout << "Add 1" << std::endl;
            // #endif// PRINT

            // auto after_cap = get_capacity(pd);
            // assert(before_cap + 1 == after_cap);
            return -1;
        } else if (header_add_res == -2) {
            // vector_add(quot, rem, pd);

            // #ifdef PRINT
            // std::cout << "Add 2" << std::endl;
            // #endif// PRINT

            // auto after_cap = get_capacity(pd);
            // assert(before_cap + 1 == after_cap);
            return -2;
        } else if (header_add_res == -3) {
            return -3;
            // promote_quot_and_remove_from_body(quot, fixed_pd::max_valide_counter_value, pd);
            // vector_add(quot, rem, pd);

            // auto temp_counter = fixed_pd::Header::read_counter(quot, header);
            // assert(temp_counter == fixed_pd::counter_overflowed_val);

            // #ifdef PRINT
            // std::cout << "Add 3" << std::endl;
            // #endif// PRINT
            // auto after_cap = get_capacity(pd);
            // assert(before_cap + 1 == after_cap);
            // return 0;

        } else if (header_add_res == -4) {
            return -4;
            /* size_t max_quot = fixed_pd::Header::get_max_quot(header);
            auto temp_val = fixed_pd::Header::read_counter(max_quot, header);
            assert(temp_val < fixed_pd::counter_overflowed_val);
            assert(temp_val);

            promote_quot_and_remove_from_body(max_quot, pd);
            fixed_pd::Header::set_quot_as_overflow(max_quot, header);


            if (max_quot == quot) {
                vector_add(quot, rem, pd);
                // #ifdef PRINT
                std::cout << "Add 4" << std::endl;
                // #endif// PRINT
                auto after_cap = get_capacity(pd);
                assert(before_cap + 1 == after_cap);
                return 0;
            } else {
                // #ifdef PRINT
                std::cout << "Add 5" << std::endl;
                // #endif// PRINT

                //necessary for increasing relevent counter.
                int new_header_add_res = fixed_pd::Header::add(quot, header);
                assert(new_header_add_res == -1);
                uint64_t start = fixed_pd::Header::get_start(quot, header);
                uint64_t end = fixed_pd::Header::read_counter(quot, header);
                assert(end != fixed_pd::counter_overflowed_val);
                fixed_pd::Body::add(start, start + end - 1, rem, &body);
                auto after_cap = get_capacity(pd);
                assert(before_cap + 1 == after_cap);
                return 0;
            }
            // #ifdef PRINT
            // #endif// PRINT */

        } else {
            // #ifdef PRINT
            // std::cout << "Add 6" << std::endl;
            // #endif// PRINT
            assert(0);
        }
        return -42;
    }


    auto get_capacity(uint64_t *header, __m512i *body) -> size_t {
        return fixed_pd::Header::get_capacity(header);
        // auto b = vec.size();
        // return a + b;
    }
}// namespace Wrap_Fixed_pd