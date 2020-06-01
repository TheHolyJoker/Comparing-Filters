//
// Created by tomer on 25/05/2020.
//

#include "tests.hpp"


uint64_t xorshf96() {          //period 2^96-1
    uint64_t t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}


auto rand_item() -> string {
    int minLength = 8, charsNum = 32, numOfDiffLength = 8;
    int len = minLength bitor (rand() % numOfDiffLength);
    assert(4 < len);
    char s[len];
    for (int i = 0; i < len; ++i) s[i] = 64 bitor (rand() % charsNum);
    s[len] = 0;
    string res(s);
    return res;
}


void set_init(size_t size, set<string> *mySet) {
    int minLength = 8, charsNum = 32;
    for (int i = 0; i < size; ++i) mySet->insert(rand_item());
}


void print_name(std::string filter_name) {
    size_t temp_width = default_line_width - 5;
    std::string line = " |" + std::string(temp_width, '-') + '|';
    std::cout << line << "\n |" << std::left << std::setw(temp_width) << filter_name << "|\n" << line
              << std::endl;
}

void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors) {
    size_t max_length = 0;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }



    // values for controlling format
    const uint32_t name_width = int(max_length);
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;

    const std::string line = sep + std::string(4 * total_width - 1, '-') + '|';
    std::cout << line << '\n' << sep << left;

    const std::string spacing = std::string(name_width, ' ');

    const size_t column_num = 4;
    string columns[column_num] = {"var_name", "ratio", "actual value", "divider"};
    size_t counter = 0;
    while (counter < column_num - 1) {
        cout << std::setw(name_width) << columns[counter++] << sep;
    }
    cout << std::setw(name_width) << columns[counter] << std::string(4, ' ') << sep;
    cout << '\n' << line << '\n';

//    assert(var_num % column_num == 0);
    for (int i = 0; i < var_num; ++i) {
        std::cout << sep << std::setw(name_width) << var_names[i];
        double rate = divisors[i] / (values[i] / 1e9);
        std::cout << sep << std::setw(name_width) << rate;
        std::cout << sep << std::setw(name_width) << divisors[i];
        std::cout << sep << std::setw(name_width) << values[i];
        auto temp_length = total_width - name_width * column_num;
        assert (temp_length > 0);
        std::cout << std::string(4, ' ') << sep << '\n';
    }
    std::cout << line << endl;

}

void print_line() {
    const uint32_t name_width = 24;
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << '\n';
}

void print_round_header() {
    size_t max_length = 24;
    const size_t var_num = 4;
    string names[var_num] = {"Round", "Insertion rate (op/sec)", "lookup rate (op/sec)", "deletion rate (op/sec)"};
//    for (int i = 0; i < var_num; ++i) {
//        max_length = max(names[i].length(), max_length);
//    }


// values for controlling format
    const uint32_t name_width = max_length;
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';

    std::cout << line << '\n';
//    for (int j = 0; j < var_num; ++j) {
//        std::cout << sep << std::setw(name_width) << names[j];
//    }
    std::cout << sep << std::setw(name_width) << "Round";
    std::cout << "  " << sep << std::setw(name_width) << "Insertion rate (op/sec)";
    std::cout << sep << std::setw(name_width) << "lookup rate (op/sec)";
    std::cout << sep << std::setw(name_width) << "deletion rate (op/sec)";
    std::cout << std::string(6, ' ') << sep << '\n' << line << endl;
//    std::cout <<  sep << '\n' << line << endl;

}

void print_single_round(size_t var_num, string *var_names, const size_t *values, const size_t *divisors) {
    bool is_last_round = values[0] == values[1];
    bool is_round_contain_two_digits = (values[0] > 9);
    size_t max_length = 24;
//    for (int i = 0; i < var_num; ++i) {
//        max_length = max(var_names[i].length(), max_length);
//    }

// values for controlling format
    const uint32_t name_width = int(max_length);
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';

    std::cout << line << '\n' << sep << left;
    //Round Number
    std::cout << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    double rate;
    //Insertion rate (op/sec)
    rate = divisors[0] / (values[2] / 1e9);
    std::cout << sep << std::setw(name_width) << rate;
    //lookup rate (op/sec)
    rate = divisors[1] / (values[3] / 1e9);
    std::cout << sep << std::setw(name_width) << rate;
    //deletion rate (op/sec)
    rate = divisors[2] / (values[4] / 1e9);
    std::cout << sep << std::setw(name_width) << rate;

    auto temp_length = total_width - name_width * var_num;
    assert (temp_length > 0);
//    std::cout  << sep << '\n';
    std::cout << std::string(6, ' ') << sep << '\n';

    if (is_last_round)
        std::cout << line << '\n';

//    << line << endl;

}

void
table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count) {
    size_t var_num = 3;
    string names[3] = {"expected_FP_count", "high_load_FP_count", "mid_load_FP_count"};
    size_t values[3] = {expected_FP_count, high_load_FP_count, mid_load_FP_count};
    size_t max_length = 0;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(names[i].length(), max_length);
    }

    // values for controlling format
    const int name_width = (default_line_width - var_num) / var_num;
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const int total_width = default_line_width;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << '\n' << sep << left;

    size_t counter = 0;
    while (counter < var_num - 1) {
        cout << std::setw(name_width) << names[counter++] << sep;
    }
    cout << std::setw(name_width - 1) << names[counter] << sep;
    cout << '\n' << line << '\n' + sep;


    counter = 0;
    while (counter < var_num - 1) {
        cout << std::setw(name_width) << values[counter++] << sep;
    }
    cout << std::setw(name_width - 1) << values[counter] << sep;
    cout << '\n' << line << '\n';


}


void print_single_round_false_positive_rates(size_t lookups_repetitions, size_t expected_false_positive,
                                             size_t true_positive_counter, size_t false_positive_counter) {
    /**every lookup result is one of the following:
     * FP - False Positive
     * TP - True Positive
     * TN - True Negative.
      */
    const size_t var_num = 6;
    string names[var_num] = {"#reps", "#expected FP", "#FP", "#TP", "expected FP ratio", "actual FP ratio"};
    size_t values[var_num - 2] = {lookups_repetitions, expected_false_positive, false_positive_counter,
                                  true_positive_counter};
    size_t max_length = 24;
    for (auto &name : names) {
        max_length = max(name.length(), max_length);
    }

// values for controlling format
    const uint32_t name_width = int(max_length);
    const std::string sep = " |";
    const uint32_t total_width = (name_width + sep.size()) * var_num;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << '\n' << sep << left;

    size_t counter = 0;
    while (counter < var_num) {
        cout << std::setw(name_width) << names[counter++] << sep;
    }
    cout << '\n' << line << '\n' + sep;


    counter = 0;
    while (counter < var_num - 2) {
        cout << std::setw(name_width) << values[counter++] << sep;
    }
//    std::cout << values[0] << "/" << std::setw(name_width - is_round_contain_two_digits) << values[1];
    cout << std::setw(name_width) << values[1] / ((double) values[0]) << sep;

    cout << std::setw(name_width) << values[2] / ((double) values[0]) << sep;
    cout << '\n' << line << '\n';


}

void validate_example1() {
    size_t filter_indicator = 0;
    ulong shift = 18u;
    size_t shift_add_to_lookups = 2u;
    size_t bench_precision = 20;
    size_t remainder_length = BITS_PER_ELEMENT_MACRO;

    size_t reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;
    double l1_LF = 0.95, l2_LF = 0.65;


    bool cond = w_validate_filter<simple_bloom, uint64_t>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
                                                          l2_LF);
    assert(cond);
    cond = w_validate_filter<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
            max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF);
    assert(cond);
    cond = w_validate_filter<MortonFilter, uint64_t, false>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
                                                            l2_LF);
    assert(cond);
    cond = w_validate_filter<uint64_t, hash_table>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF);
    assert(cond);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

template<>
auto CF_rates_wrapper<s_dict32>(size_t filter_max_capacity, size_t lookup_reps,
                                size_t error_power_inv, size_t l1_counter_size,
                                size_t l2_counter_size, double level1_load_factor,
                                double level2_load_factor, ostream &os) -> ostream & {

    auto start_run_time = chrono::high_resolution_clock::now();
    auto t0 = chrono::high_resolution_clock::now();
    using Table = s_dict32;
    Table filter(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
    auto t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    print_name(std::string("s_dict32"));
    CF_rates_core<Table>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
    return os;
}*/


int old_main(int argc, char **argv) {
    //Default values
    size_t filter_indicator = 0;
    ulong shift = 18u;
    size_t shift_add_to_lookups = 2u;
    size_t bench_precision = 20;
    size_t remainder_length = BITS_PER_ELEMENT_MACRO;

    size_t reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;

    /**Validation of the filters */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//    size_t l1_counter_size = 3, l2_counter_size = 7;
    double l1_LF = 0.95, l2_LF = 0.65;

    bool cond = w_validate_filter<simple_bloom, uint64_t>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
                                                          l2_LF);
    assert(cond);
    cond = w_validate_filter<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
            max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF);
    assert(cond);
    cond = w_validate_filter<MortonFilter, uint64_t, false>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF,
                                                            l2_LF);
    assert(cond);
    cond = w_validate_filter<uint64_t, hash_table>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, l1_LF, l2_LF);
    assert(cond);

    /**Parsing*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char *end;
    size_t values[4]{filter_indicator, shift, shift_add_to_lookups, bench_precision};
    for (int i = 1; i < argc; ++i) {
        values[i - 1] = strtol(argv[i], &end, 10);
    }

    filter_indicator = values[0];
    shift = values[1];
    shift_add_to_lookups = values[2];
    bench_precision = values[3];

    /**Old Benchmarking*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;
    switch (filter_indicator) {
        case 0:
            benchmark_wrapper<simple_bloom, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
            benchmark_wrapper<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
                    max_distinct_capacity, reps, remainder_length, bench_precision);
            benchmark_wrapper<MortonFilter, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
            benchmark_wrapper<uint64_t, hash_table>(max_distinct_capacity, reps, remainder_length, bench_precision);
            break;
        case 1:
            benchmark_wrapper<simple_bloom, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
        case 2:
            benchmark_wrapper<cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO>, uint64_t>(
                    max_distinct_capacity, reps, remainder_length, bench_precision);
        case 3:
            benchmark_wrapper<MortonFilter, uint64_t>(max_distinct_capacity, reps, remainder_length, bench_precision);
        case 4:
            benchmark_wrapper<uint64_t, hash_table>(max_distinct_capacity, reps, remainder_length, bench_precision);
        default:
            break;
    }

    return 0;
}

/*

//    v_true_positive<simple_pd>(1u << 12u);
//    bool cond =  v_true_positive<bloomfilter::bloom<uint64_t, 8, false, HashUtil>>(1u<<12u);
//    bool cond = v_true_positive<simple_bloom>(1u << 3u);
//    assert(cond);
//    cond = v_return_false_when_empty<simple_bloom>();
//    assert(cond);
//    std::cout << "Bloom passed" << std::endl;
//
//    cond = v_true_positive_att<uint64_t, 8, false, hashing::TwoIndependentMultiplyShift, bloomfilter::bloom>(8);
//    assert(cond);

//    cond = v_true_positive_att< uint64_t, 8, false, HashUtil, bloomfilter::bloom<uint64_t, 8, false,HashUtil>>(1000);
//    cond = v_true_positive_att<bloomfilter::bloom<uint64_t, 8, false,HashUtil>>(1000);

//    cond = w_validate_filter<uint64_t, 8, false, hashing::TwoIndependentMultiplyShift, bloomfilter::bloom>(
//            max_distinct_capacity, reps, 8, l1_LF, l2_LF);
//    assert(cond);

//    using Table_bloom = FilterAPI<simple_bloom>;
//    ulong shift = 15u;
//    size_t reps = 1u << (shift + 3u), max_distinct_capacity = 1u << shift;
//    size_t remainder_length = BITS_PER_ELEMENT_MACRO;
//    size_t l1_counter_size = 3, l2_counter_size = 7;
//    double l1_LF = 0.95, l2_LF = 0.65;
//    bool cond;
*/
