#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
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
std::atomic_bool gProgramCompleted(false);
std::atomic_int gTasksCompleted(0);
pthread_cond_t gQueueConditional;

// Typedefs & Structs

typedef struct Matrix {
  Matrix() = default;
  Matrix(const uint16_t matrixSize) : size(matrixSize) {
    data = new int*[size];
    for (uint16_t i = 0; i < size; ++i) {
      data[i] = new int[size] {0};
    }
  }

  ~Matrix() {
    for (uint16_t i = 0; i < size; ++i) {
      delete[] data[i];
    }
    delete[] data;
  }

  void Print() {
    for (uint16_t i = 0; i < size; ++i) {
      for (uint16_t j = 0; j < size; ++j) {
        std::printf("%u ", data[i][j]);
      }
      std::printf("\n");
    }
  }
  const uint16_t size;
  int** data;
} Matrix;

typedef struct TaskData {
  TaskData(const Matrix& inA, const Matrix& inB, Matrix& inC, const uint16_t inStart, const uint16_t inEnd) :
    fMatrixA(inA),
    fMatrixB(inB),
    fMatrixC(inC),
    fStart(inStart),
    fEnd(inEnd) {}
  ~TaskData() = default;
  const Matrix& fMatrixA;
  const Matrix& fMatrixB;
  Matrix& fMatrixC;
  const uint16_t fStart;
  const uint16_t fEnd;
} TaskData;

typedef void*(*TaskFn)(void*);
typedef std::pair<TaskFn, TaskData> Task;
typedef std::queue<Task> Queue;

typedef struct TaskQueue {
  TaskQueue() {
    pthread_cond_init(&gQueueConditional, NULL);
    pthread_mutex_init(&fQueueMutex, NULL);
  };

  ~TaskQueue() {
    pthread_cond_destroy(&gQueueConditional);
    pthread_mutex_destroy(&fQueueMutex);
  };

  void SetTask(Task inTask) {
    pthread_mutex_lock(&fQueueMutex);
    fQueue.push(inTask);
    pthread_mutex_unlock(&fQueueMutex);
  }

  std::optional<Task> GetTask() {
    if (fQueue.size() > 0) {
      Task aReturnTask = fQueue.front();
      fQueue.pop();
      return aReturnTask;
    }
    return std::nullopt;
  }

  void FinaliseTasks() {
    pthread_cond_broadcast(&gQueueConditional);
  }

  const inline std::size_t Size() {
    return fQueue.size();
  }
  pthread_mutex_t fQueueMutex;
  private:
  Queue fQueue;
} TaskQueue;

typedef struct ThreadPool {
  ThreadPool(const unsigned int numThreads, TaskQueue& inTaskQueue) : fThreads(numThreads, 0), fTaskQueue(inTaskQueue) {
  }
  ~ThreadPool() {
    while (fTaskQueue.Size()) {
    }
    gProgramCompleted = true;
    fTaskQueue.FinaliseTasks();
    for (std::size_t i = 0; i < fThreads.size(); ++i) {
      pthread_join(fThreads[i], NULL);
    }
  }

  void Run() {
    InitPool();
  }

  void InitPool () {
    for (std::size_t i = 0; i < fThreads.size(); ++i) {
      pthread_create(&fThreads[i], NULL, &ThreadLoop, &fTaskQueue);
    }
  }
  std::vector<pthread_t> fThreads;
  TaskQueue& fTaskQueue;
} ThreadPool;

void* ThreadLoop(void* args) {
  TaskQueue* aTaskQueue = static_cast<TaskQueue*>(args);
  while (!gProgramCompleted) {
    int aMutexRetCode = pthread_mutex_lock(&aTaskQueue->fQueueMutex);
    while (!aTaskQueue->Size() && !gProgramCompleted) {
      pthread_cond_wait(&gQueueConditional, &aTaskQueue->fQueueMutex);
    }
    std::optional<Task> aTask = aTaskQueue->GetTask();
    pthread_mutex_unlock(&aTaskQueue->fQueueMutex);
    if (aTask.has_value())
      aTask->first(&aTask->second);
  }
  pthread_exit(NULL);
}

void GenerateMatrixData(Matrix& inMatrix) {
  std::random_device device;
  std::mt19937 rng(device());
  std::uniform_int_distribution<uint8_t> distribution(0, 100);
  for (uint16_t i = 0; i < inMatrix.size; ++i) {
    for (uint16_t j = 0; j < inMatrix.size; ++j) {
      inMatrix.data[i][j] = distribution(rng);
    }
  }
}

void GenerateMatrices(Matrix& A, Matrix& B) {
  GenerateMatrixData(A);
  GenerateMatrixData(B);
}

void* MultiplyRow(void* args) {
  TaskData* aTask = static_cast<TaskData*>(args);
  const Matrix& A = aTask->fMatrixA;
  const Matrix& B = aTask->fMatrixB;
  Matrix& C = aTask->fMatrixC;
  for (uint16_t i = aTask->fStart; i < aTask->fEnd; ++i) {
    for (uint16_t j = 0; j < A.size; ++j) {
      for (uint16_t k = 0; k < A.size; ++k) {
        C.data[i][j] += A.data[i][k] * B.data[k][j];
      }
    }
  }
  const int aSignalRetCode = pthread_cond_signal(&gQueueConditional);
  if (aSignalRetCode) {
    std::printf("Error: Conditional signal returned %d\n", aSignalRetCode);
  }
  gTasksCompleted++;
  return NULL;
}

void MultiplyMatrices(const Matrix& A, const Matrix& B, Matrix& C, const uint16_t matrixSize, const uint16_t inMaxThreads = 0) {
  if (!matrixSize)

  // i: Row selector
  for (uint16_t i = 0; i < matrixSize; ++i) {
    // j: Column selector
    for (uint16_t j = 0; j < matrixSize; ++j) {
      // k: Row/Column element selector
      for (uint16_t k = 0; k < matrixSize; ++k) {
        C.data[i][j] += A.data[i][k] * B.data[k][j];
      }
    }
  }
}

int main() {
  static constexpr uint16_t matrixSize = 24;
  Matrix A(matrixSize);
  Matrix B(matrixSize);
  Matrix C(matrixSize);
  GenerateMatrices(A, B);

  auto start = std::chrono::high_resolution_clock::now();
  MultiplyMatrices(A, B, C, matrixSize);
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::printf("duration = %ld microseconds\n", duration.count());
  // A.Print();
  // std::printf("-----------\n");
  // B.Print();
  // std::printf("-----------\n");
  // C.Print();

  return 0;
}