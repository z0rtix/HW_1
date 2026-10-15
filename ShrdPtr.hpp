#ifndef SHRD_PTR_HPP
#define SHRD_PTR_HPP

#include <cstddef>
#include <utility>
#include <stdexcept>
#include <type_traits>

#include "UnqPtr.hpp"


template <typename T>
class ShrdPtr {
    private:
        T* ptr_ = nullptr;
        size_t* ref_count_ = nullptr;

        template <typename U>
        friend class ShrdPtr;

        void release_ref() noexcept {
            if (ref_count_) {
                --(*ref_count_);
                if (*ref_count_ == 0) {
                    delete ptr_;
                    delete ref_count_;
                }
            }
            ptr_ = nullptr;
            ref_count_ = nullptr;
        }

    public:
        constexpr ShrdPtr() noexcept : ptr_(nullptr), ref_count_(nullptr) {}

        explicit ShrdPtr(T* ptr) {
            if (ptr) {
                ptr_ = ptr;
                ref_count_ = new size_t(1);
            }
        }

        template <typename U> requires (std::is_convertible_v<U*, T*>)
        explicit ShrdPtr(UnqPtr<U>&& unique_ptr) {
            if (unique_ptr.get()) {
                ptr_ = unique_ptr.release();
                ref_count_ = new size_t(1);
            }
        }

        ~ShrdPtr() noexcept {
            release_ref();
        }

        ShrdPtr(const ShrdPtr& other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_) {
            if (ref_count_) {
                ++(*ref_count_);
            }
        }

        template <typename U> requires (std::is_convertible_v<U*, T*> && !std::is_same_v<T, U>)
        ShrdPtr(const ShrdPtr<U>& other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_) {
            if (ref_count_) {
                ++(*ref_count_);
            }
        }

        ShrdPtr& operator=(const ShrdPtr& other) noexcept {
            if (this != &other) {
                release_ref();
                ptr_ = other.ptr_;
                ref_count_ = other.ref_count_;
                if (ref_count_) {
                    ++(*ref_count_);
                }
            }
            return *this;
        }

        template <typename U> requires (std::is_convertible_v<U*, T*> && !std::is_same_v<T, U>)
        ShrdPtr& operator=(const ShrdPtr<U>& other) noexcept {
            release_ref();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            if (ref_count_) {
                ++(*ref_count_);
            }
            return *this;
        }

        ShrdPtr(ShrdPtr&& other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_) {
            other.ptr_ = nullptr;
            other.ref_count_ = nullptr;
        }

        template <typename U> requires (std::is_convertible_v<U*, T*> && !std::is_same_v<T, U>)
        ShrdPtr(ShrdPtr<U>&& other) noexcept : ptr_(other.ptr_), ref_count_(other.ref_count_) {
            other.ptr_ = nullptr;
            other.ref_count_ = nullptr;
        }

        ShrdPtr& operator=(ShrdPtr&& other) noexcept {
            if (this != &other) {
                release_ref();
                ptr_ = other.ptr_;
                ref_count_ = other.ref_count_;
                other.ptr_ = nullptr;
                other.ref_count_ = nullptr;
            }
            return *this;
        }

        template <typename U> requires (std::is_convertible_v<U*, T*> && !std::is_same_v<T, U>)
        ShrdPtr& operator=(ShrdPtr<U>&& other) noexcept {
            release_ref();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            other.ptr_ = nullptr;
            other.ref_count_ = nullptr;
            return *this;
        }

        T& operator*() const {
            if (!ptr_) {
                throw std::runtime_error("Dereferencing null ShrdPtr");
            }
            return *ptr_;
        }

        T* operator->() const {
            if (!ptr_) {
                throw std::runtime_error("Accessing null ShrdPtr");
            }
            return ptr_;
        }

        T* get() const noexcept {
            return ptr_;
        }

        size_t use_count() const noexcept {
            return ref_count_ ? *ref_count_ : 0;
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }

        void reset(T* new_ptr = nullptr) {
            release_ref();
            if (new_ptr) {
                ptr_ = new_ptr;
                ref_count_ = new size_t(1);
            }
        }
};


template <typename T, typename... Args>
	requires (!std::is_array_v<T>)
ShrdPtr<T> MakeShrd(Args&&... args) {
	return ShrdPtr<T>(new T(std::forward<Args>(args)...));
}


#endif