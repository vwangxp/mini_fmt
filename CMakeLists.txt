cmake_minimum_required(VERSION 3.28)
project(mini_fmt CXX)

# Declare header-only interface target
add_library(mini_fmt INTERFACE)
add_library(mini_fmt::mini_fmt ALIAS mini_fmt)
target_compile_features(mini_fmt INTERFACE cxx_std_20)


# Configure include directory scope
target_include_directories(mini_fmt INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Enable testing target optionally
option(MINI_FMT_BUILD_TESTS "Build mini_fmt unit tests" ON)
if(MINI_FMT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

