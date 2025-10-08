# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
cmake_minimum_required(VERSION 3.22)

project(Replica)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")

set(THIRD_PARTY "${CMAKE_CURRENT_LIST_DIR}/3rdparty/")

find_package(Eigen3 NO_MODULE)
if(NOT ${Eigen3_FOUND})
    message(STATUS "Using 3rd party Eigen3")
    set(Eigen_INCLUDE_DIRS "${THIRD_PARTY}/eigen")
endif()

# use system Pangolin if available
find_package(Pangolin 0.9)
if(NOT ${Pangolin_FOUND})
    message(STATUS "Using 3rd party Pangolin")
    list(APPEND CMAKE_PREFIX_PATH "${THIRD_PARTY}/Pangolin/build")
    find_package(Pangolin 0.9 REQUIRED)
endif()

find_package(dl REQUIRED)

add_subdirectory("./ReplicaSDK")
