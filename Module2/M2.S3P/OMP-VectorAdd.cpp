#include <omp.h>
#include <time.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size) {
  #pragma omp parallel default(none) shared(vector) firstprivate(size)
  {
    std::printf("Size value in thread (firstprivate) = %d\n", size);
    std::random_device device;
    static thread_local std::mt19937 rng(device());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(0, 100);
    #pragma omp for
    for (int i = 0; i < size; ++i) {
      // rand() is not re-entrant, meaning that threads will block others from accessing rand() Hence the 23 seconds runtimes
      // rand_r() was producing duplicate results
      vector[i] = distribution(rng);
    }
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

  #pragma omp parallel for
  for (int i = 0; i < size; ++i) {
    v3[i] = v1[i] + v2[i];
  }

  auto stop = high_resolution_clock::now();

  auto duration = duration_cast<microseconds>(stop - start);

  cout << duration.count() << endl;

  return 0;
}