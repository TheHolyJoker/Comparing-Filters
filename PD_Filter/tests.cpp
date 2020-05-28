////
//// Created by tomer on 25/05/2020.
////
//
//#include "tests.hpp"
//
//
//auto rand_string(int minLength, int charsNum, int numOfDiffLength) -> string {
//    int len = minLength bitor (rand() % numOfDiffLength);
//    assert(4 < len);
//    char s[len];
//    for (int i = 0; i < len; ++i) s[i] = 64 bitor (rand() % charsNum);
//    s[len] = 0;
//    string res(s);
//    return res;
//}
//
//
//void set_init(size_t size, set<string> *mySet, int minLength, int charsNum) {
//    for (int i = 0; i < size; ++i) mySet->insert(rand_string(minLength, charsNum));
//}
//
//
//void print_name(std::string filter_name) {
//    std::string line = " |" + std::string(default_line_width - 1, '-') + '|';
//    std::cout << line << "\n |" << std::left << std::setw(default_line_width - 1) << filter_name << "|\n" << line
//              << std::endl;
//}
//
//void table_print_rates(size_t var_num, string *var_names, size_t *values, size_t *divisors) {
//    size_t max_length = 0;
//    for (int i = 0; i < var_num; ++i) {
//        max_length = max(var_names[i].length(), max_length);
//    }
//
//
//
//    // values for controlling format
//    const uint32_t name_width = int(max_length);
//    const int int_width = 7;
//    const int dbl_width = 12;
//    const int num_flds = 7;
//    const std::string sep = " |";
//    const uint32_t total_width = name_width * 2u + int_width * 2u + dbl_width * 3u + sep.size() * num_flds;
//    const std::string line = sep + std::string(total_width - 1, '-') + '|';
//    std::cout << line << '\n' << sep << left;
//
//    const std::string spacing = std::string(name_width, ' ');
//
//    const size_t column_num = 4;
//    string columns[column_num] = {"var_name", "ratio", "actual value", "divider"};
//    size_t counter = 0;
//    while (counter < column_num - 1) {
//        cout << std::setw(name_width) << columns[counter++] << sep;
//    }
//    cout << std::setw(name_width) << columns[counter] << std::string(4, ' ') << sep;
//    cout << '\n' << line << '\n';
//
////    assert(var_num % column_num == 0);
//    for (int i = 0; i < var_num; ++i) {
//        std::cout << sep << std::setw(name_width) << var_names[i];
//        double rate = divisors[i] / (values[i] / 1e9);
//        std::cout << sep << std::setw(name_width) << rate;
//        std::cout << sep << std::setw(name_width) << divisors[i];
//        std::cout << sep << std::setw(name_width) << values[i];
//        auto temp_length = total_width - name_width * column_num;
//        assert (temp_length > 0);
//        std::cout << std::string(4, ' ') << sep << '\n';
//    }
//    std::cout << line << endl;
//
//}
//
//void
//table_print_false_positive_rates(size_t expected_FP_count, size_t high_load_FP_count, size_t mid_load_FP_count) {
//    size_t var_num = 3;
//    string names[3] = {"expected_FP_count", "high_load_FP_count", "mid_load_FP_count"};
//    size_t values[3] = {expected_FP_count, high_load_FP_count, mid_load_FP_count};
//    size_t max_length = 0;
//    for (int i = 0; i < var_num; ++i) {
//        max_length = max(names[i].length(), max_length);
//    }
//
//    // values for controlling format
//    const int name_width = (default_line_width - var_num) / var_num;
//    const int int_width = 7;
//    const int dbl_width = 12;
//    const int num_flds = 7;
//    const std::string sep = " |";
//    const int total_width = default_line_width;
//    const std::string line = sep + std::string(total_width - 1, '-') + '|';
//    std::cout << line << '\n' << sep << left;
//    size_t counter = 0;
//    while (counter < var_num - 1) {
//        cout << std::setw(name_width) << names[counter++] << sep;
//    }
//    cout << std::setw(name_width - 1) << names[counter] << sep;
//    cout << '\n' << line << '\n' + sep;
//
//
//    counter = 0;
//    while (counter < var_num - 1) {
//        cout << std::setw(name_width) << values[counter++] << sep;
//    }
//    cout << std::setw(name_width - 1) << values[counter] << sep;
//    cout << '\n' << line << '\n';
//
//
//}
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//template<>
//auto CF_rates_wrapper<s_dict32>(size_t filter_max_capacity, size_t lookup_reps,
//                                                         size_t error_power_inv, size_t l1_counter_size,
//                                                         size_t l2_counter_size, double level1_load_factor,
//                                                         double level2_load_factor, ostream &os) -> ostream & {
//
//    auto start_run_time = chrono::high_resolution_clock::now();
//    auto t0 = chrono::high_resolution_clock::now();
//    using Table = s_dict32;
//    Table filter(filter_max_capacity, error_power_inv, level1_load_factor, level2_load_factor);
//    auto t1 = chrono::high_resolution_clock::now();
//    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
//
//    print_name(std::string("s_dict32"));
//    CF_rates_core<Table>(&filter, filter_max_capacity, lookup_reps, init_time, error_power_inv, os);
//    return os;
//}