### Benchmarks
All benchmarks are tested under CPU with spec in `cpu_spec.txt`.
```
Run on (6 X 2592 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 9216 KiB (x2)
Load Average: 0.44, 0.19, 0.11
----------------------------------------------------------------------------------------------------
Benchmark                                                          Time             CPU   Iterations
----------------------------------------------------------------------------------------------------
Http_SIMD_SSE42_AVX2_Zero_Dynamic_Memory_Allocation_Parser      98.0 ns         98.0 ns      6966642
Http_Zero_Dynamic_Memory_Allocation_Parser                       522 ns          522 ns      1371287
Http_Parsing_Method_Two                                         1353 ns         1353 ns       516235
Http_State_Machine_Parser                                       1393 ns         1393 ns       476815
```
