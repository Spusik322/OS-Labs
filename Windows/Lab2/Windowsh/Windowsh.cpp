#define NOMINMAX
#include <iostream>
#include <vector>
#include <chrono>
#include <windows.h>
#include <algorithm>
#include <fstream>
#include <random>
#include <mutex>

std::mutex result_mutex;

struct ThreadData {
    const std::vector<std::vector<int>>* A;
    const std::vector<std::vector<int>>* B;
    std::vector<std::vector<int>>* C;
    int blockRowA;
    int blockColA;
    int blockRowB;
    int blockColB;
    int blockSize;
};

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
    int n = (int)A.size();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

DWORD WINAPI multiplyBlocks(LPVOID param) {
    ThreadData* data = static_cast<ThreadData*>(param);
    int n = static_cast<int>(data->A->size());
    
    int startRowA = data->blockRowA * data->blockSize;
    int endRowA = std::min((data->blockRowA + 1) * data->blockSize, n);
    int startColA = data->blockColA * data->blockSize;
    int endColA = std::min((data->blockColA + 1) * data->blockSize, n);
    
    int startRowB = data->blockRowB * data->blockSize;
    int endRowB = std::min((data->blockRowB + 1) * data->blockSize, n);
    int startColB = data->blockColB * data->blockSize;
    int endColB = std::min((data->blockColB + 1) * data->blockSize, n);
    
    for (int i = startRowA; i < endRowA; ++i) {
        for (int j = startColB; j < endColB; ++j) {
            int sum = 0;
            for (int k = 0; k < data->blockSize; ++k) {
                int colA = startColA + k;
                int rowB = startRowB + k;
                
                if (colA < endColA && rowB < endRowB) {
                    sum += (*data->A)[i][colA] * (*data->B)[rowB][j];
                }
            }
            
            std::lock_guard<std::mutex> lock(result_mutex);
            (*data->C)[i][j] += sum;
        }
    }

    delete data;
    return 0;
}

int main() {
    const int n = 1024;
    const int blockSize = 64;
    int treads = 0;

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

    std::vector<HANDLE> handles;

    const DWORD MAX_WAIT = MAXIMUM_WAIT_OBJECTS;

    for (int i = 0; i < blocksPerDim; ++i) {
        for (int j = 0; j < blocksPerDim; ++j) {
            for (int k = 0; k < blocksPerDim; ++k) {
                ThreadData* data = new ThreadData;
                data->A = &A;
                data->B = &B;
                data->C = &C_parallel;
                data->blockRowA = i;
                data->blockColA = k;
                data->blockRowB = k;
                data->blockColB = j;
                data->blockSize = blockSize;

                HANDLE h = CreateThread(NULL, 0, multiplyBlocks, data, 0, NULL);
                if (h) {
                    treads++;
                    handles.push_back(h);
                } else {
                    std::cerr << "CreateThread failed for block (" << i << "," << j << ")\n";
                    delete data;
                }

                if (handles.size() >= MAX_WAIT) {
                    DWORD waitRes = WaitForMultipleObjects((DWORD)handles.size(), handles.data(), TRUE, INFINITE);
                    if (waitRes == WAIT_FAILED) {
                        std::cerr << "WaitForMultipleObjects failed\n";
                    }
                    for (HANDLE hh : handles) CloseHandle(hh);
                    handles.clear();
                }
            }
        }
    }

    if (!handles.empty()) {
        DWORD waitRes = WaitForMultipleObjects((DWORD)handles.size(), handles.data(), TRUE, INFINITE);
        if (waitRes == WAIT_FAILED) {
            std::cerr << "WaitForMultipleObjects failed\n";
        }
        for (HANDLE hh : handles) CloseHandle(hh);
        handles.clear();
    }

    auto parallelTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Parallel (block " << blockSize << " ,treads " << treads << "): " << parallelTime
              << " ms\n";
    std::cout << "Speedup: " << (double)simpleTime / parallelTime << "x" << std::endl;

    bool correct = true;
    for (int i = 0; i < n && correct; ++i) {
        for (int j = 0; j < n && correct; ++j) {
            if (C_simple[i][j] != C_parallel[i][j]) {
                correct = false;
                std::cout << "Mismatch at (" << i << "," << j << "): "
                          << C_simple[i][j] << " vs " << C_parallel[i][j] << "\n";
            }
        }
    }

    std::cout << "Results are " << (correct ? "CORRECT" : "INCORRECT") << std::endl;

    std::ofstream resultsFile("../../results.txt", std::ios_base::app);
    if (resultsFile.is_open()) {
        resultsFile << "Matrix: " << n << "x" << n << std::endl
                    << "BlockSize: " << blockSize << std::endl
                    << "Threads: " << treads << std::endl
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