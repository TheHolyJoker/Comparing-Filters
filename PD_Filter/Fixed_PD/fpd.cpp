#include "fpd.hpp"

namespace fixed_pd {
    namespace Header {
        namespace v_fixed_pd {
            void header_to_array(const header_type *header, uint64_t *a) {
                for (size_t i = 0; i < QUOT_RANGE; i++) {
                    a[i] = read_counter(i, header);
                }
            }
            auto array_to_header_word(const uint64_t *a, size_t a_size) -> uint64_t {
                assert(a_size == word_capacity);// zero last words.
                uint64_t shift = 0;
                uint64_t res = 0;
                for (size_t i = 0; i < a_size; i++) {
                    uint64_t temp = a[i] << shift;
                    assert((temp >> shift) == a[i]);
                    assert(!(res & temp));
                    res |= temp;
                    shift += counter_size;
                }
                return res;
            }

            void write_k_header_words(header_type *header, const uint64_t *a, size_t a_size, size_t k) {
                assert(a_size == (k * word_capacity));// zero last words.
                // constexpr size_t words_count = (QUOT_RANGE * counter_size) / word_capacity;
                constexpr bool is_aligned = ((QUOT_RANGE * counter_size) % word_capacity) == 0;
                assert(k <= words_count);
                for (size_t i = 0; i < k; i++) {
                    const uint64_t *temp_pointer = a;
                    size_t to_add = i * word_capacity;

                    assert(a_size > to_add);
                    size_t temp_a_size = word_capacity;// default value.

                    if ((a_size - to_add) < temp_a_size)
                        temp_a_size = a_size - to_add;

                    header[i] = array_to_header_word(a + to_add, temp_a_size);
                }
            }

            void array_to_header(header_type *header, const uint64_t *a);

            void array_to_header(header_type *header, const uint64_t *a, size_t a_size) {
                assert(a_size <= QUOT_RANGE);// zero last words.
                // size_t b_size = (QUOT_RANGE / word_capacity) * word_capacity;
                // uint64_t b[b_size] = {0};
                // memcpy(b, a, a_size * CHAR_BIT);


                // constexpr size_t words_count = (QUOT_RANGE * counter_size) / word_capacity;
                constexpr bool is_aligned = ((QUOT_RANGE * counter_size) % word_capacity) == 0;
                if (is_aligned) {
                    write_k_header_words(header, a, a_size, words_count);
                    return;
                }
                write_k_header_words(header, a, a_size, words_count - 1);
                size_t a_index = (words_count - 1) * word_capacity;
                assert(a_index <= a_size);
                size_t new_a_size = a_size - a_index;

                uint64_t shift = 0;
                uint64_t temp_last_word = 0;
                for (size_t i = 0; i < new_a_size; i++) {
                    uint64_t temp = a[i + a_index] << shift;
                    assert((temp >> shift) == a[i]);
                    assert(!(temp_last_word & temp));
                    temp_last_word |= temp;
                    shift += counter_size;
                }
                if (new_a_size)
                    shift -= counter_size;


                uint64_t m_header = (header[words_count] & MASK(shift));
                assert(!(m_header & temp_last_word));
                uint64_t last_word = m_header | temp_last_word;
                header[words_count] = last_word;
            }

            auto compare_headers(const header_type *h1, const header_type *h2) -> bool {
                for (size_t i = 0; i < QUOT_RANGE; i++) {
                    bool temp = (read_counter(i, h1) == read_counter(i, h2));
                    if (!temp) {
                        return false;
                    }
                }
                return true;

                // constexpr size_t words_count = (QUOT_RANGE * counter_size) / word_capacity;
                // constexpr bool is_aligned = ((QUOT_RANGE * counter_size) % word_capacity) == 0;
                // constexpr bool is_byte_aligned = ((QUOT_RANGE * counter_size) % 8) == 0;
                // constexpr size_t byte_length = (QUOT_RANGE * counter_size) / 8;

                // auto ness_res = memcmp(h1, h2, byte_length);
                // if (ness_res != 0){
                //     return false;
                // }

                // bool res =
                // constexpr size_t rel_byte_index = (QUOT_RANGE * counter_size) & 7;
            }

            auto compare_arrays(const uint64_t *a1, const uint64_t *a2) -> bool {
                for (size_t i = 0; i < QUOT_RANGE; i++) {
                    bool temp = (a1[i] == a2[i]);
                    if (!temp) {
                        return false;
                    }
                }
                return true;
            }

        }// namespace v_fixed_pd

        namespace tests {
            auto rand_array(uint64_t *a, size_t a_size) -> void {
                constexpr uint64_t mask = counter_overflowed_val;
                for (size_t i = 0; i < a_size; i++) {
                    a[i] = rand() & counter_overflowed_val;
                }
            }

            auto decode_encode_test(header_type *header) -> bool {
                // constexpr size_t words_count = (QUOT_RANGE * counter_size) / word_capacity;
                uint64_t a[QUOT_RANGE] = {0};
                uint64_t temp_header[words_count] = {0};
                v_fixed_pd::header_to_array(header, a);
                v_fixed_pd::array_to_header(temp_header, a, QUOT_RANGE);

                return v_fixed_pd::compare_headers(header, temp_header);
            }

            auto array_to_header_by_insertions(header_type *header, const uint64_t *a, size_t a_size) -> bool {
                // does not work anymore. because of the policy of overflowing pd.
                assert(0);
                for (size_t i = 0; i < a_size; i++) {
                    for (size_t j = 0; j < a[i]; j++) {
                        add(i, header);
                    }
                }

                uint64_t b[QUOT_RANGE] = {0};
                v_fixed_pd::header_to_array(header, b);
                return v_fixed_pd::compare_arrays(a, b);
            }

            bool t0(size_t reps) {
                // constexpr size_t words_count = (QUOT_RANGE * counter_size) / word_capacity;
                for (size_t i = 0; i < reps; i++) {
                    header_type header[words_count] = {0};

                    uint64_t a[QUOT_RANGE] = {0};
                    rand_array(a, QUOT_RANGE);

                    header_type header2[words_count] = {0};
                    v_fixed_pd::array_to_header(header2, a, QUOT_RANGE);
                    bool r1 = decode_encode_test(header2);
                    bool r2 = array_to_header_by_insertions(header, a, QUOT_RANGE);

                    if (!(r1 && r2))
                        return false;
                    // if (!r2)
                    //     return false;
                    // bool b1 = decode_encode_test(header, )
                }
                return true;
            }

        }// namespace tests

        void set_quot_as_overflow(uint64_t quot, header_type *header) {
            constexpr uint64_t mask = MASK(counter_size);
            const size_t w_index = quot / word_capacity;
            const size_t rel_index = quot % word_capacity;

            // assert(curr_val < last_val);
            // header[w_index] += (last_val - curr_val) << (rel_index * counter_size);

            uint64_t temp_mask = mask << (rel_index * counter_size);
            header[w_index] |= temp_mask;
            bool cond = ((header[w_index] & temp_mask) == temp_mask);
            assert(cond);
        }
        // auto read_counter(uint64_t quot, const header_type *header) -> size_t {
        //     const size_t w_index = quot / word_capacity;
        //     const size_t rel_index = quot % word_capacity;
        //     return (header[w_index] >> (rel_index * counter_size)) & MASK(counter_size);
        // }

        auto did_quot_overflowed(uint64_t quot, header_type *header) -> bool {
            return read_counter(quot, header) == counter_overflowed_val;
        }
        // auto get_capacity(const header_type *header) -> size_t {
        //     return sum_words(header);
        // }
    }// namespace Header

    auto to_bin(uint64_t x, size_t length) -> std::string {
        assert(length <= 64);
        uint64_t b = 1ULL;
        std::string res = "";
        for (size_t i = 0; i < length; i++) {
            res += (b & x) ? "1" : "0";
            b <<= 1ul;
        }
        return res;
    }


    auto space_string(std::string s, size_t gap) -> std::string {
        std::string new_s = "";
        for (size_t i = 0; i < s.size(); i += gap) {
            if (i) {
                if ((i % (gap * gap)) == 0) {
                    new_s += "|";
                } else if ((i % gap) == 0) {
                    new_s += ".";
                }
            }
            new_s += s.substr(i, gap);
        }
        return new_s;
    }
    
    void spaced_bin(uint64_t word, size_t gap, size_t length) {
        std::string s = to_bin(word, length);
        s = space_string(s, gap);
        std::cout << "w: \t" << s << std::endl;
    }

    void p_format_word(uint64_t word, size_t gap, uint64_t length) {
        std::string s = to_bin(word, length);
        s = space_string(s, gap);
        std::cout << "w: \t" << s << std::endl;
    }

    // void p_format_word_by_int_and_sum(uint64_t word, size_t gap = counter_size, uint64_t length = 64);
    // {

    // }


}// namespace fixed_pd