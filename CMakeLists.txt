cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
project(multithreading DESCRIPTION "Multithreading" VERSION 0.1.0 LANGUAGES CXX)

# Logger
add_library(logger STATIC src/logger.cpp src/logger.hpp)
target_precompile_headers(logger PRIVATE src/main.hpp)
target_include_directories(logger PUBLIC src)

if(WIN32)
  target_compile_definitions(logger PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

if(VCPKG_FOUND)
  # Requires "boost-asio" and "boost-lockfree" in vcpkg.json dependencies.
  find_package(Boost CONFIG COMPONENTS asio lockfree)
  if(Boost_FOUND)
    target_link_libraries(logger PUBLIC ${Boost_LIBRARIES})
    target_compile_definitions(logger PUBLIC BOOST_USE_WINDOWS_H)
    if(boost_asio_FOUND)
      target_compile_definitions(logger PUBLIC LOGGER_USE_BOOST_ASIO)
    endif()
    if(boost_lockfree_FOUND)
      target_compile_definitions(logger PUBLIC LOGGER_USE_BOOST_LOCKFREE)
    endif()
  endif()

  # Requires "spdlog" in vcpkg.json dependencies.
  find_package(spdlog CONFIG)
  if(spdlog_FOUND)
    target_link_libraries(logger PUBLIC spdlog::spdlog)
    target_compile_definitions(logger PUBLIC LOGGER_USE_SPDLOG)
  endif()

  # Requires { "name": "tbb", "default-features": false } in vcpkg.json dependencies.
  find_package(TBB CONFIG)
  if(TBB_FOUND)
    target_link_libraries(logger PUBLIC TBB::tbb TBB::tbbmalloc)
    target_compile_definitions(logger PUBLIC LOGGER_USE_TBB)
  endif()
endif()

add_executable(multithreading src/main.cpp src/main.manifest)
target_precompile_headers(multithreading REUSE_FROM logger)
target_link_libraries(multithreading PRIVATE logger)

if(VCPKG_FOUND)
  find_package(benchmark CONFIG REQUIRED)
  add_executable(benchmarks src/benchmarks.cpp src/main.manifest)
  target_precompile_headers(benchmarks REUSE_FROM logger)
  target_link_libraries(benchmarks PRIVATE logger benchmark::benchmark)
  set_target_properties(benchmarks PROPERTIES EXCLUDE_FROM_ALL $<CONFIG:Debug>)
endif()
