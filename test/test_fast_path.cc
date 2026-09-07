#include <iostream>
#include <mini_fmt/mini_fmt.h>

int main() {
  // Test integerx_formatter fast path
  mini_fmt::integerx_formatter<int> fmt;
  mini_fmt::parse_context ctx("");
  fmt.parse(ctx);

  std::cout << "After parse empty spec:" << std::endl;
  std::cout << "  specs.type = " << static_cast<int>(fmt.specs.type) << std::endl;
  std::cout << "  specs.width = " << fmt.specs.width << std::endl;
  std::cout << "  presentation::default_type = " << static_cast<int>(mini_fmt::presentation::default_type) << std::endl;

  // Check fast path condition
  bool fast_path = (fmt.specs.type == mini_fmt::presentation::default_type && fmt.specs.width == 0);
  std::cout << "  Should take fast path: " << (fast_path ? "YES" : "NO") << std::endl;

  mini_fmt::string_buffer buf;
  mini_fmt::buffer_appender out(buf);
  fmt.format(42, out);

  std::cout << "Result: " << std::string(buf.data(), buf.size()) << std::endl;

  return 0;
}
