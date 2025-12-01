#include "SimpleAllocator.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

template <template <class> class A>
void test_vector() {
  using VectorInt = std::vector<int, A<int>>;
  using String = std::basic_string<char, std::char_traits<char>, A<char>>;

  VectorInt v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  std::cout << "v =";
  for (auto x : v)
    std::cout << " " << x;
  std::cout << "\n";

  v.reserve(10);
  v.resize(6, 42);

  std::cout << "after resize:";
  for (auto x : v)
    std::cout << " " << x;
  std::cout << "\n";

  VectorInt v2 = v;
  std::cout << "copy v2:";
  for (auto x : v2)
    std::cout << " " << x;
  std::cout << "\n";

  VectorInt v3 = std::move(v2);
  std::cout << "moved v3:";
  for (auto x : v3)
    std::cout << " " << x;
  std::cout << "\n";

  using pair = std::pair<int, String>;
  std::vector<pair, A<pair>> o;
  o.emplace_back(10, "hello");
  o.emplace_back(20, "world");

  for (const auto& [i, s] : o)
    std::cout << "{ " << i << ", " << s << " }\n";

  int sum = 0;
  for (auto it = v3.begin(); it != v3.end(); ++it)
    sum += *it;
  std::cout << "sum(v3) = " << sum << "\n";

  std::cout << "tests finished.\n";
}

int main() {
  test_vector<std::allocator>();
  test_vector<SimpleAllocator>();
}
