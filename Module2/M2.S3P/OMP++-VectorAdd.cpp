#include <omp.h>
#include <time.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size) {
  #pragma omp parallel for default(none) shared(vector, size)
  for (int i = 0; i < size; ++i) {
    vector[i] = rand() % 100;
  }
}

int main() {
  unsigned long size = 100000000;

  srand(time(0));

  int *v1, *v2, *v3;

  auto start = high_resolution_clock::now();

  v1 = (int *)malloc(size * sizeof(int *));
  v2 = (int *)malloc(size * sizeof(int *));
  v3 = (int *)malloc(size * sizeof(int *));

  randomVector(v1, size);
  randomVector(v2, size);

  uint64_t totalAtomic = 0;
  uint64_t totalReduction = 0;
  uint64_t totalCritical = 0;
  uint64_t blockSum = 0;

  #pragma omp parallel default(none) private(blockSum) shared(size, v1, v2, v3, totalAtomic, totalReduction, totalCritical)
  {
    #pragma omp for private(blockSum) reduction(+ : totalReduction)/*  schedule(static,1) */
    for (int i = 0; i < size; ++i) {
      v3[i] = v1[i] + v2[i];
      totalReduction += v3[i];
      blockSum += v3[i];
      #pragma omp atomic update
      totalAtomic += v3[i];
    }
    #pragma omp critical
    {
      totalCritical += blockSum;
    }
  }

  auto stop = high_resolution_clock::now();

  auto duration = duration_cast<microseconds>(stop - start);
  uint64_t testTotal = 0;
  for (int i = 0; i < size; ++i) {
    testTotal += v3[i];
  }

  cout << "Time taken by function: " << duration.count() << " microseconds"
       << endl;
  std::cout << "Total (Atomic) = " << totalAtomic << "\n";
  std::cout << "Total (Reduction) = " << totalReduction << "\n";
  std::cout << "Total (Critical) = " << totalReduction << "\n";
  std::cout << "Total (test) = " << testTotal << "\n";

  return 0;
}