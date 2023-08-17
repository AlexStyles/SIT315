#!/bin/bash

g++ OMP-VectorAdd.cpp -fopenmp -o vectoradd.o -O3 -ftree-vectorize
g++ OMP++-VectorAdd.cpp -fopenmp -o vectoradd++.o -ftree-vectorize -O3
g++ ThreadsVectorAdd.cpp -o threads.o -pthread
g++ SequentialVectorAdd.cpp -o sequential.o