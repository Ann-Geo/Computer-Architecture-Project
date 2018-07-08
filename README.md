# Computer-Architecture-Project
Title: Cache Performance Analysis for Different Benchmarks with and without Prefetch Unit

Project Description

The objective of this project is to perform the cache performance analysis of four different benchmarks with and without the presence of prefetching unit. The benchmarks that are used here are Dhrystone, Linpack, Whetstone and gcc from SPEC2000 benchmark suite. Address traces from these benchmarks are generated using Intel’s Pin tool and the traces are given to the simulator program written for 8-way set associative and fully associative caches with and without prefetch unit. The prefetchers explored in this project are next line prefetcher and stride prefetcher. The cache used here is a shared cache with size 64KB and block size of 8 bytes. The prefetch unit size and the confidence level of stride prefetcher are varied from 4kB to 16kB and 3 to 5 respectively to analyze the performance of the cache on these benchmarks. The cache hit rates are analyzed for 8-way associative and full way associative cache with and without the presence of prefetchers. 
