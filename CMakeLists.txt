#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

project(Multiverse LANGUAGES CXX)

cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# packages
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58.0 REQUIRED COMPONENTS
    program_options
    filesystem
    system
    thread
    date_time
    regex
    unit_test_framework
    timer
)
find_package(OpenSSL 1.0.0 REQUIRED) 
find_package(sodium 1.0.8 REQUIRED)
find_package(Readline 5.0 REQUIRED)

set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf 3.3.0 REQUIRED)

if(OPENSSL_VERSION VERSION_EQUAL 1.1.0 OR 
    (OPENSSL_VERSION VERSION_GREATER 1.1.0 AND OPENSSL_VERSION VERSION_LESS 1.2.0))
    add_definitions(-DUSE_SSL_110)
endif()

# libs
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    find_library(WS2_32_LIB NAMES ws2_32)
    find_library(WSSOCK32_LIB NAMES wsock32)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt")
endif()

# sub directories
add_subdirectory(src)
add_subdirectory(walleve)
add_subdirectory(crypto)
add_subdirectory(common)
add_subdirectory(mpvss)
add_subdirectory(storage)
add_subdirectory(network)
add_subdirectory(test)
add_subdirectory(delegate)
add_subdirectory(leveldb)
add_subdirectory(jsonrpc)
add_subdirectory(snappy)
