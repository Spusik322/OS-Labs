#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <random>
#include <mutex>
#include <queue>
#include <condition_variable>

template<class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : capacity(size) {}

    void Send(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]() { return closed_ || queue_.size() < capacity; });
        if (closed_) {
            throw std::runtime_error("Channel is closed");
        }
        queue_.push(std::move(value));
        not_empty_.notify_one();
    }

    std::pair<T, bool> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]() { return closed_ || !queue_.empty(); });
        if (queue_.empty()) {
            return std::make_pair(T(), false);
        }
        T value = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return std::make_pair(std::move(value), true);
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    size_t capacity;
    bool closed_ = false;
};

struct BlockTask {
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
                   const BlockTask& task) {
    int n = static_cast<int>(A.size());
    int startRowA = task.blockRowA * task.blockSize;
    int endRowA = std::min((task.blockRowA + 1) * task.blockSize, n);
    int startColA = task.blockColA * task.blockSize;
    int endColA = std::min((task.blockColA + 1) * task.blockSize, n);
    
    int startRowB = task.blockRowB * task.blockSize;
    int endRowB = std::min((task.blockRowB + 1) * task.blockSize, n);
    int startColB = task.blockColB * task.blockSize;
    int endColB = std::min((task.blockColB + 1) * task.blockSize, n);
    
    for (int i = startRowA; i < endRowA; ++i) {
        for (int j = startColB; j < endColB; ++j) {
            int sum = 0;
            for (int k = 0; k < task.blockSize; ++k) {
                int colA = startColA + k;
                int rowB = startRowB + k;
                
                if (colA < endColA && rowB < endRowB) {
                    sum += A[i][colA] * B[rowB][j];
                }
            }
            
            C[i][j] += sum;
        }
    }
}

void workerThread(BufferedChannel<BlockTask>& channel,
                 const std::vector<std::vector<int>>& A,
                 const std::vector<std::vector<int>>& B,
                 std::vector<std::vector<int>>& C) {
    while (true) {
        auto task = channel.Recv();
        if (!task.second) {
            break;
        }
        multiplyBlocks(A, B, C, task.first);
    }
}

int main() {
    const int n = 2048;
    const int blockSize = 1024;

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
    const int numThreads = std::thread::hardware_concurrency();
    BufferedChannel<BlockTask> taskChannel(blocksPerDim * blocksPerDim);

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread, std::ref(taskChannel), 
                           std::cref(A), std::cref(B), std::ref(C_parallel));
    }

    int tasksCreated = 0;
    for (int i = 0; i < blocksPerDim; ++i) {
        for (int j = 0; j < blocksPerDim; ++j) {
            for (int k = 0; k < blocksPerDim; ++k) {
                BlockTask task{i, k, k, j, blockSize};
                taskChannel.Send(std::move(task));
                ++tasksCreated;
            }
        }
    }

    taskChannel.Close();

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    } 

    auto parallelTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Parallel (block " << blockSize << " ,threads " << numThreads << " ,tasks " << tasksCreated << "): "
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

    std::ofstream resultsFile("../../results.txt", std::ios_base::app);
    if (resultsFile.is_open()) {
        resultsFile << "Matrix: " << n << "x" << n << std::endl
                    << "BlockSize: " << blockSize << std::endl
                    << "Threads: " << numThreads << std::endl
                    << "Tasks: " << tasksCreated << std::endl
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