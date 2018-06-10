cmake_minimum_required(VERSION 2.8)

project(exam_list)
add_executable(
    ${PROJECT_NAME}
    list.hpp
    #testing.cpp
    #gtest/gtest.h
    #gtest/gtest_main.cc
    #gtest/gtest-all.cc
    main.cpp
    )

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
endif()

