#ifndef UNQPTRARRAY_HPP
#define UNQPTRARRAY_HPP

#include <cstddef>
#include <type_traits>

#include "UnqPtr.hpp"


template <typename T>
class UnqPtr<T[]> {
	private:
		T* ptr_ = nullptr;

	public:
		constexpr UnqPtr() noexcept = default;
		explicit UnqPtr(T* ptr) noexcept : ptr_(ptr) {}

		~UnqPtr() noexcept {
			delete[] ptr_;
		}

		UnqPtr(const UnqPtr&) = delete;
		UnqPtr& operator=(const UnqPtr&) = delete;

		UnqPtr(UnqPtr&& other) noexcept : ptr_(other.ptr_) {
			other.ptr_ = nullptr;
		}

		UnqPtr& operator=(UnqPtr&& other) noexcept {
			if (this != &other) {
				delete[] ptr_;
				ptr_ = other.ptr_;
				other.ptr_ = nullptr;
			}
			return *this;
		}

		T& operator[](size_t index) const {
			return ptr_[index];
		}

		T* get() const noexcept { return ptr_; }
		explicit operator bool() const noexcept { return ptr_ != nullptr; }

		T* release() noexcept {
			T* tmp = ptr_;
			ptr_ = nullptr;
			return tmp;
		}

		void reset(T* ptr = nullptr) noexcept {
			if (ptr_ != ptr) {
				delete[] ptr_;
				ptr_ = ptr;
			}
		}
};


template <typename T>
	requires std::is_unbounded_array_v<T>
UnqPtr<T> MakeUnq(size_t size) {
	using ElementType = std::remove_extent_t<T>;
	return UnqPtr<T>(new ElementType[size]());
}


#endif