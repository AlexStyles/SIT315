#include <omp.h>
#include <time.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size) {
  const int blockSize = (size / omp_get_max_threads());
#pragma omp parallel
  {
    int threadId = omp_get_thread_num();
    const uint32_t blockStart = threadId * blockSize;
    const uint32_t blockEnd = blockStart + blockSize;
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(0, 100);
#pragma omp for
    for (int i = blockStart; i < blockEnd; i++) {
      vector[i] = distribution(rng);
    }
  }
  for (int i = blockSize * omp_get_max_threads(); i < size; ++i) {
    vector[i] = rand() % 100;
  }
}

int main() {
  unsigned long size = 100000000;
  const int blockSize = (size / omp_get_max_threads());

  srand(time(0));

  int *v1, *v2, *v3;

  auto start = high_resolution_clock::now();

  v1 = (int *)malloc(size * sizeof(int *));
  v2 = (int *)malloc(size * sizeof(int *));
  v3 = (int *)malloc(size * sizeof(int *));

  randomVector(v1, size);

  randomVector(v2, size);

  #pragma omp parallel
  {
    const int threadId = omp_get_thread_num();
    const uint32_t blockStart = threadId * blockSize;
    const uint32_t blockEnd = blockStart + blockSize;
    #pragma omp for
    for (int i = blockStart; i < blockEnd; ++i) {
      v3[i] = v1[i] + v2[i];
    }
  }
  for (int i = blockSize * omp_get_max_threads(); i < size; ++i) {
    v3[i] = v1[i] + v2[i];
  }

  auto stop = high_resolution_clock::now();

  auto duration = duration_cast<microseconds>(stop - start);

  cout << "Time taken by function: " << duration.count() << " microseconds"
       << endl;

  return 0;
}