
#include <iostream>
#include <vector>
#include <chrono>
#include <windows.h>

// Структура для передачи параметров в поток
struct ThreadData {
    const std::vector<std::vector<int>>* A;
    const std::vector<std::vector<int>>* B;
    std::vector<std::vector<int>>* C;
    int startRow;
    int endRow;
    int startCol;
    int endCol;
};

// Обычное умножение матриц
long long multiplySimple(const std::vector<std::vector<int>>& A,
    const std::vector<std::vector<int>>& B,
    std::vector<std::vector<int>>& C) {
    int n = A.size();
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// Умножение блоков
DWORD WINAPI multiplyBlock(LPVOID param) {
    ThreadData* data = static_cast<ThreadData*>(param);
    int n = data->A->size();

    // Проверяем границы, чтобы не выйти за пределы матрицы
    int endRow = min(data->endRow, n);
    int endCol = min(data->endCol, n);

    for (int i = data->startRow; i < endRow; i++) {
        for (int j = data->startCol; j < endCol; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                sum += (*data->A)[i][k] * (*data->B)[k][j];
            }
            (*data->C)[i][j] = sum;
        }
    }

    delete data;
    return 0;
}

int main() {
    // Создаем матрицу 500x500, заполненную двойками
    const int n = 500;
    const int blockSize = 20; // Теперь используем блоки по 20

    std::vector<std::vector<int>> A(n, std::vector<int>(n, 2));
    std::vector<std::vector<int>> B(n, std::vector<int>(n, 2));
    std::vector<std::vector<int>> C_simple(n, std::vector<int>(n, 0));
    std::vector<std::vector<int>> C_parallel(n, std::vector<int>(n, 0));

    // Обычное умножение
    long long simpleTime = multiplySimple(A, B, C_simple);
    std::cout << "Matrix " << n << "x" << n << " - Simple: " << simpleTime << " ms\n";

    // Блочное умножение
    auto start = std::chrono::high_resolution_clock::now();

    // Создаем потоки для блоков
    std::vector<HANDLE> threads;
    int rowBlocks = (n + blockSize - 1) / blockSize;
    int colBlocks = (n + blockSize - 1) / blockSize;

    std::cout << "Creating " << rowBlocks * colBlocks << " threads for block size " << blockSize << std::endl;

    for (int i = 0; i < rowBlocks; i++) {
        for (int j = 0; j < colBlocks; j++) {
            ThreadData* data = new ThreadData;
            data->A = &A;
            data->B = &B;
            data->C = &C_parallel;
            data->startRow = i * blockSize;
            data->endRow = (i + 1) * blockSize; // Не используем min здесь
            data->startCol = j * blockSize;
            data->endCol = (j + 1) * blockSize; // Не используем min здесь

            HANDLE thread = CreateThread(NULL, 0, multiplyBlock, data, 0, NULL);
            if (thread) {
                threads.push_back(thread);
            }
            else {
                std::cerr << "Failed to create thread for block (" << i << "," << j << ")" << std::endl;
                delete data;
            }
        }
    }

    // Ждем завершения всех потоков
    if (!threads.empty()) {
        WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
    }

    // Закрываем handles потоков
    for (HANDLE h : threads) {
        CloseHandle(h);
    }

    auto parallelTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Parallel (block " << blockSize << ", " << threads.size() << " threads): "
        << parallelTime << " ms, Speedup: " << (double)simpleTime / parallelTime << "x\n";

    // Проверяем корректность результатов
    bool correct = true;
    for (int i = 0; i < n && correct; i++) {
        for (int j = 0; j < n && correct; j++) {
            if (C_simple[i][j] != C_parallel[i][j]) {
                correct = false;
                std::cout << "Mismatch at (" << i << "," << j << "): "
                    << C_simple[i][j] << " vs " << C_parallel[i][j] << std::endl;
            }
        }
    }

    std::cout << "Results are " << (correct ? "CORRECT" : "INCORRECT") << std::endl;

    return 0;
}