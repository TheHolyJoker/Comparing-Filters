cmake_minimum_required(VERSION 3.15)
project(Filter_Comparison)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "-O3")
SET(CMAKE_C_FLAGS "-O3")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
#target_include_directories(Filter_Comparison PRIVATE ${./Filter_PD})
#target_include_directories(./Filter_PD)
#include_directories("./Filter_PD")
#add_subdirectory(./Filter_Quotient)
#add_subdirectory(./Filter_BF)
#add_subdirectory(./libcuckoo)
#SET(CMAKE_CXX_FLAGS -pthread)

add_executable(Filter_Comparison
        Comparing/b_helpers.cpp
        Comparing/b_helpers.hpp
        Comparing/benchmarking_filters.cpp
        Comparing/benchmarking_filters.hpp

        Filter_BF/New_BF/General_BF/GeneralBF.cpp
        Filter_BF/New_BF/General_BF/GeneralBF.h
        Filter_BF/New_BF/Grey_Box_Function_Dir/GreyBoxFunction.cpp
        Filter_BF/New_BF/Grey_Box_Function_Dir/GreyBoxFunction.h
        Filter_BF/New_BF/Hash_files/Hash.cpp
        Filter_BF/New_BF/Hash_files/Hash.h
#        Filter_BF/New_BF/Hash_files/MurmurHash3.cpp
#        Filter_BF/New_BF/Hash_files/MurmurHash3.h
#        Filter_BF/New_BF/Remote/Remote.cpp
#        Filter_BF/New_BF/Remote/Remote.h
#        Filter_BF/New_BF/V_tests/tests.cpp
#        Filter_BF/New_BF/V_tests/tests.h
#        Filter_BF/New_BF/Wrapper/Wrapper.cpp
#        Filter_BF/New_BF/Wrapper/Wrapper.h
#        Filter_BF/New_BF/main.cpp
#        Filter_BF/main.cpp

        Filter_PD/CPD/CPD.cpp
        Filter_PD/CPD/CPD.hpp
        Filter_PD/CPD/CPD_validator.cpp
        Filter_PD/CPD/CPD_validator.hpp
        Filter_PD/CPD/Counters_Array.cpp
        Filter_PD/CPD/Counters_Array.hpp
        Filter_PD/CPD/Naive_Counter_Endec.cpp
        Filter_PD/CPD/Naive_Counter_Endec.hpp
        Filter_PD/Filters/Dict/dict.cpp
        Filter_PD/Filters/Dict/dict.hpp
        Filter_PD/Filters/Dict/multi_dict.cpp
        Filter_PD/Filters/Dict/multi_dict.hpp
        Filter_PD/Filters/POW2C/const_filter.cpp
        Filter_PD/Filters/POW2C/const_filter.h
        Filter_PD/Filters/POW2C/gen_2Power.cpp
        Filter_PD/Filters/POW2C/gen_2Power.h
        Filter_PD/Filters/POW2C/pow2c_filter.cpp
        Filter_PD/Filters/POW2C/pow2c_filter.h
        Filter_PD/Filters/POW2C/pow2c_naive_filter.cpp
        Filter_PD/Filters/POW2C/pow2c_naive_filter.h
        Filter_PD/Global_functions/basic_functions.cpp
        Filter_PD/Global_functions/basic_functions.h
        Filter_PD/Global_functions/cuckoo_and_counting_macros.hpp
        Filter_PD/Global_functions/macros.h
        Filter_PD/Hash/Hash.cpp
        Filter_PD/Hash/Hash.h
#        Filter_PD/Hash/MurmurHash3.cpp
#        Filter_PD/Hash/MurmurHash3.h
        Filter_PD/Hash/Permutation.cpp
        Filter_PD/Hash/Permutation.h
        Filter_PD/Hash/hashutil/hashutil.h
        Filter_PD/Hash/hashutil_backup/hashutil.h
        Filter_PD/Hash/hashutil_backup/lookup3.h
        Filter_PD/Hash/static_hashing.h
        Filter_PD/Hash_Table/hash_table.cpp
        Filter_PD/Hash_Table/hash_table.hpp
        Filter_PD/Hash_Table/hash_table_var.cpp
        Filter_PD/Hash_Table/hash_table_var.hpp
        Filter_PD/Hash_Table/multi_hash_table.cpp
        Filter_PD/Hash_Table/multi_hash_table.hpp
        Filter_PD/PD/Constant_size_PD/const_Body.cpp
        Filter_PD/PD/Constant_size_PD/const_Body.h
        Filter_PD/PD/Constant_size_PD/const_Header.cpp
        Filter_PD/PD/Constant_size_PD/const_Header.h
        Filter_PD/PD/Constant_size_PD/const_PD.cpp
        Filter_PD/PD/Constant_size_PD/const_PD.h
        Filter_PD/PD/Contiguous_PD/cg_PD.cpp
        Filter_PD/PD/Contiguous_PD/cg_PD.h
        Filter_PD/PD/Contiguous_PD/constant_contiguous_PD/cc_PD.cpp
        Filter_PD/PD/Contiguous_PD/constant_contiguous_PD/cc_PD.h
        Filter_PD/PD/Naive_PD/cg_naive_PD.cpp
        Filter_PD/PD/Naive_PD/cg_naive_PD.h
        Filter_PD/PD/Naive_PD/naive_Body.cpp
        Filter_PD/PD/Naive_PD/naive_Body.h
        Filter_PD/PD/Naive_PD/naive_Header.cpp
        Filter_PD/PD/Naive_PD/naive_Header.h
        Filter_PD/PD/Naive_PD/naive_PD.cpp
        Filter_PD/PD/Naive_PD/naive_PD.h
        Filter_PD/PD/Non_Contiguous_PD/Body.cpp
        Filter_PD/PD/Non_Contiguous_PD/Body.h
        Filter_PD/PD/Non_Contiguous_PD/Header.cpp
        Filter_PD/PD/Non_Contiguous_PD/Header.h
        Filter_PD/PD/Non_Contiguous_PD/PD.cpp
        Filter_PD/PD/Non_Contiguous_PD/PD.h
        Filter_PD/PD/Safe_PD/safe_PD.cpp
        Filter_PD/PD/Safe_PD/safe_PD.h
        Filter_PD/PD/Safe_PD/v_Body.cpp
        Filter_PD/PD/Safe_PD/v_Body.h
        Filter_PD/PD/Safe_PD/v_Header.cpp
        Filter_PD/PD/Safe_PD/v_Header.h
        Filter_PD/Tests/Benchmarking/benchmark_counting_filter.cpp
        Filter_PD/Tests/Benchmarking/benchmark_counting_filter.hpp
        Filter_PD/Tests/Benchmarking/benchmark_tests.cpp
        Filter_PD/Tests/Benchmarking/benchmark_tests.h
        Filter_PD/Tests/Benchmarking/old_benchmark_tests.cpp
        Filter_PD/Tests/Benchmarking/old_benchmark_tests.h
        Filter_PD/Tests/Test_output_formatting/test_printer.cpp
        Filter_PD/Tests/Test_output_formatting/test_printer.hpp
        Filter_PD/Tests/Validation/validate_bit_op.cpp
        Filter_PD/Tests/Validation/validate_bit_op.hpp
        Filter_PD/Tests/Validation/validate_counter_PD.cpp
        Filter_PD/Tests/Validation/validate_counter_PD.hpp
        Filter_PD/Tests/Validation/validate_counters.cpp
        Filter_PD/Tests/Validation/validate_counters.hpp
        Filter_PD/Tests/Validation/validate_counting_filter.cpp
        Filter_PD/Tests/Validation/validate_counting_filter.hpp
        Filter_PD/Tests/Validation/validate_filter.cpp
        Filter_PD/Tests/Validation/validate_filter.hpp
        Filter_PD/Tests/Validation/validate_hash_table.cpp
        Filter_PD/Tests/Validation/validate_hash_table.hpp
        Filter_PD/Tests/Validation/validation_tests.cpp
        Filter_PD/Tests/Validation/validation_tests.h
        Filter_PD/bit_operations/bit_op.cpp
        Filter_PD/bit_operations/bit_op.h
        Filter_PD/bit_operations/bit_word_converter.cpp
        Filter_PD/bit_operations/bit_word_converter.hpp
        Filter_PD/bit_operations/my_bit_op.cpp
        Filter_PD/bit_operations/my_bit_op.hpp
#        Filter_PD/main.cpp
        Filter_Quotient/GlobalFunctions/globalFunction.cpp
        Filter_Quotient/GlobalFunctions/globalFunction.h
        Filter_Quotient/Hash/Hash.cpp
        Filter_Quotient/Hash/Hash.h
#        Filter_Quotient/Hash/MurmurHash3.cpp
#        Filter_Quotient/Hash/MurmurHash3.h
        Filter_Quotient/QuotientFilter/BasicFilter.cpp
        Filter_Quotient/QuotientFilter/BasicFilter.h
        Filter_Quotient/QuotientFilter/BitArray.cpp
        Filter_Quotient/QuotientFilter/BitArray.h
        Filter_Quotient/QuotientFilter/DataBase.cpp
        Filter_Quotient/QuotientFilter/DataBase.h
        Filter_Quotient/QuotientFilter/Filter_QF.cpp
        Filter_Quotient/QuotientFilter/Filter_QF.h
        Filter_Quotient/Tests/DBtests.cpp
        Filter_Quotient/Tests/DBtests.h
        #        Filter_Quotient/main.cpp
        main.cpp

        Shared_files/MurmurHash3.h Shared_files/MurmurHash3.cpp
        #        main.cpp Comparing/benchmarking_filters.cpp Comparing/benchmarking_filters.hpp Comparing/b_helpers.cpp Comparing/b_helpers.hpp)
        Comparing/benchmark_integer.cpp Comparing/benchmark_integer.hpp)

#target_link_libraries(Clion_Code cuckoofilter)
