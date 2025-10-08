#!/usr/bin/env bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved

cd 3rdparty/Pangolin
cmake -B build
cmake --build build -j

cd ../../
cmake -B build
cmake --build build -j
