cmake_minimum_required(VERSION 3.6)
project(lingx)

option(LNX_BUILD_TEST "build test" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/config.cmake)

add_subdirectory(src)

if(LNX_BUILD_TEST)
  enable_testing()
  add_subdirectory(test)
endif()
