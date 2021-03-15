# Comparing Filters
Currently benchmarking:
1. Bloomfilter (BF)
2. Cuckoo filter (CF)
3. SIMD blocked Bloom filter (SIMD) 
4. Morton filter (MF)
5. Pocket Dictionary filter (PD)

## Validation
  The files `test.hpp test.cpp` contain validation tests on the filter.
  1. Making sure the filter does not have a false negative. (Indicating the element is in not in the filter when it is.)
  2. Checking the filter false positive rate is as expected. 
  Filter often have a parameter controlling on the false positive probability $\epsilon$, when it is increased, the filter uses more space, and has smaller error probability.  
  
 ## Benchmark
 Currently checking only insertions and lookups performances, in different loads. 
 
 ## Usage
 
 ### Dependencies
 Since CF uses `openssl` library, the project won't compile unless it is installed. (See CF [repository](https://github.com/efficient/cuckoofilter))


 ### To build
 ```
 git clone -b Simpler https://github.com/TheHolyJoker/Comparing_Filters.git
 cd Comparing_Filters
 mkdir build
 cd build
 cmake --build ./ --target Filters
 ```
 ### To run
 In `build` directory run
 
 ```
 ./Filters <filter indicator> <exponent of number of keys> <lookup factor> <rounds>
 ```
 1. `filter indicator`: Which filter to test. 
    1. To include BF in the test,`filter indicator & 1` should be true.
    2. To include CF in the test,`filter indicator & 2` should be true.
    3. To include SIMD in the test,`filter indicator & 4` should be true.
    4. To include MF in the test,`filter indicator & 8` should be true.
    5. To include PD in the test,`filter indicator & 16` should be true.
    
    The default value is -1 to test all filters.
 2. `exponent of the number of keys`: Every filter is built to contain at most 2^`exponent of the number of keys`.<br> 
 The default value is 24. (should not be set to less than 16 or MF might fail)
 3. `lookup factor`: Lookup exponent factor. If set to d and n insertions will be performed, then n*2^d lookups will be performed. <br>
 The default value is 2
 4. `rounds`: The benchmark performs insertion, and then lookup where each time a fraction of `1/rounds` of the total number of elements is queried. <br>
  The default value is 32.
 
 ## Credit
 Large parts of the code and its structure are taken from https://github.com/FastFilter/fastfilter_cpp.
 
 Cuckoo filter is from https://github.com/efficient/cuckoofilter by Bin Fan et al. <br />
 
 SIMD blocked Bloom filter is from https://github.com/apache/impala. <br />
 
 Morton filter is from https://github.com/AMDComputeLibraries/morton_filter.<br />
 
 Counting Quotient Filter (CQF) is from https://github.com/splatlab/cqf. (Currently not in use). <br />
 
 Pocket Dictionary is work in progress see https://github.com/TomerEven/Pocket_Dictionary.
 The Pocket Dictionary class that uses advanced SIMD instructions, is taken from [here](https://github.com/jbapple/cuckoofilter/tree/crates/src) by Jim Apple ([@Jbapple](https://github.com/jbapple)).
 
 ### Papers
 **Bloom filter** https://en.wikipedia.org/wiki/Bloom_filter
 
 **Cuckoo Filter**
 ["Cuckoo Filter: Better Than Bloom" by Bin Fan, Dave Andersen and Michael Kaminsky](https://www.cs.cmu.edu/~dga/papers/cuckoo-conext2014.pdf)

**SIMD blocked Bloom filter**
[Cache-, Hash- and Space-Efficient Bloom Filters](https://algo2.iti.kit.edu/singler/publications/cacheefficientbloomfilters-wea2007.pdf)

**Morton filter** [Morton Filters: Faster, Space-Efficient Cuckoo Filters via
Biasing, Compression, and Decoupled Logical Sparsity](https://www.vldb.org/pvldb/vol11/p1041-breslow.pdf), 

**Quotient Filter** [A General-Purpose Counting Filter: Counting Quotient Filter (CQF)](https://www3.cs.stonybrook.edu/~ppandey/files/p775-pandey.pdf)

**Pocket Dictionary** [Fully-Dynamic Space-Efficient Dictionaries and Filters with
Constant Number of Memory Accesses](https://arxiv.org/pdf/1911.05060.pdf)
 
 
 ## To do
1. Add Filters
    1. **Vacuum-Filter** [paper](http://www.vldb.org/pvldb/vol13/p197-wang.pdf) [repository](https://github.com/wuwuz/Vacuum-Filter)
    2. **Quotient-Filter** [repository](https://github.com/splatlab/cqf)
2. Counting filter benchmark. 
