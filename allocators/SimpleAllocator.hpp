#ifndef SIMPLE_ALLOCATOR_HPP
#define SIMPLE_ALLOCATOR_HPP

#include <cstddef>
#include <cstring>
#include <new>
#include <unistd.h>

inline void* global_base = NULL;

template <typename T>
struct SimpleAllocator {
  using value_type = T;

  SimpleAllocator() noexcept = default;
  template <typename U>
  SimpleAllocator(const SimpleAllocator<U>&) noexcept {}

  struct BlockMeta {
    size_t size;
    BlockMeta* next;
    int free;
  };

  static BlockMeta* find_free_block(BlockMeta** last, size_t size) {
    auto current = reinterpret_cast<BlockMeta*>(global_base);
    while (current && !(current->free && current->size >= size)) {
      *last = current;
      current = current->next;
    }
    return current;
  }

  static BlockMeta* request_space(BlockMeta* last, size_t size) {
    auto block = reinterpret_cast<BlockMeta*>(sbrk(0));
    void* request = sbrk(size + sizeof(BlockMeta));
    if (request == (void*)-1) {
      return NULL;
    }

    if (last) {
      last->next = block;
    }
    block->size = size;
    block->next = NULL;
    block->free = 0;
    return block;
  }

  [[nodiscard]] T* allocate(std::size_t n) {
    auto size = n * sizeof(T);
    BlockMeta* block;

    if (size <= 0) {
      return NULL;
    }

    if (!global_base) {
      block = request_space(NULL, size);
      if (!block) {
        return NULL;
      }
      global_base = block;
    } else {
      auto last = reinterpret_cast<BlockMeta*>(global_base);
      block = find_free_block(&last, size);
      if (!block) {
        block = request_space(last, size);
        if (!block) {
          return NULL;
        }
      } else {
        block->free = 0;
      }
    }

    void* p = (block + 1);

    if (!p)
      throw std::bad_alloc();

    return static_cast<T*>(p);
  }

  void deallocate(T* ptr, std::size_t) noexcept {
    if (!ptr)
      return;

    BlockMeta* block_ptr = (BlockMeta*)ptr - 1;
    block_ptr->free = 1;
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
