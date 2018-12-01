cmake_minimum_required(VERSION 3.8)

project("Indeksearch")

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

add_executable(indexer
    ${SOURCE_DIR}/main.cpp
    ${INCLUDE_DIR}/TrigramIndex.h
    ${SOURCE_DIR}/TrigramIndex.cpp)

target_link_libraries(indexer Qt5::Widgets)
