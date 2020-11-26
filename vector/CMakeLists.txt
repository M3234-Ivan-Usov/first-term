cmake_minimum_required(VERSION 3.1)

project(VECTOR)
set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

include_directories(${VECTOR_SOURCE_DIR})

add_executable(vector_testing
               main.cpp
               vector.h
               gtest/gtest-all.cc
               gtest/gtest.h
               gtest/gtest_main.cc)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address,leak -fno-sanitize-recover=all -D_GLIBCXX_DEBUG")
endif()

target_link_libraries(vector_testing Threads::Threads)
