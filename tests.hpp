#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <cassert>
#include <utility>
#include <string>
#include "UnqPtr.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtrArray.hpp"
#include "ShrdPtrArray.hpp"
#include "DynamicArray.hpp"

#define RESET_COLOR "\033[0m"
#define GREEN_COLOR "\033[32m"
#define CYAN_COLOR  "\033[36m"
#define BOLD_COLOR  "\033[1m"

inline int g_passedTestsCount = 0;

#define TEST_CASE(name, code) \
	do { \
		code \
		++g_passedTestsCount; \
		std::cout << GREEN_COLOR << " [✓] " << RESET_COLOR << name << "\n"; \
	} while (0)

class BaseTracker {
	public:
		inline static int alive = 0;
		BaseTracker() noexcept { ++alive; }
		BaseTracker(const BaseTracker&) noexcept { ++alive; }
		BaseTracker& operator=(const BaseTracker&) noexcept = default;
		virtual ~BaseTracker() noexcept { --alive; }
};

class DerivedTracker : public BaseTracker {
	public:
		inline static int derived_alive = 0;
		DerivedTracker() noexcept { ++derived_alive; }
		DerivedTracker(const DerivedTracker& other) noexcept : BaseTracker(other) { ++derived_alive; }
		DerivedTracker& operator=(const DerivedTracker&) noexcept = default;
		~DerivedTracker() noexcept override { --derived_alive; }
};

inline void TestUnqPtrDetailed() {
	TEST_CASE("UnqPtr: создание, владение и деструкция (RAII)", {
		assert(BaseTracker::alive == 0);
		{
			auto p = MakeUnq<BaseTracker>();
			assert(BaseTracker::alive == 1);
			assert(p.get() != nullptr);
			assert(static_cast<bool>(p));
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("UnqPtr: разыменование через operator* и operator->", {
		struct Point { int x = 10; int y = 20; };
		auto p = MakeUnq<Point>();
		assert(p->x == 10);
		assert((*p).y == 20);
		p->x = 55;
		assert((*p).x == 55);
	});

	TEST_CASE("UnqPtr: конструктор перемещения", {
		assert(BaseTracker::alive == 0);
		{
			auto p1 = MakeUnq<BaseTracker>();
			assert(BaseTracker::alive == 1);
			auto p2 = std::move(p1);
			assert(p1.get() == nullptr);
			assert(!p1);
			assert(p2.get() != nullptr);
			assert(BaseTracker::alive == 1);
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("UnqPtr: оператор перемещающего присваивания", {
		assert(BaseTracker::alive == 0);
		{
			auto p1 = MakeUnq<BaseTracker>();
			auto p2 = MakeUnq<BaseTracker>();
			assert(BaseTracker::alive == 2);
			p2 = std::move(p1);
			assert(p1.get() == nullptr);
			assert(p2.get() != nullptr);
			assert(BaseTracker::alive == 1);
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("UnqPtr: release() сохраняет объект в куче без деструктора", {
		assert(BaseTracker::alive == 0);
		auto p = MakeUnq<BaseTracker>();
		assert(BaseTracker::alive == 1);
		BaseTracker* raw = p.release();
		assert(p.get() == nullptr);
		assert(BaseTracker::alive == 1);
		delete raw;
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("UnqPtr: reset() корректно заменяет и удаляет старый ресурс", {
		assert(BaseTracker::alive == 0);
		auto p = MakeUnq<BaseTracker>();
		assert(BaseTracker::alive == 1);
		p.reset(new BaseTracker());
		assert(BaseTracker::alive == 1);
		p.reset();
		assert(BaseTracker::alive == 0);
		assert(p.get() == nullptr);
	});
}

inline void TestUnqPtrArrayDetailed() {
	TEST_CASE("UnqPtr<T[]>: выделение памяти и доступ по индексу operator[]", {
		auto arr = MakeUnq<int[]>(5);
		for (size_t i = 0; i < 5; ++i) {
			arr[i] = static_cast<int>((i + 1) * 10);
		}
		assert(arr[0] == 10);
		assert(arr[2] == 30);
		assert(arr[4] == 50);
	});

	TEST_CASE("UnqPtr<T[]>: вызов delete[] для всех элементов массива", {
		assert(BaseTracker::alive == 0);
		{
			auto arr = MakeUnq<BaseTracker[]>(6);
			assert(BaseTracker::alive == 6);
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("UnqPtr<T[]>: перемещение массива и сброс reset()", {
		assert(BaseTracker::alive == 0);
		{
			auto arr1 = MakeUnq<BaseTracker[]>(3);
			assert(BaseTracker::alive == 3);
			auto arr2 = std::move(arr1);
			assert(arr1.get() == nullptr);
			assert(arr2.get() != nullptr);
			assert(BaseTracker::alive == 3);
			arr2.reset();
			assert(BaseTracker::alive == 0);
		}
		assert(BaseTracker::alive == 0);
	});
}

inline void TestShrdPtrDetailed() {
	TEST_CASE("ShrdPtr: подсчет ссылок при копировании и деструкции", {
		assert(BaseTracker::alive == 0);
		{
			auto sp1 = MakeShrd<BaseTracker>();
			assert(sp1.use_count() == 1);
			assert(BaseTracker::alive == 1);
			{
				auto sp2 = sp1;
				assert(sp1.use_count() == 2);
				assert(sp2.use_count() == 2);
				assert(BaseTracker::alive == 1);
				{
					auto sp3 = sp2;
					assert(sp1.use_count() == 3);
				}
				assert(sp1.use_count() == 2);
			}
			assert(sp1.use_count() == 1);
			assert(BaseTracker::alive == 1);
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("ShrdPtr: перемещение передает владение без изменения счетчика", {
		auto sp1 = MakeShrd<int>(321);
		assert(sp1.use_count() == 1);
		auto sp2 = std::move(sp1);
		assert(sp1.get() == nullptr);
		assert(sp1.use_count() == 0);
		assert(sp2.use_count() == 1);
		assert(*sp2 == 321);
	});

    TEST_CASE("ShrdPtr: копирующее и перемещающее присваивание", {
		assert(BaseTracker::alive == 0);
		{
			auto sp1 = MakeShrd<BaseTracker>();
			auto sp2 = MakeShrd<BaseTracker>();
			assert(BaseTracker::alive == 2);
			sp2 = sp1;
			assert(BaseTracker::alive == 1);
			assert(sp1.use_count() == 2);
			assert(sp2.use_count() == 2);

			auto sp3 = MakeShrd<BaseTracker>();
			assert(BaseTracker::alive == 2);
			sp3 = std::move(sp2); // sp3 освобождает свой старый объект (alive стал 1) и забирает sp2
			assert(sp2.get() == nullptr);
			assert(sp3.use_count() == 2);
			assert(BaseTracker::alive == 1); // ТЕПЕРЬ СТРОГО 1, ТАК КАК СТАРЫЙ ОБЪЕКТ SP3 УДАЛЕН!
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("ShrdPtr: метод reset() корректно уменьшает счетчик", {
		assert(BaseTracker::alive == 0);
		auto sp1 = MakeShrd<BaseTracker>();
		auto sp2 = sp1;
		assert(sp1.use_count() == 2);
		sp1.reset();
		assert(sp1.get() == nullptr);
		assert(sp1.use_count() == 0);
		assert(sp2.use_count() == 1);
		assert(BaseTracker::alive == 1);
		sp2.reset();
		assert(BaseTracker::alive == 0);
	});
}

inline void TestShrdPtrArrayDetailed() {
	TEST_CASE("ShrdPtr<T[]>: разделяемый массив с доступом по индексу", {
		auto arr1 = MakeShrd<int[]>(4);
		arr1[0] = 100;
		arr1[3] = 400;
		{
			auto arr2 = arr1;
			assert(arr1.use_count() == 2);
			assert(arr2.use_count() == 2);
			assert(arr2[0] == 100);
			assert(arr2[3] == 400);
			arr2[0] = 777;
		}
		assert(arr1.use_count() == 1);
		assert(arr1[0] == 777);
	});

	TEST_CASE("ShrdPtr<T[]>: корректная деструкция delete[] для всех элементов", {
		assert(BaseTracker::alive == 0);
		{
			auto arr = MakeShrd<BaseTracker[]>(5);
			assert(BaseTracker::alive == 5);
			assert(arr.use_count() == 1);
		}
		assert(BaseTracker::alive == 0);
	});
}

inline void TestPolymorphismAndAdvanced() {
	TEST_CASE("Полиморфизм: проверка виртуального деструктора через BaseTracker", {
		assert(BaseTracker::alive == 0);
		assert(DerivedTracker::derived_alive == 0);
		{
			UnqPtr<DerivedTracker> d = MakeUnq<DerivedTracker>();
			assert(BaseTracker::alive == 1);
			assert(DerivedTracker::derived_alive == 1);

			UnqPtr<BaseTracker> b = std::move(d);
			assert(d.get() == nullptr);
			assert(b.get() != nullptr);
		}
		assert(BaseTracker::alive == 0);
		assert(DerivedTracker::derived_alive == 0);
	});

	TEST_CASE("Конвертация UnqPtr в ShrdPtr (повышение владения)", {
		assert(BaseTracker::alive == 0);
		{
			UnqPtr<DerivedTracker> unq = MakeUnq<DerivedTracker>();
			ShrdPtr<BaseTracker> shrd(std::move(unq));
			assert(unq.get() == nullptr);
			assert(shrd.get() != nullptr);
			assert(shrd.use_count() == 1);
			assert(BaseTracker::alive == 1);
		}
		assert(BaseTracker::alive == 0);
	});

	TEST_CASE("Краевые случаи: самоприсваивание и пустые указатели", {
		auto p = MakeUnq<int>(10);
		auto& ref_p = p;
		p = std::move(ref_p);
		assert(p.get() != nullptr);
		assert(*p == 10);

		auto sp = MakeShrd<int>(20);
		auto& ref_sp = sp;
		sp = ref_sp;
		assert(sp.use_count() == 1);
		assert(*sp == 20);

		sp = std::move(ref_sp);
		assert(sp.use_count() == 1);
		assert(*sp == 20);

		UnqPtr<int> empty_u;
		assert(!empty_u);
		empty_u.reset();

		ShrdPtr<int> empty_s;
		assert(!empty_s);
		assert(empty_s.use_count() == 0);
		empty_s.reset();
	});
}

inline void TestDynamicArrayDetailed() {
	TEST_CASE("DynamicArray: базовые операции добавления и индексации", {
		DynamicArray<int> arr;
		for (int i = 0; i < 10; ++i) {
			arr.Append(i * 10);
		}
		assert(arr.GetSize() == 10);
		assert(arr[0] == 0);
		assert(arr[5] == 50);
		assert(arr[9] == 90);
	});

	TEST_CASE("DynamicArray: корректная деструкция объектов BaseTracker", {
		assert(BaseTracker::alive == 0);
		{
			DynamicArray<BaseTracker> arr;
			for (int i = 0; i < 5; ++i) {
				BaseTracker tracker;
				arr.Append(tracker);
			}
			assert(BaseTracker::alive == 5);
		}
		assert(BaseTracker::alive == 0);
	});
}

inline void RunAllTests() {
	g_passedTestsCount = 0;

	std::cout << "\n" << CYAN_COLOR << "========================================" << RESET_COLOR << "\n";
	std::cout << BOLD_COLOR << "        ЗАПУСК МОДУЛЬНЫХ ТЕСТОВ         " << RESET_COLOR << "\n";
	std::cout << CYAN_COLOR << "========================================" << RESET_COLOR << "\n\n";

	TestUnqPtrDetailed();
	TestUnqPtrArrayDetailed();
	TestShrdPtrDetailed();
	TestShrdPtrArrayDetailed();
	TestPolymorphismAndAdvanced();
	TestDynamicArrayDetailed();

	std::cout << "\n" << CYAN_COLOR << "----------------------------------------" << RESET_COLOR << "\n";
	std::cout << GREEN_COLOR << BOLD_COLOR << "Все " << g_passedTestsCount << " наборов тестов успешно пройдены!" << RESET_COLOR << "\n";
	std::cout << CYAN_COLOR << "========================================" << RESET_COLOR << "\n\n";
}

#endif