# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
include(FindPackageHandleStandardArgs)

find_path (dl_INCLUDE_DIRS dlfcn.h
    PATHS /usr/local/include /usr/include ${CMAKE_EXTRA_INCLUDES}
)

find_library(dl_LIBRARIES dl
    PATHS /usr/local/lib /usr/lib /lib ${CMAKE_EXTRA_LIBRARIES}
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(dl DEFAULT_MSG dl_INCLUDE_DIRS dl_LIBRARIES)
