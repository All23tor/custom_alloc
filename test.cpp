#include "PoolAllocator.hpp"
#include "SimpleAllocator.hpp"
#include <iostream>
#include <memory>
#include <set>

template <template <class> class A>
void test_set() {
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

  // Check find
  auto it = s.find(3);
  if (it != s.end())
    std::cout << "found 3\n";
  else
    std::cout << "did not find 3\n";

  it = s.find(42);
  if (it != s.end())
    std::cout << "found 42?!\n";
  else
    std::cout << "42 not found\n";

  // Copy test
  SetInt s2 = s;
  std::cout << "copy s2:";
  for (auto x : s2)
    std::cout << " " << x;
  std::cout << "\n";

  // Move test
  SetInt s3 = std::move(s2);
  std::cout << "moved s3:";
  for (auto x : s3)
    std::cout << " " << x;
  std::cout << "\n";

  // Erase test
  s3.erase(3);
  std::cout << "after erase(3):";
  for (auto x : s3)
    std::cout << " " << x;
  std::cout << "\n";

  std::cout << "set tests finished.\n";
}

int main() {
  test_set<std::allocator>();
  test_set<SimpleAllocator>();
}
