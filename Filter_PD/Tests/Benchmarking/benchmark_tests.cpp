//
// Created by tomer on 10/29/19.
//

#include "benchmark_tests.h"

auto random_vector(size_t size) -> vector<uint64_t> {
    random_device rnd_device;
    mt19937_64 mersenne_engine{rnd_device()};
    uniform_int_distribution<u_int64_t> dist{0, ~0ULL};

    auto gen = [&dist, &mersenne_engine]() { return dist(mersenne_engine); };

    vector<uint64_t> vec(size);
    generate(begin(vec), end(vec), gen);
}

auto rand_string(int minLength, int charsNum, int numOfDiffLength) -> string {
    int len = minLength bitor (rand() % numOfDiffLength);
    assert(4 < len);
    char s[len];
    for (int i = 0; i < len; ++i) s[i] = 64 bitor (rand() % charsNum);
    s[len] = 0;
    string res(s);
    return res;
}

void set_init(size_t size, set<string> *mySet, int minLength, int charsNum) {
    for (int i = 0; i < size; ++i) mySet->insert(rand_string(minLength, charsNum));
}

void vec_init(size_t size, vector<uint32_t> *my_vec) {
    random_device rnd_device;
    mt19937 mersenne_engine{rnd_device()};
    uniform_int_distribution<u_int64_t> dist{0, ~0ULL};

    auto gen = [&dist, &mersenne_engine]() { return dist(mersenne_engine); };
    my_vec->resize(size);
    generate(begin(*my_vec), end(*my_vec), gen);
}

void vector_lexicographic_init(size_t size, vector<string> *vec, int minLength, int charsNum) {
    size_t start_val = (1ul << (ulong) minLength) - 1;
    for (size_t i = 0; i < size; ++i) vec->push_back(to_string(start_val++));
}

auto b0(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
        ostream &os) -> ostream & {
    clock_t startRunTime = clock();
    set<string> member_set, nom_set;

    clock_t t0 = clock();
    size_t n = ceil(f * number_of_pds * load_factor);
    set_init(n, &member_set);
    double member_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    pow2c_filter a(number_of_pds, m, f, l);
    double init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    for (auto iter : member_set) a.insert(&iter);
    double insertion_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    set_init(lookup_reps, &nom_set);
    double nom_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
    t0 = clock();
//    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];
    for (auto iter : nom_set) a.lookup(&iter);

    double lookup_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double total_run_time = (clock() - startRunTime) / ((double) CLOCKS_PER_SEC);


    test_printer(n, 0, lookup_reps, false, set_ratio, counter, member_set_init_time, nom_set_init_time,
                 init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;

}

auto
b0_naive(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
         ostream &os) -> ostream & {
    clock_t startRunTime = clock();
    set<string> member_set, nom_set;

    clock_t t0 = clock();
    size_t n = ceil(f * number_of_pds * load_factor);
    set_init(n, &member_set);
    double member_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    pow2c_naive_filter a(number_of_pds, m, f, l);
    double init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    for (auto iter : member_set) a.insert(&iter);
    double insertion_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    set_init(lookup_reps, &nom_set);
    double nom_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
    t0 = clock();
//    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];
    for (auto iter : nom_set) a.lookup(&iter);

    double lookup_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double total_run_time = (clock() - startRunTime) / ((double) CLOCKS_PER_SEC);


    test_printer(n, 0, lookup_reps, false, set_ratio, counter, member_set_init_time, nom_set_init_time,
                 init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;

}

auto const_filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                        ostream &os) -> ostream & {
    clock_t startRunTime = clock();
    set<string> member_set, nom_set;

    clock_t t0 = clock();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    set_init(n, &member_set);
    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = clock();
    const_filter a(number_of_pds, m, f, l);
    double init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = clock();
    for (auto iter : member_set) a.insert(&iter);
    double insertion_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = clock();
    set_init(lookup_reps, &nom_set);
    double nom_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
    t0 = clock();
//    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];
    for (auto iter : nom_set) a.lookup(&iter);

    double lookup_time = (double) (clock() - t0) / (CLOCKS_PER_SEC);
    double total_run_time = (double) (clock() - startRunTime) / (CLOCKS_PER_SEC);


    test_printer(n, 0, lookup_reps, false, set_ratio, counter, member_set_init_time, nom_set_init_time,
                 init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
}

/*

ostream &
filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps, ostream &os) {
    auto start_run_time = chrono::high_resolution_clock::now();
    vector<uint32_t> member_vec, nom_vec;

    auto t0 = chrono::high_resolution_clock::now();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    vec_init(n, &member_vec);
    auto t1 = chrono::high_resolution_clock::now();
    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    pow2c_filter a(number_of_pds, m, f, l);
    t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    for (auto iter : member_vec) a.insert(iter);
    t1 = chrono::high_resolution_clock::now();
    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    vec_init(lookup_reps, &nom_vec);
    t1 = chrono::high_resolution_clock::now();
    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
    double set_ratio = nom_vec.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
//    for (auto iter : nom_vec) ++counter[w.lookup_verifier(&iter, call_adapt)];
    t0 = chrono::high_resolution_clock::now();
    for (auto iter : nom_vec) a.lookup(iter);
    t1 = chrono::high_resolution_clock::now();
    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();

//    chrono::microseconds
//    a.print_stats();

    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
               init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
}
*/


auto const_filter_rates32(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                          ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();
    vector<uint32_t> member_vec, nom_vec;

    auto t0 = chrono::high_resolution_clock::now();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    vec_init(n, &member_vec);
    auto t1 = chrono::high_resolution_clock::now();
    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = chrono::high_resolution_clock::now();
    const_filter a(number_of_pds, m, f, l);
    t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    for (auto iter : member_vec) a.insert(iter);
    t1 = chrono::high_resolution_clock::now();
    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    vec_init(lookup_reps, &nom_vec);
    t1 = chrono::high_resolution_clock::now();
    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
    double set_ratio = nom_vec.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
//    for (auto iter : nom_vec) ++counter[w.lookup_verifier(&iter, call_adapt)];
    t0 = chrono::high_resolution_clock::now();
    for (auto iter : nom_vec) a.lookup(iter);
    t1 = chrono::high_resolution_clock::now();
    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();

//    chrono::microseconds
//    a.print_stats();

    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
               init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
}


auto
cuckoo_filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, const size_t l, size_t lookup_reps,
                    ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();
    vector<uint32_t> member_vec, nom_vec;

    auto t0 = chrono::high_resolution_clock::now();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    vec_init(n, &member_vec);
    auto t1 = chrono::high_resolution_clock::now();
    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = chrono::high_resolution_clock::now();
    CuckooFilter<uint32_t, 7> a(n);
    t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    for (auto iter : member_vec) a.Add(iter);
    t1 = chrono::high_resolution_clock::now();
    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    vec_init(lookup_reps, &nom_vec);
    t1 = chrono::high_resolution_clock::now();
    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
    double set_ratio = nom_vec.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
//    for (auto iter : nom_vec) ++counter[w.lookup_verifier(&iter, call_adapt)];
    t0 = chrono::high_resolution_clock::now();
    for (auto iter : nom_vec) a.Contain(iter);
    t1 = chrono::high_resolution_clock::now();
    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();
//    chrono::microseconds
    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
               init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
    /*clock_t startRunTime = clock();
    vector<uint32_t> member_vec, nom_vec;

    clock_t t0 = clock();

    size_t total_elements_inserted = ceil((double) f * number_of_pds * load_factor);
    vec_init(total_elements_inserted, &member_vec);
    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = clock();
//    const size_t bits_per_element = l;
    CuckooFilter<uint32_t, 7> a(total_elements_inserted);
//    a(number_of_pds, m, f, l);
    double init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = clock();
    for (auto iter: member_vec) a.Add(iter);
    double insertion_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = clock();
    vec_init(lookup_reps, &nom_vec);
    double nom_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;
    double set_ratio = nom_vec.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
    t0 = clock();
//    for (auto iter : nom_vec) ++counter[w.lookup_verifier(&iter, call_adapt)];
    for (auto iter : nom_vec) a.Contain(iter);

    double lookup_time = (double) (clock() - t0) / (CLOCKS_PER_SEC);
    double total_run_time = (double) (clock() - startRunTime) / (CLOCKS_PER_SEC);


    test_printer(total_elements_inserted, 0, lookup_reps, false, set_ratio, counter, member_set_init_time,
                 nom_set_init_time,
                 init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
*/
}

template<class T>
auto template_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                    ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();
    vector<uint32_t> member_vec, nom_vec;

    auto t0 = chrono::high_resolution_clock::now();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    vec_init(n, &member_vec);
    auto t1 = chrono::high_resolution_clock::now();
    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = chrono::high_resolution_clock::now();
    T a(number_of_pds, m, f, l);
    t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    for (auto iter : member_vec) a.insert(iter);
    t1 = chrono::high_resolution_clock::now();
    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    vec_init(lookup_reps, &nom_vec);
    t1 = chrono::high_resolution_clock::now();
    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
    double set_ratio = nom_vec.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
//    for (auto iter : nom_vec) ++counter[w.lookup_verifier(&iter, call_adapt)];
    t0 = chrono::high_resolution_clock::now();
    for (auto iter : nom_vec) a.lookup(iter);
    t1 = chrono::high_resolution_clock::now();
    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();

    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
               init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
}

template<class T>
auto
filter_rates(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
             ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();
    set<string> member_set, nom_set;

    auto t0 = chrono::high_resolution_clock::now();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    set_init(n, &member_set);
    auto t1 = chrono::high_resolution_clock::now();
    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;

    t0 = chrono::high_resolution_clock::now();
    T a = T(number_of_pds, m, f, l);
    t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    for (auto iter : member_set) a.insert(&iter);
    t1 = chrono::high_resolution_clock::now();
    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    set_init(lookup_reps, &nom_set);
    t1 = chrono::high_resolution_clock::now();
    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
//    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];
    t0 = chrono::high_resolution_clock::now();
    for (auto iter : nom_set) a.lookup(&iter);
    t1 = chrono::high_resolution_clock::now();
    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();

    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
               init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
}

auto
filter_rates_simple_pd(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                       ostream &os) -> ostream & {
    auto start_run_time = chrono::high_resolution_clock::now();
    set<string> member_set, nom_set;

    auto t0 = chrono::high_resolution_clock::now();
    size_t n = ceil((double) f * number_of_pds * load_factor);
    set_init(n, &member_set);
    auto t1 = chrono::high_resolution_clock::now();
    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;
    t0 = chrono::high_resolution_clock::now();
    auto a = pow2c_filter(number_of_pds, m, f, l);
    t1 = chrono::high_resolution_clock::now();
    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    for (auto iter : member_set) a.insert(&iter);
    t1 = chrono::high_resolution_clock::now();
    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    set_init(lookup_reps, &nom_set);
    t1 = chrono::high_resolution_clock::now();
    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
//    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];
    t0 = chrono::high_resolution_clock::now();
    for (auto iter : nom_set) a.lookup(&iter);
    t1 = chrono::high_resolution_clock::now();
    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();

    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
               init_time, insertion_time, lookup_time, total_run_time, os);

//    os << a;
    return os;
}

void test_table_header_print() {
    if (!to_print_header)
        return;
    to_print_header = false;
    cout << left;
    cout << setw(WIDTH) << "number of elements";
    cout << setw(WIDTH) << "insertion/millisecond";
    cout << setw(WIDTH) << "lookup/millisecond";
    cout << setw(WIDTH) << "total runtime in millisecond" << endl;
}

/**Old testing function. should make changes for PD*/
auto
test_table(size_t n, double eps, size_t lookups_num, double set_ratio, int *counter, ulong member_set_init_time,
           ulong nom_set_init_time, ulong init_time, ulong insertion_time, ulong lookup_time, ulong total_run_time,
           ostream &os) -> ostream & {

    test_table_header_print();
    cout << setw(WIDTH) << n;
    cout << setw(WIDTH) << n / (insertion_time / 1e6);
    cout << setw(WIDTH) << lookups_num / (lookup_time / 1e6);
    cout << setw(WIDTH) << (total_run_time / 1e6) << endl;
    if (set_ratio < 1)
        cout << "\t\t" << "set_ratio is less than 1: " << set_ratio << endl;
    return os;
/*void b1(size_t f, size_t m, size_t l, size_t lookup_reps, ostream &os) {
    clock_t startRunTime = clock();
    set<string> member_set, nom_set;

    clock_t t0 = clock();
    set_init(insertion_reps, &member_set);
    double member_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    Wrapper w(n, eps, is_adaptive, remote_name);
    double init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    for (auto iter : member_set) w.insert(&iter, 0);
    double insertion_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    set_init(lookup_reps, &nom_set);
    double nom_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
    t0 = clock();
    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];

    double lookup_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double total_run_time = (clock() - startRunTime) / ((double) CLOCKS_PER_SEC);


    test_printer(n, eps, lookup_reps, is_adaptive, set_ratio, counter, member_set_init_time, nom_set_init_time,
                 init_time, insertion_time, lookup_time, total_run_time, os);

    os << w;
    return os;


}*/
}

auto test_table(size_t n, size_t lookups_num, double set_ratio, ulong insertion_time, ulong lookup_time,
                ulong total_run_time, ostream &os) -> ostream & {

    test_table_header_print();
    cout << setw(WIDTH) << n;
    cout << setw(WIDTH) << n / (insertion_time / 1e6);
    cout << setw(WIDTH) << lookups_num / (lookup_time / 1e6);
    cout << setw(WIDTH) << (total_run_time / 1e6) << endl;
    if (set_ratio < 1)
        cout << "\t\t" << "set_ratio is less than 1: " << set_ratio << endl;
    return os;
}


auto test_printer(size_t n, double eps, size_t lookups_num, bool is_adaptive, double set_ratio, int *counter,
                  double member_set_init_time, double nom_set_init_time, double init_time, double insertion_time,
                  double lookup_time, double total_run_time, ostream &os) -> ostream & {
//    counter_printer(os, counter);
    string name = "not adaptive";
    if (is_adaptive)
        name = "adaptive";
    double ans = ((double) counter[1]) / lookups_num;
    os << "member_set_init_time: " << member_set_init_time << endl;
    os << "nom_set_init_time: " << nom_set_init_time << endl;
    os << "nom_set to lookups_num ratio : " << set_ratio << endl << endl;
    os << "false positive rate:" << ans;
    os << "\tExpected: " << eps << "\terror deviation: " << 100 * (abs(ans - eps) / eps) << "%" << endl;
//    cout << "false positive rate:" << ans << endl;
    os << name << " init_time: " << init_time << endl;
    cout << "insertion_time: " << insertion_time << "\t" << n / insertion_time << " el/sec.\t";
    os << name << " insertion_time: " << insertion_time << "\t" << n / insertion_time << " el/sec" << endl;
    cout << "lookup_time: " << lookup_time << "\t" << lookups_num / lookup_time << " el/sec.\t";
    os << name << " lookup_time: " << lookup_time << "\t" << lookups_num / lookup_time << " el/sec" << endl << endl;
    os << "total_run_time: " << total_run_time << endl;
    cout << "total_run_time: " << total_run_time << endl;
    return os;
    //    ((double) n) / _helper_init_time << "el per sec" << endl;

}

auto lookup_result_array_printer(int *counter, size_t lookup_reps, size_t fp_size, double load_factor) -> ostream & {
    const size_t space = 24;
    cout << "\t";
    cout << setw(space) << left << "True Negative";
    cout << "\t";
    cout << setw(space) << left << "False Positive";
    cout << "\t";
    cout << setw(space) << left << "True Positive";
    cout << "\t";
    cout << setw(space) << left << "Load Factor" << endl;

    string s1 = to_string(counter[0]) + "(" + to_string(100 * counter[0] / (double) lookup_reps).substr(0, 3) + "%)";
    string s2 = to_string(counter[1]) + "(" + to_string(100 * counter[1] / (double) lookup_reps).substr(0, 3) + "%)";
    string s3 = to_string(counter[2]) + "(" + to_string(100 * counter[2] / (double) lookup_reps).substr(0, 3) + "%)";
    cout << "\t";
    cout << left << setw(space) << left << s1;
    cout << "\t";
    cout << left << setw(space) << left << s2;
    cout << "\t";
    cout << left << setw(space) << left << s3;
/*
//    cout << setw(space - 8) << left << counter[1] << "(" << 100 * counter[1] / (double) lookup_reps << left << "%)";
//    cout << setw(space) << counter[2] << "(" << 100 * counter[2] / (double) lookup_reps << left << "%)";
*/
    cout << "\t";
    cout << setw(space) << left << setw(space) << load_factor << endl;
    double exp_fp_ratio = 1 / (double) (1ull << fp_size), actual_fp_ratio = counter[1] / (double) (lookup_reps);
    auto dev = actual_fp_ratio / exp_fp_ratio;
    cout << "\t";
    cout << left << setw(space) << "Expected fp ratio";
    cout << "\t";
    cout << left << setw(space) << "Actual fp ratio";
    cout << "\t";
    cout << left << setw(space) << "Deviation ratio" << endl;
    cout << "\t";
    cout << left << setw(space) << to_sci(exp_fp_ratio);
    cout << "\t";
    cout << left << setw(space) << to_sci(actual_fp_ratio);
    cout << "\t";
    cout << left << setw(space) << to_sci(dev) << endl;
//    printf("Expected fp is :%f, got:%f", 1 / double(fp_size), counter[1] / (double) lookup_reps);
//    printf("Deviation from expected value is %f .\t Load factor is %f .\n", div, load_factor);

}

auto filter_r0(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l) -> bool {
    pow2c_filter d = pow2c_filter(number_of_pds, m, f, l);
    vector<string> member_set, non_member_set, to_be_deleted_set;
    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);
    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &member_set);
    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);

    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);

    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    for (auto iter : non_member_set) {
        bool c1, c2;
        c1 = find(member_set.begin(), member_set.end(), iter) != member_set.end();
        c2 = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c1 || c2)
            continue;
        if (d.lookup(&iter)) {
            cout << "False Positive:" << endl;
        }
    }

    counter = 0;
    for (auto iter : to_be_deleted_set) {
        d.remove(&iter);
        counter++;
    };

    counter = 0;
    for (auto iter : member_set) {
        bool c = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c)
            continue;
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    return true;

}

auto filter_naive_r0(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l) -> bool {
    pow2c_naive_filter d = pow2c_naive_filter(number_of_pds, m, f, l);
    vector<string> member_set, non_member_set, to_be_deleted_set;
    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);
    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &member_set);
    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);

    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);

    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    for (auto iter : non_member_set) {
        bool c1, c2;
        c1 = find(member_set.begin(), member_set.end(), iter) != member_set.end();
        c2 = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c1 || c2)
            continue;
        if (d.lookup(&iter)) {
            cout << "False Positive:" << endl;
        }
    }

    counter = 0;
    for (auto iter : to_be_deleted_set) {
        d.remove(&iter);
        counter++;
    };

    counter = 0;
    for (auto iter : member_set) {
        bool c = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c)
            continue;
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    return true;

}

auto const_filter_r0(size_t number_of_pds, float load_factor, size_t l) -> bool {
    size_t f = 32, m = 32;
    const_filter d = const_filter(number_of_pds, m, f, l);
    vector<string> member_set, non_member_set, to_be_deleted_set;
    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);
    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &member_set);
    vector_lexicographic_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);

    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);

    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    for (auto iter : non_member_set) {
        bool c1, c2;
        c1 = find(member_set.begin(), member_set.end(), iter) != member_set.end();
        c2 = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c1 || c2)
            continue;
        if (d.lookup(&iter)) {
            cout << "False Positive:" << endl;
        }
    }

    counter = 0;
    for (auto iter : to_be_deleted_set) {
        d.remove(&iter);
        counter++;
    };

    counter = 0;
    for (auto iter : member_set) {
        bool c = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c)
            continue;
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    return true;

}


auto filter_r1(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l) -> bool {
    pow2c_filter d = pow2c_filter(number_of_pds, m, f, l);
    set<string> member_set, non_member_set, to_be_deleted_set;
    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);

    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(number_of_elements_in_the_filter, &non_member_set);

    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);

    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    for (auto iter : non_member_set) {
        if (member_set.count(iter) || to_be_deleted_set.count(iter))
            continue;
        if (d.lookup(&iter)) {
            cout << "False Positive:" << endl;
        }
    }

    for (auto iter : to_be_deleted_set) d.remove(&iter);

    counter = 0;
    for (auto iter : member_set) {
        if (to_be_deleted_set.count(iter))
            continue;
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    return true;
}


auto filter_naive_r1(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l) -> bool {
    pow2c_naive_filter d = pow2c_naive_filter(number_of_pds, m, f, l);
    set<string> member_set, non_member_set, to_be_deleted_set;
    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(number_of_elements_in_the_filter, &non_member_set);

    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);

    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    for (auto iter : non_member_set) {
        if (member_set.count(iter) || to_be_deleted_set.count(iter))
            continue;
        if (d.lookup(&iter)) {
            cout << "False Positive:" << endl;
        }
    }

    for (auto iter : to_be_deleted_set) d.remove(&iter);

    counter = 0;
    for (auto iter : member_set) {
        if (to_be_deleted_set.count(iter))
            continue;
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    return true;
}


template<class T>
void filter_fp_rates(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l, size_t lookup_reps,
                     ostream &os) {

    auto d = T(number_of_pds, m, f, l);
    if (d.is_const_size()) {
        assert(false);
        f = 32, m = 32, l = 8;
    }

    auto number_of_elements_in_the_filter = floor(load_factor * number_of_pds * f);

    set<string> member_set, lookup_set, to_be_deleted_set;
    set_init(number_of_elements_in_the_filter / 2, &member_set);
    set_init(number_of_elements_in_the_filter / 2, &to_be_deleted_set);
    set_init(lookup_reps, &lookup_set);

    //Insertion.
    for (auto iter : member_set) d.insert(&iter);
    for (auto iter : to_be_deleted_set) d.insert(&iter);


    //Lookup validation of member_set elements.
    size_t counter = 0;
    for (auto iter : member_set) {
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    //Lookup validation of to_be_deleted_set elements .
    for (auto iter : to_be_deleted_set) {
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    //Evaluating false-positive ratio, before performing series of deletions.
    cout << "Before deletion" << endl;
    // [TN, FP, TP]
    int result_array[3] = {0, 0, 0};
    for (auto iter : lookup_set) {
        bool c1, c2;
        c1 = member_set.count(iter);
        c2 = to_be_deleted_set.count(iter);
/*
//        c1 = find(member_set.begin(), member_set.end(), iter) != member_set.end();
//        c2 = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
*/
        bool in_filter = c1 or c2;
        if (in_filter) cout << "in filter." << endl;
        if (d.lookup(&iter)) {
            if (in_filter) { result_array[2] += 1; }
            else { result_array[1] += 1; }
        } else {
            if (in_filter) {
                cout << "Had a false negative!" << endl;
            } else {
                result_array[0] += 1;
            }
        }
    }
    lookup_result_array_printer(result_array, lookup_reps, l, load_factor);

    counter = 0;
    for (auto iter : to_be_deleted_set) {
        d.remove(&iter);
        counter++;
    };

    counter = 0;
    for (auto iter : member_set) {
        bool c = to_be_deleted_set.count(iter);
//        bool c = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
        if (c) {
            cout << "here1" << endl;
            continue;
        }
        counter++;
        if (!d.lookup(&iter)) {
            cout << "False negative:" << endl;
            d.lookup(&iter);
            assert(d.lookup(&iter));
        }
    }

    cout << "\nAfter deletion" << endl;
    // [TN, FP, TP]
    result_array[0] = 0;
    result_array[1] = 0;
    result_array[2] = 0;

    for (auto iter : lookup_set) {
        bool c1 = member_set.count(iter);
        bool c2 = to_be_deleted_set.count(iter);
//        bool c1 = find(member_set.begin(), member_set.end(), iter) != member_set.end();
//        bool c2 = find(to_be_deleted_set.begin(), to_be_deleted_set.end(), iter) != to_be_deleted_set.end();
//        bool inside_filter = c1 & !c2;
        if (d.lookup(&iter)) {
            if (c1) { result_array[2] += 1; }
            else { result_array[1] += 1; }
        } else {
            if (c1 and (not c2)) {
                cout << "Had a false negative!" << endl;
            } else {
                result_array[0] += 1;
            }
        }
    }

    lookup_result_array_printer(result_array, lookup_reps, l, load_factor / 2);
}
/*
//
//template<class D>
//auto filter_rates_core(D *filter, size_t max_capacity, size_t lookup_reps, ostream &os) -> ostream & {
//    auto start_run_time = chrono::high_resolution_clock::now();
//    set<string> member_set, nom_set;
//
//    auto t0 = chrono::high_resolution_clock::now();
//    size_t n = max_capacity;
//    set_init(n, &member_set);
//    auto t1 = chrono::high_resolution_clock::now();
//    auto member_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
////    double member_set_init_time = (double) (clock() - t0) / CLOCKS_PER_SEC;
//
//    t0 = chrono::high_resolution_clock::now();
//    t1 = chrono::high_resolution_clock::now();
//    auto init_time = chrono::duration_cast<ns>(t1 - t0).count();
//
//    t0 = chrono::high_resolution_clock::now();
//    for (auto iter : member_set) filter->insert(&iter);
//    t1 = chrono::high_resolution_clock::now();
//    auto insertion_time = chrono::duration_cast<ns>(t1 - t0).count();
//
//    t0 = chrono::high_resolution_clock::now();
//    set_init(lookup_reps, &nom_set);
//    t1 = chrono::high_resolution_clock::now();
//    auto nom_set_init_time = chrono::duration_cast<ns>(t1 - t0).count();
//    double set_ratio = nom_set.size() / (double) lookup_reps;
//
//    // [TN, FP, TP]
//    int counter[3] = {0, 0, 0};
////    for (auto iter : nom_set) ++counter[w.lookup_verifier(&iter, call_adapt)];
//    t0 = chrono::high_resolution_clock::now();
//    for (auto iter : nom_set) filter->lookup(&iter);
//    t1 = chrono::high_resolution_clock::now();
//    auto lookup_time = chrono::duration_cast<ns>(t1 - t0).count();
//    auto total_run_time = chrono::duration_cast<ns>(t1 - start_run_time).count();
//
//    test_table(n, 0, lookup_reps, set_ratio, counter, member_set_init_time, nom_set_init_time,
//               init_time, insertion_time, lookup_time, total_run_time, os);
//
////    os << a;
//    return os;
//}
*/


//template bool
//validate_filter<gen_2Power<PD>>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
//                                   size_t lookup_reps);


template void
filter_fp_rates<const_filter>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l, size_t lookup_reps,
                              ostream &os);

void eval_fp_ratio();

template void
filter_fp_rates<pow2c_naive_filter>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
                                    size_t lookup_reps, ostream &os);

template void
filter_fp_rates<pow2c_filter>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l, size_t lookup_reps,
                              ostream &os);

template void
filter_fp_rates<gen_2Power<cg_PD>>(size_t number_of_pds, float load_factor, size_t m, size_t f, size_t l,
                                   size_t lookup_reps, ostream &os);

template ostream &
filter_rates<gen_2Power<cg_PD>>(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l,
                                size_t lookup_reps, ostream &os);

template ostream &
filter_rates<gen_2Power<PD>>(size_t number_of_pds, float load_factor, size_t f, size_t m, size_t l, size_t lookup_reps,
                             ostream &os);
/*

template
auto filter_rates_core<multi_dict64>(multi_dict64 *filter, size_t max_capacity, size_t lookup_reps,
                                     ostream &os) -> ostream &;
*/
