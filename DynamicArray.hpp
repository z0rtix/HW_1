#ifndef DYNAMIC_ARRAY_HPP
#define DYNAMIC_ARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include "ShrdPtr.hpp"


template <typename T>
class DynamicArray {
    private:
        ShrdPtr<T>* buffer_ = nullptr;
        size_t size_ = 0;
        size_t capacity_ = 0;

        void Reserve(size_t new_capacity) {
            if (new_capacity <= capacity_) {
                return;
            }

            ShrdPtr<T>* new_buffer = new ShrdPtr<T>[new_capacity];
            for (size_t i = 0; i < size_; ++i) {
                new_buffer[i] = std::move(buffer_[i]);
            }

            delete[] buffer_;
            buffer_ = new_buffer;
            capacity_ = new_capacity;
        }

    public:
        DynamicArray() : buffer_(nullptr), size_(0), capacity_(0) {}

        explicit DynamicArray(size_t initial_capacity) 
            : buffer_(nullptr), size_(0), capacity_(0) {
            if (initial_capacity > 0) {
                Reserve(initial_capacity);
            }
        }

        ~DynamicArray() {
            delete[] buffer_;
        }

        DynamicArray(const DynamicArray&) = delete;
        DynamicArray& operator=(const DynamicArray&) = delete;

        DynamicArray(DynamicArray&& other) noexcept : buffer_(other.buffer_), size_(other.size_), capacity_(other.capacity_) {
            other.buffer_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        DynamicArray& operator=(DynamicArray&& other) noexcept {
            if (this != &other) {
                delete[] buffer_;
                buffer_ = other.buffer_;
                size_ = other.size_;
                capacity_ = other.capacity_;

                other.buffer_ = nullptr;
                other.size_ = 0;
                other.capacity_ = 0;
            }
            return *this;
        }

        void Append(const T& value) {
            if (size_ == capacity_) {
                size_t next_capacity = (capacity_ == 0) ? 2 : capacity_ * 2;
                Reserve(next_capacity);
            }
            buffer_[size_] = MakeShrd<T>(value);
            ++size_;
        }

        T& Get(size_t index) const {
            if (index >= size_) {
                throw std::out_of_range("Index out of range in DynamicArray");
            }
            return *buffer_[index];
        }

        T& operator[](size_t index) const {
            return Get(index);
        }

        size_t GetSize() const noexcept {
            return size_;
        }

        size_t GetCapacity() const noexcept {
            return capacity_;
        }
};


#endif