cmake_minimum_required(VERSION 3.8)

project("Librarian")

include_directories(${PROJECT_SOURCE_DIR})

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set (TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set (INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)

add_executable(index_test
    ${TEST_DIR}/gtest/gtest.h
    ${TEST_DIR}/gtest/gtest_main.cc
    ${TEST_DIR}/gtest/gtest-all.cc
    ${TEST_DIR}/index_test.cpp
    ${INCLUDE_DIR}/TrigramIndex.h
    ${SOURCE_DIR}/TrigramIndex.cpp)

target_link_libraries(index_test Qt5::Widgets -lpthread stdc++fs)
