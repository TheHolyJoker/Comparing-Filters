//
// Created by tomer on 10/25/19.
//

#include "basic_functions.h"


void print_array(uint8_t *a, size_t a_size) {
    cout << "[" << (int) a[0];
    for (size_t i = 1; i < a_size; ++i) {
        cout << ", " << (int) a[i];
    }
    cout << "]" << endl;

}


void print_array(int *a, size_t a_size) {
    cout << "[" << a[0];
    for (size_t i = 1; i < a_size; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;

}

void print_array(uint32_t *a, size_t a_size) {
    cout << "[" << a[0];
    for (size_t i = 1; i < a_size; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;

}

void print_bool_array(bool *a, size_t a_size) {
    cout << "[" << a[0];
    for (size_t i = 0; i < a_size; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;
}


void print_array_as_consecutive_memory(const uint32_t *a, size_t a_size) {
    for (size_t i = 0; i < a_size; ++i) {
        cout << my_bin(a[i]);
    }
    cout << endl;
}

void print_vector(vector<int> *v) {
    cout << "[" << v->at(0);
    for (size_t i = 1; i < v->size(); ++i) {
        cout << ", " << v->at(i);
    }
    cout << "]" << endl;

}

void print_vector(vector<bool> *v) {
    cout << "[" << v->at(0);
    for (size_t i = 1; i < v->size(); ++i) {
        cout << ", " << v->at(i);
    }
    cout << "]" << endl;

}

void to_vector(vector<bool> *vec, const uint8_t *a, size_t a_size) {
    size_t bit_in_slot = (sizeof(a[0]) * 8);
    size_t vec_new_size = bit_in_slot * a_size, vec_index = 0;
    vec->resize(vec_new_size);
    for (size_t i = 0; i < a_size; ++i) {
        size_t b = 1ULL << (bit_in_slot - 1);
        for (size_t j = 0; j < bit_in_slot; ++j) {
            vec->at(vec_index) = (b & a[i]);
            b >>= 1ULL;
            vec_index++;
        }
    }
}


string my_bin(size_t n, size_t length) {
    string s;
    ulong b = 1ull << (ulong) (32 - 1);
    for (int i = 0; i < 32; ++i) {
        if (b & n)
            s.append("1");
        else
            s.append("0");
        b >>= 1ul;
    }
//    cout << s << endl;
    return s;
}

string to_sci(double x, int add_to_counter, size_t precision) {
    if (x == 0) { return "0E0"; }
    if (x < 0) { return "-" + to_sci(-x); }
    if (round(x) == x) { return to_sci(x - 1e-5); }

    string temp = to_string(x);
    auto dot_index = temp.find('.');
    assert(temp.find('.') != -1);

    if (dot_index == 1) {
        if (temp[0] != '0') {
            return temp.substr(0, precision + 2) + "E" + to_string(add_to_counter);
        }
        int counter = 0;
        while (x < 1) {
            x *= 10;
            counter--;
        }
        return to_sci(x, counter, precision);

        /*
        auto counter = 1;
        for (int i = 2; i < temp.length(); ++i) {
            if (temp[i] == '0')
                counter++;
            else
                break;
        }

        string res = (to_string(temp[counter + 2]) + ".") + temp.substr(counter + 3, counter + 3 + precision);
        res += "E-" + to_string(counter + add_to_counter);
        return res;*/
    }

    size_t counter = 0;
    while (x > 10) {
        x /= 10;
        counter++;
    }
    return to_sci(x, counter, precision);

}


int loglog2(int x) {
    log(x);
}


void formatting() {
    /*Taken from http://www.cplusplus.com/forum/beginner/181119/*/
    // arbitrary test data, test repeats the same data thrice
    std::string fname = "Ebenezer";
    std::string lname = "Scrooge";
    int hourWork = 23;
    int hourRate = 5;
    double gp = 123.48501;
    double taxamt = 2.4392;
    double netpay = 121.04579;

    // values for controlling format
    const int name_width = 15;
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const int total_width = name_width * 2 + int_width * 2 + dbl_width * 3 + sep.size() * num_flds;
    const std::string line = sep + std::string(total_width - 1, '-') + '|';

    std::cout << line << '\n' << sep
              << std::setw(name_width) << "var " << sep << std::setw(name_width) << "last name" << sep
              << std::setw(int_width) << "hours" << sep << std::setw(int_width) << "rate" << sep
              << std::setw(dbl_width) << "gross pay" << sep << std::setw(dbl_width) << "tax" << sep
              << std::setw(dbl_width) << "net pay" << sep << '\n' << line << '\n';

    for (int i = 0; i < 3; ++i) {
        std::cout << sep << std::setw(name_width) << fname << sep << std::setw(name_width) << lname << sep
                  << std::setw(int_width) << hourWork << sep << std::setw(int_width) << hourRate << sep
                  << std::fixed << std::setprecision(2)
                  << std::setw(dbl_width) << gp << sep << std::setw(dbl_width) << taxamt << sep
                  << std::setw(dbl_width) << netpay << sep << '\n';
    }
    std::cout << line << '\n';
}

void table_print(size_t var_num, string *var_names, size_t *values) {
    /*const auto var_num = 9;
        string var_names[var_num] = {"start", "start_array_index", "start_bit_pos",
                                     "end", "end_array_index", "end_bit_pos",
                                     "new_end", "new_end_array_index", "new_end_bit_pos"};
        size_t values[var_num] = {start, start / slot_size, start % slot_size,
                                  end, end / slot_size, end % slot_size,
                                  new_end, new_end / slot_size, new_end % slot_size};

        size_t max_length = 0;
        for (auto & var_name : var_names) {
            max_length = max(var_name.length(), max_length);
        }*/
    size_t max_length = 0;
    for (int i = 0; i < var_num; ++i) {
        max_length = max(var_names[i].length(), max_length);
    }

    // values for controlling format
    const int name_width = int(max_length);
    const int int_width = 7;
    const int dbl_width = 12;
    const int num_flds = 7;
    const std::string sep = " |";
    const int total_width = name_width * 2 + int_width * 2 + dbl_width * 3 + sep.size() *
                                                                             num_flds; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
    const std::string line = sep + std::string(total_width - 1, '-') + '|';
    std::cout << line << '\n' << sep << std::setw(name_width) << left << "var " << sep << std::setw(name_width)
              << "value"
              << sep << '\n' << line << '\n';

    for (int i = 0; i < var_num; ++i) {
        std::cout << sep << std::setw(name_width) << var_names[i] << sep << std::setw(name_width) << values[i] << sep
                  << '\n';
    }
    std::cout << line << endl;

}



template<typename T>
auto my_ceil(T x, T y) -> T {
    return (x + y - 1) / y;
}

template
auto my_ceil<size_t>(size_t x, size_t y) -> size_t;

