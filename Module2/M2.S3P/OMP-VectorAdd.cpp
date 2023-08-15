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
  const int blockSize = (size / omp_get_max_threads());

  srand(time(0));

  int *v1, *v2, *v3;

  auto start = high_resolution_clock::now();

  v1 = (int *)malloc(size * sizeof(int *));
  v2 = (int *)malloc(size * sizeof(int *));
  v3 = (int *)malloc(size * sizeof(int *));

  randomVector(v1, size);
  randomVector(v2, size);

  #pragma omp parallel for
  for (int i = 0; i < size; ++i) {
    v3[i] = v1[i] + v2[i];
  }

  auto stop = high_resolution_clock::now();

  auto duration = duration_cast<microseconds>(stop - start);

  cout << "Time taken by function: " << duration.count() << " microseconds"
       << endl;

  return 0;
}