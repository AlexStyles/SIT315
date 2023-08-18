#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>

#include <pthread.h>

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

  for (uint8_t i = 0; i < matrixSize; ++i) {
    for (uint8_t j = 0; j < matrixSize; ++j) {
      for (uint8_t k = 0; k < matrixSize; ++k) {
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

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  std::printf("duration = %ld microseconds\n", duration.count());
  // A.Print();
  // std::printf("-----------\n");
  // B.Print();
  // std::printf("-----------\n");
  // C.Print();

  return 0;
}