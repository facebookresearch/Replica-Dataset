# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
cmake_minimum_required(VERSION 2.6.0)

project(Replica)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")

set(THIRD_PARTY "${CMAKE_CURRENT_LIST_DIR}/3rdparty/")

set(Eigen_INCLUDE_DIRS "${THIRD_PARTY}/eigen") 
set(Pangolin_INCLUDE_DIRS "${THIRD_PARTY}/Pangolin/build/src/include")
list(APPEND Pangolin_INCLUDE_DIRS "${THIRD_PARTY}/Pangolin/include") 
set(Pangolin_LIBRARIES "${THIRD_PARTY}/Pangolin/build/src/libpangolin.so") 

find_package(dl REQUIRED)

add_subdirectory("./ReplicaSDK")
