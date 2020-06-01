# Comparing Filters
Currently benchmarking:
1. Bloomfilter (BF)
2. Cuckoo filter (CF)
3. Morton filter (MF)
4. Pocket Dictionary filter (PD)

## Validation
  The files `test.hpp test.cpp` contain validation tests on the filter.
  1. Making sure the filter does not have false negative. (Indicating the element is in not in the filter, when it is.)
  2. Checking the filter false positive rate is as expected. 
  Filter aften have a parameter contoroling on the false positive probability $\epsilon$, when it is increased, the filter uses more space, and has smaller error probability.  
  
 ## Benchmark
 Currently checking only insertions and lookups, in different loads.
 
 
 ## Usage
 '''
 git clone -b Simpler https://github.com/TheHolyJoker/Comparing_Filters.git
 cd Comparing_Filters
 mkdir build
 cd build
 cmake ..
 make
 ./Filters
 '''
 
