CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1)
PROJECT(bogeyman-tunnel)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static")

set(LIBJSONCPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp)
set(LIBJSONCPP_INCLUDE ${LIBJSONCPP_ROOT}/include)
set(LIBJSONCPP_BINARY_DIR ${CMAKE_BINARY_DIR}/jsoncpp)
set(LIBJSONCPP_LIBRARY ${LIBJSONCPP_BINARY_DIR}/src/lib_json)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(ExternalProject)

# Add jsoncpp
ExternalProject_Add(
    libjsoncpp
    SOURCE_DIR ${LIBJSONCPP_ROOT}
    BINARY_DIR ${LIBJSONCPP_BINARY_DIR}
    CMAKE_CACHE_ARGS "-DJSONCPP_WITH_TESTS:bool=0"
    INSTALL_COMMAND "")

include_directories(${LIBJSONCPP_INCLUDE})
link_directories(${LIBJSONCPP_LIBRARY})

include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(src)