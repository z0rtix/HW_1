#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <iostream>
#include <chrono>
#include <memory>
#include <vector>
#include <iomanip>

#include "UnqPtr.hpp"
#include "ShrdPtr.hpp"


using Clock = std::chrono::high_resolution_clock;

inline long long BenchRaw(size_t n) {
    volatile int sink = 0;
    auto start = Clock::now();
    for (size_t i = 0; i < n; i++) {
        int* p = new int(i);
        sink += *p;
        delete p;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline long long BenchOurUnq(size_t n) {
    volatile int sink = 0;
    auto start = Clock::now();
    for (size_t i = 0; i < n; i++) {
        auto p = MakeUnq<int>(i);
        sink += *p;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline long long BenchStdUnq(size_t n) {
    volatile int sink = 0;
    auto start = Clock::now();
    for (size_t i = 0; i < n; i++) {
        auto p = std::make_unique<int>(i);
        sink += *p;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline long long BenchOurShrd(size_t n) {
    volatile int sink = 0;
    auto start = Clock::now();
    for (size_t i = 0; i < n; i++) {
        auto p1 = MakeShrd<int>(i);
        auto p2 = p1;
        sink += *p2;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline long long BenchStdShrd(size_t n) {
    volatile int sink = 0;
    auto start = Clock::now();
    for (size_t i = 0; i < n; i++) {
        auto p1 = std::make_shared<int>(i);
        auto p2 = p1;
        sink += *p2;
    }
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline void RunBenchmarks() {
    std::vector<size_t> counts = {100'000, 1'000'000, 3'000'000};

    std::cout << "--- Нагрузочные тесты (время в миллисекундах) ---\n\n";
    std::cout << std::left 
              << std::setw(12) << "N (кол-во)"
              << std::setw(12) << "Сырые ptr"
              << std::setw(12) << "Наш Unq"
              << std::setw(14) << "std::unique"
              << std::setw(12) << "Наш Shrd"
              << std::setw(14) << "std::shared"
              << "\n";
    std::cout << std::string(76, '-') << "\n";

    for (size_t n : counts) {
        long long t_raw = BenchRaw(n);
        long long t_our_unq = BenchOurUnq(n);
        long long t_std_unq = BenchStdUnq(n);
        long long t_our_shrd = BenchOurShrd(n);
        long long t_std_shrd = BenchStdShrd(n);

        std::cout << std::left 
                  << std::setw(12) << n
                  << std::setw(12) << t_raw
                  << std::setw(12) << t_our_unq
                  << std::setw(14) << t_std_unq
                  << std::setw(12) << t_our_shrd
                  << std::setw(14) << t_std_shrd
                  << "\n";
    }

    std::cout << "\nБенчмарк успешно завершён!\n";
}

#endif