cmake_minimum_required(VERSION 3.10)
project(Comparing_Filters_Simpler)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512bw")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512dq")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -mbmi -mbmi2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlzcnt")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2") SET(CMAKE_CXX_FLAGS
# "${CMAKE_CXX_FLAGS} -g -ggdb") SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
# -fpermissive") SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512vbmi") SET(CMAKE_CXX_FLAGS
# "${CMAKE_CXX_FLAGS} -march=skylake-avx512")

# include_directories(Bloom_Filter)
include_directories(PD_Filter)
include_directories(Tests)
include_directories(morton)
# include_directories(xorfilter)
include_directories(cuckoofilter)

find_package(OpenSSL REQUIRED)

set(PROJECT_SOURCES
    # main.cpp
    hashutil.h
    Tests/wrappers.hpp
    Tests/tests.hpp
    Tests/tests.cpp
    Tests/new_tests.hpp
    Tests/new_tests.cpp
    Tests/printutil.cpp
    Tests/printutil.hpp
    PD_Filter/macros.h
    Hash_functions/xxhash64.h
    Hash_functions/wyhash.h
    Hash_functions/woothash.h
    Hash_functions/MurmurHash3.h
    Hash_functions/MurmurHash3.cpp
    Hash_functions/BobJenkins.h
    Hash_functions/BobJenkins.cpp

    PD_Filter/HashTables/hashTable64.hpp
    PD_Filter/HashTables/hashTable_Aligned.hpp
    PD_Filter/HashTables/hashTable_Packed.hpp
    PD_Filter/HashTables/hashTable_CuckooFilter.hpp
    PD_Filter/HashTables/Spare_Low_Load.hpp
    
    PD_Filter/Dict512/pd512.hpp
    PD_Filter/Dict512/pd512.cpp
    PD_Filter/Dict512/pd512_plus.hpp
    PD_Filter/Dict512/pd512_plus.cpp
    PD_Filter/Dict512/Dict512.hpp
    PD_Filter/Dict512/twoChoicer.hpp
    PD_Filter/Dict512/Dict512_With_CF.hpp
    PD_Filter/Dict512/Dict512_SparseSpare.hpp
    PD_Filter/Dict512/Dict512_Ver2.hpp
    PD_Filter/Dict512/Dict512_Ver3.hpp
    
    PD_Filter/Dict320/pd320.hpp
    PD_Filter/Dict320/pd320.cpp
    PD_Filter/Dict320/Dict320.hpp
    PD_Filter/Dict320/Dict320_v2.hpp
    PD_Filter/Dict320/twoChoicer320.hpp

    PD_Filter/basic_function_util.h PD_Filter/basic_function_util.cpp
)

add_executable(Filters main.cpp ${PROJECT_SOURCES})
# Add the compile options ONLY for Filters.
target_compile_options(Filters PRIVATE -O3 -DNDEBUG)
add_executable(valid_test main.cpp ${PROJECT_SOURCES})
# Add the compile options ONLY for valid_test.
target_compile_options(valid_test PRIVATE -O0)

add_executable(test_o2 main.cpp ${PROJECT_SOURCES})
# Add the compile options ONLY for valid_test.
target_compile_options(test_o2 PRIVATE -O2)


target_link_libraries(Filters OpenSSL::SSL)
target_link_libraries(valid_test OpenSSL::SSL)
target_link_libraries(test_o2 OpenSSL::SSL)
# add_executable(
#   Filters
#   main.cpp
#   hashutil.h
#   robin_hood.h
#   Tests/wrappers.hpp
#   Tests/tests.hpp
#   Tests/tests.cpp
#   Tests/new_tests.hpp
#   Tests/new_tests.cpp
#   Tests/printutil.cpp
#   Tests/printutil.hpp
#   Bloom_Filter/counting_bloom.h
#   Bloom_Filter/simd-block.h
#   Bloom_Filter/simd-block-fixed-fpp.h
#   PD_Filter/PD.cpp
#   PD_Filter/hashutil.hpp
#   PD_Filter/hashutil.cpp
#   PD_Filter/hash_table.hpp
#   PD_Filter/hash_table.cpp
#   PD_Filter/dict.hpp
#   PD_Filter/macros.h
#   Hash_functions/xxhash64.h
#   Hash_functions/wyhash.h
#   Hash_functions/woothash.h
#   Hash_functions/MurmurHash3.h
#   Hash_functions/MurmurHash3.cpp
#   Hash_functions/BobJenkins.h
#   Hash_functions/BobJenkins.cpp
#   # xxhash.h
#   # main.cpp
#   # morton/hash_util.h
#   # morton/block.h
#   # morton/bf.h
#   # morton/block.h
#   # morton/compressed_cuckoo_config.h
#   # morton/compressed_cuckoo_filter.h
#   # morton/fixed_point.h
#   # morton/morton_filter.h
#   # morton/morton_sample_configs.h
#   # morton/morton_util.h
#   # morton/test_util.h
#   # morton/util.h
#   # morton/vector_types.h
#   PD_Filter/TPD_Filter/T_dict.hpp
#   PD_Filter/TPD_Filter/att_hTable.hpp
#   PD_Filter/TPD_Filter/TPD.hpp
#   PD_Filter/TPD_Filter/old_dict512.hpp
#   # TPD_Filter/pd-test.cpp
#   PD_Filter/TPD_Filter/pd512_wrapper.hpp
#   PD_Filter/TPD_Filter/basic_function_util.cpp
#   PD_Filter/TPD_Filter/basic_function_util.h
#   PD_Filter/Analyse/analyse.hpp
#   PD_Filter/Analyse/analyse.cpp
#   PD_Filter/Dict512/pd512.hpp
#   PD_Filter/Dict512/pd512.cpp
#   PD_Filter/Dict512/Dict512.hpp
#   PD_Filter/Dict512/twoChoicer.hpp
#   PD_Filter/Dict512/hashTable_Aligned.hpp
#   PD_Filter/Dict512/hashTable_Packed.hpp
#   PD_Filter/Dict512/hashTable_CuckooFilter.hpp
#   PD_Filter/Dict512/Dict512_With_CF.hpp
#   PD_Filter/Dict512/Dict512_Ver2.hpp
#   PD_Filter/Dict320/pd320.hpp
#   PD_Filter/Dict320/pd320.cpp
#   PD_Filter/Dict320/pd320_v2.hpp
#   PD_Filter/Dict320/pd320_v2.cpp
#   PD_Filter/Dict320/Dict320.hpp
#   PD_Filter/Dict320/Dict320_v2.hpp
#   PD_Filter/Dict320/twoChoicer320.hpp
#   PD_Filter/Safe/safe512.hpp
#   PD_Filter/Safe/cc_pd.hpp
#   PD_Filter/Validation/validate_hash_table.hpp
#   PD_Filter/Validation/validate_hash_table.cpp
#   # Safe/Constant_size_PD
# )

target_link_libraries(Filters OpenSSL::SSL)
