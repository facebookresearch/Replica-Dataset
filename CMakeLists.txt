cmake_minimum_required(VERSION 2.6.0)

project(PTex)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")

message(${CMAKE_MODULE_PATH})

set(THIRD_PARTY "${CMAKE_CURRENT_LIST_DIR}/3rdparty/")

include_directories(SYSTEM "${THIRD_PARTY}/eigen-git-mirror") 
include_directories(SYSTEM "${THIRD_PARTY}/Pangolin") 

#find_package(Eigen3 REQUIRED)
#find_package(Pangolin REQUIRED)
find_package(dl REQUIRED)

add_subdirectory("./PTexLib")
add_subdirectory("./Example")
