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

typedef void*(*Task)(void*);
typedef std::queue<Task> Queue;

typedef struct TaskQueue {
  TaskQueue() {
    pthread_cond_init(&fQueueConditional, NULL);
    pthread_mutex_init(&fQueueMutex, NULL);
  };

  ~TaskQueue() {
    pthread_cond_destroy(&fQueueConditional);
    pthread_mutex_destroy(&fQueueMutex);
  };

  void SetTask(Task inTask) {
    pthread_mutex_lock(&fQueueMutex);
    fQueue.push(inTask);
    pthread_mutex_unlock(&fQueueMutex);
  }

  Task GetTask() {
    if (fQueue.size() > 0) {
      Task aReturnTask = fQueue.front();
      fQueue.pop();
      return aReturnTask;
    }
    return NULL;
  }

  void FinaliseTasks() {
    pthread_cond_broadcast(&fQueueConditional);
  }

  const inline std::size_t Size() {
    return fQueue.size();
  }
  pthread_cond_t fQueueConditional;
  pthread_mutex_t fQueueMutex;
  private:
  Queue fQueue;
} TaskQueue;

// Global variables
std::atomic_bool programCompleted(false);
std::atomic_int tasksCompleted(0);

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
  ThreadPool(const unsigned int numThreads, TaskQueue& inTaskQueue) : fThreads(numThreads, 0), fTaskQueue(inTaskQueue) {
  }
  ~ThreadPool() {
    while (fTaskQueue.Size()) {
      std::printf("Tasks still in queue\n");
      //Wait 
    }
    programCompleted = true;
    fTaskQueue.FinaliseTasks();
    for (std::size_t i = 0; i < fThreads.size(); ++i) {
      pthread_join(fThreads[i], NULL);
    }
    std::printf("ThreadPool destructor finished joining threads\n");
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

void* TestFunction(void* args) {
  std::printf("Hello from thread %lu\n", pthread_self());
  pthread_cond_t* aConditionVar = static_cast<pthread_cond_t*>(args);
  tasksCompleted++;
  // usleep(500000);
  const int aSignalRetCode = pthread_cond_signal(aConditionVar);
  if (aSignalRetCode) {
    std::printf("Error: Conditional signal returned %d\n", aSignalRetCode);
  }
  return NULL;
}

void* ThreadLoop(void* args) {
  TaskQueue* aTaskQueue = static_cast<TaskQueue*>(args);
  while (!programCompleted) {
    int aMutexRetCode = pthread_mutex_lock(&aTaskQueue->fQueueMutex);
    while (!aTaskQueue->Size() && !programCompleted) {
      pthread_cond_wait(&aTaskQueue->fQueueConditional, &aTaskQueue->fQueueMutex);
    }
    Task aTask = aTaskQueue->GetTask();
    pthread_mutex_unlock(&aTaskQueue->fQueueMutex);
    if (aTask)
      aTask(&aTaskQueue->fQueueConditional);
  }
  pthread_exit(NULL);
}

void RunTestTasks() {
  TaskQueue aTaskQueue;
  for (int i = 0; i < 10000; ++i) {
    aTaskQueue.SetTask(&TestFunction);
  }
  ThreadPool aThreadPool(12, aTaskQueue);
  aThreadPool.Run();
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

  RunTestTasks();

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::printf("duration = %ld microseconds\nTasks completed = %d\n", duration.count(), tasksCompleted.load());
  // A.Print();
  // std::printf("-----------\n");
  // B.Print();
  // std::printf("-----------\n");
  // C.Print();

  return 0;
}