#include "benchmark.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

#include "UnqPtr.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtrArray.hpp"
#include "ShrdPtrArray.hpp"

#define RESET_COLOR  "\033[0m"
#define CYAN_COLOR   "\033[36m"
#define YELLOW_COLOR "\033[33m"
#define BOLD_COLOR   "\033[1m"

// Ассемблерный барьер — запрещает Clang/GCC выбрасывать цикл в 0 мс
template <typename T>
static void Escape(T const* p) {
    asm volatile("" : : "g"(p) : "memory");
}

void RunBenchmarks() {
    constexpr size_t kIterations = 1'000'000;

    std::cout << "\n" << CYAN_COLOR << "==========================================================" << RESET_COLOR << "\n";
    std::cout << BOLD_COLOR << "            БЕНЧМАРКИ ПРОИЗВОДИТЕЛЬНОСТИ                  " << RESET_COLOR << "\n";
    std::cout << "  Количество операций: " << YELLOW_COLOR << kIterations << RESET_COLOR << " на каждый тест\n";
    std::cout << CYAN_COLOR << "==========================================================" << RESET_COLOR << "\n\n";

    std::cout << std::left << std::setw(32) << "Тип операции" 
              << std::setw(16) << "Время (мс)" 
              << "Относительно сырого ptr\n";
    std::cout << "----------------------------------------------------------\n";

    // 1. Сырой указатель
    double raw_ms = 0.0;
    {
        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < kIterations; ++i) {
            int* p = new int(static_cast<int>(i));
            Escape(p);
            delete p;
        }
        auto end = std::chrono::steady_clock::now();
        raw_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << std::left << std::setw(32) << "Сырой указатель (new/delete)"
                  << std::setw(16) << std::fixed << std::setprecision(2) << raw_ms 
                  << "1.00x (база)\n";
    }

    // 2. UnqPtr
    {
        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < kIterations; ++i) {
            auto p = MakeUnq<int>(static_cast<int>(i));
            Escape(p.get());
        }
        auto end = std::chrono::steady_clock::now();
        double unq_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double ratio = (raw_ms > 0.001) ? (unq_ms / raw_ms) : 1.0;

        std::cout << std::left << std::setw(32) << "UnqPtr (MakeUnq)"
                  << std::setw(16) << std::fixed << std::setprecision(2) << unq_ms 
                  << std::fixed << std::setprecision(2) << ratio << "x\n";
    }

    // 3. ShrdPtr
    {
        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < kIterations; ++i) {
            auto p = MakeShrd<int>(static_cast<int>(i));
            Escape(p.get());
        }
        auto end = std::chrono::steady_clock::now();
        double shrd_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double ratio = (raw_ms > 0.001) ? (shrd_ms / raw_ms) : 1.0;

        std::cout << std::left << std::setw(32) << "ShrdPtr (MakeShrd)"
                  << std::setw(16) << std::fixed << std::setprecision(2) << shrd_ms 
                  << std::fixed << std::setprecision(2) << ratio << "x\n";
    }

    // 4. Массив UnqPtr<int[]>
    {
        constexpr size_t kArraySize = 64;
        constexpr size_t kArrayIters = 200'000;

        auto start = std::chrono::steady_clock::now();
        for (size_t i = 0; i < kArrayIters; ++i) {
            auto arr = MakeUnq<int[]>(kArraySize);
            Escape(arr.get());
        }
        auto end = std::chrono::steady_clock::now();
        double arr_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << std::left << std::setw(32) << "UnqPtr<int[]> (200k x 64 elem)"
                  << std::setw(16) << std::fixed << std::setprecision(2) << arr_ms 
                  << "[OK]\n";
    }

    std::cout << CYAN_COLOR << "==========================================================" << RESET_COLOR << "\n\n";
}