# TODO: do we need anything out of cmake 3? should be more aggressive?

# Versions < 2.8.8 do not support Object Libraries
cmake_minimum_required(VERSION 3.1.0)

# Enforce the C++ standard
set (CXX_STANDARD_REQUIRED true)

include(GenerateExportHeader)
include(InstallRequiredSystemLibraries)

# Ensure that RPATH is used on OSX
set(CMAKE_MACOSX_RPATH 1)


find_package(PkgConfig)
pkg_check_modules(LIBMONGOC libmongoc-${LIBMONGOC_REQUIRED_ABI_VERSION})


option(BUILD_UNIT_TESTS "Build unit tests")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default is Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0")

add_subdirectory(src)

