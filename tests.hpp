#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <cassert>
#include <string_view>
#include <stdexcept>
#include <chrono>
#include <memory>
#include <vector>
#include <iomanip>

#include "UnqPtr.hpp"
#include "ShrdPtr.hpp"
#include "DynamicArray.hpp"


struct BaseTracker {
    static inline int alive = 0;

    BaseTracker() { ++alive; }
    BaseTracker(const BaseTracker&) { ++alive; }
    BaseTracker& operator=(const BaseTracker&) = default;
    virtual ~BaseTracker() { --alive; }

    virtual std::string_view Identify() const {
        return "BaseTracker";
    }
};


struct DerivedTracker : public BaseTracker {
    DerivedTracker() : BaseTracker() {}
    ~DerivedTracker() override = default;

    std::string_view Identify() const override {
        return "DerivedTracker";
    }
};


inline void TestUnqPtr() {
    std::cout << "--- Тестирование UnqPtr ---\n";
    assert(BaseTracker::alive == 0);

    {
        auto ptr = MakeUnq<BaseTracker>();
        assert(BaseTracker::alive == 1);
        assert(ptr);
        assert(ptr.get() != nullptr);
    }
    assert(BaseTracker::alive == 0);
    std::cout << "[✓] 👍 Деструктор и базовое освобождение памяти\n";

    {
        UnqPtr<BaseTracker> null_ptr;
        assert(!null_ptr);
        assert(null_ptr.get() == nullptr);

        bool caught = false;
        try {
            (void)*null_ptr;
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }
    std::cout << "[✓] 👍 Защита от разыменования nullptr\n";

    {
        BaseTracker* raw = nullptr;
        {
            auto ptr = MakeUnq<BaseTracker>();
            assert(BaseTracker::alive == 1);
            raw = ptr.release();
            assert(!ptr);
            assert(BaseTracker::alive == 1);
        }
        assert(BaseTracker::alive == 1);
        delete raw;
        assert(BaseTracker::alive == 0);
    }
    std::cout << "[✓] 👍 Метод release (передача без удаления)\n";

    {
        auto ptr = MakeUnq<BaseTracker>();
        assert(BaseTracker::alive == 1);
        ptr.reset(new BaseTracker());
        assert(BaseTracker::alive == 1);
        ptr.reset();
        assert(BaseTracker::alive == 0);
    }
    std::cout << "[✓] 👍 Метод reset (замена и зануление)\n";

    {
        UnqPtr<DerivedTracker> d = MakeUnq<DerivedTracker>();
        UnqPtr<BaseTracker> b = std::move(d);
        assert(!d);
        assert(b);
        assert(b->Identify() == "DerivedTracker");
    }
    assert(BaseTracker::alive == 0);
    std::cout << "[✓] 👍 Подтипизация (Derived -> Base)\n\n";
}

inline void TestShrdPtr() {
    std::cout << "--- Тестирование ShrdPtr ---\n";
    assert(BaseTracker::alive == 0);

    {
        ShrdPtr<BaseTracker> sp1 = MakeShrd<BaseTracker>();
        assert(sp1.use_count() == 1);
        assert(BaseTracker::alive == 1);

        {
            ShrdPtr<BaseTracker> sp2 = sp1;
            assert(sp1.use_count() == 2);
            assert(sp2.use_count() == 2);

            ShrdPtr<BaseTracker> sp3;
            sp3 = sp2;
            assert(sp1.use_count() == 3);
        }
        assert(sp1.use_count() == 1);
        assert(BaseTracker::alive == 1);
    }
    assert(BaseTracker::alive == 0);
    std::cout << "[✓] 👍 Счетчик ссылок (use_count) и очистка\n";

    {
        ShrdPtr<BaseTracker> null_sp;
        assert(null_sp.use_count() == 0);
        assert(!null_sp);

        bool caught = false;
        try {
            (void)*null_sp;
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
    }
    std::cout << "[✓] 👍 Пустой дескриптор и перехват исключения\n";

    {
        ShrdPtr<BaseTracker> sp = MakeShrd<BaseTracker>();
        assert(sp.use_count() == 1);
        auto& self = sp;
        sp = self;
        assert(sp.use_count() == 1);
        assert(BaseTracker::alive == 1);
    }
    assert(BaseTracker::alive == 0);
    std::cout << "[✓] 👍 Защита от самоприсваивания\n";

    {
        auto unq = MakeUnq<DerivedTracker>();
        ShrdPtr<BaseTracker> shrd(std::move(unq));
        assert(!unq);
        assert(shrd.use_count() == 1);
        assert(shrd->Identify() == "DerivedTracker");
        assert(BaseTracker::alive == 1);
    }
    assert(BaseTracker::alive == 0);
    std::cout << "[✓] 👍 Передача UnqPtr -> ShrdPtr с подтипизацией\n\n";
}

inline void TestDynamicArray() {
    std::cout << "--- Тестирование DynamicArray ---\n";
    {
        DynamicArray<int> arr;
        assert(arr.GetSize() == 0);

        arr.Append(10);
        arr.Append(20);
        arr.Append(30);

        assert(arr.GetSize() == 3);
        assert(arr.Get(0) == 10);
        assert(arr.Get(1) == 20);
        assert(arr[2] == 30);

        bool caught = false;
        try {
            (void)arr.Get(99);
        } catch (const std::out_of_range&) {
            caught = true;
        }
        assert(caught);
    }
    std::cout << "[✓] 👍 Корректность методов Append, Get и проверка границ\n";

    assert(BaseTracker::alive == 0);
    {
        DynamicArray<BaseTracker> arr;
        {
            BaseTracker item;
            arr.Append(item);
            arr.Append(item);
        }
        assert(BaseTracker::alive == 2);
    }
    assert(BaseTracker::alive == 0);
    std::cout << "[✓] 👍 Автоматическое освобождение памяти контейнера\n\n";
}

inline void RunBenchmarks() {
    using Clock = std::chrono::high_resolution_clock;
    std::vector<size_t> counts = {50000, 200000, 500000};

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
        // 1. Сырые указатели
        auto s1 = Clock::now();
        {
            std::vector<int*> vec(n);
            for (size_t i = 0; i < n; ++i) {
                vec[i] = new int(static_cast<int>(i));
            }
            for (size_t i = 0; i < n; ++i) {
                delete vec[i];
            }
        }
        auto e1 = Clock::now();
        long long t_raw = std::chrono::duration_cast<std::chrono::milliseconds>(e1 - s1).count();

        // 2. Наш UnqPtr
        auto s2 = Clock::now();
        {
            std::vector<UnqPtr<int>> vec;
            vec.reserve(n);
            for (size_t i = 0; i < n; ++i) {
                vec.push_back(MakeUnq<int>(static_cast<int>(i)));
            }
        }
        auto e2 = Clock::now();
        long long t_our_unq = std::chrono::duration_cast<std::chrono::milliseconds>(e2 - s2).count();

        // 3. std::unique_ptr
        auto s3 = Clock::now();
        {
            std::vector<std::unique_ptr<int>> vec;
            vec.reserve(n);
            for (size_t i = 0; i < n; ++i) {
                vec.push_back(std::make_unique<int>(static_cast<int>(i)));
            }
        }
        auto e3 = Clock::now();
        long long t_std_unq = std::chrono::duration_cast<std::chrono::milliseconds>(e3 - s3).count();

        // 4. Наш ShrdPtr
        auto s4 = Clock::now();
        {
            std::vector<ShrdPtr<int>> vec;
            vec.reserve(n);
            for (size_t i = 0; i < n; ++i) {
                vec.push_back(MakeShrd<int>(static_cast<int>(i)));
            }
        }
        auto e4 = Clock::now();
        long long t_our_shrd = std::chrono::duration_cast<std::chrono::milliseconds>(e4 - s4).count();

        // 5. std::shared_ptr
        auto s5 = Clock::now();
        {
            std::vector<std::shared_ptr<int>> vec;
            vec.reserve(n);
            for (size_t i = 0; i < n; ++i) {
                vec.push_back(std::make_shared<int>(static_cast<int>(i)));
            }
        }
        auto e5 = Clock::now();
        long long t_std_shrd = std::chrono::duration_cast<std::chrono::milliseconds>(e5 - s5).count();

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