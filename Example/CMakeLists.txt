cmake_minimum_required(VERSION 2.6.0)

project(Example)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)
find_package(dl REQUIRED)
find_package(PTex REQUIRED)

set(PTEX_SHADER_DIR "${CMAKE_CURRENT_LIST_DIR}/Shaders" CACHE PATH "Where the shaders live")

include_directories(${Pangolin_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${dl_INCLUDE_DIRS})
include_directories(${PTEX_INCLUDE_DIRS})

file(GLOB srcs *.cpp)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O3 -msse2 -msse3 -Wall -std=c++11 -fopenmp -DSHADER_DIR=${PTEX_SHADER_DIR}")

add_executable(ExampleGUI ExampleGUI.cpp MirrorSurface.cpp)

target_link_libraries(ExampleGUI
                      ${Pangolin_LIBRARIES}
                      ${dl_LIBRARIES}
                      ${PTEX_LIBRARY}
                      stdc++fs
)

add_executable(Example Example.cpp)

target_link_libraries(Example
                      ${Pangolin_LIBRARIES}
                      ${dl_LIBRARIES}
                      ${PTEX_LIBRARY}
                      stdc++fs
)
