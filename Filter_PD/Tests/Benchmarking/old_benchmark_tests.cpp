/*
//
// Created by tomer on 10/29/19.
//

#include "old_benchmark_tests.h"


string rand_string(int minLength, int charsNum, int numOfDiffLength) {
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

bool naive_true_positive_validation(GeneralBF *vbp, size_t reps) {
    set<string> temp_set;
    set_init(reps, &temp_set, MIN_ELEMENT_LENGTH, MIN_CHAR_RANGE);

    for (auto s: temp_set)
        vbp->add(&s);

    size_t counter = 0;
    for (auto s: temp_set) {
        counter++;
        if (!vbp->lookup(&s)) {
            cout << "First error in " << counter << ". Out of " << temp_set.size() << endl;
            return false;
        }
    }
    return true;

}

bool empty_true_negative_validation(GeneralBF *vbp, size_t reps) {
    if (vbp->get_filter_on_bits() > 0) {
        cout << "Filter_QF is not empty!" << endl;
        return false;
    }
    set<string> temp_set;
    set_init(reps, &temp_set, MIN_ELEMENT_LENGTH, MIN_CHAR_RANGE);


    size_t counter = 0;
    for (auto s: temp_set) {
        counter++;
        if (vbp->lookup(&s)) {
            cout << "First error in " << counter << ". Out of " << temp_set.size() << endl;
            return false;
        }
    }
    return true;
}

ostream &validate(size_t expected_el_num, double eps, size_t insertion_num, size_t lookup_reps, bool is_adaptive,
                  bool call_adapt, ostream &os) {
    set<string> insert_set, lookup_set;
    set_init(insertion_num, &insert_set);

    Wrapper w(expected_el_num, eps, is_adaptive);
    for (auto s: insert_set)
        w.insert(&s, 0);

    set_init(lookup_reps, &lookup_set);
    int res_array[3] = {0, 0, 0};

    for (auto s: lookup_set)
        ++res_array[w.lookup_verifier(&s, call_adapt)];

    for (auto s: insert_set)
        assert(w.naive_lookup(&s) == maybe_in_filter);

    print_array(res_array, 3, os);
    os << w << endl;
    return os;
}


ostream &
validate_adaptiveness(size_t expected_el_num, double eps, size_t insertion_num, size_t lookup_reps, size_t fp_reps,
                      ostream &os) {
    set<string> insert_set, lookup_set;
    set_init(insertion_num, &insert_set);

    Wrapper w(expected_el_num, eps, true);
    for (auto s: insert_set)
        w.insert(&s, 0);

    set_init(lookup_reps, &lookup_set);
    int res_array[3] = {0, 0, 0};

    for (auto s: lookup_set) {
        actual_lookup_res temp_res = w.lookup_verifier(&s, true);
        if (temp_res == false_positive) {
            actual_lookup_res second_res = w.lookup_verifier(&s, true);
            if (second_res == false_positive)
                assert(false);
            if (!(--fp_reps)) {
                cout << "passed fp validation." << endl;
                ++res_array[temp_res];
                break;
            }
        }
        ++res_array[temp_res];
    }
    for (auto s: insert_set)
        assert(w.naive_lookup(&s) == maybe_in_filter);

    print_array(res_array, 3, os);
    os << w << endl;
    return os;
}

void auto_rates(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
                bool remote_opt, bool naive_check, Remote_name remote_name, bool to_cout) {
    if (to_cout) {
        if (remote_opt)
            rates_attempt(n, eps, insertion_reps, lookup_reps, is_adaptive, call_adapt, remote_name, cout);
        else if (naive_check)
            naive_rates(n, eps, insertion_reps, lookup_reps, is_adaptive, call_adapt, remote_name, cout);
        else
            rates(n, eps, insertion_reps, lookup_reps, is_adaptive, call_adapt, remote_name, cout);
        return;
    }


    filebuf file;
    string temp_path = test_get_output_file_path(is_adaptive, call_adapt);
//    cout << "temp path is: " << temp_path << endl;
    file.open(temp_path, ios::out);
    ostream os(&file);

    if (remote_opt)
        rates_attempt(n, eps, insertion_reps, lookup_reps, is_adaptive, call_adapt, remote_name, os);
    else if (naive_check)
        naive_rates(n, eps, insertion_reps, lookup_reps, is_adaptive, call_adapt, remote_name, os);
    else
        rates(n, eps, insertion_reps, lookup_reps, is_adaptive, call_adapt, remote_name, os);
    file.close();

}

ostream &
rates(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
      Remote_name remote_name, ostream &os) {
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
}

ostream &
rates_attempt(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
              Remote_name remote_name, ostream &os) {
    clock_t startRunTime = clock();
    set<string> member_set, nom_set;

    clock_t t0 = clock();
    set_init(insertion_reps, &member_set);
    double member_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    Wrapper w(n, eps, is_adaptive, remote_name);
    double init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    for (auto iter : member_set) w.insert(&iter, 0, true);
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
}

ostream &naive_rates(size_t n, double eps, size_t insertion_reps, size_t lookup_reps, bool is_adaptive, bool call_adapt,
                     Remote_name remote_name, ostream &os) {
    clock_t startRunTime = clock();
    set<string> member_set, nom_set;

    clock_t t0 = clock();
    set_init(insertion_reps, &member_set);
    double member_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    Wrapper w(n, eps, is_adaptive, remote_name);
    double init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    for (auto iter : member_set) w.naive_add(&iter, 0);
    double insertion_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);

    t0 = clock();
    set_init(lookup_reps, &nom_set);
    double nom_set_init_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double set_ratio = nom_set.size() / (double) lookup_reps;

    // [TN, FP, TP]
    int counter[3] = {0, 0, 0};
    t0 = clock();
    for (auto iter : nom_set) ++counter[w.naive_lookup(&iter)];

    double lookup_time = (clock() - t0) / ((double) CLOCKS_PER_SEC);
    double total_run_time = (clock() - startRunTime) / ((double) CLOCKS_PER_SEC);


    test_printer(n, eps, lookup_reps, is_adaptive, set_ratio, counter, member_set_init_time, nom_set_init_time,
                 init_time, insertion_time, lookup_time, total_run_time, os);

    os << w;
    return os;
}

ostream &test_printer(size_t n, double eps, size_t lookups_num, bool is_adaptive, double set_ratio, int *counter,
                      double member_set_init_time, double nom_set_init_time, double init_time, double insertion_time,
                      double lookup_time, double total_run_time, ostream &os) {
    counter_printer(os, counter);
    string name = "not adaptive";
    if (is_adaptive)
        name = "adaptive";
    double ans = ((double) counter[1]) / lookups_num;
    os << "member_set_init_time: " << member_set_init_time << endl;
    os << "nom_set_init_time: " << nom_set_init_time << endl;
    os << "nom_set to lookups_num ratio : " << set_ratio << endl << endl;
    os << "false positive rate:" << ans;
    os << "\tExpected: " << eps << "\terror deviation: " << 100 * (abs(ans - eps) / eps) << "%" << endl;
    cout << "false positive rate:" << ans << endl;
    os << name << " init_time: " << init_time << endl;
    cout << name << " insertion_time: " << insertion_time << "\t" << n / insertion_time << " el/sec" << endl;
    os << name << " insertion_time: " << insertion_time << "\t" << n / insertion_time << " el/sec" << endl;
    cout << name << " lookup_time: " << lookup_time << "\t" << lookups_num / lookup_time << " el/sec" << endl << endl;
    os << name << " lookup_time: " << lookup_time << "\t" << lookups_num / lookup_time << " el/sec" << endl << endl;
    os << "total_run_time: " << total_run_time << endl;
    cout << "total_run_time: " << total_run_time << endl;
    return os;
    //    ((double) n) / _helper_init_time << "el per sec" << endl;

}

ostream &counter_printer(ostream &os, int *counter) {
    os << "[ true_negative, false_positive, true_positive]" << "\t[" << counter[0] << ", " << counter[1] << " ,"
       << counter[2] << "]" << endl;
    return os;
}


double get_relative_deviation(double act_res, double exp_res) {
    return abs(act_res - exp_res) / exp_res;
}


string test_get_output_file_path(bool is_adaptive, bool call_adapt) {
//    string temp_path = DEFAULT_OUTPUT_PATH;
    string temp_path = DEFAULT_REL_OUTPUT_PATH;
    string suffix = DEFAULT_OUTPUT_FILE_SUFFIX;
    if (!is_adaptive) {
        temp_path += "Regular/";
        size_t num = test_count_files(temp_path);
        temp_path += to_string(num);
        return temp_path + suffix;
    }

    temp_path += "Adaptive/";
    temp_path += (call_adapt) ? "With_Adapt/" : "Without_Adapt/";

    size_t num = test_count_files(temp_path);
    temp_path += to_string(num);
    return temp_path + suffix;

}

size_t test_count_files(const string &path) {
    size_t counter = 0;
    DIR *dir;
    struct dirent *ent;
    const char *char_path = path.c_str();
    if ((dir = opendir(char_path)) != nullptr) {
        while ((ent = readdir(dir)) != nullptr)
            counter++;
        closedir(dir);
        return counter;
    }
    return -1;
}*/
