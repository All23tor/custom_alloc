#ifndef POOL_ALLOCATOR_HPP
#define POOL_ALLOCATOR_HPP

#include <climits>
#include <cstddef>
#include <cstdint>

template <typename T, size_t BlockSize = 4096>
struct PoolAllocator {
  using value_type = T;
  template <typename U>
  struct rebind {
    typedef PoolAllocator<U> other;
  };

  PoolAllocator() {}
  template <class U>
  PoolAllocator(const PoolAllocator<U>& memoryPool) noexcept :
    PoolAllocator() {}
  ~PoolAllocator() noexcept {
    Slot* curr = currentBlock;
    while (curr != nullptr) {
      Slot* prev = curr->next;
      operator delete(reinterpret_cast<void*>(curr));
      curr = prev;
    }
  }

  T* allocate(std::size_t n = 1) {
    if (freeSlots != nullptr) {
      T* result = reinterpret_cast<T*>(freeSlots);
      freeSlots = freeSlots->next;
      return result;
    } else {
      if (currentSlot >= lastSlot) {
        auto newBlock = reinterpret_cast<Slot*>(operator new(BlockSize));
        newBlock->next = currentBlock;
        currentBlock = newBlock;

        char* body = newBlock + sizeof(Slot*);
        std::size_t bodyPadding = padPointer(body, alignof(Slot));
        currentSlot = reinterpret_cast<Slot*>(body + bodyPadding);
        lastSlot =
          reinterpret_cast<Slot*>(newBlock + BlockSize - sizeof(Slot) + 1);
      }
      return reinterpret_cast<T*>(currentSlot++);
    }
  }

  void deallocate(T* p, std::size_t) {
    if (p != nullptr) {
      reinterpret_cast<Slot*>(p)->next = freeSlots;
      freeSlots = reinterpret_cast<Slot*>(p);
    }
  }

  std::size_t max_size() const noexcept {
    std::size_t maxBlocks = -1 / BlockSize;
    return (BlockSize - sizeof(char*)) / sizeof(Slot) * maxBlocks;
  }

private:
  union Slot {
    T element;
    Slot* next;
  };

  Slot* currentBlock{};
  Slot* currentSlot{};
  Slot* lastSlot{};
  Slot* freeSlots{};

  std::size_t padPointer(char* p, std::size_t align) const noexcept {
    std::uintptr_t result = reinterpret_cast<std::uintptr_t>(p);
    return ((align - result) % align);
  }

  static_assert(BlockSize >= 2 * sizeof(Slot), "BlockSize too small.");
};

#endif
