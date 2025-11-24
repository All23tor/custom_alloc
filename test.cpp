#include "SimpleAllocator.hpp"
#include <iostream>
#include <string>
#include <vector>

template <class T>
using test_vector = std::vector<T, SimpleAllocator<T>>;

int main() {
  test_vector<int> v;
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

  test_vector<int> v2 = v;
  std::cout << "copy v2:";
  for (auto x : v2)
    std::cout << " " << x;
  std::cout << "\n";

  test_vector<int> v3 = std::move(v2);
  std::cout << "moved v3:";
  for (auto x : v3)
    std::cout << " " << x;
  std::cout << "\n";

  test_vector<std::pair<int, std::string>> o;
  o.emplace_back(10, "hello");
  o.emplace_back(20, "world");

  for (const auto& [i, s] : o)
    std::cout << "{ " << i << ", " << s << " }\n";

  int sum = 0;
  for (auto it = v3.begin(); it != v3.end(); ++it)
    sum += *it;
  std::cout << "sum(v3) = " << sum << "\n";

  try {
    test_vector<int> fail;
    fail.reserve(static_cast<std::size_t>(-1) / 2);
  } catch (const std::exception& e) {

    std::cout << "caught expected exception: " << e.what() << "\n";
  }

  std::cout << "tests finished.\n";
  return 0;
}
