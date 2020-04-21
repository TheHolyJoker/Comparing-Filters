//
// Created by tomer on 4/17/19.
//

#include "Remote.h"


void Remote::add_possibly_uninit_key(string *val, size_t key) {
    this->db.update(key, *val + DELIM);

}

void Remote::add_new_key(string *val, size_t key) {
    this->db.insert(key, *val + DELIM);

}

void Remote::add(string *val, size_t *keys, size_t keys_num) {
    for (int i = 0; i < keys_num; ++i)
        this->add_possibly_uninit_key(val, keys[i]);

}

void Remote::add(string *val, size_t *keys, size_t keys_num, const bool *cells_cond) {
    for (int i = 0; i < keys_num; ++i) {
        if (cells_cond[i])
            this->add_possibly_uninit_key(val, keys[i]);
        else
            this->add_new_key(val, keys[i]);
    }
}

bool Remote::lookup(string *val, size_t key) {
    string temp = this->db.query(key);
    return is_sub_str(&temp, val);
}

string Remote::retrieve(size_t key) {
    return this->db.query(key);
}

bool Remote::is_sub_str(string *all_value, string *element) {
    char *all = const_cast<char *>(all_value->c_str());
    char *sub = const_cast<char *>(element->c_str());
    char *pch;
    pch = strtok(all, DELIM);
    while (pch != nullptr) {
        if (strcmp(pch, sub) == 0)
            return true;
        pch = strtok(nullptr, DELIM);
    }
    return false;
}
