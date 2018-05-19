cmake_minimum_required(VERSION 2.8)

project("Huffman compressor")

include_directories(${PROJECT_SOURCE_DIR})

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set (TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set (LIB_SOURCE_DIR ${SOURCE_DIR}/huffman_algorithm)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set (INCLUDE_LIB_DIR ${PROJECT_SOURCE_DIR}/include)

add_executable(huffcompress
               ${SOURCE_DIR}/driver.cpp
               ${SOURCE_DIR}/file_processor.cpp
               )

add_executable(huffcompress_test
    ${TEST_DIR}/huffcompress_test.cpp
    ${TEST_DIR}/gtest.h
    ${TEST_DIR}/gtest_main.cc
    ${TEST_DIR}/gtest-all.cc
    ${SOURCE_DIR}/file_processor.cpp
    )

add_library(huffman_algorithm
    ${LIB_SOURCE_DIR}/bitstring.cpp
    ${LIB_SOURCE_DIR}/huffman_engine.cpp
    ${INCLUDE_LIB_DIR}/huffman_engine.h
    ${INCLUDE_LIB_DIR}/bitstring.h
    )

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
endif()

target_link_libraries(huffcompress huffman_algorithm)
target_link_libraries(huffcompress_test huffman_algorithm)
