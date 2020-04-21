#include <utility>

//
// Created by tomer on 4/17/19.
//

#ifndef ABF_REMOTE_REMOTE_H
#define ABF_REMOTE_REMOTE_H

#include <sstream>
#include <iterator>
#include <ostream>
#include "../General_BF/GeneralBF.h"
#include "dirent.h"


//#define DEFAULT_REL_PATH "../New_BF/BS_Paths"
#define DEFAULT_REL_NAME "../New_BF/BS_Paths/tempDir"


#define CACHE_SIZE 1ULL << 16
#define MAX_NODE_SIZE (1ULL << 6)
#define MIN_FLUSH_SIZE (MAX_NODE_SIZE / 4)

#define DELIM " "


typedef one_file_per_object_backing_store fbs;


class Remote {
    fbs bs;
    swap_space sspace;
    betree<uint64_t, string> db;

public:
    Remote(): Remote(DEFAULT_REL_NAME) {};

    explicit Remote(string path) : bs(std::move(path)), sspace(&this->bs, CACHE_SIZE),
                                   db(&sspace, MAX_NODE_SIZE, MIN_FLUSH_SIZE) {};


    void add_possibly_uninit_key(string *val, size_t key);

    void add_new_key(string *val, size_t key);

    void add(string *val, size_t key);

    void add(string *val, size_t *keys, size_t keys_num);

    void add(string *val, size_t *keys, size_t keys_num, const bool *cells_cond);

    bool lookup(string *val, size_t key);

    string retrieve(size_t key);

    static bool is_sub_str(string *all_value, string *element);

/*
    static string create_new_path();

    static size_t count_files(const string &path);

private:

    static void delete_last_path();

    static void delete_all_paths();
*/

};


#endif //ABF_REMOTE_REMOTE_H
