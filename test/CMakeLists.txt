

# 引入 GoogleTest

enable_testing()

# 单元测试可执行文件
add_executable(
  test_all 
  test_main.cc
)

# target_include_directories(test_all PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(
  test_all PRIVATE
  mini_fmt::mini_fmt
  GTest::gtest_main
)
target_compile_features(test_all PRIVATE cxx_std_20)


add_executable(test_fmt2 test_fmt.cc)
# target_include_directories(fmt_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(
  test_fmt2 PRIVATE
  mini_fmt::mini_fmt
  GTest::gtest_main
)
target_compile_features(test_fmt2 PRIVATE cxx_std_20)


add_executable(bm_mini_fmt bm_mini_fmt.cc)
target_link_libraries(bm_mini_fmt
    PRIVATE
        mini_fmt::mini_fmt
        benchmark::benchmark
        fmt::fmt
)
target_compile_features(bm_mini_fmt PRIVATE cxx_std_20)
target_compile_options(bm_mini_fmt PRIVATE -O3 -march=native -flto)

add_executable(bm_detail bm_detail.cc)
target_link_libraries(bm_detail
    PRIVATE
        mini_fmt::mini_fmt
        benchmark::benchmark
        fmt::fmt
)
target_compile_features(bm_detail PRIVATE cxx_std_20)
target_compile_options(bm_detail PRIVATE -O3 -march=native -flto)

add_executable(test_fast_path test_fast_path.cc)
target_link_libraries(test_fast_path PRIVATE mini_fmt::mini_fmt)
target_compile_features(test_fast_path PRIVATE cxx_std_20)

add_executable(bm_direct bm_direct.cc)
target_link_libraries(bm_direct
    PRIVATE
        mini_fmt::mini_fmt
        benchmark::benchmark
        fmt::fmt
)
target_compile_features(bm_direct PRIVATE cxx_std_20)
target_compile_options(bm_direct PRIVATE -O3 -march=native -flto)

add_executable(bm_direct_format bm_direct_format.cc)
target_link_libraries(bm_direct_format
    PRIVATE
        mini_fmt::mini_fmt
        benchmark::benchmark
        fmt::fmt
)
target_compile_features(bm_direct_format PRIVATE cxx_std_20)
target_compile_options(bm_direct_format PRIVATE -O3 -march=native -flto)

add_executable(test_correctness test_correctness.cc)
target_link_libraries(test_correctness
    PRIVATE
        mini_fmt::mini_fmt
        fmt::fmt
)
target_compile_features(test_correctness PRIVATE cxx_std_20)