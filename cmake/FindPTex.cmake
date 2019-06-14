# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
###############################################################################
# Find PTEX
#
# This sets the following variables:
# PTEX_FOUND - True if PTEX was found.
# PTEX_INCLUDE_DIRS - Directories containing the PTEX include files.
# PTEX_LIBRARIES - Libraries needed to use PTEX.

find_path(PTEX_INCLUDE_DIR PTexLib.h
          PATHS
          ${CMAKE_CURRENT_SOURCE_DIR}/PTexLib
          PATH_SUFFIXES PTexLib
)

find_library(PTEX_LIBRARY
             NAMES libptex.so
             PATHS ${CMAKE_CURRENT_SOURCE_DIR}/PTexLib/build ${PTEX_BUILD_PATH}/PTexLib
             PATH_SUFFIXES ${PTEX_PATH_SUFFIXES}
)

set(PTEX_INCLUDE_DIRS ${PTEX_INCLUDE_DIR})
set(PTEX_LIBRARIES ${PTEX_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PTEX DEFAULT_MSG PTEX_LIBRARY PTEX_INCLUDE_DIR)
mark_as_advanced(PTEX_LIBRARY PTEX_INCLUDE_DIR)
