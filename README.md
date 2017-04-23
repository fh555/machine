# Machine

Multi-tier storage hierarchy simulator.

## Dependencies

- **g++ 4.7+** 
- **cmake** (`apt-get install autoconf cmake`) 

## Setup
        
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make -j4
```

## Run simulator

```
cd build
./test/machine -h
./test/machine -a 3 -s 4 -f ../traces/tpcc.txt -o 1000000
```

## Sample Output

```
//===----------------------------------------------------------------------===//
//                               MACHINE                                      //
//===----------------------------------------------------------------------===//
                  caching_type : CACHE-DRAM-SSD
                     size_type : 4
                  latency_type : 1
                  caching_type : FIFO
                     file_name : ../traces/tpcc.txt
           migration_frequency : 3
              nvm_read_latency : 2
             nvm_write_latency : 4
               operation_count : 1000000
//===----------------------------------------------------------------------===//
Running trace ../traces/tpcc.txt...
+++++++++++++++++++++++++++++++++++++++++++++++++++++
-------------------------------
[CACHE] [FIFO] [8 MB] OCCUPIED: 100 %
-------------------------------
[DRAM] [FIFO] [128 MB] OCCUPIED: 100 %
-------------------------------
[NVM] [FIFO] [128 MB] OCCUPIED: 100 %
-------------------------------
[SSD] [FIFO] [1048 GB] OCCUPIED: 0 %
-------------------------------
Operation 100000 :: r 25336 0 25336 :: 146.793s 
Operation 200000 :: r 320751 0 320751 :: 251.804s 
Operation 300000 :: r 237 0 237 :: 340.104s 
Operation 400000 :: r 320772 0 320772 :: 446.076s 
Operation 500000 :: r 36550 0 36550 :: 543.289s 
Operation 600000 :: r 412 0 412 :: 636.613s 
Operation 700000 :: r 220 0 220 :: 742.605s 
Operation 800000 :: r 320849 0 320849 :: 834.085s 
Operation 900000 :: r 251261 0 251261 :: 940.122s 
Operation 1000000 :: r 4551 0 4551 :: 1036.41s 
+++++++++++++++++++++++++++++++++++++++++++++++++++++
Throughput : 964.871 (ops/s) 
+++++++++++++++++++++++++++++++++++++++++++++++++++++
READ OPS: 
     CACHE :: 709289
      DRAM :: 240407
       NVM :: 160735
       SSD :: 78935
WRITE OPS: 
     CACHE :: 85833
      DRAM :: 64547
       NVM :: 93602
       SSD :: 3485
+++++++++++++++++++++++++++++++++++++++++++++++++++++       
```

## Files

- `workload.cpp` (simulator entry point -- processes a given trace)
- `device.cpp` (device definitions)
- `cache.cpp` (polymorphic cache implementation)

## Modules

- Multiple storage tiers (with CPU CACHE, DRAM, NVM, SSD)
- Real trace files
- LRU, LFU, and ARC caching algorithms

## Parameters

- Distinguish sequential and random accesses 
- Distinguish read and write accesses
