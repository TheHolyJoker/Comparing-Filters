cmake_minimum_required(VERSION 3.10)
project(Comparing_Filters_Simpler)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512bw")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512vbmi")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -faligned-new")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=skylake-avx512")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -mbmi -mbmi2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")

include_directories(Bloom_Filter)
include_directories(PD_Filter)
include_directories(Tests)
include_directories(morton)
include_directories(xorfilter)
include_directories(cuckoofilter)

find_package(OpenSSL REQUIRED)

add_executable(
        Filters
        hashutil.h
        robin_hood.h
        Tests/wrappers.hpp
        Tests/tests.hpp Tests/tests.cpp
        Tests/new_tests.hpp Tests/new_tests.cpp
        Tests/printutil.cpp Tests/printutil.hpp
        Bloom_Filter/counting_bloom.h
        Bloom_Filter/simd-block.h
        Bloom_Filter/simd-block-fixed-fpp.h

        PD_Filter/PD.cpp
        PD_Filter/hashutil.hpp PD_Filter/hashutil.cpp
        PD_Filter/hash_table.hpp PD_Filter/hash_table.cpp
        PD_Filter/dict.hpp
        PD_Filter/macros.h

        #        main.cpp
        #        morton/hash_util.h
        #        morton/block.h
        #        morton/bf.h
        #        morton/block.h
        #        morton/compressed_cuckoo_config.h
        #        morton/compressed_cuckoo_filter.h
        #        morton/fixed_point.h
        #        morton/morton_filter.h
        #        morton/morton_sample_configs.h
        #        morton/morton_util.h
        #        morton/test_util.h
        #        morton/util.h
        #        morton/vector_types.h
        TPD_Filter/T_dict.hpp
        TPD_Filter/att_hTable.hpp
        TPD_Filter/TPD.hpp
        TPD_Filter/dict512.hpp
        TPD_Filter/pd512.hpp
        TPD_Filter/pd512.cpp
        #        TPD_Filter/pd-test.cpp
        TPD_Filter/pd512_wrapper.hpp
        TPD_Filter/basic_function_util.cpp TPD_Filter/basic_function_util.h)

target_link_libraries(Filters OpenSSL::SSL)
