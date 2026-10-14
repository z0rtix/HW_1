#ifndef UNQ_PTR_HPP
#define UNQ_PTR_HPP

#include <cstddef>
#include <utility>
#include <stdexcept>
#include <type_traits>


template <typename T>
class ShrdPtr;

template <typename T>
class UnqPtr {
    private:
        T* ptr_ = nullptr;

        template <typename U>
        friend class UnqPtr;

        template <typename U>
        friend class ShrdPtr;

    public:
        constexpr UnqPtr() noexcept : ptr_(nullptr) {}

        explicit UnqPtr(T* ptr) noexcept : ptr_(ptr) {}

        ~UnqPtr() noexcept {
            delete ptr_;
        }

        UnqPtr(const UnqPtr&) = delete;
        UnqPtr& operator=(const UnqPtr&) = delete;

        UnqPtr(UnqPtr&& other) noexcept : ptr_(other.ptr_) {
            other.ptr_ = nullptr;
        }

        UnqPtr& operator=(UnqPtr&& other) noexcept {
            if (this != &other) {
                delete ptr_;
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        template <typename U> requires (std::is_convertible_v<U*, T*> && !std::is_same_v<T, U>)
        UnqPtr(UnqPtr<U>&& other) noexcept : ptr_(other.ptr_) {
            other.ptr_ = nullptr;
        }

        template <typename U> requires (std::is_convertible_v<U*, T*> && !std::is_same_v<T, U>)
        UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            return *this;
        }

        T& operator*() const {
            if (!ptr_) {
                throw std::runtime_error("Dereferencing null UnqPtr");
            }
            return *ptr_;
        }

        T* operator->() const {
            if (!ptr_) {
                throw std::runtime_error("Accessing null UnqPtr");
            }
            return ptr_;
        }

        T* get() const noexcept {
            return ptr_;
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }

        T* release() noexcept {
            T* temp = ptr_;
            ptr_ = nullptr;
            return temp;
        }

        void reset(T* new_ptr = nullptr) noexcept {
            if (ptr_ != new_ptr) {
                delete ptr_;
                ptr_ = new_ptr;
            }
        }
};


template <typename T, typename... Args>
UnqPtr<T> MakeUnq(Args&&... args) {
    return UnqPtr<T>(new T(std::forward<Args>(args)...));
}


#endif