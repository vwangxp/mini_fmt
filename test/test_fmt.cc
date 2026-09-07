#include <iostream>

#include "mini_fmt/mini_fmt.h"

struct Point {
  int x, y;
};

// 扩展自定义类型 Point 的格式化支持
namespace mini_fmt {
template <>
struct formatter<Point> : public integerx_formatter<int> {
  // integerx_formatter<int> fmt_;
  // constexpr auto parse(parse_context& ctx) { return fmt_.parse(ctx); }
  using integerx_formatter<int>::format;

  auto format(const Point& p, buffer_appender& out) const {
    *out++ = '(';
    format(p.x, out);
    *out++ = ',';
    format(p.y, out);
    *out++ = ')';
    return out;
  }
};
}  // namespace mini_fmt

struct Student {
  std::string name_;
  int age_;
  bool sex;  // true: 1/真, false: 0/假
};

namespace mini_fmt {

template <>
struct formatter<Student> {
  formatter<std::string_view> name_fmt_;
  integerx_formatter<int> age_fmt_;
  formatter<bool> sex_fmt_;

  constexpr auto parse(parse_context& ctx) {
    auto it = name_fmt_.parse(ctx);

    if (it != ctx.end() && *it == ':') {
      parse_context age_ctx(std::string_view(it + 1, ctx.end()));
      it = age_fmt_.parse(age_ctx);
    }

    if (it != ctx.end() && *it == ':') {
      parse_context sex_ctx(std::string_view(it + 1, ctx.end()));
      it = sex_fmt_.parse(sex_ctx);
    }
    return it;
  }

  auto format(const Student& s, buffer_appender& out) const {
    name_fmt_.format(s.name_, out);

    *out++ = ',';
    *out++ = ' ';

    age_fmt_.format(s.age_, out);

    *out++ = ',';
    *out++ = ' ';

    sex_fmt_.format(s.sex, out);

    return out;
  }
};

}  // namespace mini_fmt

int main() {
  // 1. 基础标量与多进制格式化测试
  mini_fmt::print("Hello {}, hex: {:x}, UPPER: {:X}, bin: {:b}\n", "world", 255,
                  255, 5);

  // 2. 自定义类型扩展测试
  Point pt{10, 255};
  mini_fmt::print("Point coordinates: {}\n", pt);
  mini_fmt::print("Point coordinates-hex: {:x}\n", pt);
  mini_fmt::print("Point coordinates-HEX: {:X}\n", pt);

  // 3. 返回 std::string 测试
  std::string s = mini_fmt::format("Status: [{:^10e}], Value: [{:*^10.3}]\n",
                                   true, 3.14159);
  std::cout << s;

  Student s1{"Alice", 255, true};
  Student s2{"Bob", 16, true};
  mini_fmt::print("Student 1: {:10:^10d:>10z}\n", s1);
  mini_fmt::print("Student 2: {:10:^10d:>10z}\n", s2);

  // 4. pointer测试
  int a = 42;
  mini_fmt::print("Address: [{:>20p}] value: [{:10}]\n",
                  static_cast<const void*>(&a), a);
  return 0;
}
