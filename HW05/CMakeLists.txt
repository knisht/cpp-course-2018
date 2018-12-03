cmake_minimum_required(VERSION 3.10)

project("File checker")

include_directories(${PROJECT_SOURCE_DIR})

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set (TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set (INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)

add_executable(filechecker
               ${SOURCE_DIR}/core/main.cpp
               ${SOURCE_DIR}/core/checker.cpp
               ${SOURCE_DIR}/gui/mainwindow.cpp
               ${SOURCE_DIR}/gui/controlpanel.cpp
               ${SOURCE_DIR}/gui/directoryview.cpp
               ${INCLUDE_DIR}/core/checker.hpp
               ${INCLUDE_DIR}/gui/controlpanel.h
               ${INCLUDE_DIR}/gui/mainwindow.h
               ${INCLUDE_DIR}/gui/directoryview.h
               )

add_executable(core_test
               ${TEST_DIR}/checker_test.cpp
               ${TEST_DIR}/gtest/gtest.h
               ${TEST_DIR}/gtest/gtest_main.cc
               ${TEST_DIR}/gtest/gtest-all.cc
               ${SOURCE_DIR}/core/checker.cpp
               ${INCLUDE_DIR}/core/checker.hpp
               )


if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -fopenmp")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -fsanitize=address,undefined")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Weverything -Wno-c++98-compat -std=c++17 -fsanitize=address")
endif()

target_link_libraries(core_test stdc++fs -lpthread)
target_link_libraries(filechecker stdc++fs Qt5::Widgets -lpthread -lstdc++fs)
