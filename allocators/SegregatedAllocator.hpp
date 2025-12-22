#ifndef SEGREGATED_ALLOCATOR_HPP
#define SEGREGATED_ALLOCATOR_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstring>
#include <memory>
#include <new>
#include <utility>
#include <vector>

namespace sfl {
constexpr int SIZE_START = 4;
constexpr int SIZE_CLASSES = 9;
constexpr std::size_t class_index(std::size_t size) {
  if (size == 0)
    return 0;
  int width = std::bit_width(size - 1);
  return std::clamp(width - SIZE_START, 0, SIZE_CLASSES);
}
} // namespace sfl

struct Slab {
  static constexpr std::size_t SLAB_SIZE = 64 * 1024;

  struct FreeNode {
    FreeNode* next;
  };

  std::unique_ptr<char[]> memory;
  FreeNode* free_list = nullptr;

  Slab(std::size_t block_size) {
    memory = std::make_unique<char[]>(SLAB_SIZE);
    std::memset(memory.get(), 0, SLAB_SIZE);
    free_list = nullptr;

    for (std::size_t block = 0; block < SLAB_SIZE; block += block_size) {
      auto* node = reinterpret_cast<FreeNode*>(memory.get() + block);
      node->next = free_list;
      free_list = node;
    }
  }

  bool has_free() const {
    return free_list != nullptr;
  }

  void* allocate() {
    FreeNode* node = free_list;
    free_list = node->next;
    return node;
  }

  void deallocate(void* ptr) {
    auto* node = static_cast<FreeNode*>(ptr);
    node->next = free_list;
    free_list = node;
  }

  bool owns(void* ptr) const {
    auto p = reinterpret_cast<char*>(ptr);
    return p >= memory.get() && p < memory.get() + SLAB_SIZE;
  }
};

namespace SegregatedLists {
inline std::array<std::vector<Slab>, sfl::SIZE_CLASSES> slabs;
inline void reset() {
  std::ranges::for_each(slabs, &std::vector<Slab>::clear);
}
}; // namespace SegregatedLists

template <typename T>
struct SegregatedAllocator {
  using value_type = T;

  SegregatedAllocator() noexcept {}
  template <typename U>
  SegregatedAllocator(const SegregatedAllocator<U>&) noexcept {}
  template <typename U>
  struct rebind {
    using other = SegregatedAllocator<U>;
  };

  T* allocate(std::size_t n) {
    std::size_t bytes = n * sizeof(T);
    std::size_t idx = sfl::class_index(bytes);

    if (idx >= sfl::SIZE_CLASSES)
      return reinterpret_cast<T*>(new char[bytes]);

    auto& list = SegregatedLists::slabs[idx];
    auto free_slab = std::ranges::find_if(list, &Slab::has_free);

    auto& slab = free_slab != list.end() ?
      *free_slab :
      list.emplace_back(1uz << (idx + sfl::SIZE_START));

    return reinterpret_cast<T*>(slab.allocate());
  }

  void deallocate(T* ptr, std::size_t n) noexcept {
    if (!ptr)
      return;

    std::size_t bytes = n * sizeof(T);
    std::size_t idx = sfl::class_index(bytes);

    if (idx >= sfl::SIZE_CLASSES)
      return delete[] reinterpret_cast<char*>(ptr);

    auto& list = SegregatedLists::slabs[idx];
    auto owner =
      std::ranges::find_if(list, [ptr](Slab& slab) { return slab.owns(ptr); });
    if (owner != list.end())
      return owner->deallocate(ptr);
    std::unreachable();
  }
};

#endif
