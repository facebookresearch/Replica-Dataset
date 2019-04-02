cmake_minimum_required(VERSION 2.6.0)

project(PTexLib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}")

find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)
find_package(dl REQUIRED)

set(PTEX_SHADER_DIR "${CMAKE_CURRENT_LIST_DIR}/Shaders" CACHE PATH "Where the shaders live")

file(GLOB srcs *.cpp)
file(GLOB hdrs *.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msse2 -msse3 -Wall -std=c++11 -fopenmp -DSHADER_DIR=${PTEX_SHADER_DIR}")

add_library(ptex SHARED
            ${srcs}
            ${hdrs}
)

target_link_libraries(ptex
                      ${Eigen_LIBRARIES}
                      ${Pangolin_LIBRARIES}
                      ${dl_LIBRARIES}
                      stdc++fs
)
target_include_directories(ptex PUBLIC
            ${Pangolin_INCLUDE_DIRS}
            ${Eigen_INCLUDE_DIRS}
            ${dl_INCLUDE_DIRS}
            ${CMAKE_CURRENT_LIST_DIR}
)
