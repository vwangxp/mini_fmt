#include <iostream>
#include <mini_fmt/mini_fmt.h>

int main() {
  // Test which formatter path is taken
  mini_fmt::formatter<int> fmt;
  mini_fmt::parse_context ctx("");
  fmt.parse(ctx);

  mini_fmt::string_buffer buf;
  mini_fmt::buffer_appender out(buf);

  fmt.format(42, out);

  std::cout << "Result: " << std::string(buf.data(), buf.size()) << std::endl;
  std::cout << "Specs width: " << fmt.specs.width << std::endl;
  std::cout << "Specs type: " << static_cast<int>(fmt.specs.type) << std::endl;

  return 0;
}
