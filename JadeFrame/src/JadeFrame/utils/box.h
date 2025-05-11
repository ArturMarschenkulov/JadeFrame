#pragma once
#include <utility> // For std::forward
#include <memory>  // For std::unique_ptr (optional, if you want to leverage it)

namespace JadeFrame {
template<typename T>
class Box {
public:
    // Default constructor
    Box() noexcept = default;

    // Constructor from raw pointer
    explicit Box(T* ptr) noexcept
        : ptr_(ptr) {}

    // Move constructor
    Box(Box&& other) noexcept
        : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    // Move assignment operator
    auto operator=(Box&& other) noexcept -> Box& {
        if (this != &other) {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Delete copy constructor and copy assignment
    Box(const Box&) = delete;
    auto operator=(const Box&) -> Box& = delete;

    // Destructor
    ~Box() { delete ptr_; }

    // Dereference operators
    auto operator*() const -> T& { return *ptr_; }

    auto operator->() const -> T* { return ptr_; }

    // Get the raw pointer
    auto get() const noexcept -> T* { return ptr_; }

    // Release ownership of the managed object
    auto release() noexcept -> T* {
        T* temp = ptr_;
        ptr_ = nullptr;
        return temp;
    }

    // Replace the managed object
    void reset(T* ptr = nullptr) noexcept {
        if (ptr_ != ptr) {
            delete ptr_;
            ptr_ = ptr;
        }
    }

    // Swap with another Box
    void swap(Box& other) noexcept { std::swap(ptr_, other.ptr_); }

    // Factory method
    template<typename F, typename... Args>
    static auto make(F factory, Args&&... args) -> Box {
        // Call the factory function to create the object by value
        T obj = factory(std::forward<Args>(args)...);
        // Allocate a new object on the heap using the created object
        return Box(new T(std::move(obj)));
    }

private:
    T* ptr_ = nullptr;
};
} // namespace JadeFrame