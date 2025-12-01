#include "PoolAllocator.hpp"
#include "SimpleAllocator.hpp"
#include <iostream>
#include <memory>
#include <set>

size_t get_rss_kb() {
  long rss = 0;
  FILE* f = fopen("/proc/self/statm", "r");
  if (!f)
    return 0;
  if (fscanf(f, "%*s %ld", &rss) != 1) {
    fclose(f);
    return 0;
  }
  fclose(f);
  long page = sysconf(_SC_PAGESIZE);
  return (size_t)rss * (size_t)page / 1024;
}

template <template <class> class A>
void test_set() {
  size_t before = get_rss_kb();

  using SetInt = std::set<int, std::less<int>, A<int>>;
  SetInt s;

  s.insert(5);
  s.insert(1);
  s.insert(3);
  s.insert(3); // duplicate, ignored
  s.insert(9);

  std::cout << "set contents:";
  for (auto x : s)
    std::cout << " " << x;
  std::cout << "\n";

  auto it = s.find(3);
  std::cout << (it != s.end() ? "found 3\n" : "did not find 3\n");

  it = s.find(42);
  std::cout << (it != s.end() ? "found 42?!\n" : "42 not found\n");

  SetInt s2 = s;
  std::cout << "copy s2:";
  for (auto x : s2)
    std::cout << " " << x;
  std::cout << "\n";

  SetInt s3 = std::move(s2);
  std::cout << "moved s3:";
  for (auto x : s3)
    std::cout << " " << x;
  std::cout << "\n";

  s3.erase(3);
  std::cout << "after erase(3):";
  for (auto x : s3)
    std::cout << " " << x;
  std::cout << "\n";

  for (int i = 0; i < 10000; i++)
    s2.insert(i);

  size_t after = get_rss_kb();

  std::cout << "RSS before: " << before << " KB\n";
  std::cout << "RSS after : " << after << " KB\n";
  std::cout << "Increase  : " << (after - before) << " KB\n";

  std::cout << "set tests finished.\n\n";
}

int main() {
  std::cout << "=== std::allocator ===\n";
  test_set<std::allocator>();

  std::cout << "=== SimpleAllocator ===\n";
  test_set<SimpleAllocator>();
}
