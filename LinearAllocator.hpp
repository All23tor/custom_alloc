#ifndef LINEAR_ALLOCATOR_HPP
#define LINEAR_ALLOCATOR_HPP

#include <new>

// --- GESTOR DE MEMORIA COMPARTIDO ---
// Sacamos esto fuera del template para que sea común a todos los tipos de
// datos. (int, nodos de lista, vectores, etc. compartirán esta única arena).
struct LinearArena {
  static inline char* start = nullptr;
  static inline char* end = nullptr;
  static inline char* current = nullptr;

  static void init(std::size_t size_bytes) {
    delete[] LinearArena::start;
    LinearArena::start = new char[size_bytes];
    LinearArena::end = LinearArena::start + size_bytes;
    LinearArena::current = LinearArena::start;
  }
  static void reset() {
    LinearArena::current = LinearArena::start;
  }
};

template <typename T>
struct LinearAllocator {
  using value_type = T;

  LinearAllocator() noexcept {}
  template <typename U>
  LinearAllocator(const LinearAllocator<U>&) noexcept {}
  template <typename U>
  struct rebind {
    using other = LinearAllocator<U>;
  };

  T* allocate(std::size_t n) {
    std::size_t bytes_needed = n * sizeof(T);

    if (LinearArena::end - LinearArena::current < bytes_needed)
      throw std::bad_alloc();

    auto user_ptr = reinterpret_cast<T*>(LinearArena::current);
    LinearArena::current += bytes_needed;

    return user_ptr;
  }

  void deallocate(T*, std::size_t) noexcept {
    // No hace nada. Se libera todo con LinearArena::reset().
  }
};

#endif
