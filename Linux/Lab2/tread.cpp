#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

// Функция обычного умножения матриц
void multiplySimple(const std::vector<std::vector<double>>& A,
                   const std::vector<std::vector<double>>& B,
                   std::vector<std::vector<double>>& C) {
    int n = A.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < n; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Функция для умножения одного блока
void multiplyBlock(const std::vector<std::vector<double>>& A,
                  const std::vector<std::vector<double>>& B,
                  std::vector<std::vector<double>>& C,
                  int startRow, int endRow,
                  int startCol, int endCol) {
    int n = A.size();
    for (int i = startRow; i < endRow; ++i) {
        for (int j = startCol; j < endCol; ++j) {
            double sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

int main() {
    // Размер матрицы
    const int N = 5;
    
    // Создаем и заполняем матрицы вручную
    std::vector<std::vector<double>> A = {
        {1, 2, 3, 4, 5},
        {2, 3, 4, 5, 6},
        {3, 4, 5, 6, 7},
        {4, 5, 6, 7, 8},
        {5, 6, 7, 8, 9}
    };
    
    std::vector<std::vector<double>> B = {
        {9, 8, 7, 6, 5},
        {8, 7, 6, 5, 4},
        {7, 6, 5, 4, 3},
        {6, 5, 4, 3, 2},
        {5, 4, 3, 2, 1}
    };
    
    // Результирующая матрица
    std::vector<std::vector<double>> C1(N, std::vector<double>(N, 0));
    std::vector<std::vector<double>> C2(N, std::vector<double>(N, 0));
    
    // Обычное умножение
    auto start = std::chrono::high_resolution_clock::now();
    multiplySimple(A, B, C1);
    auto end = std::chrono::high_resolution_clock::now();
    auto simpleTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "Simple multiplication time: " << simpleTime << " μs\n";
    std::cout << "Result matrix C1:\n";
    for (const auto& row : C1) {
        for (const auto& val : row) {
            std::cout << std::setw(8) << val;
        }
        std::cout << std::endl;
    }
    
    // Многопоточное умножение блоками
    int blockSize = 2; // Размер блока
    std::cout << "\nBlock multiplication with block size: " << blockSize << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    int rowBlocks = (N + blockSize - 1) / blockSize;
    int colBlocks = (N + blockSize - 1) / blockSize;
    
    for (int i = 0; i < rowBlocks; ++i) {
        for (int j = 0; j < colBlocks; ++j) {
            int startRow = i * blockSize;
            int endRow = std::min((i + 1) * blockSize, N);
            int startCol = j * blockSize;
            int endCol = std::min((j + 1) * blockSize, N);
            
            threads.emplace_back(multiplyBlock, 
                               std::ref(A), std::ref(B), std::ref(C2),
                               startRow, endRow, startCol, endCol);
        }
    }
    
    // Ждем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto parallelTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "Parallel multiplication time: " << parallelTime << " μs\n";
    std::cout << "Number of threads: " << threads.size() << std::endl;
    std::cout << "Result matrix C2:\n";
    for (const auto& row : C2) {
        for (const auto& val : row) {
            std::cout << std::setw(8) << val;
        }
        std::cout << std::endl;
    }
    
    // Сравнение результатов
    bool equal = true;
    for (int i = 0; i < N && equal; ++i) {
        for (int j = 0; j < N && equal; ++j) {
            if (std::abs(C1[i][j] - C2[i][j]) > 1e-9) {
                equal = false;
            }
        }
    }
    
    std::cout << "\nResults are " << (equal ? "EQUAL" : "DIFFERENT") << std::endl;
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(simpleTime) / parallelTime << "x" << std::endl;
    
    // Тестируем разные размеры блоков
    std::cout << "\n=== Testing different block sizes ===" << std::endl;
    std::cout << std::setw(10) << "Block Size" 
              << std::setw(15) << "Threads" 
              << std::setw(15) << "Time (μs)" 
              << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(55, '-') << std::endl;
    
    for (int k = 1; k <= N; ++k) {
        std::vector<std::vector<double>> C3(N, std::vector<double>(N, 0));
        
        start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads_k;
        int rowBlocks_k = (N + k - 1) / k;
        int colBlocks_k = (N + k - 1) / k;
        
        for (int i = 0; i < rowBlocks_k; ++i) {
            for (int j = 0; j < colBlocks_k; ++j) {
                int startRow = i * k;
                int endRow = std::min((i + 1) * k, N);
                int startCol = j * k;
                int endCol = std::min((j + 1) * k, N);
                
                threads_k.emplace_back(multiplyBlock, 
                                    std::ref(A), std::ref(B), std::ref(C3),
                                    startRow, endRow, startCol, endCol);
            }
        }
        
        for (auto& thread : threads_k) {
            thread.join();
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto time_k = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        double speedup = static_cast<double>(simpleTime) / time_k;
        
        std::cout << std::setw(10) << k 
                  << std::setw(15) << threads_k.size()
                  << std::setw(15) << time_k 
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << std::endl;
    }
    
    return 0;
}