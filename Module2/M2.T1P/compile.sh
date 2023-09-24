#!/bin/bash

g++ sequential_MatrixMultiplication.cpp -o sequential_multiplication.o
g++ pthreads_MatrixMultiplication.cpp -pthread --std=c++17 -o pthread_multiplication.o
g++ openmp_MatrixMultiplication.cpp -fopenmp -o omp_multiplication.o
