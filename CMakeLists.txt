cmake_minimum_required(VERSION 3.9.0)

project(countdown CXX)

add_executable(numbers numbers.cpp)
set_target_properties(numbers PROPERTIES CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)
target_compile_options(numbers PUBLIC -Wall -Wextra -Wpedantic)
