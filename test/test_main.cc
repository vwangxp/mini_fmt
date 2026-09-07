#include <gtest/gtest.h>

#include <chrono>
#include <concepts>
#include <ranges>
#include <string>
#include <string_view>

#include "mini_fmt/mini_fmt.h"  // 包含你的 mini_fmt 头文件
#include "test_helper.h"

// 1. 为自定义枚举/结构体 Status 特化
namespace mini_fmt::test {
enum class Status { OK = 200, NotFound = 404 };
}

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

namespace mini_fmt::test {

// ============================================================================
// 1. 解析引擎测试 (parse_format_specs)
// ============================================================================
TEST(ParseFormatSpecsTest, BasicAndDefaultParsing) {
  format_specs specs;
  std::string_view fmt = "}";
  auto it = parse_format_specs(fmt.begin(), fmt.end(), specs);

  EXPECT_EQ(specs.fill, ' ');
  EXPECT_EQ(specs.align, align_type::none);
  EXPECT_EQ(specs.width, 0);
  EXPECT_EQ(specs.precision, -1);
  EXPECT_EQ(*it, '}');
}

TEST(ParseFormatSpecsTest, FullSpecifierParsing) {
  format_specs specs;
  std::string_view fmt = "*^12.4x}";
  auto it = parse_format_specs(fmt.begin(), fmt.end(), specs);

  EXPECT_EQ(specs.fill, '*');
  EXPECT_EQ(specs.align, align_type::center);
  EXPECT_EQ(specs.width, 12);
  EXPECT_EQ(specs.precision, 4);
  EXPECT_EQ(specs.type, presentation::hex);
  EXPECT_EQ(*it, '}');
}

TEST(ParseFormatSpecsTest, AlignmentWithoutFill) {
  format_specs specs;
  std::string_view fmt = ">10d}";
  auto it = parse_format_specs(fmt.begin(), fmt.end(), specs);

  EXPECT_EQ(specs.fill, ' ');
  EXPECT_EQ(specs.align, align_type::right);
  EXPECT_EQ(specs.width, 10);
  EXPECT_EQ(specs.type, presentation::dec);
}

// ============================================================================
// 2. 填充与对齐引擎测试 (write_padded)
// ============================================================================
TEST(WritePaddedTest, LeftRightCenterPadding) {
  format_specs specs;
  specs.width = 6;
  specs.fill = '#';

  // 左对齐
  {
    specs.align = align_type::left;
    basic_memory_buffer<char> buf;
    buffer_appender out(buf);
    write_padded("ab", specs, out);
    EXPECT_EQ((std::string{buf.data(), buf.size()}), "ab####");
  }

  // 右对齐
  {
    specs.align = align_type::right;
    basic_memory_buffer<char> buf;
    buffer_appender out(buf);
    write_padded("ab", specs, out);
    EXPECT_EQ((std::string{buf.data(), buf.size()}), "####ab");
  }

  // 居中对齐
  {
    specs.align = align_type::center;
    basic_memory_buffer<char> buf;
    buffer_appender out(buf);
    write_padded("ab", specs, out);
    EXPECT_EQ((std::string{buf.data(), buf.size()}), "##ab##");
  }
}

TEST(WritePaddedTest, ContentExceedsWidth) {
  format_specs specs;
  specs.width = 3;
  specs.fill = '*';
  specs.align = align_type::right;

  basic_memory_buffer<char> buf;
  buffer_appender out(buf);
  write_padded("hello", specs, out);
  std::string res(buf.data(), buf.size());
  EXPECT_EQ(res,
            "hello");  // 超长时不进行裁剪或填充
}

// ============================================================================
// 3. 基础标量格式化器测试 (Integer, Float, Bool, Pointer)
// ============================================================================

TEST(ScalarFormatterTest, IntegerFormatting) {
  // 十六进制大写与填充
  integerx_formatter<int> fmt;
  parse_context ctx("04X}");
  fmt.parse(ctx);
  auto res = test_format(fmt, 10);
  EXPECT_EQ(res, "   A");
}

TEST(ScalarFormatterTest, FloatFormatting) {
  float_formatter<double> fmt;
  parse_context ctx(".2f}");
  fmt.parse(ctx);
  auto res = test_format(fmt, 3.14159);
  EXPECT_EQ(res, "3.14");
}

TEST(ScalarFormatterTest, BoolFormatting) {
  formatter<bool> fmt;

  // 中文模式
  {
    parse_context ctx("z}");
    fmt.parse(ctx);
    auto res = test_format(fmt, true);
    EXPECT_EQ(res, "男");
  }

  // 英文模式
  {
    parse_context ctx("e}");
    fmt.parse(ctx);
    auto res = test_format(fmt, false);
    EXPECT_EQ(res, "Female");
  }
}

TEST(ScalarFormatterTest, PointerFormatting) {
  pointer_formatter fmt;

  // 空指针场景
  {
    auto res = test_format(fmt, nullptr);
    EXPECT_EQ(res, "0x0");
  }

  // 非空指针场景
  {
    int dummy = 0;
    auto res = test_format(fmt, &dummy);
    EXPECT_TRUE(res.rfind("0x", 0) == 0);  // 必须以 0x 开头
  }
}

// //
// ============================================================================
// // 4. 扩展类型测试 (Enum, Chrono, Container, Struct)
// //
// ============================================================================

TEST(ExtensionTypeTest, EnumFormatting) {
  using mini_fmt::test::Status;

  Status s = Status::NotFound;
  auto res = mini_fmt::format("{}", s);  // 直接调用顶层 format API

  EXPECT_EQ(res, "404");
}

TEST(ExtensionTypeTest, ChronoFormatting) {
  auto dur = std::chrono::milliseconds(1500);
  auto res = mini_fmt::format("{:6}", dur);
  EXPECT_EQ(res, "1500ms");
}

TEST(ExtensionTypeTest, ContainerFormatting) {
  std::vector<int> vec = {1, 2, 3, 4};
  auto res = mini_fmt::format("{}", vec);
  EXPECT_EQ(res, "[1, 2, 3, 4]");
}

TEST(ExtensionTypeTest, CustomStructStudent) {
  Student s{"Alice", 20, true};
  // 测试组合格式化器: name 宽 8, age 16进制居中, sex 中文
  auto res = mini_fmt::format("{:8:^6x:z}", s);
  EXPECT_EQ(res, "Alice   ,   14  , 男");
}

}  // namespace mini_fmt::test