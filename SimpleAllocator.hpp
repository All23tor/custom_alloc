#ifndef SIMPLE_ALLOCATOR_HPP
#define SIMPLE_ALLOCATOR_HPP

#include "simple_alloc.h"
#include <cstddef>
#include <new>

template <typename T>
struct SimpleAllocator {
  using value_type = T;

  SimpleAllocator() noexcept = default;
  template <typename U>
  SimpleAllocator(const SimpleAllocator<U>&) noexcept {}

  [[nodiscard]] T* allocate(std::size_t n) {
    if (n > static_cast<std::size_t>(-1) / sizeof(T))
      throw std::bad_array_new_length();

    void* p = simple_malloc(n * sizeof(T));
    if (!p)
      throw std::bad_alloc();

    return static_cast<T*>(p);
  }

  void deallocate(T* p, std::size_t) noexcept {
    simple_free(p);
  }

  template <class U>
  struct rebind {
    using other = SimpleAllocator<U>;
  };

  bool operator==(const SimpleAllocator&) const noexcept {
    return true;
  }
  bool operator!=(const SimpleAllocator&) const noexcept {
    return false;
  }
};

#endif
