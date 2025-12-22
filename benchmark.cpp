#include "LinearAllocator.hpp"
#include "PoolAllocator.hpp"
#include "SegregatedAllocator.hpp"
#include "SimpleAllocator.hpp"
#include <chrono>
#include <cstdint>
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
  FILE* f = fopen("/proc/self/smaps", "r");
  if (!f)
    return 0;

  char line[256];
  size_t rss_kb = 0;

  while (fgets(line, sizeof(line), f)) {
    if (std::strncmp(line, "Rss:", 4) == 0) {
      char* end;
      size_t value = std::strtoul(line + 4, &end, 10);
      rss_kb += value;
    }
  }

  fclose(f);
  return rss_kb;
}

template <template <class> class AllocatorType, typename T>
void test_list_size_type_csv(std::string_view alloc_name, std::size_t n) {
  size_t ram_base = obtener_uso_memoria_kb();

  std::list<T, AllocatorType<T>> lista;

  auto start_alloc = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < n; ++i)
    lista.emplace_back(static_cast<T>(i)) = 0;
  auto end_alloc = std::chrono::high_resolution_clock::now();

  double alloc_ms =
    std::chrono::duration<double, std::milli>(end_alloc - start_alloc).count();

  size_t ram_peak = obtener_uso_memoria_kb();

  auto start_free = std::chrono::high_resolution_clock::now();
  lista.clear();
  auto end_free = std::chrono::high_resolution_clock::now();

  double free_ms =
    std::chrono::duration<double, std::milli>(end_free - start_free).count();

  size_t ram_used = (ram_peak > ram_base) ? (ram_peak - ram_base) : 0;

  std::cout << alloc_name << ',' << type_name<T>() << ',' << n << ','
            << alloc_ms << ',' << free_ms << ',' << ram_used << '\n';
}

template <template <class> class AllocatorType>
void test_list_size_mixed_csv(std::string_view alloc_name, std::size_t n) {
  size_t ram_base = obtener_uso_memoria_kb();

  std::list<int32_t, AllocatorType<int32_t>> lista32;
  std::list<int64_t, AllocatorType<int64_t>> lista64;
  std::list<__int128_t, AllocatorType<__int128_t>> lista128;

  auto start_alloc = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < n; ++i) {
    lista32.emplace_back(i) = 0;
    lista64.emplace_back(i) = 0;
    lista128.emplace_back(i) = 0;
  }
  auto end_alloc = std::chrono::high_resolution_clock::now();

  double alloc_ms =
    std::chrono::duration<double, std::milli>(end_alloc - start_alloc).count();

  size_t ram_peak = obtener_uso_memoria_kb();

  auto start_free = std::chrono::high_resolution_clock::now();
  lista32.clear();
  lista64.clear();
  lista128.clear();
  auto end_free = std::chrono::high_resolution_clock::now();

  double free_ms =
    std::chrono::duration<double, std::milli>(end_free - start_free).count();

  size_t ram_used = (ram_peak > ram_base) ? (ram_peak - ram_base) : 0;

  std::cout << alloc_name << ',' << "mixed" << ',' << n << ',' << alloc_ms
            << ',' << free_ms << ',' << ram_used << '\n';
}

template <template <class> class AllocatorType>
void run_type_test(
  std::string_view alloc_name, std::string_view type, std::size_t n
) {
  if (type == "i8")
    test_list_size_type_csv<AllocatorType, int8_t>(alloc_name, n);
  else if (type == "i16")
    test_list_size_type_csv<AllocatorType, int16_t>(alloc_name, n);
  else if (type == "i32")
    test_list_size_type_csv<AllocatorType, int32_t>(alloc_name, n);
  else if (type == "i64")
    test_list_size_type_csv<AllocatorType, int64_t>(alloc_name, n);
  else if (type == "i128")
    test_list_size_type_csv<AllocatorType, __int128>(alloc_name, n);
  else if (type == "mixed")
    test_list_size_mixed_csv<AllocatorType>(alloc_name, n);
  else {
    std::cerr << "Unknown type: " << type << '\n';
    std::exit(1);
  }
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage:\n"
              << "  " << argv[0] << " <Allocator> <Type> <N>\n\n"
              << "Allocators: Standard | Simple | Pool | Linear | Segregated\n"
              << "Types: i8 | i16 | i32 | i64 | i128 | mixed\n";
    return 1;
  }

  std::string_view allocator = argv[1];
  std::string_view type = argv[2];
  std::size_t n = std::strtoull(argv[3], nullptr, 10);

  std::cout << "allocator,type,n,alloc_ms,free_ms,ram_kb\n";

  if (allocator == "Standard")
    run_type_test<std::allocator>("Standard", type, n);
  else if (allocator == "Simple")
    run_type_test<SimpleAllocator>("Simple", type, n);
  else if (allocator == "Pool")
    run_type_test<PoolAllocator>("Pool", type, n);
  else if (allocator == "Linear")
    run_type_test<LinearAllocator>("Linear", type, n);
  else if (allocator == "Segregated")
    run_type_test<SegregatedAllocator>("Segregated", type, n);
  else {
    std::cerr << "Unknown allocator: " << allocator << '\n';
    return 1;
  }

  return 0;
}
