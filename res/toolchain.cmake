# Toolchain
include_guard(GLOBAL)
if(CMAKE_BUILD_TYPE)
  mark_as_advanced(CMAKE_BUILD_TYPE)
endif()
if(CMAKE_VERBOSE_MAKEFILE)
  mark_as_advanced(CMAKE_VERBOSE_MAKEFILE)
endif()
if(VCPKG_TARGET_TRIPLET)
  mark_as_advanced(VCPKG_TARGET_TRIPLET)
endif()

# Vcpkg
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  find_program(VCPKG NAMES vcpkg)
  if(NOT VCPKG)
    set(VSWHERE_ROOT "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer")
    cmake_path(CONVERT "${VSWHERE_ROOT}" TO_CMAKE_PATH_LIST VSWHERE_ROOT NORMALIZE)
    find_program(VSWHERE NAMES vswhere REQUIRED PATHS "${VSWHERE_ROOT}")
    list(APPEND VSWHERE -nologo -nocolor -utf8 -format value -property resolvedInstallationPath)
    list(APPEND VSWHERE -products "*" -requires Microsoft.VisualStudio.Component.Vcpkg -latest)
    execute_process(COMMAND ${VSWHERE} ENCODING UTF-8 OUTPUT_VARIABLE VS_ROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    cmake_path(CONVERT "${VS_ROOT}" TO_CMAKE_PATH_LIST VS_ROOT NORMALIZE)
    find_program(VCPKG NAMES vcpkg PATHS "${VS_ROOT}/VC/vcpkg")
  endif()
  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  find_program(VCPKG NAMES vcpkg PATHS /opt/vcpkg)
  set(VCPKG_TARGET_TRIPLET "x64-linux")
else()
  message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

if(VCPKG)
  get_filename_component(VCPKG_ROOT "${VCPKG}" DIRECTORY)

  set(VCPKG_INSTALL_OPTIONS "--x-no-default-features")
  set(VCPKG_INSTALL_OPTIONS "${VCPKG_INSTALL_OPTIONS};--clean-buildtrees-after-build")
  set(VCPKG_INSTALL_OPTIONS "${VCPKG_INSTALL_OPTIONS};--clean-packages-after-build")
  #set(VCPKG_INSTALL_OPTIONS "${VCPKG_INSTALL_OPTIONS};--no-print-usage")

  if(NOT DEFINED ENV{VCPKG_DOWNLOADS})
    set(ENV{VCPKG_DOWNLOADS} "${CMAKE_SOURCE_DIR}/.cache/vcpkg/downloads")
  endif()

  if(NOT DEFINED ENV{VCPKG_BINARY_SOURCES})
    set(ENV{VCPKG_BINARY_SOURCES} "clear;files,${CMAKE_SOURCE_DIR}/.cache/vcpkg/binaries,readwrite")
  endif()

  #set(ENV{VCPKG_FORCE_SYSTEM_BINARIES} "ON")
  set(ENV{VCPKG_DISABLE_METRICS} "ON")
  set(VCPKG_APPLOCAL_DEPS OFF)

  include("${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    include("${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/toolchain.cmake")
    file(GLOB TOOLCHAIN_DLLS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/*.dll")
    foreach(TOOLCHAIN_DLL ${TOOLCHAIN_DLLS})
      get_filename_component(TOOLCHAIN_DLL_NAME "${TOOLCHAIN_DLL}" NAME)
      configure_file(${TOOLCHAIN_DLL} "${CMAKE_BINARY_DIR}/bin/${TOOLCHAIN_DLL_NAME}" COPYONLY)
    endforeach()
  endif()

  set(VCPKG_FOUND ON CACHE BOOL "")
endif()

# Standard
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  cmake_policy(SET CMP0092 NEW)
  add_compile_options(/std:c++latest /utf-8 /W4 /wd4100)
else()
  set(CMAKE_CXX_STANDARD 26 CACHE STRING "")
  set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "")
  set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE BOOL "")
endif()

# Debug Information
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
add_compile_definitions($<$<CONFIG:Debug>:_GLIBCXX_DEBUG>)

# Configurations
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug)
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG ";Release" CACHE STRING "")

# Output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/bin")
