#include <chrono>
#include <cstdint>
#include <iostream>
#include <queue>
#include <random>

#include <pthread.h>
#include <atomic>
#include <unistd.h>

// https://stackoverflow.com/questions/7859754/can-i-keep-threads-alive-and-give-them-other-workloads

// Function declarations
void* ThreadLoop(void*);
void* TestFunction(void*);

// Global variables
std::queue<void*(*)(void*)> taskQueue;
pthread_cond_t queueConditional;
pthread_mutex_t queueMutex;
std::atomic_bool programCompleted(false);
std::atomic_int(0);

// Typedefs & Structs
typedef struct Matrix {
  Matrix() = default;
  Matrix(const uint8_t matrixSize) : size(matrixSize) {
    data = new int*[size];
    for (uint8_t i = 0; i < size; ++i) {
      data[i] = new int[size] {0};
    }
  }

  ~Matrix() {
    for (uint8_t i = 0; i < size; ++i) {
      delete[] data[i];
    }
    delete[] data;
  }

  void Print() {
    for (uint8_t i = 0; i < size; ++i) {
      for (uint8_t j = 0; j < size; ++j) {
        std::printf("%u ", data[i][j]);
      }
      std::printf("\n");
    }
  }
  const uint8_t size;
  int** data;
} Matrix;

typedef struct ThreadPool {
  ThreadPool(const unsigned int numThreads) : threads(numThreads, 0) {
    InitPool();
  }
  ~ThreadPool() {
    std::printf("ThreadPool destructor joining threads\n");
    while (taskQueue.size()) {
      //Wait 
    }
    programCompleted = true;
    for (std::size_t i = 0; i < threads.size(); ++i) {
      pthread_join(threads[i], NULL);
    }
    std::printf("ThreadPool destructor finished joining threads\n");
  }

  void InitPool () {
    for (std::size_t i = 0; i < threads.size(); ++i) {
      std::printf("Creating thread %lu\n", i);
      pthread_create(&threads[i], NULL, &ThreadLoop, NULL);
    }
  }
  std::vector<pthread_t> threads;
} ThreadPool;

void* TestFunction(void* args) {
  std::printf("Hello from thread %lu\n", pthread_self());
  usleep(500000);
  pthread_cond_signal(&queueConditional);
  return NULL;
}

void* ThreadLoop(void* args) {
  while(!programCompleted) {
    pthread_mutex_lock(&queueMutex);
    while (taskQueue.empty() && !programCompleted) {
      // std::printf("Thread %lu: Waiting for task...\n", pthread_self());
      pthread_cond_wait(&queueConditional, &queueMutex);
    }
    auto task = taskQueue.front();
    taskQueue.pop();
    pthread_mutex_unlock(&queueMutex);
    task(NULL);
  }
  pthread_exit(NULL);
}

void GenerateMatrixData(Matrix& inMatrix) {
  std::random_device device;
  std::mt19937 rng(device());
  std::uniform_int_distribution<uint8_t> distribution(0, 100);
  for (uint8_t i = 0; i < inMatrix.size; ++i) {
    for (uint8_t j = 0; j < inMatrix.size; ++j) {
      inMatrix.data[i][j] = distribution(rng);
    }
  }
}

void GenerateMatrices(Matrix& A, Matrix& B) {
  GenerateMatrixData(A);
  GenerateMatrixData(B);
}

void MultiplyMatrices(Matrix& A, Matrix& B, Matrix& C, const uint8_t matrixSize) {
  if (!matrixSize)
    return;

  // i: Row calculation
  for (uint8_t i = 0; i < matrixSize; ++i) {
    // j: C element selection
    for (uint8_t j = 0; j < matrixSize; ++j) {
      for (uint8_t k = 0; k < matrixSize; ++k) {
        // k: A & B element selection
        C.data[i][j] += A.data[i][k] * B.data[k][j];
      }
    }
  }
}

int main() {
  static constexpr uint8_t matrixSize = 100;
  Matrix A(matrixSize);
  Matrix B(matrixSize);
  Matrix C(matrixSize);
  GenerateMatrices(A, B);

  auto start = std::chrono::high_resolution_clock::now();
  MultiplyMatrices(A, B, C, matrixSize);
  auto stop = std::chrono::high_resolution_clock::now();

  {
    ThreadPool aThreadPool(12);
    for (int i = 0; i < 100; ++i) {
      taskQueue.push(&TestFunction);
    }
    while (taskQueue.size()) {

    }
  }
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::printf("duration = %ld microseconds\n", duration.count());
  // A.Print();
  // std::printf("-----------\n");
  // B.Print();
  // std::printf("-----------\n");
  // C.Print();

  return 0;
}