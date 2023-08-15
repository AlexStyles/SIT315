#!/bin/bash

g++ OMP-VectorAdd.cpp -fopenmp -o vectoradd.o
g++ OMP++-VectorAdd.cpp -fopenmp -o vectoradd++.o
# g++ OMP-VectorAdd.cpp -fopenmp