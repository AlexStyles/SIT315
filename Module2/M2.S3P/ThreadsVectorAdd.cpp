#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <vector>
#include <thread>
#include <random>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
  std::random_device device;
  static thread_local std::mt19937 rng(device());
  std::uniform_int_distribution<std::mt19937::result_type> distribution(0, 100);
  for (int i = 0; i < size; i++)
  {
      // Generate a random number between 0-99 and insert into vector at index i
      *vector = distribution(rng);
      ++vector;
  }
}

void GenerateVectorData(int vector[], int vectorSize)
{
  static const uint32_t maxThreads = std::thread::hardware_concurrency();
  uint32_t blockSize = 1;
  if (vectorSize > maxThreads) {
    blockSize = vectorSize / maxThreads;
  }
  const uint32_t remainingBlockSize = (vectorSize % blockSize);
  uint32_t blockStart = 0;
  std::vector<std::thread> threads(maxThreads);
  threads.reserve(maxThreads);

  for (size_t threadNumber = 0; threadNumber < threads.size(); ++threadNumber) {
    threads[threadNumber] = std::thread(randomVector, &vector[blockStart], blockSize);
    blockStart += blockSize;
  }
  if (remainingBlockSize) {
    // Generate vector
    randomVector(&vector[blockStart], remainingBlockSize);
  }
  for (size_t threadNumber = 0; threadNumber < threads.size(); ++threadNumber) {
    threads[threadNumber].join();
  }
}

void Add(int* vector1, int* vector2, int* vector3, int blockSize) {
  for (int i = 0; i < blockSize; ++i) {
    *vector3 = *(vector2) + *(vector1);
    ++vector1;
    ++vector2;
    ++vector3;
  }
}

void AddVectors(int* vector1, int* vector2, int* vector3, int vectorSize) {
  static const uint32_t maxThreads = std::thread::hardware_concurrency();
  uint32_t blockSize = 1;
  if (vectorSize > maxThreads) {
    blockSize = vectorSize / maxThreads;
  }
  const uint32_t remainingBlockSize = (vectorSize % blockSize);
  uint32_t blockStart = 0;
  std::vector<std::thread> threads(maxThreads);
  threads.reserve(maxThreads);

  for (size_t threadNumber = 0; threadNumber < threads.size(); ++threadNumber) {
    threads[threadNumber] = std::thread(Add, &vector1[blockStart], &vector2[blockStart], &vector3[blockStart], blockSize);
    blockStart += blockSize;
  }
  if (remainingBlockSize) {
    // Generate vector
    Add(&vector1[blockStart], &vector2[blockStart], &vector3[blockStart], remainingBlockSize);
  }
  for (size_t threadNumber = 0; threadNumber < threads.size(); ++threadNumber) {
    threads[threadNumber].join();
  }
}

int main(){
    unsigned long size = 100000000;
    srand(time(0));
    int *v1, *v2, *v3;

    // Generate a timestamp for the start of the program
    auto start = high_resolution_clock::now();

    // Allocate memory for arrays v1, v2 & v3, allowing for 'size' number of integer type elements
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));

    GenerateVectorData(v1, size);
    GenerateVectorData(v2, size);

    // For every element i of arrays v1 & v2, insert the sum of v1[i] and v2[i] into v3 [i]
    AddVectors(v1, v2, v3, size);

    auto stop = high_resolution_clock::now();

    for (int i = 0; i < 5; ++i)
      std::printf("v3 = %d, v2 = %d, v1 = %d\n", v3[i], v2[i], v1[i]);

    // Determine the runtime of the program by subtracting the finish time by the start time
    auto duration = duration_cast<microseconds>(stop - start);
    cout << duration.count() << endl;

    return 0;
}