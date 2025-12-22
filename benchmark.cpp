#include "LinearAllocator.hpp"
#include "PoolAllocator.hpp"
#include "SegregatedAllocator.hpp"
#include "SimpleAllocator.hpp"
#include <chrono>
#include <iostream>
#include <list>

template <typename T>
constexpr std::string_view type_name();
template <>
constexpr std::string_view type_name<int8_t>() {
  return "i8";
}
template <>
constexpr std::string_view type_name<int16_t>() {
  return "i16";
}
template <>
constexpr std::string_view type_name<int32_t>() {
  return "i32";
}
template <>
constexpr std::string_view type_name<int64_t>() {
  return "i64";
}
template <>
constexpr std::string_view type_name<__int128>() {
  return "i128";
}

size_t obtener_uso_memoria_kb() {
  long rss = 0;
  FILE* f = fopen("/proc/self/statm", "r");
  if (!f)
    return 0;
  if (fscanf(f, "%*s %ld", &rss) != 1) {
    fclose(f);
    return 0;
  }
  fclose(f);
  return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE) / 1024;
}

template <template <class> class AllocatorType, typename T>
void test_list_size_type_csv(std::string_view alloc_name, std::size_t n) {
  size_t ram_base = obtener_uso_memoria_kb();

  double alloc_ms = 0.0;
  double free_ms = 0.0;

  {
    std::list<T, AllocatorType<T>> lista;

    auto start_alloc = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < n; ++i)
      lista.push_back(static_cast<T>(i));
    auto end_alloc = std::chrono::high_resolution_clock::now();

    alloc_ms =
      std::chrono::duration<double, std::milli>(end_alloc - start_alloc)
        .count();

    size_t ram_peak = obtener_uso_memoria_kb();

    auto start_free = std::chrono::high_resolution_clock::now();
    lista.clear();
    auto end_free = std::chrono::high_resolution_clock::now();

    free_ms =
      std::chrono::duration<double, std::milli>(end_free - start_free).count();

    size_t ram_used = (ram_peak > ram_base) ? (ram_peak - ram_base) : 0;

    std::cout << alloc_name << ',' << type_name<T>() << ',' << n << ','
              << alloc_ms << ',' << free_ms << ',' << ram_used << '\n';
  }
}

template <template <class> class AllocatorType>
void sweep_allocator_csv(std::string_view name) {
  constexpr std::size_t sizes[] = {1'000, 5'000, 10'000, 50'000, 100'000};

  for (std::size_t n : sizes) {
    test_list_size_type_csv<AllocatorType, int8_t>(name, n);
    test_list_size_type_csv<AllocatorType, int16_t>(name, n);
    test_list_size_type_csv<AllocatorType, int32_t>(name, n);
    test_list_size_type_csv<AllocatorType, int64_t>(name, n);
    test_list_size_type_csv<AllocatorType, __int128>(name, n);
  }
}

int main() {
  LinearArena::init(1024 * 1024 * 100);
  std::cout << "allocator,type,n,alloc_ms,free_ms,ram_kb\n";

  sweep_allocator_csv<std::allocator>("Standard");
  sweep_allocator_csv<SimpleAllocator>("Simple");
  sweep_allocator_csv<PoolAllocator>("Pool");
  sweep_allocator_csv<LinearAllocator>("Linear");
  sweep_allocator_csv<SegregatedAllocator>("Segregated");
}
