#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c"
//
// Created by tomer on 10/25/19.
//

#include "validation_tests.h"

namespace namePD {

    auto t1(bool to_print) -> bool {
        size_t m = 16, f = 8, l = 5;
        PD d = PD(m, f, l);
        if (to_print) {
            cout << "start print:\n";
            d.print();
            cout << endl;
        }
        for (size_t i = 0; i < 8; ++i) {
            size_t q = i / 2;
            FP_TYPE r = i + 1;
            d.insert(q, r);
//        d.print();
//        cout << endl;
            if (to_print) { d.print(); }
            if (not d.lookup(q, r)) {
                cout << i << " index failed." << endl;
                d.print();
                d.lookup(q, r);
                return false;
            }
//        cout << i << " finished.\n$$$\n" << endl;
        }
        cout << "Passed t1!" << endl;
        return true;
    }

    auto t2(bool to_print) -> bool {
        size_t m = 16, f = 8, l = 5;
        PD d = PD(m, f, l);
//    cout << "start print:\n";
//    d.print();
//    cout << endl;
        for (size_t i = 0; i < 8; ++i) {
            size_t q = i / 2;
            FP_TYPE r = i + 1;
            d.insert(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (not d.lookup(q, r)) {
                cout << i << " lookup failed." << endl;
                return false;
            }
            d.remove(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (d.lookup(q, r)) {
                cout << i << " lookup after deletion failed." << endl;
                return false;
            }
        }
        cout << "Passed t2!" << endl;
        return true;
    }

    auto t3(bool to_print) -> bool {
        size_t m = 16, f = 8, l = 5;
        PD d = PD(m, f, l);
        for (size_t i = 0; i < 8; ++i) {
            size_t q = i / 2;
            FP_TYPE r = i + 1;
            d.insert(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (not d.lookup(q, r)) {
                cout << i << " lookup failed." << endl;
                return false;
            }
        }
        if (to_print) { cout << "insertion done!" << endl; }
        for (size_t i = 0; i < 8; ++i) {
            size_t q = i / 2;
            FP_TYPE r = i + 1;
            d.remove(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (d.lookup(q, r)) {
                cout << i << " lookup failed." << endl;
                return false;
            }
        }


//    return true;



        /*d.remove(q, r);
        if (to_print) {
            cout << i << "\t";
            d.print();
        }
        if (d.lookup(q, r)) {
            cout << i << " lookup after deletion failed." << endl;
            return false;
        }*/
        cout << "Passed t3!" << endl;
        return true;
    }

    auto t4(bool to_print) -> bool {
        size_t m = 16, f = 8, l = 5;
        PD d = PD(m, f, l);
        for (size_t i = 8; i > 0; --i) {
            size_t q = (i - 1) / 2;
            FP_TYPE r = i;
            d.insert(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (not d.lookup(q, r)) {
                cout << i << " lookup failed." << endl;
                return false;
            }
        }
        if (to_print) { cout << "insertion done!" << endl; }
        for (size_t i = 0; i < 8; ++i) {
            size_t q = i / 2;
            FP_TYPE r = i + 1;
            if (not d.lookup(q, r)) {
                cout << i << " lookup before failed." << endl;
                return false;
            }
            d.remove(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (d.lookup(q, r)) {
                cout << i << " lookup after failed." << endl;
                return false;
            }
        }
        cout << "Passed t4!" << endl;
        return true;
    }

    auto t5(size_t m, size_t f, size_t l, bool to_print) -> bool {
        assert(f < (1ULL << l));
        PD d = PD(m, f, l);
        for (size_t i = f; i > 0; --i) {
            size_t q = (i - 1) / 2;
            FP_TYPE r = i;
            d.insert(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (not d.lookup(q, r)) {
                cout << i << " lookup failed." << endl;
                return false;
            }
        }
        if (to_print) { cout << "insertion done!" << endl; }
        for (size_t i = 0; i < f; ++i) {
            size_t q = i / 2;
            FP_TYPE r = i + 1;
            if (not d.lookup(q, r)) {
                cout << i << " lookup before failed." << endl;
                return false;
            }
            d.remove(q, r);
            if (to_print) {
                cout << i << "\t";
                d.print();
            }
            if (d.lookup(q, r)) {
                cout << i << " lookup after failed." << endl;
                return false;
            }
        }
        cout << "Passed t5!" << endl;
        return true;
    }

    auto validate_PD_single_run(size_t reps, bool to_print) -> bool {
        srand(clock());
        size_t f = rand() % (256 - 32) + 32;
        size_t m = f + 1 + (rand() % (f - 1));
        size_t l = 11 + (rand() % (31 - 11));

        PD d = PD(m, f, l);


        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }

        return r0_core(reps, &d, m, f, l, to_print);
        return false;
    }

    template<class T>
    auto validate_CPD(size_t reps, bool to_seed, bool to_print) -> bool {
        if (to_seed)
            srand(clock());
        size_t f = (rand() % (512 - 32)) + 32;
        size_t m = f;
        size_t l = 3 + (rand() % (31 - 3));
        size_t counter_size = 3;

        T d = T(m, f, l, counter_size);


        auto res = validate_PD_core<T>(reps, &d, m, f, l, to_print, 0);
        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
            cout << "counter_size is: " << l << endl;
        }
        return res;
    }

    template<class T>
    auto validate_PD(size_t reps, bool to_seed, bool to_print) -> bool {
        if (to_seed)
            srand(clock());
        size_t f = (rand() % (512 - 32)) + 32;
        size_t m = f;
        size_t l = 3 + (rand() % (31 - 3));

        T d = T(m, f, l);


        auto res = validate_PD_core<T>(reps, &d, m, f, l, to_print, 0);
        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }
        return res;
    }

    template<class T>
    auto validate_PD_higher_load(size_t reps, float load_factor, bool to_seed, bool to_print) -> bool {
        if (to_seed)
            srand(clock());
        size_t f = rand() % (512 - 32) + 32;
        size_t m = f;
        size_t l = 11 + (rand() % (31 - 11));

//    if ((m + f) % l == 0)
//        cout << "l divides (m+f)." << endl;

        T d = T(m, f, l);


        auto res = validate_PD_core<T>(reps, &d, m, f, l, to_print, load_factor);
        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }
        return res;
    }


    template<class T>
    auto validate_PD_core(size_t reps, T *d, size_t m, size_t f, size_t l, bool to_print, float load_factor) -> bool {
        auto naive_d = naive_PD(m, f, l);
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        bool same_res = true;

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % m;
            out_r[i] = rand() % SL(l - 1);
        }
        if (load_factor != 0) {
            int lim = floor(f * load_factor);
            for (int i = 0; i < lim; ++i) {
                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (to_print) { cout << "in: " << q << ", " << r << endl; }

                d->insert(q, r);
                naive_d.insert(q, r);
                in_q.push_back(q);
                in_r.push_back(r);
                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);
            }
        }
        for (size_t i = 0; i < reps; ++i) {
            if (to_print) {
                cout << *d;
                cout << endl;
                cout << i << "\t";
            }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (to_print) { cout << "in: " << q << ", " << r << endl; }

                same_res = (d->lookup(q, r) == naive_d.lookup(q, r));
                if (!same_res) {
                    break_point_helper();
                    cout << "case 1 fail " << i << endl;
                    return false;
                }
                /*    d->lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            break_point_helper();
                            cout << "case 1 fail " << i << endl;
                            return false;
                        }
                    } else {
                        break_point_helper();
                        cout << "case 1 fail " << i << endl;
                        return false;
                    }*/
                d->insert(q, r);
                naive_d.insert(q, r);
                same_res = (d->lookup(q, r) == naive_d.lookup(q, r));
                if (!same_res) {
                    break_point_helper();
                    d->lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del: " << q << ", " << r << endl; }
//            bool BPC = (q == 105) and (i == 6);
                same_res = (d->lookup(q, r) == naive_d.lookup(q, r));
                if (!same_res) {
                    break_point_helper();
                    d->lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }


                d->remove(q, r);
                naive_d.remove(q, r);

                same_res = (d->lookup(q, r) == naive_d.lookup(q, r));
                if (!same_res) {
                    auto a_res = d->lookup(q, r);
                    bool v_res = naive_d.lookup(q, r);
                    break_point_helper();
                    d->lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    return false;
                }
                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        if (to_print) { cout << "Validation of template pd passed." << endl; }
        return true;

    }

/*
bool does_to_vectors_contain_two_elements_in_specific_index(){
    auto it = find(in_q.begin(), in_q.end(), q);
    if (it != in_q.end()) {
        size_t index2 = distance(in_q.begin(), it);
        if (in_q[index2] != r) {
            break_point_helper();
            cout << "case 1 fail " << i << endl;
            return false;
        }
    } else {
        break_point_helper();
        cout << "case 1 fail " << i << endl;
        return false;
    }
}
*/

    auto r0_core(size_t reps, PD *d, size_t m, size_t f, size_t l, bool to_print) -> bool {
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % m;
            out_r[i] = rand() % SL(l - 1);
        }

        for (size_t i = 0; i < reps; ++i) {
            if (to_print) { cout << i << "\t"; }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                if (to_print) { cout << "in" << endl; }

                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (d->lookup(q, r)) {
                    d->lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            cout << "case 1 fail " << i << endl;
                            return false;
                        }
                    } else {
                        cout << "case 1 fail " << i << endl;
                        return false;
                    }

                }
                d->insert(q, r);
                if (not d->lookup(q, r)) {
                    d->lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del\t" << endl; }

                if (not d->lookup(q, r)) {
                    d->lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }

                d->remove(q, r);
                if (d->lookup(q, r)) {
                    d->lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    return false;
                }


                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        cout << "validate_PD_single_run passed" << endl;
        return true;

    }

    auto validate_PD_by_load_factor(size_t reps, float load_factor, size_t m, size_t f, bool to_print) -> bool {
        srand(clock());
//    size_t f = rand() % (256 - 32) + 32;
//    size_t m = f + 1 + (rand() % (f - 1));
        size_t l = 11 + (rand() % (31 - 11));

        PD d = PD(m, f, l);


        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }

        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % m;
            out_r[i] = rand() % SL(l - 1);
        }
        while (d.get_capacity() < floor(load_factor * f)) {
            size_t index = rand() % out_q.size();
            size_t q = out_q[index], r = out_r[index];
            d.insert(q, r);
            in_q.push_back(q);
            in_r.push_back(r);

            out_q.erase(out_q.begin() + index);
            out_r.erase(out_r.begin() + index);
        }

        for (size_t i = 0; i < reps; ++i) {
            if (to_print) { cout << i << "\t"; }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                if (to_print) { cout << "in" << endl; }

                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            cout << "case 1 fail " << i << endl;
                            return false;
                        }
                    } else {
                        cout << "case 1 fail " << i << endl;
                        return false;
                    }

                }
                d.insert(q, r);
                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del\t" << endl; }

                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }

                d.remove(q, r);
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    return false;
                }


                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        cout << "validate_PD_by_load_factor passed" << endl;
        return true;

    }

    auto validate_safe_PD(size_t reps, bool to_print) -> bool {
        size_t f = rand() % (256 - 32) + 32;
        size_t m = f + 1 + (rand() % (f - 1));
        size_t l = 11 + (rand() % (31 - 11));

        safe_PD d = safe_PD(m, f, l);
        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % m;
            out_r[i] = rand() % SL(l - 1);
        }

        for (size_t i = 0; i < reps; ++i) {
            if (to_print) { cout << i << "\t"; }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (to_print) { cout << "in: " << q << ", " << r << endl; }

                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            cout << "case 1 fail " << i << endl;
                            return false;
                        }
                    } else {
                        cout << "case 1 fail " << i << endl;
                        return false;
                    }

                }
                d.insert(q, r);
                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del\t" << endl; }

                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }

                d.remove(q, r);
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    return false;
                }


                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        cout << "validate_safe_PD passed" << endl;
        return true;

    }

    auto validate_safe_PD_const_case(size_t reps, bool to_print) -> bool {
        size_t f = 32;
        size_t m = 32;
        size_t l = 8;

        safe_PD d = safe_PD(m, f, l);
        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % 32;
            out_r[i] = rand() % 256;
        }

        for (size_t i = 0; i < reps; ++i) {
            if (to_print) d.print();
            if (to_print) { cout << i << "\t"; }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (to_print) { cout << "in: " << q << ", " << r << endl; }

                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            cout << "case 1 fail " << i << endl;
                            return false;
                        }
                    } else {
                        cout << "case 1 fail " << i << endl;
                        return false;
                    }

                }
                d.insert(q, r);
                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del: " << q << ", " << r << endl; }

                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }

                d.remove(q, r);
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    return false;
                }


                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        cout << "validate_safe_PD passed" << endl;
        return true;

    }

    auto r1(size_t reps, bool to_print) -> bool {
        default_random_engine generator; // NOLINT(cert-msc32-c,cert-msc51-cpp)
        uniform_int_distribution<int> f_dist(32, 256);
        size_t f = f_dist(generator);
        uniform_int_distribution<int> m_dist(f + 1, f << 1ul);
        size_t m = m_dist(generator);
        uniform_int_distribution<int> l_dist(11, 31);
        size_t l = l_dist(generator);

        cout << "m is: " << m << endl;
        cout << "f is: " << f << endl;
        cout << "l is: " << l << endl;


        uniform_int_distribution<int> remainder_dist(0, MASK((l - 1)));
        uniform_int_distribution<int> quotient_dist(0, m - 1);
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);
        for (size_t i = 0; i < f; ++i) {
            in_q[i] = quotient_dist(generator);
            in_r[i] = remainder_dist(generator);
        }

        for (size_t i = 0; i < reps; ++i) {
            if (rand() % 2) {
                if (out_q.empty())
                    continue;

                size_t index = rand() % out_q.size();
//            d.in
            }
        }



//    uint m = uniform_int_distribution<int> distribution(32, 256);
        return true;
    }

    auto validate_const_PD(size_t reps, bool to_print) -> bool {
        size_t f = 32;
//    size_t m = f + 1 + (rand() % (f - 1));
        size_t l = 8;

        const_PD d = const_PD(true);
        if (to_print) {
//        cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
//        cout << "l is: " << l << endl;
        }
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % 32;
            out_r[i] = rand() % 256;
        }

        for (size_t i = 0; i < reps; ++i) {
            if (to_print) d.print();
            if (to_print)cout << endl;
            if (to_print) { cout << i << "\t"; }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (to_print) { cout << "in: " << q << ", " << r << endl; }

                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            cout << "case 1 fail " << i << endl;
                            break_point_helper();
                            return false;
                        }
                    } else {
                        cout << "case 1 fail " << i << endl;
                        break_point_helper();
                        return false;
                    }

                }
                d.insert(q, r);
                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    break_point_helper();
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del: " << q << ", " << r << endl; }

                if (not d.lookup(q, r)) {
                    break_point_helper();
                    d.lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }

                d.remove(q, r);
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    break_point_helper();
                    return false;
                }


                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        cout << "const_pd_r0 passed" << endl;
        return true;
    }

    auto naive_pd_r0(size_t reps, bool to_print) -> bool {

        size_t f = rand() % (256 - 32) + 32;
        size_t m = f + 1 + (rand() % (f - 1));
        size_t l = 11 + (rand() % (31 - 11));

        naive_PD d = naive_PD(m, f, l);
        if (to_print) {
            cout << "m is: " << m << endl;
            cout << "f is: " << f << endl;
            cout << "l is: " << l << endl;
        }
        vector<int> in_q(0), out_q(f), in_r(0), out_r(f);

        for (size_t i = 0; i < f; ++i) {
            out_q[i] = rand() % m;
            out_r[i] = rand() % SL(l - 1);
        }

        for (size_t i = 0; i < reps; ++i) {
            if (to_print) { cout << i << "\t"; }
            if (rand() % 2) {
                if (out_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }
                if (to_print) { cout << "in" << endl; }

                size_t index = rand() % out_q.size();
                size_t q = out_q[index], r = out_r[index];
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    auto it = find(in_q.begin(), in_q.end(), q);
                    if (it != in_q.end()) {
                        size_t index2 = distance(in_q.begin(), it);
                        if (in_q[index2] != r) {
                            cout << "case 1 fail " << i << endl;
                            return false;
                        }
                    } else {
                        cout << "case 1 fail " << i << endl;
                        return false;
                    }

                }
                d.insert(q, r);
                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 2 fail " << i << endl;
                    return false;
                }

                in_q.push_back(q);
                in_r.push_back(r);

                out_q.erase(out_q.begin() + index);
                out_r.erase(out_r.begin() + index);


            } else {
                if (in_q.empty()) {
                    if (to_print) { cout << "emp" << endl; }
                    continue;
                }

                size_t index = rand() % in_q.size();
                size_t q = in_q[index], r = in_r[index];
                if (to_print) { cout << "del\t" << endl; }

                if (not d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 3 fail " << i << endl;
                    return false;
                }

                d.remove(q, r);
                if (d.lookup(q, r)) {
                    d.lookup(q, r);
                    cout << "case 4 fail " << i << endl;
                    return false;
                }


                out_q.push_back(q);
                out_r.push_back(r);

                in_q.erase(in_q.begin() + index);
                in_r.erase(in_r.begin() + index);
            }
        }
        cout << "naive_pd_r0 passed" << endl;
        return true;

    }

    auto vector_rw_t1() -> bool {
        vector<bool> v(32, false);
        size_t fp_size = 11, index = 1;
        for (size_t i = 0; i < 129; ++i) {
            for (int j = 0; j < 10; ++j) {
                index = j;
                write_FP_to_vector_by_index(&v, index, i, fp_size);
                bool cond = (i == read_FP_from_vector_by_index(&v, index, fp_size));
                if (!cond) {
                    cout << "Fail index " << i << endl;
                    cout << "got " << read_FP_from_vector_by_index(&v, index, fp_size);
                    cout << " insted of" << i << endl;
                    return false;
                }
                write_FP_to_vector_by_index(&v, index, 0, fp_size);
                cond = (0 == read_FP_from_vector_by_index(&v, index, fp_size));
                if (!cond) {
                    cout << "Case 2. Fail index " << i << endl;
                    return false;
                }
            }
        }
        cout << "Passed vector_rw_t1!" << endl;
        return true;
    }

    auto validate_header_get_interval_function(size_t reps) -> bool {
        size_t a_size = 4;
        HEADER_BLOCK_TYPE a[a_size];
        for (size_t j = 0; j < reps; ++j) {
            for (size_t i = 0; i < a_size; ++i) {
                a[i] = random();
            }
            auto zero_count = array_zero_count(a, a_size);
            uint32_t q = random() % zero_count;
            cout << j << ":\t";
            validate_get_interval_functions(a, a_size, q);
        }
        return true;
    }

    auto validate_get_interval_function_constant(size_t reps) -> bool {
        const size_t a_size = 2;
        HEADER_BLOCK_TYPE a[a_size];
        for (size_t j = 0; j < reps; ++j) {
            for (size_t i = 0; i < a_size; ++i) { // NOLINT(modernize-loop-convert)
                a[i] = random();
            }
            auto zero_count = array_zero_count(a, a_size);
            uint32_t q = random() % zero_count;
//        cout << j << ":\t";
            size_t s1 = 0, e1 = 0, s2 = -1, e2 = -1, s3 = -2, e3 = -2;
            get_interval_attempt(a, a_size, q, &s1, &e1);
            get_interval(a[0], a[1], q, &s2, &e2);
            get_interval2(a[0], a[1], q, &s3, &e3);
            bool c1 = (s1 == s2);
            c1 &= s1 == s3;
            bool c2 = (e1 == e2);
            c2 &= e1 == e3;
            if (not(c1 && c2)) {
                cout << j;
                cout << ": error" << endl;
                get_interval2(a[0], a[1], q, &s3, &e3);
                return false;
            }
//        if (not(s1 == s2 and e1 == e2)) return false;
//        validate_get_interval_functions(a, a_size, q);
        }
        return true;
    }

    auto validate_push_function(size_t reps) -> bool {
        size_t a_size = 2;
        HEADER_BLOCK_TYPE a[a_size];
        D_TYPE w1, w2, val1, val2;
        for (size_t j = 0; j < reps; ++j) {

            val1 = w1 = a[0] = random();
            val2 = w2 = a[1] = random();
            size_t end = random() % 64;

            static_push(a, 2, end);
            static_push(&w1, &w2, end);
            bool cond1 = w1 == a[0];
            bool cond2 = w2 == a[1];

            if (cond1 && cond2) continue;
//        cout << val1, val2 << endl;
            cout << val1 << ", " << val2 << ", " << end << endl;
            cout << a[0] << ", " << a[1] << endl;
            cout << w1 << ", " << w2 << endl;
//        printf("%d, %d", val1, val2);
//        printf("%d, %d", a[0], a[1]);
//        printf("%d, %d", w1, w2);
            return false;

        }
        return true;
    }

    auto validate_pull_function(size_t reps) -> bool {
        size_t a_size = 2;
        HEADER_BLOCK_TYPE a[a_size];
        D_TYPE w1, w2, val1, val2;
        for (size_t j = 0; j < reps; ++j) {

            val1 = w1 = a[0] = random();
            val2 = w2 = a[1] = random();
            size_t end = 0;
            while (end == 0) end = random() % 64;

            static_pull(a, 2, end);
            static_pull(&w1, &w2, end);
            bool cond1 = w1 == a[0];
            bool cond2 = w2 == a[1];

            if (cond1 && cond2) continue;

            break_point_helper();
            cout << val1 << ", " << val2 << ", " << end << endl;
            cout << a[0] << ", " << a[1] << endl;
            cout << w1 << ", " << w2 << endl;
            return false;

        }
        return true;
    }

    auto validate_rank_function(size_t reps) -> bool {
        const size_t a_size = 2;
        HEADER_BLOCK_TYPE a[a_size];
//    D_TYPE w1, w2, val1, val2;
        for (size_t j = 0; j < reps; ++j) {
            for (unsigned int &i : a) i = random();
            size_t s1 = 0, e1 = 0, s2 = -1, e2 = -1;
            auto zero_count = array_zero_count(a, a_size);
            uint32_t q = random() % zero_count;

            get_interval_attempt(a, a_size, q, &s1, &e1);
            get_interval_by_rank(a, a_size, q, &s2, &e2);

            bool c1 = (s1 == s2);
            bool c2 = (e1 == e2);
            if (c1 && c2) continue;

            break_point_helper();
            cout << q << endl;
            cout << s1 << ", " << e1 << endl;
            cout << s2 << ", " << e2 << endl;
            return false;

        }
        return true;
    }

    auto validate_header_type(size_t reps) -> bool {
        naive_Header naive_header(32, 32, 42);
        auto const_header = const_Header();

//    for (size_t i = 0; i < reps; ++i) {
        for (size_t j = 0; j < 32; ++j) {
            size_t s1 = -1, e1 = -1, s2 = -2, e2 = -2;
            naive_header.insert(j, &s1, &e1);
            const_header.insert(j, &s2, &e2);
            uint32_t w_arr[2] = {0, 0};
            const_header.get_w1w2(&w_arr[0], &w_arr[2]);
            int res = compare_vector_and_array(naive_header.get_vec(), w_arr, 2);
            if (res != -1)
                return false;
        }
//    }
        return true;
    }

    void break_point_helper() {
        usleep(80000);
    }

    template auto validate_PD_higher_load<cg_PD>(size_t reps, float load_factor, bool to_seed, bool to_print) -> bool;

    template auto validate_PD_higher_load<PD>(size_t reps, float load_factor, bool to_seed, bool to_print) -> bool;

    template auto validate_CPD<CPD>(size_t reps, bool to_seed, bool to_print) -> bool;

    template auto validate_PD<cg_PD>(size_t reps, bool to_seed, bool to_print) -> bool;

    template auto validate_PD<PD>(size_t reps, bool to_seed, bool to_print) -> bool;

    template auto
    validate_PD_core<cg_PD>(size_t reps, cg_PD *d, size_t m, size_t f, size_t l, bool to_print,
                            float load_factor) -> bool;

    template auto
    validate_PD_core<PD>(size_t reps, PD *d, size_t m, size_t f, size_t l, bool to_print, float load_factor) -> bool;
//bool interval_t0() {
//
//}
}
#pragma clang diagnostic pop