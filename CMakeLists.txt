cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
project(multithreading DESCRIPTION "Multithreading" VERSION 0.1.0 LANGUAGES CXX)

# Logger
add_library(logger STATIC src/logger.cpp src/logger.hpp)
if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(logger PUBLIC /std:c++latest)
else()
  target_compile_features(logger PUBLIC cxx_std_26)
endif()
target_precompile_headers(logger PRIVATE src/main.hpp)
target_include_directories(logger PUBLIC src)

if(WIN32)
  target_compile_definitions(logger PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

add_executable(multithreading src/main.cpp src/main.manifest)
target_precompile_headers(multithreading REUSE_FROM logger)
target_link_libraries(multithreading PRIVATE logger)

find_package(benchmark CONFIG)
if(TARGET benchmark::benchmark)
  add_executable(benchmarks src/benchmarks.cpp src/main.manifest)
  target_precompile_headers(benchmarks REUSE_FROM logger)
  target_link_libraries(benchmarks PRIVATE logger benchmark::benchmark)
  set_target_properties(benchmarks PROPERTIES EXCLUDE_FROM_ALL $<CONFIG:Debug>)
endif()
