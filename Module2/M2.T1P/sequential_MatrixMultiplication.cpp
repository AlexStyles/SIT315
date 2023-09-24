#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>

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

void MultiplyMatrices(const Matrix& A, const Matrix& B, Matrix& C, const uint16_t matrixSize, const uint16_t inMaxThreads = 0) {
  if (!matrixSize)
    return;

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
  static constexpr uint16_t matrixSize = 5000;
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