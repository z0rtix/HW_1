#ifndef SHRDPTRARRAY_HPP
#define SHRDPTRARRAY_HPP

#include <cstddef>
#include <type_traits>

#include "ShrdPtr.hpp"


template <typename T>
class ShrdPtr<T[]> {
	private:
		T* ptr_ = nullptr;
		size_t* ref_count_ = nullptr;

		void Release() noexcept {
			if (ref_count_) {
				--(*ref_count_);
				if (*ref_count_ == 0) {
					delete[] ptr_;
					delete ref_count_;
				}
				ptr_ = nullptr;
				ref_count_ = nullptr;
			}
		}

	public:
		constexpr ShrdPtr() noexcept = default;

		explicit ShrdPtr(T* ptr) : ptr_(ptr) {
			if (ptr_) {
				ref_count_ = new size_t(1);
			}
		}

		~ShrdPtr() noexcept {
			Release();
		}

		ShrdPtr(const ShrdPtr& other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_) {
			if (ref_count_) {
				++(*ref_count_);
			}
		}

		ShrdPtr& operator=(const ShrdPtr& other) noexcept {
			if (this != &other) {
				Release();
				ptr_ = other.ptr_;
				ref_count_ = other.ref_count_;
				if (ref_count_) {
					++(*ref_count_);
				}
			}
			return *this;
		}

		ShrdPtr(ShrdPtr&& other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_) {
			other.ptr_ = nullptr;
			other.ref_count_ = nullptr;
		}

		ShrdPtr& operator=(ShrdPtr&& other) noexcept {
			if (this != &other) {
				Release();
				ptr_ = other.ptr_;
				ref_count_ = other.ref_count_;
				other.ptr_ = nullptr;
				other.ref_count_ = nullptr;
			}
			return *this;
		}

		T& operator[](size_t index) const {
			return ptr_[index];
		}

		T* get() const noexcept { return ptr_; }
		size_t use_count() const noexcept { return ref_count_ ? *ref_count_ : 0; }
		size_t UseCount() const noexcept { return use_count(); }
		explicit operator bool() const noexcept { return ptr_ != nullptr; }

		void reset(T* ptr = nullptr) {
			Release();
			if (ptr) {
				ptr_ = ptr;
				ref_count_ = new size_t(1);
			}
		}

		void Reset(T* ptr = nullptr) {
			reset(ptr);
		}
};


template <typename T>
	requires std::is_unbounded_array_v<T>
ShrdPtr<T> MakeShrd(size_t size) {
	using ElementType = std::remove_extent_t<T>;
	return ShrdPtr<T>(new ElementType[size]());
}


#endif