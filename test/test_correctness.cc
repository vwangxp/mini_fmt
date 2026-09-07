#include <iostream>
#include <mini_fmt/mini_fmt.h>
#include <fmt/core.h>

int main() {
  // Test various cases
  std::cout << "=== mini_fmt tests ===" << std::endl;
  std::cout << mini_fmt::format("Simple: {}", 42) << std::endl;
  std::cout << mini_fmt::format("Negative: {}", -123) << std::endl;
  std::cout << mini_fmt::format("Zero: {}", 0) << std::endl;
  std::cout << mini_fmt::format("Large: {}", 999999) << std::endl;
  std::cout << mini_fmt::format("Prefix {} suffix", 777) << std::endl;

  std::cout << "\n=== fmt tests ===" << std::endl;
  std::cout << fmt::format("Simple: {}", 42) << std::endl;
  std::cout << fmt::format("Negative: {}", -123) << std::endl;
  std::cout << fmt::format("Zero: {}", 0) << std::endl;
  std::cout << fmt::format("Large: {}", 999999) << std::endl;
  std::cout << fmt::format("Prefix {} suffix", 777) << std::endl;

  // Test with runtime value to prevent constant folding
  int runtime_val = 0;
  std::cin >> runtime_val;
  if (runtime_val == 0) runtime_val = 42;

  std::cout << "\n=== Runtime value test ===" << std::endl;
  std::cout << "mini_fmt: " << mini_fmt::format("Value: {}", runtime_val) << std::endl;
  std::cout << "fmt: " << fmt::format("Value: {}", runtime_val) << std::endl;

  return 0;
}
