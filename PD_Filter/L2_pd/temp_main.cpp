#include "L2Bucket_mq.hpp"

void simplest_test() {
    auto b = MainBucket<48, 32, 8>();
    assert(!b.find(0, 0, 0));
    b.insert(0, 0, 0);
    assert(b.find(0, 0, 0));
    auto del_res = b.conditional_remove(0, 0, 0);
    assert(del_res);
    assert(!b.find(0, 0, 0));
    //    return;
}

void att() {
    std::cout << "*** " << __FILE__ << ":" << __LINE__ << " *** " << std::endl;
}

void leading_trailing_zeros() {
    std::cout << "_lzcnt_u64(0)  :" << _lzcnt_u64(0) << std::endl;
    std::cout << "_lzcnt_u64(1)  :" << _lzcnt_u64(1) << std::endl;
    std::cout << "_lzcnt_u64(2)  :" << _lzcnt_u64(2) << std::endl;
    std::cout << "_lzcnt_u64(4)  :" << _lzcnt_u64(4) << std::endl;
    std::cout << "_lzcnt_u64(m63):" << _lzcnt_u64(1ULL << 63) << std::endl;

    std::cout << "_tzcnt_u64(0)  :" << _tzcnt_u64(0) << std::endl;
    std::cout << "_tzcnt_u64(1)  :" << _tzcnt_u64(1) << std::endl;
    std::cout << "_tzcnt_u64(2)  :" << _tzcnt_u64(2) << std::endl;
    std::cout << "_tzcnt_u64(4)  :" << _tzcnt_u64(4) << std::endl;
    std::cout << "_tzcnt_u64(m63):" << _tzcnt_u64(1ULL << 63) << std::endl;
}

int old_main() {
    uint64_t a[4] = {0};
    a[0] = 0x1234'1334'1334'1334;
    a[1] = 0x1334'1234'1334'1334;
    a[2] = 0x1334'1334'1234'1334;
    a[3] = 0x1334'1334'1334'1234;

    auto res1 = bits_memcpy::my_cmp_epu<16, 4>(0x1234, a);

    std::cout << "res1: " << res1 << std::endl;
    print_memory::print_word_LE(res1, GAP);

    // att();

    bool res = MainBucket_tests::insert_find_all<48, 32, 8>();
    assert(res);
    for (size_t i = 0; i < (1ul << 10ul); i++) {
        res = MainBucket_tests::rand_test1<48, 32, 8>();
        if (!res) {
            std::cout << "i: " << i << std::endl;
            return -1;
        }
    }
    std::cout << "pass1" << std::endl;

    MainBucket_tests::recursive_add_delete<48, 32, 8>(1 << 25);
    //    assert(res);
    std::cout << "pass2" << std::endl;

    res = MainBucket_tests::recursive_add_delete_with_map<48, 32, 8>(1 << 25);
    assert(res);
    std::cout << "pass3" << std::endl;
    res = MainBucket_tests::rand_test1<48, 32, 8>();
    assert(res);
    std::cout << "pass4" << std::endl;
    //    bool res = rt0<48, 32, 8>();
    //    assert(res);
    std::cout << "Passed" << std::endl;
    return 0;
}

void Q_tests_main(){
    bool res = Quotients_tests::true_negative_lookup_test<48, 32, 4>();
    assert(res);

    res = Quotients_tests::true_negative_lookup_test_all<48, 32, 4>();
    assert(res);
    res = Quotients_tests::simplest_test<48, 32, 4>();
    assert(res);

    for (size_t i = 0; i < (1 << 12); i++) {
        bool temp = Quotients_tests::simplest_test<48, 32, 4>();
        assert(temp);
    }

    for (size_t i = 0; i < (1 << 13); i++) {
        bool temp = Quotients_tests::rand_test18<48, 32, 4>();
        assert(temp);
    }

    for (size_t i = 0; i < (1 << 13); i++) {
        bool temp = Quotients_tests::rand_test18<48, 32, 4>();
        assert(temp);
    }

    res = Quotients_tests::recursive_add_delete_with_map<48, 32, 4>(1 << 10);
    assert(res);

}

int main() {
    std::cout << "Temp_Main" << std::endl;
//    leading_trailing_zeros();


    return 0;
}