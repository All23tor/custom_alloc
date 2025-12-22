#ifndef LINEAR_ALLOCATOR_HPP
#define LINEAR_ALLOCATOR_HPP

#include <new>

// --- GESTOR DE MEMORIA COMPARTIDO ---
// Sacamos esto fuera del template para que sea común a todos los tipos de
// datos. (int, nodos de lista, vectores, etc. compartirán esta única arena).
namespace LinearArena {
inline char* start = nullptr;
inline char* end = nullptr;
inline char* current = nullptr;

static void init(std::size_t size_bytes) {
  delete[] start;
  start = new char[size_bytes];
  end = start + size_bytes;
  current = start;
}
static void reset() {
  current = start;
}
}; // namespace LinearArena

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
    if (LinearArena::start == nullptr)
      LinearArena::init(1024 * 1024 * 100);

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
