#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <random>
#include <mutex>

std::mutex result_mutex;

void fillMatrixRandom(std::vector<std::vector<int>>& matrix, int minVal = 1, int maxVal = 10) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minVal, maxVal);
    
    for (auto& row : matrix) {
        for (auto& elem : row) {
            elem = dis(gen);
        }
    }
}

long long multiplySimple(const std::vector<std::vector<int>>& A,
                         const std::vector<std::vector<int>>& B,
                         std::vector<std::vector<int>>& C) {
    int n = static_cast<int>(A.size());
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void multiplyBlocks(const std::vector<std::vector<int>>& A,
                   const std::vector<std::vector<int>>& B,
                   std::vector<std::vector<int>>& C,
                   int blockRowA, int blockColA,
                   int blockRowB, int blockColB,
                   int blockSize) {
    int n = static_cast<int>(A.size());
    int startRowA = blockRowA * blockSize;
    int endRowA = std::min((blockRowA + 1) * blockSize, n);
    int startColA = blockColA * blockSize;
    int endColA = std::min((blockColA + 1) * blockSize, n);
    
    int startRowB = blockRowB * blockSize;
    int endRowB = std::min((blockRowB + 1) * blockSize, n);
    int startColB = blockColB * blockSize;
    int endColB = std::min((blockColB + 1) * blockSize, n);
    
    for (int i = startRowA; i < endRowA; ++i) {
        for (int j = startColB; j < endColB; ++j) {
            int sum = 0;
            for (int k = 0; k < blockSize; ++k) {
                int colA = startColA + k;
                int rowB = startRowB + k;
                
                if (colA < endColA && rowB < endRowB) {
                    sum += A[i][colA] * B[rowB][j];
                }
            }
            
            std::lock_guard<std::mutex> lock(result_mutex);
            C[i][j] += sum;
        }
    }
}

int main() {
    const int n = 1024;
    const int blockSize = 32;

    std::vector<std::vector<int>> A(n, std::vector<int>(n));
    std::vector<std::vector<int>> B(n, std::vector<int>(n));
    std::vector<std::vector<int>> C_simple(n, std::vector<int>(n, 0));
    std::vector<std::vector<int>> C_parallel(n, std::vector<int>(n, 0));

    fillMatrixRandom(A, 1, 100);
    fillMatrixRandom(B, 1, 100);

    long long simpleTime = multiplySimple(A, B, C_simple);
    std::cout << "Matrix " << n << "x" << n << " - Simple: " << simpleTime << " ms\n";

    auto start = std::chrono::high_resolution_clock::now();

    int blocksPerDim = (n + blockSize - 1) / blockSize;
    
    std::vector<std::thread> threads;
    int threadsCreated = 0;

    for (int i = 0; i < blocksPerDim; ++i) {
        for (int j = 0; j < blocksPerDim; ++j) {
            for (int k = 0; k < blocksPerDim; ++k) {
                threads.emplace_back(multiplyBlocks,
                                   std::cref(A), std::cref(B), std::ref(C_parallel),
                                   i, k, k, j, blockSize);
                ++threadsCreated;
            }
        }
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    } 

    auto parallelTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Parallel (block " << blockSize << " ,threads " << threadsCreated << "): "
              << parallelTime << " ms\n";
    std::cout << "Speedup: " << (double)simpleTime / parallelTime << "x\n";

    bool correct = true;
    for (int i = 0; i < n && correct; ++i)
        for (int j = 0; j < n && correct; ++j)
            if (C_simple[i][j] != C_parallel[i][j]) {
                correct = false;
                std::cout << "Mismatch at (" << i << "," << j << "): "
                          << C_simple[i][j] << " vs " << C_parallel[i][j] << "\n";
            }

    std::cout << "Results are " << (correct ? "CORRECT" : "INCORRECT") << std::endl;

    std::ofstream resultsFile("../results.txt", std::ios_base::app);
    if (resultsFile.is_open()) {
        resultsFile << "Matrix: " << n << "x" << n << std::endl
                    << "BlockSize: " << blockSize << std::endl
                    << "Threads: " << threadsCreated << std::endl
                    << "SimpleTime: " << simpleTime << "ms" << std::endl
                    << "ParallelTime: " << parallelTime << "ms" << std::endl
                    << "Speedup: " << (double)simpleTime / parallelTime << "x" << std::endl
                    << "Correct: " << (correct ? "YES" : "NO") << std::endl
                    << std::endl;
        resultsFile.close();
        std::cout << "Results saved to results.txt" << std::endl;
    } else {
        std::cerr << "Failed to open results.txt for writing" << std::endl;
    }
    return 0;
}