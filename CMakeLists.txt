cmake_minimum_required(VERSION 3.15)
project(Filters)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "-O3")
SET(CMAKE_C_FLAGS "-O3")

#include_directories(.)
include_directories(Bloom_Filter)
include_directories(PD_Filter)
include_directories(Tests)
include_directories(morton)
include_directories(xorfilter)

find_package(OpenSSL REQUIRED)

add_executable(
        Filters
        hashutil.h
        Tests/wrappers.hpp
        Tests/tests.hpp Tests/tests.cpp
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
 )

target_link_libraries(Filters OpenSSL::SSL)