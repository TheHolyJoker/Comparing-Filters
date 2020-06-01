# Comparing Filters
Currently benchmarking:
1. Bloomfilter (BF)
2. Cuckoo filter (CF)
3. SIMD blocked Bloom filter (SIMD) 
4. Morton filter (MF)
5. Pocket Dictionary filter (PD)

## Validation
  The files `test.hpp test.cpp` contain validation tests on the filter.
  1. Making sure the filter does not have false negative. (Indicating the element is in not in the filter, when it is.)
  2. Checking the filter false positive rate is as expected. 
  Filter aften have a parameter contoroling on the false positive probability $\epsilon$, when it is increased, the filter uses more space, and has smaller error probability.  
  
 ## Benchmark
 Currently checking only insertions and lookups, in different loads.
 
 
 ## Usage
 ```
 git clone -b Simpler https://github.com/TheHolyJoker/Comparing_Filters.git
 cd Comparing_Filters
 mkdir build
 cd build
 cmake ..
 make
 ./Filters
 ```
 
 ## Credit
 Large parts of the code and its sturcture are taken from https://github.com/FastFilter/fastfilter_cpp.
 
 Cuckoo filter is from https://github.com/efficient/cuckoofilter by Bin Fan et al. <br />
 SIMD blocked Bloom filter is from https://github.com/apache/impala. <br />
 Morton filter is from https://github.com/AMDComputeLibraries/morton_filter .<br />
 Counting Quotient Filter (CQF) is from https://github.com/splatlab/cqf. (Currently not in use). <br />
 Pocket Dictionary is work in progress see https://github.com/TomerEven/Pocket_Dictionary.
 
 ### Papers
 **Bloom filter** https://en.wikipedia.org/wiki/Bloom_filter
 
 **Cuckoo Filter**
 ["Cuckoo Filter: Better Than Bloom" by Bin Fan, Dave Andersen and Michael Kaminsky](https://www.cs.cmu.edu/~dga/papers/cuckoo-conext2014.pdf)

**SIMD blocked Bloom filter**
[Cache-, Hash- and Space-Efficient Bloom Filters](https://algo2.iti.kit.edu/singler/publications/cacheefficientbloomfilters-wea2007.pdf)

**Morton filter** [Morton Filters: Faster, Space-Efficient Cuckoo Filters via
Biasing, Compression, and Decoupled Logical Sparsity](https://www.vldb.org/pvldb/vol11/p1041-breslow.pdf), 

**Pocket Dictionary** [Fully-Dynamic Space-Efficient Dictionaries and Filters with
Constant Number of Memory Accesses](https://arxiv.org/pdf/1911.05060.pdf)
 
 
 ## To do
1. Add **Vacuum-Filter** [paper](http://www.vldb.org/pvldb/vol13/p197-wang.pdf) [repository](https://github.com/wuwuz/Vacuum-Filter) <br>
Add **Quotient-Filter** [repository](https://github.com/splatlab/cqf)
2. Counting filter benchmark. 
