#include "L2Bucket_mq.hpp"

void simplest_test(){
    auto b = L2Bucket_mq<48, 32, 8>();
    assert(!b.find(0, 0, 0));
    b.insert(0, 0, 0);
    assert(b.find(0, 0, 0));
    auto del_res = b.conditional_remove(0, 0, 0);
    assert(del_res);
    assert(!b.find(0, 0, 0));
    return;
}

int main() {
    std::cout << "Temp_Main" << std::endl;

    
     bool res = l2_bucket_tests::insert_find_all<48, 32, 8>();
    assert(res);
    // for (size_t i = 0; i < (1ul << 10ul); i++) {
    //     res = l2_bucket_tests::rand_test1<48, 32, 8>();
    //     if (!res) {
    //         std::cout << "i: " << i << std::endl;
    //         return -1;
    //     }
    // }

    // res = l2_bucket_tests::recursive_add_delete<48, 32, 8>(1 << 25);
    res = l2_bucket_tests::recursive_add_delete_with_map<48, 32, 8>(1 << 25);

    assert(res);
    // res = l2_bucket_tests::rand_test1<48, 32, 8>();
    // assert(res);
    //    bool res = rt0<48, 32, 8>();
    //    assert(res);
    std::cout << "Passed" << std::endl;
    return 0;
}