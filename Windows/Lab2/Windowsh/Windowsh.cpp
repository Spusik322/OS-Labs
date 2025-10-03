#define NOMINMAX
#include <iostream>
#include <vector>
#include <chrono>
#include <windows.h>
#include <algorithm>
#include <fstream>

struct ThreadData {
    const std::vector<std::vector<int>>* A;
    const std::vector<std::vector<int>>* B;
    std::vector<std::vector<int>>* C;
    int startRow;
    int endRow;
    int startCol;
    int endCol;
};

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

DWORD WINAPI multiplyBlock(LPVOID param) {
    ThreadData* data = static_cast<ThreadData*>(param);
    int n = (int)data->A->size();
    int endRow = std::min(data->endRow, n);
    int endCol = std::min(data->endCol, n);

    for (int i = data->startRow; i < endRow; ++i) {
        for (int j = data->startCol; j < endCol; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += (*data->A)[i][k] * (*data->B)[k][j];
            }
            (*data->C)[i][j] = sum;
        }
    }

    delete data;
    return 0;
}

int main() {
    const int n = 1024;
    const int blockSize = 256;
    int treads = 0;

    std::vector<std::vector<int>> A(n, std::vector<int>(n, 2));
    std::vector<std::vector<int>> B(n, std::vector<int>(n, 2));
    std::vector<std::vector<int>> C_simple(n, std::vector<int>(n, 0));
    std::vector<std::vector<int>> C_parallel(n, std::vector<int>(n, 0));

    long long simpleTime = multiplySimple(A, B, C_simple);
    std::cout << "Matrix " << n << "x" << n << " - Simple: " << simpleTime << " ms\n";

    auto start = std::chrono::high_resolution_clock::now();

    int rowBlocks = (n + blockSize - 1) / blockSize;
    int colBlocks = (n + blockSize - 1) / blockSize;

    std::vector<HANDLE> handles;
    handles.reserve(256);

    const DWORD MAX_WAIT = MAXIMUM_WAIT_OBJECTS;

    for (int bi = 0; bi < rowBlocks; ++bi) {
        for (int bj = 0; bj < colBlocks; ++bj) {
            ThreadData* data = new ThreadData;
            data->A = &A;
            data->B = &B;
            data->C = &C_parallel;
            data->startRow = bi * blockSize;
            data->endRow = std::min((bi + 1) * blockSize, n);
            data->startCol = bj * blockSize;
            data->endCol = std::min((bj + 1) * blockSize, n);

            HANDLE h = CreateThread(NULL, 0, multiplyBlock, data, 0, NULL);
            if (h) {
                treads++;
                handles.push_back(h);
            } else {
                std::cerr << "CreateThread failed for block (" << bi << "," << bj << ")\n";
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