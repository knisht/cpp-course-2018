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
find_package(Qt5Concurrent)

add_executable(index_test
    ${TEST_DIR}/gtest/gtest.h
    ${TEST_DIR}/gtest/gtest_main.cc
    ${TEST_DIR}/gtest/gtest-all.cc
    ${TEST_DIR}/index_test.cpp
    ${INCLUDE_DIR}/trigramindex.h
    ${INCLUDE_DIR}/trigram.h
    ${INCLUDE_DIR}/substringoccurrence.h
    ${INCLUDE_DIR}/document.h
    ${INCLUDE_DIR}/taskcontext.h
    ${SOURCE_DIR}/trigramindex.cpp
    ${SOURCE_DIR}/document.cpp
    ${SOURCE_DIR}/trigram.cpp
    )

add_executable(librarian
    ${INCLUDE_DIR}/mainwindow.h
    ${INCLUDE_DIR}/substringoccurrence.h
    ${INCLUDE_DIR}/document.h
    ${INCLUDE_DIR}/trigramindex.h
    ${INCLUDE_DIR}/indexdriver.h
    ${INCLUDE_DIR}/trigram.h
    ${INCLUDE_DIR}/taskcontext.h
    ${INCLUDE_DIR}/textviewdriver.h
    ${INCLUDE_DIR}/settingswindow.h
    ${SOURCE_DIR}/textviewdriver.cpp
    ${SOURCE_DIR}/trigram.cpp
    ${SOURCE_DIR}/document.cpp
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/mainwindow.cpp
    ${SOURCE_DIR}/trigramindex.cpp
    ${SOURCE_DIR}/indexdriver.cpp
    ${SOURCE_DIR}/mainwindow.ui
    ${SOURCE_DIR}/settingswindow.ui
    ${SOURCE_DIR}/settingswindow.cpp
    )


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -fsanitize=undefined")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Weverything -Wno-c++98-compat -std=c++17 -fsanitize=address")
endif()


target_link_libraries(index_test Qt5::Widgets Qt5::Concurrent -lpthread stdc++fs)
target_link_libraries(librarian Qt5::Widgets Qt5::Concurrent -lpthread)
