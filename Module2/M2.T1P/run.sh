#!/bin/bash

DATA_SIZES=(50 100 200 500 1000 2000 5000 10000)

for SIZE in "${DATA_SIZES[@]}"; do
  # sed -i "s/matrixSize = [0-9]*;/matrixSize = ${SIZE};/" openmp_MatrixMultiplication.cpp
  # sed -i "s/matrixSize = [0-9]*;/matrixSize = ${SIZE};/" pthreads_MatrixMultiplication.cpp
  sed -i "s/matrixSize = [0-9]*;/matrixSize = ${SIZE};/" sequential_MatrixMultiplication.cpp
  ./compile.sh
  for i in {1..50}; do
    # ./omp_multiplication.o | awk '{print $3}' >> "omp_${SIZE}.log" 
    # ./pthread_multiplication.o | awk '{print $3}' >> "pthread_${SIZE}.log"
    ./sequential_multiplication.o | awk '{print $3}' >> "sequential_${SIZE}.log" 
  done
done